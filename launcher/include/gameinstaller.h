// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>

class LauncherCore;
class Profile;

class GameInstaller : public QObject
{
    Q_OBJECT
public:
    GameInstaller(LauncherCore &launcher, Profile &profile, QObject *parent = nullptr);

    Q_INVOKABLE void installGame();

Q_SIGNALS:
    void installFinished();

private:
    LauncherCore &m_launcher;
    Profile &m_profile;
};