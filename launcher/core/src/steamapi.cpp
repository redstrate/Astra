#include "steamapi.h"

#include <steam/steam_api.h>

SteamAPI::SteamAPI() {
#ifdef USE_STEAM
    SteamAPI_Init();
#endif
}