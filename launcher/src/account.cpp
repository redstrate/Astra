// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "account.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <qt5keychain/keychain.h>

#include "cotp.h"
#include "launchercore.h"

Account::Account(LauncherCore &launcher, const QString &key, QObject *parent)
    : QObject(parent)
    , m_config(key)
    , m_key(key)
    , m_launcher(launcher)
{
    fetchAvatar();
}

QString Account::uuid() const
{
    return m_key;
}

QString Account::name() const
{
    return m_config.name();
}

void Account::setName(const QString &name)
{
    if (m_config.name() != name) {
        m_config.setName(name);
        m_config.save();
        Q_EMIT nameChanged();
    }
}

int Account::language() const
{
    return m_config.language();
}

void Account::setLanguage(const int value)
{
    if (m_config.language() != value) {
        m_config.setLanguage(value);
        m_config.save();
        Q_EMIT languageChanged();
    }
}

QString Account::lodestoneId() const
{
    return m_config.lodestoneId();
}

void Account::setLodestoneId(const QString &id)
{
    if (m_config.lodestoneId() != id) {
        m_config.setLodestoneId(id);
        m_config.save();
        fetchAvatar();
        Q_EMIT lodestoneIdChanged();
    }
}

QString Account::avatarUrl() const
{
    return m_url.toString();
}

bool Account::isSapphire() const
{
    return m_config.isSapphire();
}

void Account::setIsSapphire(bool value)
{
    if (m_config.isSapphire() != value) {
        m_config.setIsSapphire(value);
        m_config.save();
        Q_EMIT isSapphireChanged();
    }
}

QString Account::lobbyUrl() const
{
    return m_config.lobbyUrl();
}

void Account::setLobbyUrl(const QString &value)
{
    if (m_config.lobbyUrl() != value) {
        m_config.setLobbyUrl(value);
        m_config.save();
        Q_EMIT lobbyUrlChanged();
    }
}

bool Account::rememberPassword() const
{
    return m_config.rememberPassword();
}

void Account::setRememberPassword(const bool value)
{
    if (m_config.rememberPassword() != value) {
        m_config.setRememberPassword(value);
        m_config.save();
        Q_EMIT rememberPasswordChanged();
    }
}

bool Account::rememberOTP() const
{
    return m_config.rememberOTP();
}

void Account::setRememberOTP(const bool value)
{
    if (m_config.rememberOTP() != value) {
        m_config.setRememberOTP(value);
        m_config.save();
        Q_EMIT rememberOTPChanged();
    }
}

bool Account::useOTP() const
{
    return m_config.useOTP();
}

void Account::setUseOTP(const bool value)
{
    if (m_config.useOTP() != value) {
        m_config.setUseOTP(value);
        m_config.save();
        Q_EMIT useOTPChanged();
    }
}

Account::GameLicense Account::license() const
{
    return static_cast<GameLicense>(m_config.license());
}

void Account::setLicense(const GameLicense license)
{
    if (static_cast<GameLicense>(m_config.license()) != license) {
        m_config.setLicense(static_cast<int>(license));
        m_config.save();
        Q_EMIT licenseChanged();
    }
}

bool Account::isFreeTrial() const
{
    return m_config.isFreeTrial();
}

void Account::setIsFreeTrial(const bool value)
{
    if (m_config.isFreeTrial() != value) {
        m_config.setIsFreeTrial(value);
        m_config.save();
        Q_EMIT isFreeTrialChanged();
    }
}

QString Account::getPassword()
{
    return getKeychainValue(QStringLiteral("password"));
}

void Account::setPassword(const QString &password)
{
    setKeychainValue(QStringLiteral("password"), password);
}

QString Account::getOTP()
{
    auto otpSecret = getKeychainValue(QStringLiteral("otp-secret"));

    char *totp = get_totp(otpSecret.toStdString().c_str(), 6, 30, SHA1, nullptr);
    QString totpStr(totp);
    free(totp);

    return totpStr;
}

QDir Account::getConfigDir() const
{
    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir userDir = dataDir.absoluteFilePath(QStringLiteral("user"));
    return userDir.absoluteFilePath(m_key);
}

void Account::fetchAvatar()
{
    if (lodestoneId().isEmpty()) {
        return;
    }

    const auto cacheLocation = QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0] + QStringLiteral("/avatars");
    if (!QDir().exists(cacheLocation)) {
        QDir().mkpath(cacheLocation);
    }

    const QString filename(QStringLiteral("%1/%2.jpg").arg(cacheLocation, lodestoneId()));
    if (!QFile(filename).exists()) {
        qDebug() << "Did not find lodestone character " << lodestoneId() << " in cache, fetching from xivapi.";
        QNetworkRequest request(QStringLiteral("https://xivapi.com/character/%1").arg(lodestoneId()));
        const auto reply = m_launcher.mgr->get(request);
        connect(reply, &QNetworkReply::finished, [this, filename, reply] {
            auto document = QJsonDocument::fromJson(reply->readAll());
            if (document.isObject()) {
                const QNetworkRequest avatarRequest(document.object()[QLatin1String("Character")].toObject()[QLatin1String("Avatar")].toString());
                auto avatarReply = m_launcher.mgr->get(avatarRequest);
                QObject::connect(avatarReply, &QNetworkReply::finished, [this, filename, avatarReply] {
                    QFile file(filename);
                    file.open(QIODevice::ReadWrite);
                    file.write(avatarReply->readAll());
                    file.close();

                    m_url = QStringLiteral("file:///%1").arg(filename);
                    Q_EMIT avatarUrlChanged();
                });
            }
        });
    } else {
        m_url = QStringLiteral("file:///%1").arg(filename);
        Q_EMIT avatarUrlChanged();
    }
}

void Account::setKeychainValue(const QString &key, const QString &value)
{
    auto job = new QKeychain::WritePasswordJob(QStringLiteral("Astra"), this);
    job->setTextData(value);
    job->setKey(m_key + QStringLiteral("-") + key);
    job->setInsecureFallback(m_launcher.isSteamDeck()); // The Steam Deck does not have secrets provider in Game Mode
    job->start();
}

QString Account::getKeychainValue(const QString &key)
{
    auto loop = new QEventLoop(this);

    auto job = new QKeychain::ReadPasswordJob(QStringLiteral("Astra"), this);
    job->setKey(m_key + QStringLiteral("-") + key);
    job->setInsecureFallback(m_launcher.isSteamDeck());
    job->start();

    QString value;

    QObject::connect(job, &QKeychain::ReadPasswordJob::finished, [loop, job, &value](QKeychain::Job *j) {
        Q_UNUSED(j)
        value = job->textData();
        loop->quit();
    });

    loop->exec();

    return value;
}
