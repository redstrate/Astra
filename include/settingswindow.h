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
    QCheckBox* showBanner = nullptr;
    QCheckBox* showNewsList = nullptr;

    // game
    QLineEdit* nameEdit = nullptr;
    QComboBox* directXCombo = nullptr;
    QLineEdit* currentGameDirectory = nullptr;
    QLabel* expansionVersionLabel = nullptr;
    QPushButton* gameDirectoryButton = nullptr;

    // wine
    QComboBox* wineTypeCombo;
    QPushButton* selectWineButton;
    QLineEdit* winePathLabel;
    QLineEdit* winePrefixDirectory;
    QPushButton* configureGamescopeButton;
    QLabel* wineVersionLabel;

    QCheckBox* useGamescope, *useEsync, *useGamemode;
    QCheckBox* enableWatchdog;

    // login
    QCheckBox* encryptArgumentsBox = nullptr;
    QComboBox* serverType = nullptr;
    QLineEdit* lobbyServerURL = nullptr;
    QCheckBox* rememberUsernameBox = nullptr, *rememberPasswordBox = nullptr;
    QComboBox* gameLicenseBox = nullptr;
    QCheckBox* useOneTimePassword = nullptr;

    // dalamud
    QCheckBox* enableDalamudBox = nullptr;
    QLabel* dalamudVersionLabel = nullptr;
    QLabel* dalamudAssetVersionLabel = nullptr;
    QLabel* nativeLauncherVersionLabel = nullptr;
    QCheckBox* dalamudOptOutBox = nullptr;
    QComboBox* dalamudChannel = nullptr;

    bool currentlyReloadingControls = false;

    LauncherWindow& window;
    LauncherCore& core;
};
