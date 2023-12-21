// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class LauncherCore;

class SteamAPI : public QObject
{
public:
    explicit SteamAPI(QObject *parent = nullptr);

    void setLauncherMode(bool isLauncher);

    [[nodiscard]] bool isDeck() const;
};