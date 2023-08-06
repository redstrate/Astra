// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "steamapi.h"

#ifdef ENABLE_STEAM
#include <steam/steam_api.h>
#endif

#include "launchercore.h"

SteamAPI::SteamAPI(LauncherCore &core, QObject *parent)
    : QObject(parent)
    , core(core)
{
#ifdef ENABLE_STEAM
    if (core.isSteam()) {
        qputenv("SteamAppId", "39210");
        qputenv("SteamGameId", "39210");

        if (!SteamAPI_Init())
            qDebug() << "Failed to initialize steam api!";
    }
#endif
}

void SteamAPI::setLauncherMode(bool isLauncher)
{
#ifdef ENABLE_STEAM
    if (core.isSteam()) {
        SteamUtils()->SetGameLauncherMode(isLauncher);
    }
#else
    Q_UNUSED(isLauncher)
#endif
}

bool SteamAPI::isDeck() const
{
#ifdef ENABLE_STEAM
    if (core.isSteam()) {
        return SteamUtils()->IsSteamRunningOnSteamDeck();
    } else {
        return false;
    }
#else
    return false;
#endif
}
