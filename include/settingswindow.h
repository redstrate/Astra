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
    SettingsWindow(int defaultTab, LauncherWindow& window, LauncherCore& core, QWidget* parent = nullptr);

public slots:
    void reloadControls();

private:
    ProfileSettings& getCurrentProfile();

    QListWidget* profileWidget = nullptr;
    QPushButton* deleteProfileButton = nullptr;

    // general
    QCheckBox* closeWhenLaunched = nullptr;

    // game
    QLineEdit* nameEdit = nullptr;
    QComboBox* directXCombo = nullptr;
    QLineEdit* currentGameDirectory = nullptr;
    QLabel* expansionVersionLabel = nullptr;

    // wine
    QComboBox* wineVersionCombo;
    QPushButton* selectWineButton;
    QLineEdit* winePathLabel;
    QLineEdit* winePrefixDirectory;
    QPushButton* configureGamescopeButton;

    QCheckBox* useGamescope, *useEsync, *useGamemode;
    QCheckBox* enableWatchdog;

    // login
    QCheckBox* encryptArgumentsBox = nullptr;
    QComboBox* serverType = nullptr;
    QLineEdit* lobbyServerURL = nullptr;
    QCheckBox* rememberUsernameBox = nullptr, *rememberPasswordBox = nullptr;
    QCheckBox* useSteamBox = nullptr;

    // dalamud
    QCheckBox* enableDalamudBox = nullptr;
    QLabel* dalamudVersionLabel = nullptr;
    QLabel* dalamudAssetVersionLabel = nullptr;
    QCheckBox* dalamudOptOutBox = nullptr;

    bool currentlyReloadingControls = false;

    LauncherWindow& window;
    LauncherCore& core;
};
