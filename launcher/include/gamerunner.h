// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QProcess>

class LauncherCore;
class Profile;
struct LoginAuth;

class GameRunner : public QObject
{
public:
    explicit GameRunner(LauncherCore &launcher, QObject *parent = nullptr);

    /// Begins the game executable, but calls to Dalamud if needed.
    void beginGameExecutable(Profile &settings, const std::optional<LoginAuth> &auth);

private:
    /// Starts a vanilla game session with no Dalamud injection.
    void beginVanillaGame(const QString &gameExecutablePath, Profile &profile, const std::optional<LoginAuth> &auth);

    /// Starts a game session with Dalamud injected.
    void beginDalamudGame(const QString &gameExecutablePath, Profile &profile, const std::optional<LoginAuth> &auth);

    /// Returns the game arguments needed to properly launch the game. This encrypts it too if needed, and it's already joined!
    QString getGameArgs(const Profile &profile, const std::optional<LoginAuth> &auth);

    /// This wraps it in wine if needed.
    void launchExecutable(const Profile &settings, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup);

    /// Set a Wine registry key
    /// \param settings The profile that's being launched
    /// \param key The path to the registry key, such as HKEY_CURRENT_USER\\Software\\Wine
    /// \param value The registry key name, like "HideWineExports"
    /// \param data What to set the value as, like "1" or "0"
    void addRegistryKey(const Profile &settings, const QString &key, const QString &value, const QString &data);

    void setWindowsVersion(const Profile &settings, const QString &version);

    LauncherCore &m_launcher;
};