// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

#include "squareboot.h"

class Account;
class ProfileConfig;

class Profile : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from ProfileManager")

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString gamePath READ gamePath WRITE setGamePath NOTIFY gamePathChanged)
    Q_PROPERTY(QString winePath READ winePath WRITE setWinePath NOTIFY winePathChanged)
    Q_PROPERTY(QString winePrefixPath READ winePrefixPath WRITE setWinePrefixPath NOTIFY winePrefixPathChanged)
    Q_PROPERTY(bool watchdogEnabled READ watchdogEnabled WRITE setWatchdogEnabled NOTIFY enableWatchdogChanged)
    Q_PROPERTY(WineType wineType READ wineType WRITE setWineType NOTIFY wineTypeChanged)
    Q_PROPERTY(bool esyncEnabled READ esyncEnabled WRITE setESyncEnabled NOTIFY useESyncChanged)
    Q_PROPERTY(bool gamescopeEnabled READ gamescopeEnabled WRITE setGamescopeEnabled NOTIFY useGamescopeChanged)
    Q_PROPERTY(bool gamemodeEnabled READ gamemodeEnabled WRITE setGamemodeEnabled NOTIFY useGamemodeChanged)
    Q_PROPERTY(bool directx9Enabled READ directx9Enabled WRITE setDirectX9Enabled NOTIFY useDX9Changed)
    Q_PROPERTY(bool gamescopeFullscreen READ gamescopeFullscreen WRITE setGamescopeFullscreen NOTIFY gamescopeFullscreenChanged)
    Q_PROPERTY(bool gamescopeBorderless READ gamescopeBorderless WRITE setGamescopeBorderless NOTIFY gamescopeBorderlessChanged)
    Q_PROPERTY(int gamescopeWidth READ gamescopeWidth WRITE setGamescopeWidth NOTIFY gamescopeWidthChanged)
    Q_PROPERTY(int gamescopeHeight READ gamescopeHeight WRITE setGamescopeHeight NOTIFY gamescopeHeightChanged)
    Q_PROPERTY(int gamescopeRefreshRate READ gamescopeRefreshRate WRITE setGamescopeRefreshRate NOTIFY gamescopeRefreshRateChanged)
    Q_PROPERTY(bool dalamudEnabled READ dalamudEnabled WRITE setDalamudEnabled NOTIFY dalamudEnabledChanged)
    Q_PROPERTY(DalamudInjectMethod dalamudInjectMethod READ dalamudInjectMethod WRITE setDalamudInjectMethod NOTIFY dalamudInjectMethodChanged)
    Q_PROPERTY(int dalamudInjectDelay READ dalamudInjectDelay WRITE setDalamudInjectDelay NOTIFY dalamudInjectDelayChanged)
    Q_PROPERTY(DalamudChannel dalamudChannel READ dalamudChannel WRITE setDalamudChannel NOTIFY dalamudChannelChanged)
    Q_PROPERTY(bool argumentsEncrypted READ argumentsEncrypted WRITE setArgumentsEncrypted NOTIFY encryptedArgumentsChanged)
    Q_PROPERTY(bool isGameInstalled READ isGameInstalled NOTIFY gameInstallChanged)
    Q_PROPERTY(Account *account READ account WRITE setAccount NOTIFY accountChanged)
    Q_PROPERTY(QString expansionVersionText READ expansionVersionText NOTIFY gameInstallChanged)
    Q_PROPERTY(QString dalamudVersionText READ dalamudVersionText NOTIFY gameInstallChanged)
    Q_PROPERTY(QString wineVersionText READ wineVersionText NOTIFY wineChanged)
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY loggedInChanged)

public:
    explicit Profile(LauncherCore &launcher, const QString &key, QObject *parent = nullptr);

    enum class WineType {
        System,
        Custom,
        Builtin, // macos only
        XIVOnMac // macos only
    };
    Q_ENUM(WineType)

    enum class DalamudChannel { Stable, Staging, Net5 };
    Q_ENUM(DalamudChannel)

    enum class DalamudInjectMethod { Entrypoint, DLLInject };
    Q_ENUM(DalamudInjectMethod)

    [[nodiscard]] QString uuid() const;

    [[nodiscard]] QString name() const;
    void setName(const QString &name);

    [[nodiscard]] QString gamePath() const;
    void setGamePath(const QString &path);

    [[nodiscard]] QString winePath() const;
    void setWinePath(const QString &path);

    [[nodiscard]] QString winePrefixPath() const;
    void setWinePrefixPath(const QString &path);

    [[nodiscard]] bool watchdogEnabled() const;
    void setWatchdogEnabled(bool value);

    [[nodiscard]] WineType wineType() const;
    void setWineType(WineType type);

    [[nodiscard]] bool esyncEnabled() const;
    void setESyncEnabled(bool value);

    [[nodiscard]] bool gamescopeEnabled() const;
    void setGamescopeEnabled(bool value);

    [[nodiscard]] bool gamemodeEnabled() const;
    void setGamemodeEnabled(bool value);

    [[nodiscard]] bool directx9Enabled() const;
    void setDirectX9Enabled(bool value);

    [[nodiscard]] bool gamescopeFullscreen() const;
    void setGamescopeFullscreen(bool value);

    [[nodiscard]] bool gamescopeBorderless() const;
    void setGamescopeBorderless(bool value);

    [[nodiscard]] int gamescopeWidth() const;
    void setGamescopeWidth(int value);

    [[nodiscard]] int gamescopeHeight() const;
    void setGamescopeHeight(int value);

    [[nodiscard]] int gamescopeRefreshRate() const;
    void setGamescopeRefreshRate(int value);

    [[nodiscard]] bool dalamudEnabled() const;
    void setDalamudEnabled(bool value);

    [[nodiscard]] DalamudChannel dalamudChannel() const;
    void setDalamudChannel(DalamudChannel channel);

    [[nodiscard]] DalamudInjectMethod dalamudInjectMethod() const;
    void setDalamudInjectMethod(DalamudInjectMethod value);

    [[nodiscard]] int dalamudInjectDelay() const;
    void setDalamudInjectDelay(int value);

    [[nodiscard]] bool argumentsEncrypted() const;
    void setArgumentsEncrypted(bool value);

    [[nodiscard]] Account *account() const;
    [[nodiscard]] QString accountUuid() const;
    void setAccount(Account *account);

    void readGameData();
    Q_INVOKABLE void readGameVersion();
    void readWineInfo();

    [[nodiscard]] QString expansionVersionText() const;
    [[nodiscard]] QString dalamudVersionText() const;
    [[nodiscard]] QString wineVersionText() const;

    [[nodiscard]] QString dalamudChannelName() const;

    [[nodiscard]] bool isGameInstalled() const;
    [[nodiscard]] bool isWineInstalled() const;

    [[nodiscard]] QString bootVersion() const;
    [[nodiscard]] QString baseGameVersion() const;
    [[nodiscard]] int numInstalledExpansions() const;
    [[nodiscard]] QString expansionVersion(int index) const;

    [[nodiscard]] int dalamudAssetVersion() const;
    void setDalamudAssetVersion(int version);

    [[nodiscard]] QString runtimeVersion() const;

    [[nodiscard]] QString dalamudVersion() const;
    void setDalamudVersion(const QString &version);

    BootData *bootData();
    GameData *gameData();

    bool loggedIn() const;
    void setLoggedIn(bool value);

Q_SIGNALS:
    void gameInstallChanged();
    void nameChanged();
    void gamePathChanged();
    void winePathChanged();
    void winePrefixPathChanged();
    void enableWatchdogChanged();
    void wineTypeChanged();
    void useESyncChanged();
    void useGamescopeChanged();
    void useGamemodeChanged();
    void useDX9Changed();
    void gamescopeFullscreenChanged();
    void gamescopeBorderlessChanged();
    void gamescopeWidthChanged();
    void gamescopeHeightChanged();
    void gamescopeRefreshRateChanged();
    void dalamudEnabledChanged();
    void dalamudChannelChanged();
    void dalamudInjectMethodChanged();
    void dalamudInjectDelayChanged();
    void encryptedArgumentsChanged();
    void accountChanged();
    void wineChanged();
    void loggedInChanged();

private:
    QString m_uuid;
    QString m_wineVersion;
    ProfileConfig *m_config = nullptr;
    Account *m_account = nullptr;

    QVector<QString> m_expansionNames;

    BootData *m_bootData = nullptr;
    GameData *m_gameData = nullptr;

    physis_Repositories m_repositories = {};
    const char *m_bootVersion = nullptr;

    QString m_dalamudVersion;
    int m_dalamudAssetVersion = -1;
    QString m_runtimeVersion;

    bool m_loggedIn = false;

    LauncherCore &m_launcher;
};