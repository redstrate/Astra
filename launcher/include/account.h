// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <qcorotask.h>

#include "accountconfig.h"

class LauncherCore;

class Account : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from AccountManager")

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString lodestoneId READ lodestoneId WRITE setLodestoneId NOTIFY lodestoneIdChanged)
    Q_PROPERTY(QString avatarUrl READ avatarUrl NOTIFY avatarUrlChanged)
    Q_PROPERTY(bool isSapphire READ isSapphire WRITE setIsSapphire NOTIFY isSapphireChanged)
    Q_PROPERTY(QString lobbyUrl READ lobbyUrl WRITE setLobbyUrl NOTIFY lobbyUrlChanged)
    Q_PROPERTY(bool rememberPassword READ rememberPassword WRITE setRememberPassword NOTIFY rememberPasswordChanged)
    Q_PROPERTY(bool rememberOTP READ rememberOTP WRITE setRememberOTP NOTIFY rememberOTPChanged)
    Q_PROPERTY(bool useOTP READ useOTP WRITE setUseOTP NOTIFY useOTPChanged)
    Q_PROPERTY(GameLicense license READ license WRITE setLicense NOTIFY licenseChanged)
    Q_PROPERTY(bool isFreeTrial READ isFreeTrial WRITE setIsFreeTrial NOTIFY isFreeTrialChanged)

public:
    explicit Account(LauncherCore &launcher, const QString &key, QObject *parent = nullptr);

    enum class GameLicense { WindowsStandalone, WindowsSteam, macOS };
    Q_ENUM(GameLicense)

    [[nodiscard]] QString uuid() const;

    [[nodiscard]] QString name() const;
    void setName(const QString &name);

    [[nodiscard]] int language() const;
    void setLanguage(int value);

    [[nodiscard]] QString lodestoneId() const;
    void setLodestoneId(const QString &id);

    [[nodiscard]] QString avatarUrl() const;

    [[nodiscard]] bool isSapphire() const;
    void setIsSapphire(bool value);

    [[nodiscard]] QString lobbyUrl() const;
    void setLobbyUrl(const QString &url);

    [[nodiscard]] bool rememberPassword() const;
    void setRememberPassword(bool value);

    [[nodiscard]] bool rememberOTP() const;
    void setRememberOTP(bool value);

    [[nodiscard]] bool useOTP() const;
    void setUseOTP(bool value);

    [[nodiscard]] GameLicense license() const;
    void setLicense(GameLicense license);

    [[nodiscard]] bool isFreeTrial() const;
    void setIsFreeTrial(bool value);

    Q_INVOKABLE QString getPassword();
    void setPassword(const QString &password);

    Q_INVOKABLE QString getOTP();
    Q_INVOKABLE void setOTPSecret(const QString &secret);

    /// Returns the path to the FFXIV config folder
    [[nodiscard]] QDir getConfigDir() const;

    /// Updates FFXIV.cfg with some recommended options like turning the opening cutscene movie off
    void updateConfig();

Q_SIGNALS:
    void nameChanged();
    void languageChanged();
    void lodestoneIdChanged();
    void avatarUrlChanged();
    void isSapphireChanged();
    void lobbyUrlChanged();
    void rememberPasswordChanged();
    void rememberOTPChanged();
    void useOTPChanged();
    void licenseChanged();
    void isFreeTrialChanged();

private:
    void fetchAvatar();

    /*
     * Sets a value in the keychain. This function is asynchronous.
     */
    void setKeychainValue(const QString &key, const QString &value);

    /*
     * Retrieves a value from the keychain. This function is synchronous.
     */
    QCoro::Task<QString> getKeychainValue(const QString &key);

    AccountConfig m_config;
    QString m_key;
    QUrl m_url;
    LauncherCore &m_launcher;
};