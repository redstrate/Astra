#pragma once

#include <QDialog>

class LauncherCore;
class LauncherWindow;
struct ProfileSettings;

class AutoLoginWindow : public QDialog {
    Q_OBJECT
public:
    AutoLoginWindow(ProfileSettings& settings, LauncherCore& core, QWidget* parent = nullptr);

signals:
    void loginCanceled();
};
