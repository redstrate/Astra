// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gameinstaller.h"

#include <KLocalizedString>
#include <QDir>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <algorithm>
#include <qcoronetworkreply.h>

#include "account.h"
#include "assetupdater.h"
#include "astra_log.h"
#include "compatibilitytoolinstaller.h"
#include "gamerunner.h"
#include "launchercore.h"
#include "sapphirelogin.h"
#include "squareenixlogin.h"
#include "utility.h"

LauncherCore::LauncherCore()
    : QObject()
{
    m_settings = new LauncherSettings(this);
    m_mgr = new QNetworkAccessManager(this);
    m_sapphireLogin = new SapphireLogin(*this, this);
    m_squareEnixLogin = new SquareEnixLogin(*this, this);
    m_profileManager = new ProfileManager(*this, this);
    m_accountManager = new AccountManager(*this, this);
    m_runner = new GameRunner(*this, this);

    m_profileManager->load();
    m_accountManager->load();

    // restore profile -> account connections
    for (auto profile : m_profileManager->profiles()) {
        if (auto account = m_accountManager->getByUuid(profile->accountUuid())) {
            profile->setAccount(account);
        }
    }

    // set default profile, if found
    if (auto profile = m_profileManager->getProfileByUUID(m_settings->config()->currentProfile()); profile != nullptr) {
        setCurrentProfile(profile);
    }

    m_loadingFinished = true;
    Q_EMIT loadingFinished();
}

void LauncherCore::initializeSteam()
{
    m_steamApi = new SteamAPI(this);
    m_steamApi->setLauncherMode(true);
}

void LauncherCore::login(Profile *profile, const QString &username, const QString &password, const QString &oneTimePassword)
{
    Q_ASSERT(profile != nullptr);

    auto loginInformation = new LoginInformation(this);
    loginInformation->profile = profile;
    loginInformation->username = username;
    loginInformation->password = password;
    loginInformation->oneTimePassword = oneTimePassword;

    if (profile->account()->rememberPassword()) {
        profile->account()->setPassword(password);
    }

    beginLogin(*loginInformation);
}

bool LauncherCore::autoLogin(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    QString otp;
    if (profile->account()->useOTP()) {
        if (!profile->account()->rememberOTP()) {
            Q_EMIT loginError(i18n("This account does not have an OTP secret set, but requires it for login."));
            return false;
        }

        otp = profile->account()->getOTP();
        if (otp.isEmpty()) {
            Q_EMIT loginError(i18n("Failed to generate OTP, review the stored secret."));
            return false;
        }
    }

    login(profile, profile->account()->name(), profile->account()->getPassword(), otp);
    return true;
}

GameInstaller *LauncherCore::createInstaller(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    return new GameInstaller(*this, *profile, this);
}

CompatibilityToolInstaller *LauncherCore::createCompatInstaller()
{
    return new CompatibilityToolInstaller(*this, this);
}

void LauncherCore::clearAvatarCache()
{
    const auto cacheLocation = QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0] + QStringLiteral("/avatars");
    if (QDir(cacheLocation).exists()) {
        QDir(cacheLocation).removeRecursively();
    }
}

void LauncherCore::refreshNews()
{
    fetchNews();
}

Profile *LauncherCore::currentProfile() const
{
    return m_profileManager->getProfile(m_currentProfileIndex);
}

void LauncherCore::setCurrentProfile(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    const int newIndex = m_profileManager->getProfileIndex(profile->uuid());
    if (newIndex != m_currentProfileIndex) {
        m_currentProfileIndex = newIndex;
        m_settings->config()->setCurrentProfile(profile->uuid());
        m_settings->config()->save();
        Q_EMIT currentProfileChanged();
    }
}

[[nodiscard]] QString LauncherCore::autoLoginProfileName() const
{
    return m_settings->config()->autoLoginProfile();
}

[[nodiscard]] Profile *LauncherCore::autoLoginProfile() const
{
    if (m_settings->config()->autoLoginProfile().isEmpty()) {
        return nullptr;
    }
    return m_profileManager->getProfileByUUID(m_settings->config()->autoLoginProfile());
}

void LauncherCore::setAutoLoginProfile(Profile *profile)
{
    if (profile == nullptr) {
        m_settings->config()->setAutoLoginProfile({});
        m_settings->config()->save();
        Q_EMIT autoLoginProfileChanged();
        return;
    }

    auto uuid = profile->uuid();
    if (uuid != m_settings->config()->autoLoginProfile()) {
        m_settings->config()->setAutoLoginProfile(uuid);
        m_settings->config()->save();
        Q_EMIT autoLoginProfileChanged();
    }
}

void LauncherCore::buildRequest(const Profile &settings, QNetworkRequest &request)
{
    setSSL(request);

    if (settings.account()->license() == Account::GameLicense::macOS) {
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

void LauncherCore::setSSL(QNetworkRequest &request)
{
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

void LauncherCore::setupIgnoreSSL(QNetworkReply *reply)
{
    Q_ASSERT(reply != nullptr);

    if (m_settings->preferredProtocol() == QStringLiteral("http")) {
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
    if (m_steamApi != nullptr) {
        return m_steamApi->isDeck();
    } else {
        return false;
    }
}

bool LauncherCore::isPatching() const
{
    return m_isPatching;
}

QNetworkAccessManager *LauncherCore::mgr()
{
    return m_mgr;
}

LauncherSettings *LauncherCore::settings()
{
    return m_settings;
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

QCoro::Task<> LauncherCore::beginLogin(LoginInformation &info)
{
    info.profile->account()->updateConfig();

    auto assetUpdater = new AssetUpdater(*info.profile, *this, this);
    if (co_await assetUpdater->update()) {
        std::optional<LoginAuth> auth;
        if (info.profile->account()->isSapphire()) {
            auth = co_await m_sapphireLogin->login(info.profile->account()->lobbyUrl(), info);
        } else {
            auth = co_await m_squareEnixLogin->login(&info);
        }

        if (auth != std::nullopt) {
            Q_EMIT stageChanged(i18n("Launching game..."));

            if (isSteam()) {
                m_steamApi->setLauncherMode(false);
            }

            m_runner->beginGameExecutable(*info.profile, *auth);
        }
    }

    assetUpdater->deleteLater();
}

QCoro::Task<> LauncherCore::fetchNews()
{
    qInfo(ASTRA_LOG) << "Fetching news...";

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("lang"), QStringLiteral("en-us"));
    query.addQueryItem(QStringLiteral("media"), QStringLiteral("pcapp"));

    QUrl url;
    url.setScheme(m_settings->preferredProtocol());
    url.setHost(QStringLiteral("frontier.%1").arg(m_settings->squareEnixServer()));
    url.setPath(QStringLiteral("/news/headline.json"));
    url.setQuery(query);

    QNetworkRequest request(QUrl(QStringLiteral("%1&%2").arg(url.toString(), QString::number(QDateTime::currentMSecsSinceEpoch()))));
    request.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("application/json, text/plain, */*"));
    request.setRawHeader(QByteArrayLiteral("Origin"), QByteArrayLiteral("https://launcher.finalfantasyxiv.com"));
    request.setRawHeader(QByteArrayLiteral("Referer"),
                         QStringLiteral("https://launcher.finalfantasyxiv.com/v600/index.html?rc_lang=%1&time=%2")
                             .arg(QStringLiteral("en-us"), QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-dd-HH")))
                             .toUtf8());
    Utility::printRequest(QStringLiteral("GET"), request);

    auto reply = mgr()->get(request);
    co_await reply;

    auto document = QJsonDocument::fromJson(reply->readAll());

    auto headline = new Headline(this);
    if (document.isEmpty()) {
        headline->failedToLoad = true;
    }

    const auto parseNews = [](QJsonObject object) -> News {
        News news;
        news.date = QDateTime::fromString(object[QLatin1String("date")].toString(), Qt::DateFormat::ISODate);
        news.id = object[QLatin1String("id")].toString();
        news.tag = object[QLatin1String("tag")].toString();
        news.title = object[QLatin1String("title")].toString();

        if (object[QLatin1String("url")].toString().isEmpty()) {
            news.url = QUrl(QStringLiteral("https://na.finalfantasyxiv.com/lodestone/news/detail/%1").arg(news.id));
        } else {
            news.url = QUrl(object[QLatin1String("url")].toString());
        }

        return news;
    };

    for (auto bannerObject : document.object()[QLatin1String("banner")].toArray()) {
        auto banner = Banner();
        banner.link = QUrl(bannerObject.toObject()[QLatin1String("link")].toString());
        banner.bannerImage = QUrl(bannerObject.toObject()[QLatin1String("lsb_banner")].toString());

        headline->banners.push_back(banner);
    }

    for (auto newsObject : document.object()[QLatin1String("news")].toArray()) {
        auto news = parseNews(newsObject.toObject());
        headline->news.push_back(news);
    }

    for (auto pinnedObject : document.object()[QLatin1String("pinned")].toArray()) {
        auto pinned = parseNews(pinnedObject.toObject());
        headline->pinned.push_back(pinned);
    }

    for (auto pinnedObject : document.object()[QLatin1String("topics")].toArray()) {
        auto pinned = parseNews(pinnedObject.toObject());
        headline->topics.push_back(pinned);
    }

    m_headline = headline;
    Q_EMIT newsChanged();
}

#include "moc_launchercore.cpp"