// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

#include <physis.hpp>

class Account;
class ProfileConfig;

class Profile : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use from ProfileManager")

    Q_PROPERTY(QString winePath READ winePath WRITE setWinePath NOTIFY winePathChanged)
    Q_PROPERTY(bool hasDirectx9 READ hasDirectx9 NOTIFY hasDirectx9Changed)
    Q_PROPERTY(bool isGameInstalled READ isGameInstalled NOTIFY gameInstallChanged)
    Q_PROPERTY(Account *account READ account WRITE setAccount NOTIFY accountChanged)
    Q_PROPERTY(QString expansionVersionText READ expansionVersionText NOTIFY gameInstallChanged)
    Q_PROPERTY(QString dalamudVersionText READ dalamudVersionText NOTIFY gameInstallChanged)
    Q_PROPERTY(QString wineVersionText READ wineVersionText NOTIFY wineChanged)
    Q_PROPERTY(QString subtitle READ subtitle NOTIFY gameInstallChanged)
    Q_PROPERTY(ProfileConfig *config READ config CONSTANT)

public:
    explicit Profile(const QString &key, QObject *parent = nullptr);
    ~Profile() override;

    enum WineType {
        BuiltIn,
        Custom
    };
    Q_ENUM(WineType)

    enum DalamudChannel {
        Stable,
        Staging,
        Local
    };
    Q_ENUM(DalamudChannel)

    enum DalamudInjectMethod {
        Entrypoint,
        DLLInject
    };
    Q_ENUM(DalamudInjectMethod)

    [[nodiscard]] QString uuid() const;

    [[nodiscard]] QString winePath() const;
    void setWinePath(const QString &path);

    [[nodiscard]] bool hasDirectx9() const;

    [[nodiscard]] Account *account() const;
    void setAccount(Account *account);

    void readGameVersion();

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

    [[nodiscard]] QString frontierUrl() const;

    [[nodiscard]] int dalamudAssetVersion() const;
    void setDalamudAssetVersion(int version);

    [[nodiscard]] QString runtimeVersion() const;

    [[nodiscard]] QString dalamudVersion() const;
    void setDalamudVersion(const QString &version);

    /// @brief Sets whether or not the Dalamud version is applicable to the current game version.
    /// @note If this is false, Dalamud will not launch.
    void setDalamudApplicable(bool applicable);

    /// @return If Dalamud is enabled, and it's also applicable for the current game version.
    [[nodiscard]] bool dalamudShouldLaunch() const;

    [[nodiscard]] QString compatibilityToolVersion() const;
    void setCompatibilityToolVersion(const QString &version);

    BootData *bootData() const;
    GameData *gameData() const;

    [[nodiscard]] QString subtitle() const;

    ProfileConfig *config() const;

Q_SIGNALS:
    void gameInstallChanged();
    void winePathChanged();
    void accountChanged();
    void wineChanged();
    void hasDirectx9Changed();

private:
    void readGameData();
    void readWineInfo();
    void readDalamudInfo();

    QString m_uuid;
    QString m_wineVersion;
    ProfileConfig *m_config = nullptr;
    Account *m_account = nullptr;

    QList<QString> m_expansionNames;

    BootData *m_bootData = nullptr;
    GameData *m_gameData = nullptr;

    physis_Repositories m_repositories = {};
    const char *m_bootVersion = nullptr;

    QString m_dalamudVersion;
    int m_dalamudAssetVersion = -1;
    QString m_runtimeVersion;
    QString m_compatibilityToolVersion;
    bool m_dalamudApplicable = false;

    QString m_frontierUrl;
};
