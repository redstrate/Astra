#pragma once

#include <QObject>

class LauncherCore;

class SteamAPI : public QObject
{
public:
    explicit SteamAPI(LauncherCore &core, QObject *parent = nullptr);

    void setLauncherMode(bool isLauncher);

    [[nodiscard]] bool isDeck() const;

private:
    LauncherCore &core;
};