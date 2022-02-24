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

class SettingsWindow : public QDialog {
public:
    SettingsWindow(LauncherWindow& window, LauncherCore& core, QWidget* parent = nullptr);

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
    QLabel* expansionVersionLabel = nullptr;

    // wine
    QComboBox* wineVersionCombo;
    QPushButton* selectWineButton;
    QLabel* winePathLabel;
    QLabel* winePrefixDirectory;

    QCheckBox* useGamescope, *useEsync, *useGamemode;
    QCheckBox* enableWatchdog;

    // login
    QCheckBox* encryptArgumentsBox = nullptr;
    QCheckBox* enableDalamudBox = nullptr;
    QComboBox* serverType = nullptr;
    QLineEdit* lobbyServerURL = nullptr;
    QCheckBox* rememberUsernameBox = nullptr, *rememberPasswordBox = nullptr;
    QCheckBox* useSteamBox = nullptr;

    bool currentlyReloadingControls = false;

    LauncherWindow& window;
    LauncherCore& core;
};
