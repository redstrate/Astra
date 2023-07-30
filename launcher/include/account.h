#pragma once

#include <QObject>

#include "accountconfig.h"

class LauncherCore;

class Account : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
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

    QString uuid() const;

    QString name() const;
    void setName(const QString &name);

    QString lodestoneId() const;
    void setLodestoneId(const QString &id);

    QString avatarUrl() const;

    bool isSapphire() const;
    void setIsSapphire(bool value);

    QString lobbyUrl() const;
    void setLobbyUrl(const QString &url);

    bool rememberPassword() const;
    void setRememberPassword(bool value);

    bool rememberOTP() const;
    void setRememberOTP(bool value);

    bool useOTP() const;
    void setUseOTP(bool value);

    GameLicense license() const;
    void setLicense(GameLicense license);

    bool isFreeTrial() const;
    void setIsFreeTrial(bool value);

    Q_INVOKABLE QString getPassword() const;
    void setPassword(const QString &password);

    Q_INVOKABLE QString getOTP() const;

Q_SIGNALS:
    void nameChanged();
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
    void setKeychainValue(const QString &key, const QString &value) const;

    /*
     * Retrieves a value from the keychain. This function is synchronous.
     */
    QString getKeychainValue(const QString &key) const;

    AccountConfig m_config;
    QString m_key;
    QUrl m_url;
    LauncherCore &m_launcher;
};