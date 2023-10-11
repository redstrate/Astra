// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include "launchercore.h"

class SapphireLogin : QObject
{
public:
    explicit SapphireLogin(LauncherCore &window, QObject *parent = nullptr);

    /// Begins the login process for Sapphire servers
    /// \param info The required login information
    QCoro::Task<std::optional<LoginAuth>> login(const QString &lobbyUrl, const LoginInformation &info);

    void registerAccount(const QString &lobbyUrl, const LoginInformation &info);

private:
    LauncherCore &m_launcher;
};