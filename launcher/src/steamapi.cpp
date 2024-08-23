// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "steamapi.h"

#include "launchercore.h"

SteamAPI::SteamAPI(QObject *parent)
    : QObject(parent)
{
    // TODO: stub
}

void SteamAPI::setLauncherMode(bool isLauncher)
{
    Q_UNUSED(isLauncher)
    // TODO: stub
}

bool SteamAPI::isDeck() const
{
    // TODO: stub
    return false;
}
