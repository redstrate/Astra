#pragma once

#include <QDialog>
#include <QListWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

class LauncherCore;
class LauncherWindow;
struct ProfileSettings;

class GamescopeSettingsWindow : public QDialog {
public:
    GamescopeSettingsWindow(ProfileSettings& settings, LauncherCore& core, QWidget* parent = nullptr);
};
