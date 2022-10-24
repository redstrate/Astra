#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

#include "virtualdialog.h"

class LauncherCore;
class LauncherWindow;
struct ProfileSettings;

class GamescopeSettingsWindow : public VirtualDialog {
public:
    GamescopeSettingsWindow(DesktopInterface& interface, ProfileSettings& settings, LauncherCore& core, QWidget* parent = nullptr);
};
