// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qcorotask.h>

#include "launchercore.h"
#include "patcher.h"

class SquareEnixLogin : public QObject
{
    Q_OBJECT

public:
    explicit SquareEnixLogin(LauncherCore &window, QObject *parent = nullptr);

    /// Begins the login process for official Square Enix servers
    /// \param info The required login information
    /// \return Arguments used for logging into the game, if successful
    QCoro::Task<std::optional<LoginAuth>> login(LoginInformation *info);

private:
    /// Checks the gate status to see if the servers are closed for maintenance
    /// \return False if the gate is closed, true if open.
    QCoro::Task<bool> checkGateStatus();

    /// Checks the login status to see if the servers are closed for maintenance
    /// \return False if logging in is disabled, true if open.
    QCoro::Task<bool> checkLoginStatus();

    /// Check for updates to the boot components. Even though we don't use these, it's checked by later login steps.
    QCoro::Task<> checkBootUpdates();

    using StoredInfo = std::pair<QString, QUrl>;

    /// Get the _STORED_ value used in the oauth step
    QCoro::Task<std::optional<StoredInfo>> getStoredValue();

    /// Logs into the server
    /// \return Returns false if the oauth call failed for some reason
    QCoro::Task<bool> loginOAuth();

    /// Registers a new session with the login server and patches the game if necessary
    /// \return Returns false if the session registration failed for some reason
    QCoro::Task<bool> registerSession();

    /// Returns the hashes of the boot components
    QCoro::Task<QString> getBootHash();

    /// Gets the SHA1 hash of a file
    static QString getFileHash(const QString &file);

    Patcher *m_patcher = nullptr;

    QString m_SID, m_username;
    LoginAuth m_auth;
    LoginInformation *m_info = nullptr;
    bool m_lastRunHasPatched = true;

    LauncherCore &m_launcher;
};
