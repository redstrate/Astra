#pragma once

#include <QQmlApplicationEngine>

#include "launchercore.h"

/*
 * The tablet-oriented (name to change), touch and gamepad-driven interface for Astra. The interface is
 * simpler due to size constraints.
 */
class TabletInterface {
public:
    TabletInterface(LauncherCore& core);

private:
    QQmlApplicationEngine* applicationEngine = nullptr;
};