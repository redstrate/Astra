#include "steamapi.h"
#include "launchercore.h"

#ifdef ENABLE_STEAM
#include <steam/steam_api.h>
#endif

SteamAPI::SteamAPI(LauncherCore& core) {
#ifdef ENABLE_STEAM
    if(core.isSteam) {
        SteamAPI_Init();
    }
#endif
}