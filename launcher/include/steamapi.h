// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class LauncherCore;

class SteamAPI : public QObject
{
public:
    explicit SteamAPI(LauncherCore &core, QObject *parent = nullptr);
    ~SteamAPI();

    void setLauncherMode(bool isLauncher);

    [[nodiscard]] bool isDeck() const;

private:
    LauncherCore &core;
};