// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>
#include <qcorotask.h>

class AccountConfig;

class Account : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from AccountManager")

    Q_PROPERTY(AccountConfig *config READ config CONSTANT)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY avatarUrlChanged)
    Q_PROPERTY(bool needsPassword READ needsPassword NOTIFY needsPasswordChanged)

public:
    explicit Account(const QString &key, QObject *parent = nullptr);
    ~Account() override;

    enum GameLicense {
        WindowsStandalone,
        WindowsSteam,
        macOS
    };
    Q_ENUM(GameLicense)

    [[nodiscard]] QString uuid() const;

    [[nodiscard]] QString avatarUrl() const;

    Q_INVOKABLE QString getPassword();
    void setPassword(const QString &password);

    void setAvatarUrl(const QString &url);

    Q_INVOKABLE QString getOTP();
    Q_INVOKABLE void setOTPSecret(const QString &secret);

    /// Returns the path to the FFXIV config folder
    [[nodiscard]] QDir getConfigDir() const;
    [[nodiscard]] Q_INVOKABLE QString getConfigPath() const;

    [[nodiscard]] bool needsPassword() const;

    AccountConfig *config() const;

Q_SIGNALS:
    void avatarUrlChanged();
    bool needsPasswordChanged();

private:
    QCoro::Task<> fetchPassword();

    /**
     * @brief Sets a value in the keychain. This function is asynchronous.
     */
    void setKeychainValue(const QString &key, const QString &value);

    /**
     * @brief Retrieves a value from the keychain. This function is synchronous.
     */
    QCoro::Task<QString> getKeychainValue(const QString &key);

    AccountConfig *m_config;
    QString m_key;
    QString m_avatarUrl;
    bool m_needsPassword = false;
};
