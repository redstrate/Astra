#pragma once

class LauncherCore;

class SteamAPI {
public:
    explicit SteamAPI(LauncherCore& core);

    void setLauncherMode(bool isLauncher);

    bool isDeck() const;
};