#include "steamapi.h"

#ifdef USE_STEAM
#include <steam/steam_api.h>
#endif

SteamAPI::SteamAPI() {
#ifdef USE_STEAM
    SteamAPI_Init();
#endif
}