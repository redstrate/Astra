#pragma once

class LauncherCore;

class SteamAPI {
public:
    explicit SteamAPI(LauncherCore& core);

    void setLauncherMode(bool isLauncher);

    [[nodiscard]] bool isDeck() const;

private:
    LauncherCore& core;
};