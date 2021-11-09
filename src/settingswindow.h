#pragma once

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QLineEdit>

class LauncherWindow;
struct ProfileSettings;

class SettingsWindow : public QWidget {
public:
    SettingsWindow(LauncherWindow& window, QWidget* parent = nullptr);

public slots:
    void reloadControls();

private:
    void openPath(const QString path);
    ProfileSettings& getCurrentProfile();

    QListWidget* profileWidget = nullptr;

    QLineEdit* nameEdit = nullptr;
    QComboBox* directXCombo = nullptr;

    bool currentlyReloadingControls = false;

    LauncherWindow& window;
};