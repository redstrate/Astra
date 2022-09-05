#include "steamapi.h"

#ifdef ENABLE_STEAM
#include <steam/steam_api.h>
#endif

SteamAPI::SteamAPI() {
#ifdef ENABLE_STEAM
    SteamAPI_Init();
#endif
}