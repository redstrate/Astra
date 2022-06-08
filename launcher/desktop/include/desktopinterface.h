#pragma once

#include "launcherwindow.h"

/*
 * The desktop, mouse and keyboard-driven interface for Astra. Primarily meant
 * for regular desktop usage.
 */
class DesktopInterface {
public:
    DesktopInterface(LauncherCore& core);

private:
    LauncherWindow* window = nullptr;
};