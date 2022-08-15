#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

class LauncherCore;
class LauncherWindow;
struct ProfileSettings;

class GamescopeSettingsWindow : public QDialog {
public:
    GamescopeSettingsWindow(ProfileSettings& settings, LauncherCore& core, QWidget* parent = nullptr);
};
