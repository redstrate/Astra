// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <KLocalizedString>
#include <QDir>
#include <QImage>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <algorithm>
#include <qcoronetworkreply.h>

#include "account.h"
#include "accountconfig.h"
#include "assetupdater.h"
#include "astra_log.h"
#include "benchmarkinstaller.h"
#include "compatibilitytoolinstaller.h"
#include "gamerunner.h"
#include "launchercore.h"
#include "sapphirelogin.h"
#include "squareenixlogin.h"
#include "utility.h"

#ifdef BUILD_SYNC
#include "charactersync.h"
#include "syncmanager.h"
#endif

#ifdef HAS_DBUS
#include <QDBusConnection>
#include <QDBusReply>
#include <QGuiApplication>
#endif

using namespace Qt::StringLiterals;

LauncherCore::LauncherCore()
    : QObject()
{
    m_config = new Config(KSharedConfig::openConfig(QStringLiteral("astrarc"), KConfig::SimpleConfig, QStandardPaths::AppConfigLocation), this);
    m_mgr = new QNetworkAccessManager(this);
    m_sapphireLogin = new SapphireLogin(*this, this);
    m_squareEnixLogin = new SquareEnixLogin(*this, this);
    m_profileManager = new ProfileManager(this);
    m_accountManager = new AccountManager(this);
    m_runner = new GameRunner(*this, this);

    connect(m_accountManager, &AccountManager::accountAdded, this, &LauncherCore::fetchAvatar);
    connect(m_accountManager, &AccountManager::accountLodestoneIdChanged, this, &LauncherCore::fetchAvatar);

    connect(this, &LauncherCore::gameClosed, this, &LauncherCore::handleGameExit);

#ifdef BUILD_SYNC
    m_syncManager = new SyncManager(this);
#endif

    m_profileManager->load();
    m_accountManager->load();

    // restore profile -> account connections
    for (const auto profile : m_profileManager->profiles()) {
        if (const auto account = m_accountManager->getByUuid(profile->accountUuid())) {
            profile->setAccount(account);
        }
    }

    // set default profile, if found
    if (const auto profile = m_profileManager->getProfileByUUID(currentProfileId())) {
        setCurrentProfile(profile);
    }

    m_loadingFinished = true;
    Q_EMIT loadingFinished();
}

LauncherCore::~LauncherCore()
{
    m_config->save();
}

void LauncherCore::initializeSteam()
{
    m_steamApi = new SteamAPI(this);
    m_steamApi->setLauncherMode(true);
}

void LauncherCore::login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword)
{
    Q_ASSERT(profile != nullptr);

    inhibitSleep();

    const auto loginInformation = new LoginInformation(this);
    loginInformation->profile = profile;

    // Benchmark never has to login, of course
    if (!profile->isBenchmark()) {
        loginInformation->username = username;
        loginInformation->password = password;
        loginInformation->oneTimePassword = oneTimePassword;

        if (profile->account()->config()->rememberPassword()) {
            profile->account()->setPassword(password);
        }
    }

    beginLogin(*loginInformation);
}

bool LauncherCore::autoLogin(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    QString otp;
    if (profile->account()->config()->useOTP()) {
        if (!profile->account()->config()->rememberOTP()) {
            Q_EMIT loginError(i18n("This account does not have an OTP secret set, but requires it for login."));
            return false;
        }

        otp = profile->account()->getOTP();
        if (otp.isEmpty()) {
            Q_EMIT loginError(i18n("Failed to generate OTP, review the stored secret."));
            return false;
        }
    }

    login(profile, profile->account()->config()->name(), profile->account()->getPassword(), otp);
    return true;
}

void LauncherCore::immediatelyLaunch(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    m_runner->beginGameExecutable(*profile, std::nullopt);
}

GameInstaller *LauncherCore::createInstaller(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    return new GameInstaller(*this, *profile, this);
}

GameInstaller *LauncherCore::createInstallerFromExisting(Profile *profile, const QString &filePath)
{
    Q_ASSERT(profile != nullptr);

    return new GameInstaller(*this, *profile, filePath, this);
}

CompatibilityToolInstaller *LauncherCore::createCompatInstaller()
{
    return new CompatibilityToolInstaller(*this, this);
}

BenchmarkInstaller *LauncherCore::createBenchmarkInstaller(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    return new BenchmarkInstaller(*this, *profile, this);
}

BenchmarkInstaller *LauncherCore::createBenchmarkInstallerFromExisting(Profile *profile, const QString &filePath)
{
    Q_ASSERT(profile != nullptr);

    return new BenchmarkInstaller(*this, *profile, filePath, this);
}

void LauncherCore::fetchAvatar(Account *account)
{
    if (account->config()->lodestoneId().isEmpty()) {
        return;
    }

    const QString cacheLocation = QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0] + QStringLiteral("/avatars");
    Utility::createPathIfNeeded(cacheLocation);

    const QString filename = QStringLiteral("%1/%2.jpg").arg(cacheLocation, account->config()->lodestoneId());
    if (!QFile(filename).exists()) {
        qDebug(ASTRA_LOG) << "Did not find lodestone character " << account->config()->lodestoneId() << " in cache, fetching from Lodestone.";

        QUrl url;
        url.setScheme(config()->preferredProtocol());
        url.setHost(QStringLiteral("na.%1").arg(config()->mainServer())); // TODO: NA isnt the only thing in the world...
        url.setPath(QStringLiteral("/lodestone/character/%1").arg(account->config()->lodestoneId()));

        const QNetworkRequest request(url);
        Utility::printRequest(QStringLiteral("GET"), request);

        const auto reply = mgr()->get(request);
        connect(reply, &QNetworkReply::finished, [this, filename, reply, account] {
            const QString document = QString::fromUtf8(reply->readAll());
            if (!document.isEmpty()) {
                const static QRegularExpression re(
                    QStringLiteral(R"lit(<div\s[^>]*class=["|']frame__chara__face["|'][^>]*>\s*<img\s[&>]*src=["|']([^"']*))lit"));
                const QRegularExpressionMatch match = re.match(document);

                if (match.hasCaptured(1)) {
                    const QString newAvatarUrl = match.captured(1);

                    const auto avatarRequest = QNetworkRequest(QUrl(newAvatarUrl));
                    Utility::printRequest(QStringLiteral("GET"), avatarRequest);

                    auto avatarReply = mgr()->get(avatarRequest);
                    connect(avatarReply, &QNetworkReply::finished, [this, filename, avatarReply, account] {
                        QFile file(filename);
                        file.open(QIODevice::ReadWrite);
                        file.write(avatarReply->readAll());
                        file.close();

                        account->setAvatarUrl(QStringLiteral("file:///%1").arg(filename));
                    });
                }
            }
        });
    } else {
        account->setAvatarUrl(QStringLiteral("file:///%1").arg(filename));
    }
}

void LauncherCore::clearAvatarCache()
{
    const QString cacheLocation = QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0] + QStringLiteral("/avatars");
    if (QDir(cacheLocation).exists()) {
        QDir(cacheLocation).removeRecursively();
    }
}

void LauncherCore::refreshNews()
{
    fetchNews();
}

void LauncherCore::refreshLogoImage()
{
    const QDir cacheDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::CacheLocation).last();
    const QDir logoDir = cacheDir.absoluteFilePath(QStringLiteral("logos"));

    if (!logoDir.exists()) {
        Q_UNUSED(QDir().mkpath(logoDir.absolutePath()))
    }

    const auto saveTexture = [](GameData *data, const QString &path, const QString &name) {
        if (QFile::exists(name)) {
            return;
        }

        const auto file = physis_gamedata_extract_file(data, path.toStdString().c_str());
        if (file.data != nullptr) {
            const auto tex = physis_texture_parse(file);

            const QImage image(tex.rgba, tex.width, tex.height, QImage::Format_RGBA8888);
            Q_UNUSED(image.save(name))
        }
    };

    // TODO: this finds the first profile that has a valid image, but this could probably be cached per-profile
    for (int i = 0; i < m_profileManager->numProfiles(); i++) {
        const auto profile = m_profileManager->getProfile(i);
        if (profile->isGameInstalled() && profile->gameData()) {
            // A Realm Reborn
            saveTexture(profile->gameData(), QStringLiteral("ui/uld/Title_Logo.tex"), logoDir.absoluteFilePath(QStringLiteral("ffxiv.png")));

            for (int j = 0; j < profile->numInstalledExpansions(); j++) {
                const int expansionNumber = 100 * (j + 3); // logo number starts at 300 for ex1

                saveTexture(profile->gameData(),
                            QStringLiteral("ui/uld/Title_Logo%1_hr1.tex").arg(expansionNumber),
                            logoDir.absoluteFilePath(QStringLiteral("ex%1.png").arg(j + 1)));
            }
        }
    }

    QList<QString> imageFiles;

    // TODO: sort
    QDirIterator it(logoDir.absolutePath());
    while (it.hasNext()) {
        const QFileInfo logoFile(it.next());
        if (logoFile.completeSuffix() != QStringLiteral("png")) {
            continue;
        }

        imageFiles.push_back(logoFile.absoluteFilePath());
    }

    if (!imageFiles.isEmpty()) {
        m_cachedLogoImage = imageFiles.last();
        Q_EMIT cachedLogoImageChanged();
    }
}

Profile *LauncherCore::currentProfile() const
{
    return m_profileManager->getProfile(m_currentProfileIndex);
}

void LauncherCore::setCurrentProfile(const Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    const int newIndex = m_profileManager->getProfileIndex(profile->uuid());
    if (newIndex != m_currentProfileIndex) {
        m_currentProfileIndex = newIndex;

        auto stateConfig = KSharedConfig::openStateConfig();
        stateConfig->group(QStringLiteral("General")).writeEntry(QStringLiteral("CurrentProfile"), profile->uuid());
        stateConfig->sync();

        Q_EMIT currentProfileChanged();
    }
}

[[nodiscard]] QString LauncherCore::autoLoginProfileName() const
{
    return config()->autoLoginProfile();
}

[[nodiscard]] Profile *LauncherCore::autoLoginProfile() const
{
    if (config()->autoLoginProfile().isEmpty()) {
        return nullptr;
    }
    return m_profileManager->getProfileByUUID(config()->autoLoginProfile());
}

void LauncherCore::setAutoLoginProfile(const Profile *profile)
{
    if (profile != nullptr) {
        auto uuid = profile->uuid();
        if (uuid != config()->autoLoginProfile()) {
            config()->setAutoLoginProfile(uuid);
        }
    } else {
        config()->setAutoLoginProfile({});
    }

    config()->save();
    Q_EMIT autoLoginProfileChanged();
}

void LauncherCore::buildRequest(const Profile &settings, QNetworkRequest &request)
{
    Utility::setSSL(request);

    if (settings.account()->config()->license() == Account::GameLicense::macOS) {
        request.setHeader(QNetworkRequest::UserAgentHeader, QByteArrayLiteral("macSQEXAuthor/2.0.0(MacOSX; ja-jp)"));
    } else {
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QStringLiteral("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QString::fromUtf8(QSysInfo::bootUniqueId())));
    }

    request.setRawHeader(QByteArrayLiteral("Accept"),
                         QByteArrayLiteral("image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, "
                                           "application/x-ms-xbap, */*"));
    request.setRawHeader(QByteArrayLiteral("Accept-Encoding"), QByteArrayLiteral("gzip, deflate"));
    request.setRawHeader(QByteArrayLiteral("Accept-Language"), QByteArrayLiteral("en-us"));
}

void LauncherCore::setupIgnoreSSL(QNetworkReply *reply)
{
    Q_ASSERT(reply != nullptr);

    if (config()->preferredProtocol() == QStringLiteral("http")) {
        connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &errors) {
            reply->ignoreSslErrors(errors);
        });
    }
}

bool LauncherCore::isLoadingFinished() const
{
    return m_loadingFinished;
}

bool LauncherCore::isSteam() const
{
    return m_steamApi != nullptr;
}

bool LauncherCore::isSteamDeck() const
{
    return Utility::isSteamDeck();
}

bool LauncherCore::isWindows()
{
#if defined(Q_OS_WIN)
    return true;
#else
    return false;
#endif
}

bool LauncherCore::needsCompatibilityTool()
{
    return !isWindows();
}

bool LauncherCore::isPatching() const
{
    return m_isPatching;
}

bool LauncherCore::supportsSync() const
{
#ifdef BUILD_SYNC
    return true;
#else
    return false;
#endif
}

QNetworkAccessManager *LauncherCore::mgr()
{
    return m_mgr;
}

Config *LauncherCore::config() const
{
    return m_config;
}

ProfileManager *LauncherCore::profileManager()
{
    return m_profileManager;
}

AccountManager *LauncherCore::accountManager()
{
    return m_accountManager;
}

Headline *LauncherCore::headline() const
{
    return m_headline;
}

QString LauncherCore::cachedLogoImage() const
{
    return m_cachedLogoImage;
}

#ifdef BUILD_SYNC
SyncManager *LauncherCore::syncManager() const
{
    return m_syncManager;
}
#endif

QCoro::Task<> LauncherCore::beginLogin(LoginInformation &info)
{
    // Hmm, I don't think we're set up for this yet?
    if (!info.profile->isBenchmark()) {
        updateConfig(info.profile->account());
    }

#ifdef BUILD_SYNC
    const auto characterSync = new CharacterSync(*info.profile->account(), *this, this);
    if (!co_await characterSync->sync()) {
        co_return;
    }
#endif

    std::optional<LoginAuth> auth;
    if (!info.profile->isBenchmark()) {
        if (info.profile->account()->config()->isSapphire()) {
            auth = co_await m_sapphireLogin->login(info.profile->account()->config()->lobbyUrl(), info);
        } else {
            auth = co_await m_squareEnixLogin->login(&info);
        }
    }

    const auto assetUpdater = new AssetUpdater(*info.profile, *this, this);
    if (co_await assetUpdater->update()) {
        // If we expect an auth ticket, don't continue if missing
        if (!info.profile->isBenchmark() && auth == std::nullopt) {
            co_return;
        }

        Q_EMIT stageChanged(i18n("Launching game..."));

        if (isSteam()) {
            m_steamApi->setLauncherMode(false);
        }

        m_runner->beginGameExecutable(*info.profile, auth);
    }

    assetUpdater->deleteLater();
}

QCoro::Task<> LauncherCore::fetchNews()
{
    qInfo(ASTRA_LOG) << "Fetching news...";

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("lang"), QStringLiteral("en-us"));
    query.addQueryItem(QStringLiteral("media"), QStringLiteral("pcapp"));

    QUrl headlineUrl;
    headlineUrl.setScheme(config()->preferredProtocol());
    headlineUrl.setHost(QStringLiteral("frontier.%1").arg(config()->squareEnixServer()));
    headlineUrl.setPath(QStringLiteral("/news/headline.json"));
    headlineUrl.setQuery(query);

    QNetworkRequest headlineRequest(QUrl(QStringLiteral("%1&%2").arg(headlineUrl.toString(), QString::number(QDateTime::currentMSecsSinceEpoch()))));
    headlineRequest.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json, text/plain, */*"));
    headlineRequest.setRawHeader(QByteArrayLiteral("Origin"), QByteArrayLiteral("https://launcher.finalfantasyxiv.com"));
    headlineRequest.setRawHeader(
        QByteArrayLiteral("Referer"),
        QStringLiteral("%1/index.html?rc_lang=%2&time=%3")
            .arg(currentProfile()->frontierUrl(), QStringLiteral("en-us"), QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-dd-HH")))
            .toUtf8());
    Utility::printRequest(QStringLiteral("GET"), headlineRequest);

    auto headlineReply = mgr()->get(headlineRequest);
    co_await headlineReply;

    QUrl bannerUrl;
    bannerUrl.setScheme(config()->preferredProtocol());
    bannerUrl.setHost(QStringLiteral("frontier.%1").arg(config()->squareEnixServer()));
    bannerUrl.setPath(QStringLiteral("/v2/topics/%1/banner.json").arg(QStringLiteral("en-us")));
    bannerUrl.setQuery(query);

    QNetworkRequest bannerRequest(QUrl(QStringLiteral("%1&_=%3").arg(bannerUrl.toString(), QString::number(QDateTime::currentMSecsSinceEpoch()))));
    bannerRequest.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json, text/plain, */*"));
    bannerRequest.setRawHeader(QByteArrayLiteral("Origin"), QByteArrayLiteral("https://launcher.finalfantasyxiv.com"));
    bannerRequest.setRawHeader(
        QByteArrayLiteral("Referer"),
        QStringLiteral("%1/v700/index.html?rc_lang=%2&time=%3")
            .arg(currentProfile()->frontierUrl(), QStringLiteral("en-us"), QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-dd-HH")))
            .toUtf8());
    Utility::printRequest(QStringLiteral("GET"), bannerRequest);

    auto bannerReply = mgr()->get(bannerRequest);
    co_await bannerReply;

    const auto document = QJsonDocument::fromJson(headlineReply->readAll());
    const auto bannerDocument = QJsonDocument::fromJson(bannerReply->readAll());

    const auto headline = new Headline(this);
    if (document.isEmpty() && bannerDocument.isEmpty()) {
        headline->failedToLoad = true;
    } else {
        const auto parseNews = [](QJsonObject object) -> News {
            News news;
            news.date = QDateTime::fromString(object["date"_L1].toString(), Qt::DateFormat::ISODate);
            news.id = object["id"_L1].toString();
            news.tag = object["tag"_L1].toString();
            news.title = object["title"_L1].toString();

            if (object["url"_L1].toString().isEmpty()) {
                news.url = QUrl(QStringLiteral("https://na.finalfantasyxiv.com/lodestone/news/detail/%1").arg(news.id));
            } else {
                news.url = QUrl(object["url"_L1].toString());
            }

            return news;
        };

        for (const auto bannerObject : bannerDocument.object()["banner"_L1].toArray()) {
            // TODO: use new order_priority and fix_order params
            headline->banners.push_back(
                {.link = QUrl(bannerObject.toObject()["link"_L1].toString()), .bannerImage = QUrl(bannerObject.toObject()["lsb_banner"_L1].toString())});
        }

        for (const auto newsObject : document.object()["news"_L1].toArray()) {
            headline->news.push_back(parseNews(newsObject.toObject()));
        }

        for (const auto pinnedObject : document.object()["pinned"_L1].toArray()) {
            headline->pinned.push_back(parseNews(pinnedObject.toObject()));
        }

        for (const auto pinnedObject : document.object()["topics"_L1].toArray()) {
            headline->topics.push_back(parseNews(pinnedObject.toObject()));
        }
    }

    m_headline = headline;
    Q_EMIT newsChanged();
}

QCoro::Task<> LauncherCore::handleGameExit(const Profile *profile)
{
    qCDebug(ASTRA_LOG) << "Game has closed.";

    uninhibitSleep();

#ifdef BUILD_SYNC
    // TODO: once we have Steam API support we can tell Steam to delay putting the Deck to sleep until our upload is complete
    if (config()->enableSync()) {
        Q_EMIT showWindow();

        qCDebug(ASTRA_LOG) << "Game closed! Uploading character data...";
        const auto characterSync = new CharacterSync(*profile->account(), *this, this);
        co_await characterSync->sync(false);

        // Tell the user they can now quit.
        Q_EMIT stageChanged(i18n("You may now safely close the game."));

        co_return;
    }
#endif
    // Otherwise, quit when everything is finished.
    if (config()->closeWhenLaunched()) {
        QCoreApplication::exit();
    }

    co_return;
}

void LauncherCore::updateConfig(const Account *account)
{
    const auto configDir = account->getConfigDir().absoluteFilePath(QStringLiteral("FFXIV.cfg"));

    if (!QFile::exists(configDir)) {
        return;
    }

    qInfo(ASTRA_LOG) << "Updating FFXIV.cfg...";

    const auto configDirStd = configDir.toStdString();

    const auto cfgFileBuffer = physis_read_file(configDirStd.c_str());
    const auto cfgFile = physis_cfg_parse(cfgFileBuffer);

    // Ensure that the opening cutscene movie never plays, since it's broken in most versions of Wine
    physis_cfg_set_value(cfgFile, "CutsceneMovieOpening", "1");

    const auto screenshotDir = config()->screenshotDir();
    Utility::createPathIfNeeded(screenshotDir);

    const auto screenshotDirWin = Utility::toWindowsPath(screenshotDir);
    const auto screenshotDirWinStd = screenshotDirWin.toStdString();

    // Set the screenshot path
    physis_cfg_set_value(cfgFile, "ScreenShotDir", screenshotDirWinStd.c_str());

    const auto buffer = physis_cfg_write(cfgFile);

    QFile file(configDir);
    file.open(QIODevice::WriteOnly);
    file.write(reinterpret_cast<const char *>(buffer.data), buffer.size);
    file.close();
}

void LauncherCore::inhibitSleep()
{
#ifdef HAS_DBUS
    if (screenSaverDbusCookie != 0)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("/ScreenSaver"),
                                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("Inhibit"));
    message << QGuiApplication::desktopFileName();
    message << i18n("Playing FFXIV");

    const QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);
    if (reply.isValid()) {
        screenSaverDbusCookie = reply.value();
    }
#endif
}

void LauncherCore::uninhibitSleep()
{
#ifdef HAS_DBUS
    if (screenSaverDbusCookie == 0)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("/ScreenSaver"),
                                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("UnInhibit"));
    message << static_cast<uint>(screenSaverDbusCookie);
    screenSaverDbusCookie = 0;
    QDBusConnection::sessionBus().send(message);
#endif
}

QString LauncherCore::currentProfileId() const
{
    return KSharedConfig::openStateConfig()->group(QStringLiteral("General")).readEntry(QStringLiteral("CurrentProfile"));
}

#include "moc_launchercore.cpp"
