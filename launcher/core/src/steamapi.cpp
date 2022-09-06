#include "steamapi.h"
#include "launchercore.h"

#include <QtGlobal>

#ifdef ENABLE_STEAM
#include <steam/steam_api.h>
#endif

SteamAPI::SteamAPI(LauncherCore& core) {
#ifdef ENABLE_STEAM
    if(core.isSteam) {
        qputenv("SteamAppId", "39210");
        qputenv("SteamGameId", "39210");

        if(!SteamAPI_Init())
            qDebug() << "Failed to initialize steam api!";
    }
#endif
}