#pragma once

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>

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
    QLabel* currentGameDirectory = nullptr;

    QComboBox* serverType = nullptr;
    QCheckBox* rememberUsernameBox = nullptr, *rememberPasswordBox = nullptr;

    bool currentlyReloadingControls = false;

    LauncherWindow& window;
};