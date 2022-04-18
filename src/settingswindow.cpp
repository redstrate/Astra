#include "settingswindow.h"

#include <QFormLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QLabel>
#include <QFileDialog>
#include <QCheckBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QProcess>
#include <QGridLayout>
#include <QToolTip>

#include "launchercore.h"
#include "launcherwindow.h"
#include "gamescopesettingswindow.h"

SettingsWindow::SettingsWindow(int defaultTab, LauncherWindow& window, LauncherCore& core, QWidget* parent) : core(core), window(window), QDialog(parent) {
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    auto tabWidget = new QTabWidget();
    mainLayout->addWidget(tabWidget);

    // general tab
    {
        auto generalTabWidget = new QWidget();
        tabWidget->addTab(generalTabWidget, "General");

        auto layout = new QFormLayout();
        generalTabWidget->setLayout(layout);

        closeWhenLaunched = new QCheckBox("Close Astra when game is launched");
        connect(closeWhenLaunched, &QCheckBox::stateChanged, [&](int state) {
            core.appSettings.closeWhenLaunched = state;

            core.saveSettings();
        });
        layout->addWidget(closeWhenLaunched);

        showBanner = new QCheckBox("Show news banners");
        connect(showBanner, &QCheckBox::stateChanged, [&](int state) {
            core.appSettings.showBanners = state;

            core.saveSettings();
            window.reloadControls();
        });
        layout->addWidget(showBanner);

        showNewsList = new QCheckBox("Show news list");
        connect(showNewsList, &QCheckBox::stateChanged, [&](int state) {
            core.appSettings.showNewsList = state;

            core.saveSettings();
            window.reloadControls();
        });
        layout->addWidget(showNewsList);
    }

    // profile tab
    {
        auto profileTabWidget = new QWidget();
        tabWidget->addTab(profileTabWidget, "Profiles");

        auto profileLayout = new QGridLayout();
        profileTabWidget->setLayout(profileLayout);

        profileWidget = new QListWidget();
        profileWidget->addItem("INVALID *DEBUG*");
        profileWidget->setCurrentRow(0);

        connect(profileWidget, &QListWidget::currentRowChanged, this,
                &SettingsWindow::reloadControls);

        profileLayout->addWidget(profileWidget, 0, 0, 3, 1);

        auto addProfileButton = new QPushButton("Add Profile");
        connect(addProfileButton, &QPushButton::pressed, [=] {
            profileWidget->setCurrentRow(this->core.addProfile());

            this->core.saveSettings();
        });
        profileLayout->addWidget(addProfileButton, 3, 0);

        deleteProfileButton = new QPushButton("Delete Profile");
        connect(deleteProfileButton, &QPushButton::pressed, [=] {
            profileWidget->setCurrentRow(
                this->core.deleteProfile(getCurrentProfile().name));

            this->core.saveSettings();
        });
        profileLayout->addWidget(deleteProfileButton, 0, 2);

        nameEdit = new QLineEdit();
        connect(nameEdit, &QLineEdit::editingFinished, [=] {
            getCurrentProfile().name = nameEdit->text();

            reloadControls();
            this->core.saveSettings();
        });
        profileLayout->addWidget(nameEdit, 0, 1);

        auto gameBox = new QGroupBox("Game Options");
        auto gameBoxLayout = new QFormLayout();
        gameBox->setLayout(gameBoxLayout);

        profileLayout->addWidget(gameBox, 1, 1);

        directXCombo = new QComboBox();
        directXCombo->addItem("DirectX 11");
        directXCombo->addItem("DirectX 9");
        gameBoxLayout->addRow("DirectX Version", directXCombo);

        connect(directXCombo,
                static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                [=](int index) {
                    getCurrentProfile().useDX9 =
                        directXCombo->currentIndex() == 1;
                    this->core.saveSettings();
                });

        currentGameDirectory = new QLineEdit();
        currentGameDirectory->setReadOnly(true);
        gameBoxLayout->addRow("Game Directory", currentGameDirectory);

        auto gameDirButtonLayout = new QHBoxLayout();
        auto gameDirButtonContainer = new QWidget();
        gameDirButtonContainer->setLayout(gameDirButtonLayout);
        gameBoxLayout->addWidget(gameDirButtonContainer);

        auto selectDirectoryButton = new QPushButton("Select Game Directory");
        connect(selectDirectoryButton, &QPushButton::pressed, [this] {
            getCurrentProfile().gamePath =
                QFileDialog::getExistingDirectory(this, "Open Game Directory");

            this->reloadControls();
            this->core.saveSettings();

            this->core.readGameVersion();
        });
        gameDirButtonLayout->addWidget(selectDirectoryButton);

        gameDirectoryButton = new QPushButton("Open Game Directory");
        connect(gameDirectoryButton, &QPushButton::pressed,
                [&window, this] { window.openPath(getCurrentProfile().gamePath); });
        gameDirButtonLayout->addWidget(gameDirectoryButton);

#ifdef ENABLE_WATCHDOG
        enableWatchdog = new QCheckBox("Enable Watchdog (X11 only)");
        gameBoxLayout->addWidget(enableWatchdog);

        connect(enableWatchdog, &QCheckBox::stateChanged, [this](int state) {
            getCurrentProfile().enableWatchdog = state;

            this->core.saveSettings();
        });
#endif

        gameDirectoryButton->setEnabled(getCurrentProfile().isGameInstalled());

        expansionVersionLabel = new QLabel();
        expansionVersionLabel->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        gameBoxLayout->addRow("Game Version", expansionVersionLabel);

        auto loginBox = new QGroupBox("Login Options");
        auto loginBoxLayout = new QFormLayout();
        loginBox->setLayout(loginBoxLayout);

        profileLayout->addWidget(loginBox, 2, 1);

        encryptArgumentsBox = new QCheckBox();
        connect(encryptArgumentsBox, &QCheckBox::stateChanged, [=](int) {
            getCurrentProfile().encryptArguments =
                encryptArgumentsBox->isChecked();

            this->core.saveSettings();
        });
        loginBoxLayout->addRow("Encrypt Game Arguments", encryptArgumentsBox);

        serverType = new QComboBox();
        serverType->insertItem(0, "Square Enix");
        serverType->insertItem(1, "Sapphire");

        connect(serverType,
                static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                [=](int index) {
                    getCurrentProfile().isSapphire = index == 1;

                    reloadControls();
                    this->core.saveSettings();
                });

        loginBoxLayout->addRow("Server Lobby", serverType);

        gameLicenseBox = new QComboBox();
        gameLicenseBox->insertItem(0, "Windows (Standalone)");
        gameLicenseBox->insertItem(1, "Windows (Steam)");
        gameLicenseBox->insertItem(2, "macOS");

        connect(gameLicenseBox,
                static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                [=](int index) {
                    getCurrentProfile().license = (GameLicense)index;

                    this->core.saveSettings();
                });

        loginBoxLayout->addRow("Game License", gameLicenseBox);

        lobbyServerURL = new QLineEdit();
        connect(lobbyServerURL, &QLineEdit::editingFinished, [=] {
            getCurrentProfile().lobbyURL = lobbyServerURL->text();
            this->core.saveSettings();
        });
        loginBoxLayout->addRow("Lobby URL", lobbyServerURL);

        rememberUsernameBox = new QCheckBox();
        connect(rememberUsernameBox, &QCheckBox::stateChanged, [=](int) {
            getCurrentProfile().rememberUsername =
                rememberUsernameBox->isChecked();

            this->core.saveSettings();
        });
        loginBoxLayout->addRow("Remember Username", rememberUsernameBox);

        rememberPasswordBox = new QCheckBox();
        connect(rememberPasswordBox, &QCheckBox::stateChanged, [=](int) {
            getCurrentProfile().rememberPassword =
                rememberPasswordBox->isChecked();

            this->core.saveSettings();
        });
        loginBoxLayout->addRow("Remember Password", rememberPasswordBox);

        useOneTimePassword = new QCheckBox();
        connect(useOneTimePassword, &QCheckBox::stateChanged, [=](int) {
            getCurrentProfile().useOneTimePassword =
                useOneTimePassword->isChecked();

            this->core.saveSettings();
            this->window.reloadControls();
        });
        loginBoxLayout->addRow("Use One-time Password", useOneTimePassword);

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
        auto wineBox = new QGroupBox("Wine Options");
        auto wineBoxLayout = new QFormLayout();
        wineBox->setLayout(wineBoxLayout);

        profileLayout->addWidget(wineBox, 1, 2, 1, 1);

        winePathLabel = new QLineEdit();
        winePathLabel->setReadOnly(true);
        wineBoxLayout->addRow("Wine Executable", winePathLabel);

        wineTypeCombo = new QComboBox();

#if defined(Q_OS_MAC)
        wineTypeCombo->insertItem(2, "FFXIV for Mac");
        wineTypeCombo->insertItem(3, "XIV on Mac");
#endif

        wineTypeCombo->insertItem(0, "System Wine");

        // custom wine selection is broken under flatpak
#ifndef FLATPAK
        wineTypeCombo->insertItem(1, "Custom Wine");
#endif

        wineBoxLayout->addWidget(wineTypeCombo);

        selectWineButton = new QPushButton("Select Wine Executable");

#ifndef FLATPAK
        wineBoxLayout->addWidget(selectWineButton);
#endif

        connect(wineTypeCombo,
                static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                [this](int index) {
                    getCurrentProfile().wineType = (WineType)index;

                    this->core.readWineInfo(getCurrentProfile());
                    this->core.saveSettings();
                    this->reloadControls();
                });

        connect(selectWineButton, &QPushButton::pressed, [this] {
            getCurrentProfile().winePath =
                QFileDialog::getOpenFileName(this, "Open Wine Executable");

            this->core.saveSettings();
            this->reloadControls();
        });

        // wine version is reported incorrectly under flatpak too
        wineVersionLabel = new QLabel();
#ifndef FLATPAK
        wineVersionLabel->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        wineBoxLayout->addRow("Wine Version", wineVersionLabel);
#endif

        winePrefixDirectory = new QLineEdit();
        winePrefixDirectory->setReadOnly(true);
        wineBoxLayout->addRow("Wine Prefix", winePrefixDirectory);

        auto winePrefixButtonLayout = new QHBoxLayout();
        auto winePrefixButtonContainer = new QWidget();
        winePrefixButtonContainer->setLayout(winePrefixButtonLayout);
        wineBoxLayout->addWidget(winePrefixButtonContainer);

        auto selectPrefixButton = new QPushButton("Select Wine Prefix");
        connect(selectPrefixButton, &QPushButton::pressed, [this] {
            getCurrentProfile().winePrefixPath =
                QFileDialog::getExistingDirectory(this, "Open Wine Prefix");

            this->core.saveSettings();
            this->reloadControls();
        });
        winePrefixButtonLayout->addWidget(selectPrefixButton);

        auto openPrefixButton = new QPushButton("Open Wine Prefix");
        connect(openPrefixButton, &QPushButton::pressed,
                [&window, this] { window.openPath(getCurrentProfile().winePrefixPath); });
        winePrefixButtonLayout->addWidget(openPrefixButton);

        auto enableDXVKhud = new QCheckBox("Enable DXVK HUD");
        wineBoxLayout->addRow("Wine Tweaks", enableDXVKhud);

        connect(enableDXVKhud, &QCheckBox::stateChanged, [this](int state) {
            getCurrentProfile().enableDXVKhud = state;
            this->core.settings.setValue("enableDXVKhud",
                                         static_cast<bool>(state));
        });
#endif

#if defined(Q_OS_LINUX)
        useEsync = new QCheckBox(
            "Use Better Sync Primitives (Esync, Fsync, and Futex2)");
        wineBoxLayout->addWidget(useEsync);

        useEsync->setToolTip("This may improve game performance, but requires a Wine and kernel with the patches included.");

        connect(useEsync, &QCheckBox::stateChanged, [this](int state) {
            getCurrentProfile().useEsync = state;

            this->core.saveSettings();
        });

        useGamescope = new QCheckBox("Use Gamescope");
        wineBoxLayout->addWidget(useGamescope);

        useGamescope->setToolTip("Use the micro-compositor compositor that uses Wayland and XWayland to create a nested session.\nIf you primarily use fullscreen mode, this may improve input handling especially on Wayland.");

        auto gamescopeButtonLayout = new QHBoxLayout();
        auto gamescopeButtonContainer = new QWidget();
        gamescopeButtonContainer->setLayout(gamescopeButtonLayout);
        wineBoxLayout->addWidget(gamescopeButtonContainer);

        configureGamescopeButton = new QPushButton("Configure...");
        connect(configureGamescopeButton, &QPushButton::pressed, [&] {
            auto gamescopeSettingsWindow = new GamescopeSettingsWindow(
                getCurrentProfile(), this->core, this);
            gamescopeSettingsWindow->show();
        });
        gamescopeButtonLayout->addWidget(configureGamescopeButton);

        connect(useGamescope, &QCheckBox::stateChanged, [this](int state) {
            getCurrentProfile().useGamescope = state;

            this->core.saveSettings();
            this->reloadControls();
        });

        useGamemode = new QCheckBox("Use GameMode");
        wineBoxLayout->addWidget(useGamemode);

        useGamemode->setToolTip("A special game performance enhancer, which automatically tunes your CPU scheduler among other things. This may improve game performance.");

        connect(useGamemode, &QCheckBox::stateChanged, [this](int state) {
            getCurrentProfile().useGamemode = state;

            this->core.saveSettings();
        });
#endif

        auto dalamudBox = new QGroupBox("Dalamud Options");
        auto dalamudBoxLayout = new QFormLayout();
        dalamudBox->setLayout(dalamudBoxLayout);

        profileLayout->addWidget(dalamudBox, 2, 2, 1, 1);

        enableDalamudBox = new QCheckBox();
        connect(enableDalamudBox, &QCheckBox::stateChanged, [=](int) {
            getCurrentProfile().dalamud.enabled = enableDalamudBox->isChecked();

            this->core.saveSettings();
        });
        dalamudBoxLayout->addRow("Enable Dalamud Plugins", enableDalamudBox);

        dalamudOptOutBox = new QCheckBox();
        connect(dalamudOptOutBox, &QCheckBox::stateChanged, [=](int) {
            getCurrentProfile().dalamud.optOutOfMbCollection = dalamudOptOutBox->isChecked();

            this->core.saveSettings();
        });
        dalamudBoxLayout->addRow("Opt Out of Automatic Marketboard Collection", dalamudOptOutBox);

        dalamudChannel = new QComboBox();
        dalamudChannel->insertItem(0, "Stable");
        dalamudChannel->insertItem(1, "Staging");
        dalamudChannel->insertItem(2, ".NET 5");

        connect(dalamudChannel,
                static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                [=](int index) {
                    getCurrentProfile().dalamud.channel = (DalamudChannel)index;

                    this->core.saveSettings();
                });

        dalamudBoxLayout->addRow("Dalamud Update Channel", dalamudChannel);

        dalamudVersionLabel = new QLabel();
        dalamudVersionLabel->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        dalamudBoxLayout->addRow("Dalamud Version", dalamudVersionLabel);

        dalamudAssetVersionLabel = new QLabel();
        dalamudAssetVersionLabel->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        dalamudBoxLayout->addRow("Dalamud Asset Version", dalamudAssetVersionLabel);

        nativeLauncherVersionLabel = new QLabel();
        nativeLauncherVersionLabel->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        dalamudBoxLayout->addRow("Native Launcher Version", nativeLauncherVersionLabel);
    }

    tabWidget->setCurrentIndex(defaultTab);

    reloadControls();
}

void SettingsWindow::reloadControls() {
    if(currentlyReloadingControls)
        return;

    currentlyReloadingControls = true;

    auto oldRow = profileWidget->currentRow();

    profileWidget->clear();

    for(const auto& profile : core.profileList()) {
        profileWidget->addItem(profile);
    }
    profileWidget->setCurrentRow(oldRow);

    closeWhenLaunched->setChecked(core.appSettings.closeWhenLaunched);
    showBanner->setChecked(core.appSettings.showBanners);
    showNewsList->setChecked(core.appSettings.showNewsList);

    // deleting the main profile is unsupported behavior
    deleteProfileButton->setEnabled(profileWidget->currentRow() != 0);

    ProfileSettings& profile = core.getProfile(profileWidget->currentRow());
    nameEdit->setText(profile.name);

    // game
    directXCombo->setCurrentIndex(profile.useDX9 ? 1 : 0);
    currentGameDirectory->setText(profile.gamePath);

    if(!profile.isGameInstalled()) {
        expansionVersionLabel->setText("No game installed.");
    } else {
        QString expacString;

        expacString += "Boot";
        expacString += QString(" (%1)\n").arg(profile.bootVersion);

        if(profile.installedMaxExpansion >= 0) {
            expacString += "A Realm Reborn";
            expacString += QString(" (%1)\n").arg(profile.gameVersion);
        }

        if(profile.installedMaxExpansion >= 1) {
            expacString += "Heavensward";
            expacString += QString(" (%1)\n").arg(profile.expansionVersions[0]);
        }

        if(profile.installedMaxExpansion >= 2) {
            expacString += "Stormblood";
            expacString += QString(" (%1)\n").arg(profile.expansionVersions[1]);
        }

        if(profile.installedMaxExpansion >= 3) {
            expacString += "Shadowbringers";
            expacString += QString(" (%1)\n").arg(profile.expansionVersions[2]);
        }

        if(profile.installedMaxExpansion >= 4) {
            expacString += "Endwalker";
            expacString += QString(" (%1)\n").arg(profile.expansionVersions[3]);
        }

        expansionVersionLabel->setText(expacString);
    }

    // wine
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    if(!profile.isWineInstalled()) {
        wineVersionLabel->setText("Wine is not installed.");
    } else {
        wineVersionLabel->setText(profile.wineVersion);
    }

    wineTypeCombo->setCurrentIndex((int)profile.wineType);
    selectWineButton->setEnabled(profile.wineType == WineType::Custom);
    winePathLabel->setText(profile.winePath);
    winePrefixDirectory->setText(profile.winePrefixPath);
#endif

#if defined(Q_OS_LINUX)
    useEsync->setChecked(profile.useEsync);
    useGamescope->setChecked(profile.useGamescope);
    useGamemode->setChecked(profile.useGamemode);

    useGamemode->setEnabled(core.gamemodeAvailable);
    useGamescope->setEnabled(core.gamescopeAvailable);

    configureGamescopeButton->setEnabled(profile.useGamescope);
#endif

#ifdef ENABLE_WATCHDOG
    enableWatchdog->setChecked(profile.enableWatchdog);
#endif

    // login
    encryptArgumentsBox->setChecked(profile.encryptArguments);
    serverType->setCurrentIndex(profile.isSapphire ? 1 : 0);
    lobbyServerURL->setEnabled(profile.isSapphire);
    if(profile.isSapphire) {
        lobbyServerURL->setText(profile.lobbyURL);
        lobbyServerURL->setPlaceholderText("Required...");
    } else {
        lobbyServerURL->setText("neolobby0X.ffxiv.com");
    }
    rememberUsernameBox->setChecked(profile.rememberUsername);
    rememberPasswordBox->setChecked(profile.rememberPassword);
    useOneTimePassword->setChecked(profile.useOneTimePassword);
    useOneTimePassword->setEnabled(!profile.isSapphire);
    if(!useOneTimePassword->isEnabled()) {
        useOneTimePassword->setToolTip("OTP is not supported by Sapphire servers.");
    } else {
        useOneTimePassword->setToolTip("");
    }

    gameLicenseBox->setCurrentIndex((int)profile.license);
    gameLicenseBox->setEnabled(!profile.isSapphire);
    if(!gameLicenseBox->isEnabled()) {
        gameLicenseBox->setToolTip("Game licenses only matter when logging into the official Square Enix servers.");
    } else {
        gameLicenseBox->setToolTip("");
    }

    // dalamud
    enableDalamudBox->setChecked(profile.dalamud.enabled);
    if(core.dalamudVersion.isEmpty()) {
        dalamudVersionLabel->setText("Dalamud is not installed.");
    } else {
        dalamudVersionLabel->setText(core.dalamudVersion);
    }

    if(core.dalamudAssetVersion == -1) {
        dalamudAssetVersionLabel->setText("Dalamud assets are not installed.");
    } else {
        dalamudAssetVersionLabel->setText(QString::number(core.dalamudAssetVersion));
    }

    if(core.nativeLauncherVersion.isEmpty()) {
        nativeLauncherVersionLabel->setText("Native launcher is not installed.");
    } else {
        nativeLauncherVersionLabel->setText(core.nativeLauncherVersion);
    }

    dalamudOptOutBox->setChecked(profile.dalamud.optOutOfMbCollection);
    dalamudChannel->setCurrentIndex((int)profile.dalamud.channel);

    window.reloadControls();

    currentlyReloadingControls = false;
}

ProfileSettings& SettingsWindow::getCurrentProfile() {
    return this->core.getProfile(profileWidget->currentRow());
}