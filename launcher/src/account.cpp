// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "account.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <cotp.h>
#include <qcorocore.h>
#include <qt6keychain/keychain.h>

#include "astra_log.h"
#include "launchercore.h"
#include "utility.h"

using namespace Qt::StringLiterals;

Account::Account(LauncherCore &launcher, const QString &key, QObject *parent)
    : QObject(parent)
    , m_config(key)
    , m_key(key)
    , m_launcher(launcher)
{
    fetchPassword();
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
        Q_EMIT lodestoneIdChanged();
    }
}

QString Account::avatarUrl() const
{
    return m_avatarUrl;
}

bool Account::isSapphire() const
{
    return m_config.isSapphire();
}

void Account::setIsSapphire(const bool value)
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

void Account::setLobbyUrl(const QString &url)
{
    if (m_config.lobbyUrl() != url) {
        m_config.setLobbyUrl(url);
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
    return QCoro::waitFor(getKeychainValue(QStringLiteral("password")));
}

void Account::setPassword(const QString &password)
{
    setKeychainValue(QStringLiteral("password"), password);

    if (m_needsPassword) {
        m_needsPassword = false;
        Q_EMIT needsPasswordChanged();
    }
}

void Account::setAvatarUrl(const QString &url)
{
    if (m_avatarUrl != url) {
        m_avatarUrl = url;
        Q_EMIT avatarUrlChanged();
    }
}

QString Account::getOTP()
{
    const auto otpSecret = QCoro::waitFor(getKeychainValue(QStringLiteral("otp-secret")));
    if (otpSecret.isEmpty()) {
        return {};
    }

    cotp_error err;
    char *totp = get_totp(otpSecret.toStdString().c_str(), 6, 30, SHA1, &err);

    if (err == NO_ERROR) {
        QString totpStr = QString::fromLatin1(totp);
        free(totp);
        return totpStr;
    } else {
        return {};
    }
}

void Account::setOTPSecret(const QString &secret)
{
    setKeychainValue(QStringLiteral("otp-secret"), secret);
}

QDir Account::getConfigDir() const
{
    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir userDir = dataDir.absoluteFilePath(QStringLiteral("user"));
    return userDir.absoluteFilePath(m_key);
}

QString Account::getConfigPath() const
{
    return getConfigDir().absolutePath();
}

void Account::setKeychainValue(const QString &key, const QString &value)
{
    if (m_launcher.isSteamDeck()) {
        auto stateConfig = KSharedConfig::openStateConfig();

        stateConfig->group(QStringLiteral("Passwords")).writeEntry(m_key + QStringLiteral("-") + key, value);
        stateConfig->sync();
    } else {
        auto job = new QKeychain::WritePasswordJob(QStringLiteral("Astra"), this);
        job->setTextData(value);
#ifdef FLATPAK
        job->setKey(QStringLiteral("flatpak-") + m_key + QStringLiteral("-") + key);
#else
        job->setKey(m_key + QStringLiteral("-") + key);
#endif
        job->setInsecureFallback(m_launcher.isSteamDeck()); // The Steam Deck does not have secrets provider in Game Mode
        job->start();

        connect(job, &QKeychain::WritePasswordJob::finished, this, [job] {
            if (job->error() != QKeychain::NoError) {
                qWarning(ASTRA_LOG) << "Error when writing" << job->errorString();
            }
        });
    }
}

QCoro::Task<QString> Account::getKeychainValue(const QString &key)
{
    if (m_launcher.isSteamDeck()) {
        co_return KSharedConfig::openStateConfig()->group(QStringLiteral("Passwords")).readEntry(m_key + QStringLiteral("-") + key);
    } else {
        auto job = new QKeychain::ReadPasswordJob(QStringLiteral("Astra"), this);
#ifdef FLATPAK
        job->setKey(QStringLiteral("flatpak-") + m_key + QStringLiteral("-") + key);
#else
        job->setKey(m_key + QStringLiteral("-") + key);
#endif
        job->setInsecureFallback(m_launcher.isSteamDeck()); // The Steam Deck does not have secrets provider in Game Mode
        job->start();

        co_await qCoro(job, &QKeychain::ReadPasswordJob::finished);

        if (job->error() != QKeychain::NoError) {
            qWarning(ASTRA_LOG) << "Error when reading" << key << job->errorString() << "for account" << uuid();
        }

        co_return job->textData();
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ^ Could be const, but this function shouldn't be considered as such
void Account::updateConfig()
{
    const auto configDir = getConfigDir().absoluteFilePath(QStringLiteral("FFXIV.cfg"));

    if (!QFile::exists(configDir)) {
        return;
    }

    qInfo(ASTRA_LOG) << "Updating FFXIV.cfg...";

    const auto configDirStd = configDir.toStdString();

    const auto cfgFileBuffer = physis_read_file(configDirStd.c_str());
    const auto cfgFile = physis_cfg_parse(cfgFileBuffer);

    // Ensure that the opening cutscene movie never plays, since it's broken in most versions of Wine
    physis_cfg_set_value(cfgFile, "CutsceneMovieOpening", "1");

    const auto screenshotDir = m_launcher.settings()->screenshotDir();
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

bool Account::needsPassword() const
{
    return m_needsPassword;
}

QCoro::Task<> Account::fetchPassword()
{
    const QString password = co_await getKeychainValue(QStringLiteral("password"));
    m_needsPassword = password.isEmpty();
    Q_EMIT needsPasswordChanged();

    co_return;
}

#include "moc_account.cpp"