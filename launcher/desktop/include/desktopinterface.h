#pragma once

#include "launcherwindow.h"
#include "autologinwindow.h"

/*
 * The desktop, mouse and keyboard-driven interface for Astra. Primarily meant
 * for regular desktop usage.
 */
class DesktopInterface {
public:
    explicit DesktopInterface(LauncherCore& core);

private:
    LauncherWindow* window = nullptr;
    AutoLoginWindow* autoLoginWindow = nullptr;
};