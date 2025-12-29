// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <KLocalizedString>
#include <QCoroSignal>
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
#include "profileconfig.h"
#include "squareenixlogin.h"
#include "utility.h"

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
    m_squareEnixLogin = new SquareEnixLogin(*this, this);
    m_profileManager = new ProfileManager(this);
    m_accountManager = new AccountManager(this);
    m_runner = new GameRunner(*this, this);
    m_steamApi = new SteamAPI(this);

    connect(m_accountManager, &AccountManager::accountAdded, this, &LauncherCore::fetchAvatar);
    connect(m_accountManager, &AccountManager::accountLodestoneIdChanged, this, &LauncherCore::fetchAvatar);

    connect(this, &LauncherCore::gameClosed, this, &LauncherCore::handleGameExit);

    m_profileManager->load();
    m_accountManager->load();

    // restore profile -> account connections
    for (const auto profile : m_profileManager->profiles()) {
        if (const auto account = m_accountManager->getByUuid(profile->config()->account())) {
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

void LauncherCore::login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword)
{
    Q_ASSERT(profile != nullptr);

    inhibitSleep();

    const auto loginInformation = new LoginInformation(this);
    loginInformation->profile = profile;

    // Benchmark never has to log in, of course
    if (!profile->config()->isBenchmark()) {
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

        QUrl url = QUrl::fromUserInput(account->config()->lodestoneServer());
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
                    connect(avatarReply, &QNetworkReply::finished, [filename, avatarReply, account] {
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

    const auto saveTexture = [](SqPackResource *data, const QString &path, const QString &name) {
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
        const auto hashString = QSysInfo::bootUniqueId();

        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(hashString);

        QByteArray bytes = hash.result();
        bytes.resize(4);

        auto checkSum = (uint8_t)-(bytes[0] + bytes[1] + bytes[2] + bytes[3]);
        bytes.prepend(checkSum);

        request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QString::fromUtf8(bytes.toHex())));
    }

    request.setRawHeader(QByteArrayLiteral("Accept"),
                         QByteArrayLiteral("image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, "
                                           "application/x-ms-xbap, */*"));
    request.setRawHeader(QByteArrayLiteral("Accept-Encoding"), QByteArrayLiteral("gzip, deflate"));
    request.setRawHeader(QByteArrayLiteral("Accept-Language"), QByteArrayLiteral("en-us"));
    request.setRawHeader(QByteArrayLiteral("Connection"), QByteArrayLiteral("Keep-Alive"));
    request.setRawHeader(QByteArrayLiteral("Cookie"), QByteArrayLiteral("_rsid=\"\""));
}

void LauncherCore::setupIgnoreSSL(QNetworkReply *reply)
{
    Q_ASSERT(reply != nullptr);

    if (reply->request().url().scheme() == QStringLiteral("http")) {
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

SteamAPI *LauncherCore::steamApi() const
{
    return m_steamApi;
}

QCoro::Task<> LauncherCore::beginLogin(LoginInformation &info)
{
    // Hmm, I don't think we're set up for this yet?
    if (!info.profile->config()->isBenchmark()) {
        updateConfig(info.profile->account());
    }

    const auto repairs = physis_gamedata_needs_repair(info.profile->gameData());
    if (repairs.action_count > 0) {
        QString message;
        for (int i = 0; i < repairs.action_count; i++) {
            const auto action = repairs.actions[i];

            QString actionText;
            switch (action) {
            case RepairAction::VersionFileMissing:
                actionText = i18n("Version file is missing, and data has to be re-downloaded.");
                break;
            case RepairAction::VersionFileCanRestore:
                actionText = i18n("Version file is missing, but can be restored.");
                break;
            case RepairAction::VersionFileExtraSpacing:
                actionText = i18n("Version file has extra spacing.");
                break;
            default:
                actionText = i18n("Unknown repair required.");
                break;
            }

            const auto repository = repairs.repositories[i];
            message += QStringLiteral("%1: %2\n").arg(QString::fromLatin1(repository)).arg(actionText);
        }

        Q_EMIT needsRepair(message);

        const bool shouldRepair = co_await qCoro(this, &LauncherCore::repairDecided);
        if (shouldRepair) {
            const bool successful = physis_gamedata_repair(info.profile->gameData());
            if (!successful) {
                Q_EMIT miscError(i18n("Repair failed! Game data may be invalid and needs to be re-downloaded."));
                co_return;
            }

            // Re-read game version so repairs actually have an effect!
            info.profile->readGameVersion();
        } else {
            co_return;
        }
    }

    std::optional<LoginAuth> auth;
    if (!info.profile->config()->isBenchmark()) {
        auth = co_await m_squareEnixLogin->login(&info);
    }

    const auto assetUpdater = new AssetUpdater(*info.profile, *this, this);
    if (co_await assetUpdater->update()) {
        // If we expect an auth ticket, don't continue if missing
        if (!info.profile->config()->isBenchmark() && auth == std::nullopt) {
            co_return;
        }

        Q_EMIT stageChanged(i18n("Launching game..."));

        m_runner->beginGameExecutable(*info.profile, auth);
    }

    assetUpdater->deleteLater();
}

QCoro::Task<> LauncherCore::fetchNews()
{
    if (!currentProfile() || !currentProfile()->account()) {
        co_return;
    }

    qInfo(ASTRA_LOG) << "Fetching news...";

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("lang"), QStringLiteral("en-us"));
    query.addQueryItem(QStringLiteral("media"), QStringLiteral("pcapp"));

    QUrl headlineUrl = QUrl::fromUserInput(currentProfile()->account()->config()->frontierServer());
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

    QUrl bannerUrl = QUrl::fromUserInput(currentProfile()->account()->config()->frontierServer());
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

void LauncherCore::handleGameExit()
{
    qCDebug(ASTRA_LOG) << "Game has closed.";

    uninhibitSleep();

    // Otherwise, quit when everything is finished.
    if (config()->closeWhenLaunched()) {
        QCoreApplication::exit();
    }
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

QCoro::Task<> LauncherCore::beginAutoConfiguration(Account *account, QString url)
{
    // NOTE: url is intentionally copied so it doesn't deference during coroutine execution

    QUrl requestUrl = QUrl::fromUserInput(url);
    requestUrl.setPath(QStringLiteral("/.well-known/xiv"));

    auto reply = m_mgr->get(QNetworkRequest(requestUrl));
    co_await reply;

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT account->autoConfigurationResult(i18n("Configuration Error"), i18n("Failed to fetch configuration from %1:\n\n%2", url, reply->errorString()));
        co_return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject obj = document.object();

    account->config()->setGamePatchServer(obj["game_patch_server"_L1].toString());
    account->config()->setBootPatchServer(obj["boot_patch_server"_L1].toString());
    account->config()->setLobbyServer(obj["lobby_server"_L1].toString());
    account->config()->setLobbyPort(obj["lobby_port"_L1].toInt());
    account->config()->setLodestoneServer(obj["lodestone_server"_L1].toString());
    account->config()->setFrontierServer(obj["frontier_server"_L1].toString());
    account->config()->setSaveDataBankServer(obj["savedata_bank_server"_L1].toString());
    account->config()->setSaveDataBankPort(obj["savedata_bank_port"_L1].toInt());
    account->config()->setDataCenterTravelServer(obj["datacenter_travel_server"_L1].toString());
    account->config()->setLoginServer(obj["login_server"_L1].toString());

    Q_EMIT account->autoConfigurationResult(i18n("Configuration Successful"), i18n("Configuration from %1 applied! You can now log into this server.", url));

    co_return;
}

QString LauncherCore::currentProfileId() const
{
    return KSharedConfig::openStateConfig()->group(QStringLiteral("General")).readEntry(QStringLiteral("CurrentProfile"));
}

void LauncherCore::openOfficialLauncher(Profile *profile)
{
    Q_ASSERT(profile != nullptr);
    m_runner->openOfficialLauncher(*profile);
}

void LauncherCore::openSystemInfo(Profile *profile)
{
    Q_ASSERT(profile != nullptr);
    m_runner->openSystemInfo(*profile);
}

void LauncherCore::openConfigBackup(Profile *profile)
{
    Q_ASSERT(profile != nullptr);
    m_runner->openConfigBackup(*profile);
}

void LauncherCore::downloadServerConfiguration(Account *account, const QString &url)
{
    beginAutoConfiguration(account, url);
}

#include "moc_launchercore.cpp"
