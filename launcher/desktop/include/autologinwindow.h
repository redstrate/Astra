#pragma once

#include "virtualdialog.h"

class LauncherCore;
class LauncherWindow;
struct ProfileSettings;

class AutoLoginWindow : public VirtualDialog {
    Q_OBJECT
public:
    AutoLoginWindow(DesktopInterface& interface, ProfileSettings& settings, LauncherCore& core, QWidget* parent = nullptr);

signals:
    void loginCanceled();
};
