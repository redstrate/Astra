// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include "squareboot.h"

class Account;
class ProfileConfig;

class Profile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int language READ language WRITE setLanguage NOTIFY languageChanged)
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
    Q_PROPERTY(bool dalamudOptOut READ dalamudOptOut WRITE setDalamudOptOut NOTIFY dalamudOptOutChanged)
    Q_PROPERTY(DalamudChannel dalamudChannel READ dalamudChannel WRITE setDalamudChannel NOTIFY dalamudChannelChanged)
    Q_PROPERTY(bool argumentsEncrypted READ argumentsEncrypted WRITE setArgumentsEncrypted NOTIFY encryptedArgumentsChanged)
    Q_PROPERTY(bool isGameInstalled READ isGameInstalled NOTIFY gameInstallChanged)
    Q_PROPERTY(Account *account READ account WRITE setAccount NOTIFY accountChanged)
    Q_PROPERTY(QString expansionVersionText READ expansionVersionText NOTIFY gameInstallChanged)
    Q_PROPERTY(QString dalamudVersionText READ dalamudVersionText NOTIFY gameInstallChanged)
    Q_PROPERTY(QString wineVersionText READ wineVersionText NOTIFY wineChanged)

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

    QString uuid() const;

    QString name() const;
    void setName(const QString &name);

    int language() const;
    void setLanguage(int value);

    QString gamePath() const;
    void setGamePath(const QString &path);

    QString winePath() const;
    void setWinePath(const QString &path);

    QString winePrefixPath() const;
    void setWinePrefixPath(const QString &path);

    bool watchdogEnabled() const;
    void setWatchdogEnabled(bool value);

    WineType wineType() const;
    void setWineType(WineType type);

    bool esyncEnabled() const;
    void setESyncEnabled(bool value);

    bool gamescopeEnabled() const;
    void setGamescopeEnabled(bool value);

    bool gamemodeEnabled() const;
    void setGamemodeEnabled(bool value);

    bool directx9Enabled() const;
    void setDirectX9Enabled(bool value);

    bool gamescopeFullscreen() const;
    void setGamescopeFullscreen(bool value);

    bool gamescopeBorderless() const;
    void setGamescopeBorderless(bool value);

    int gamescopeWidth() const;
    void setGamescopeWidth(int value);

    int gamescopeHeight() const;
    void setGamescopeHeight(int value);

    int gamescopeRefreshRate() const;
    void setGamescopeRefreshRate(int value);

    bool dalamudEnabled() const;
    void setDalamudEnabled(bool value);

    bool dalamudOptOut() const;
    void setDalamudOptOut(bool value);

    DalamudChannel dalamudChannel() const;
    void setDalamudChannel(DalamudChannel channel);

    bool argumentsEncrypted() const;
    void setArgumentsEncrypted(bool value);

    Account *account() const;
    QString accountUuid() const;
    void setAccount(Account *account);

    void readGameData();
    void readGameVersion();
    void readWineInfo();

    QVector<QString> expansionNames;

    BootData *bootData;
    GameData *gameData;

    physis_Repositories repositories = {};
    const char *bootVersion = nullptr;

    QString dalamudVersion;
    int dalamudAssetVersion = -1;
    QString runtimeVersion;

    QString expansionVersionText() const;
    QString dalamudVersionText() const;
    QString wineVersionText() const;

    [[nodiscard]] bool isGameInstalled() const
    {
        return repositories.repositories_count > 0;
    }

    [[nodiscard]] bool isWineInstalled() const
    {
        return !m_wineVersion.isEmpty();
    }

    QString dalamudChannelName() const;

Q_SIGNALS:
    void gameInstallChanged();
    void nameChanged();
    void languageChanged();
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
    void dalamudOptOutChanged();
    void dalamudChannelChanged();
    void encryptedArgumentsChanged();
    void accountChanged();
    void wineChanged();

private:
    QString m_uuid;
    QString m_wineVersion;
    ProfileConfig *m_config = nullptr;
    Account *m_account = nullptr;
    LauncherCore &m_launcher;
};