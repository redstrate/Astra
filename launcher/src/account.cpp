// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "account.h"

#include <cotp.h>
#include <qcorocore.h>
#include <qt6keychain/keychain.h>

#include "accountconfig.h"
#include "astra_log.h"
#include "utility.h"

using namespace Qt::StringLiterals;

Account::Account(const QString &key, QObject *parent)
    : QObject(parent)
    , m_config(new AccountConfig(key, this))
    , m_key(key)
{
    fetchPassword();
}

Account::~Account()
{
    m_config->save();
}

QString Account::uuid() const
{
    return m_key;
}

QString Account::avatarUrl() const
{
    return m_avatarUrl;
}

QString Account::getPassword()
{
    // Early exit if no password is supposed to be saved
    if (!config()->rememberPassword()) {
        return QString{};
    }

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
    if (Utility::isSteamDeck()) {
        auto stateConfig = KSharedConfig::openStateConfig();

        stateConfig->group(QStringLiteral("Passwords")).writeEntry(m_key + QStringLiteral("-") + key, value);
        stateConfig->sync();
    } else {
        auto job = new QKeychain::WritePasswordJob(QStringLiteral("Astra"), this);
        job->setTextData(value);
        job->setKey(m_key + QStringLiteral("-") + key);
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
    if (Utility::isSteamDeck()) {
        co_return KSharedConfig::openStateConfig()->group(QStringLiteral("Passwords")).readEntry(m_key + QStringLiteral("-") + key);
    } else {
        auto job = new QKeychain::ReadPasswordJob(QStringLiteral("Astra"), this);
        job->setKey(m_key + QStringLiteral("-") + key);
        job->start();

        co_await qCoro(job, &QKeychain::ReadPasswordJob::finished);

        if (job->error() != QKeychain::NoError) {
            qWarning(ASTRA_LOG) << "Error when reading" << key << job->errorString() << "for account" << uuid();
        }

        co_return job->textData();
    }
}

bool Account::needsPassword() const
{
    return m_needsPassword;
}

QCoro::Task<> Account::fetchPassword()
{
    // Early exit if no password is supposed to be saved
    if (!config()->rememberPassword()) {
        m_needsPassword = true;
        co_return;
    }

    const QString password = co_await getKeychainValue(QStringLiteral("password"));
    m_needsPassword = password.isEmpty();
    Q_EMIT needsPasswordChanged();

    co_return;
}

AccountConfig *Account::config() const
{
    return m_config;
}

#include "moc_account.cpp"
