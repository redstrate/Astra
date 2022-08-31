#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
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
    void setupAccountsTab(QFormLayout& layout);

    // profile specific tabs
    void setupGameTab(QFormLayout& layout);
    void setupLoginTab(QFormLayout& layout);
    void setupWineTab(QFormLayout& layout);
    void setupDalamudTab(QFormLayout& layout);

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
    QLabel* currentGameDirectory = nullptr;
    QLabel* expansionVersionLabel = nullptr;
    QPushButton* gameDirectoryButton = nullptr;

    // wine
    QComboBox* wineTypeCombo;
    QPushButton* selectWineButton;
    QLabel* winePathLabel;
    QLabel* winePrefixDirectory;
    QPushButton* configureGamescopeButton;
    QLabel* wineVersionLabel;

    QCheckBox *useGamescope, *useEsync, *useGamemode;
    QCheckBox* enableWatchdog;

    // login
    QCheckBox* encryptArgumentsBox = nullptr;
    QComboBox* serverType = nullptr;
    QLineEdit* lobbyServerURL = nullptr;
    QCheckBox *rememberUsernameBox = nullptr, *rememberPasswordBox = nullptr;
    QComboBox* gameLicenseBox = nullptr;
    QCheckBox* freeTrialBox = nullptr;
    QCheckBox* useOneTimePassword = nullptr;
    QCheckBox* autoLoginBox = nullptr;

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
