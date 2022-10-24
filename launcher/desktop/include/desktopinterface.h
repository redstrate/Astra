#pragma once

#include <QMdiArea>
#include <QMainWindow>

#include "launcherwindow.h"
#include "autologinwindow.h"
#include "virtualdialog.h"

/*
 * The desktop, mouse and keyboard-driven interface for Astra. Primarily meant
 * for regular desktop usage.
 */
class DesktopInterface {
public:
    explicit DesktopInterface(LauncherCore& core);

    void addWindow(VirtualWindow* window);
    void addDialog(VirtualDialog* dialog);

    bool oneWindow = true;
    bool isSteamDeck = true;

private:
    QMdiArea* mdiArea = nullptr;
    QMainWindow* mdiWindow = nullptr;

    LauncherWindow* window = nullptr;
    AutoLoginWindow* autoLoginWindow = nullptr;
};