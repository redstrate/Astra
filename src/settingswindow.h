#pragma once

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

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
    QPushButton* deleteProfileButton = nullptr;

    // game
    QLineEdit* nameEdit = nullptr;
    QComboBox* directXCombo = nullptr;
    QLabel* currentGameDirectory = nullptr;

    // wine
    QComboBox* wineVersionCombo;
    QPushButton* selectWineButton;
    QLabel* winePathLabel;
    QLabel* winePrefixDirectory;

    QCheckBox* useGamescope, *useEsync, *useGamemode;

    // login
    QCheckBox* encryptArgumentsBox = nullptr;
    QCheckBox* enableDalamudBox = nullptr;
    QComboBox* serverType = nullptr;
    QLineEdit* lobbyServerURL = nullptr;
    QCheckBox* rememberUsernameBox = nullptr, *rememberPasswordBox = nullptr;

    bool currentlyReloadingControls = false;

    LauncherWindow& window;
};