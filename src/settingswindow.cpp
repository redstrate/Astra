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

#include "launchercore.h"
#include "launcherwindow.h"

SettingsWindow::SettingsWindow(LauncherWindow& window, LauncherCore& core, QWidget* parent) : core(core), window(window), QWidget(parent) {
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QGridLayout(this);
    setLayout(mainLayout);

    profileWidget = new QListWidget();
    profileWidget->addItem("INVALID *DEBUG*");
    profileWidget->setCurrentRow(0);

    connect(profileWidget, &QListWidget::currentRowChanged, this, &SettingsWindow::reloadControls);

    mainLayout->addWidget(profileWidget, 0, 0, 0, 1);

    auto addProfileButton = new QPushButton("Add Profile");
    connect(addProfileButton, &QPushButton::pressed, [=] {
        profileWidget->setCurrentRow(this->core.addProfile());

        this->core.saveSettings();
    });
    mainLayout->addWidget(addProfileButton, 2, 0);

    deleteProfileButton = new QPushButton("Delete Profile");
    connect(deleteProfileButton, &QPushButton::pressed, [=] {
        profileWidget->setCurrentRow(this->core.deleteProfile(getCurrentProfile().name));

        this->core.saveSettings();
    });
    mainLayout->addWidget(deleteProfileButton, 3, 0);

    nameEdit = new QLineEdit();
    connect(nameEdit, &QLineEdit::editingFinished, [=] {
        getCurrentProfile().name = nameEdit->text();

        reloadControls();
        this->core.saveSettings();
    });
    mainLayout->addWidget(nameEdit, 0, 1);

    auto gameBox = new QGroupBox("Game Options");
    auto gameBoxLayout = new QFormLayout();
    gameBox->setLayout(gameBoxLayout);

    mainLayout->addWidget(gameBox, 1, 1);

    directXCombo = new QComboBox();
    directXCombo->addItem("DirectX 11");
    directXCombo->addItem("DirectX 9");
    gameBoxLayout->addRow("DirectX Version", directXCombo);

    connect(directXCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        getCurrentProfile().useDX9 = directXCombo->currentIndex() == 1;
        this->core.saveSettings();
    });

    currentGameDirectory = new QLabel();
    currentGameDirectory->setWordWrap(true);
    gameBoxLayout->addRow("Game Directory", currentGameDirectory);

    auto selectDirectoryButton = new QPushButton("Select Game Directory");
    connect(selectDirectoryButton, &QPushButton::pressed, [this] {
        getCurrentProfile().gamePath = QFileDialog::getExistingDirectory(this, "Open Game Directory");

        this->reloadControls();
        this->core.saveSettings();

        this->core.readGameVersion();
    });
    gameBoxLayout->addWidget(selectDirectoryButton);

    auto gameDirectoryButton = new QPushButton("Open Game Directory");
    connect(gameDirectoryButton, &QPushButton::pressed, [this] {
        openPath(getCurrentProfile().gamePath);
    });
    gameBoxLayout->addWidget(gameDirectoryButton);

    expansionVersionLabel = new QLabel();
    gameBoxLayout->addWidget(expansionVersionLabel);

    auto loginBox = new QGroupBox("Login Options");
    auto loginBoxLayout = new QFormLayout();
    loginBox->setLayout(loginBoxLayout);

    mainLayout->addWidget(loginBox, 2, 1);

    encryptArgumentsBox = new QCheckBox();
    connect(encryptArgumentsBox, &QCheckBox::stateChanged, [=](int) {
        getCurrentProfile().encryptArguments = encryptArgumentsBox->isChecked();

        this->core.saveSettings();
    });
    loginBoxLayout->addRow("Encrypt Game Arguments", encryptArgumentsBox);

    enableDalamudBox = new QCheckBox();
    connect(enableDalamudBox, &QCheckBox::stateChanged, [=](int) {
        getCurrentProfile().enableDalamud = enableDalamudBox->isChecked();

        this->core.saveSettings();
    });
    loginBoxLayout->addRow("Enable Dalamud Injection", enableDalamudBox);

    serverType = new QComboBox();
    serverType->insertItem(0, "Square Enix");
    serverType->insertItem(1, "Sapphire");

    connect(serverType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        getCurrentProfile().isSapphire = index == 1;

        reloadControls();
        this->core.saveSettings();
    });

    loginBoxLayout->addRow("Server Lobby", serverType);

    lobbyServerURL = new QLineEdit();
    connect(lobbyServerURL, &QLineEdit::editingFinished, [=] {
        getCurrentProfile().lobbyURL = lobbyServerURL->text();
        this->core.saveSettings();
    });
    loginBoxLayout->addRow("Lobby URL", lobbyServerURL);

    rememberUsernameBox = new QCheckBox();
    connect(rememberUsernameBox, &QCheckBox::stateChanged, [=](int) {
        getCurrentProfile().rememberUsername = rememberUsernameBox->isChecked();

        this->core.saveSettings();
    });
    loginBoxLayout->addRow("Remember Username?", rememberUsernameBox);

    rememberPasswordBox = new QCheckBox();
    connect(rememberPasswordBox, &QCheckBox::stateChanged, [=](int) {
        getCurrentProfile().rememberPassword = rememberPasswordBox->isChecked();

        this->core.saveSettings();
    });
    loginBoxLayout->addRow("Remember Password?", rememberPasswordBox);

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    auto wineBox = new QGroupBox("Wine Options");
    auto wineBoxLayout = new QFormLayout();
    wineBox->setLayout(wineBoxLayout);

    mainLayout->addWidget(wineBox, 1, 2, 2, 2);

    auto infoLabel = new QLabel("This is a list of possible enhancements you can make to your Wine gaming experience.\n"
                                "This is all stuff you can do outside of the launcher, but we can take care of it for you.");
    infoLabel->setWordWrap(true);
    wineBoxLayout->addWidget(infoLabel);

    winePathLabel = new QLabel();
    winePathLabel->setWordWrap(true);
    wineBoxLayout->addRow("Wine Executable", winePathLabel);

    wineVersionCombo = new QComboBox();

#if defined(Q_OS_MAC)
    wineVersionCombo->insertItem(2, "FFXIV Built-In");
#endif

    wineVersionCombo->insertItem(0, "System Wine");
    wineVersionCombo->insertItem(1, "Custom Path...");

    wineBoxLayout->addWidget(wineVersionCombo);

    selectWineButton = new QPushButton("Select Wine Executable");
    wineBoxLayout->addWidget(selectWineButton);

    connect(wineVersionCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
        getCurrentProfile().wineVersion = index;

        this->core.readWineInfo(getCurrentProfile());
        this->core.saveSettings();
        this->reloadControls();
    });

    connect(selectWineButton, &QPushButton::pressed, [this] {
        getCurrentProfile().winePath = QFileDialog::getOpenFileName(this, "Open Wine Executable");

        this->core.saveSettings();
        this->reloadControls();
    });

    winePrefixDirectory = new QLabel();
    winePrefixDirectory->setWordWrap(true);
    wineBoxLayout->addRow("Wine Prefix", winePrefixDirectory);

    auto selectPrefixButton = new QPushButton("Select Wine Prefix");
    connect(selectPrefixButton, &QPushButton::pressed, [this] {
        getCurrentProfile().winePrefixPath = QFileDialog::getExistingDirectory(this, "Open Wine Prefix");

        this->core.saveSettings();
        this->reloadControls();
    });
    wineBoxLayout->addWidget(selectPrefixButton);

    auto openPrefixButton = new QPushButton("Open Wine Prefix");
    connect(openPrefixButton, &QPushButton::pressed, [this] {
        openPath(getCurrentProfile().winePrefixPath);
    });
    wineBoxLayout->addWidget(openPrefixButton);

    auto enableDXVKhud = new QCheckBox("Enable DXVK HUD");
    wineBoxLayout->addWidget(enableDXVKhud);

    connect(enableDXVKhud, &QCheckBox::stateChanged, [this](int state) {
        getCurrentProfile().enableDXVKhud = state;
        this->core.settings.setValue("enableDXVKhud", static_cast<bool>(state));
    });
#endif

#if defined(Q_OS_LINUX)
    useEsync = new QCheckBox("Use Esync");
    wineBoxLayout->addWidget(useEsync);

    auto esyncLabel = new QLabel("Improves general game performance, but requires a Wine built with the Esync patches.\n"
                                 "If you use the latest Wine staging, it should work.");
    esyncLabel->setWordWrap(true);
    wineBoxLayout->addWidget(esyncLabel);

    connect(useEsync, &QCheckBox::stateChanged, [this](int state) {
        getCurrentProfile().useEsync = state;

        this->core.saveSettings();
    });

    useGamescope = new QCheckBox("Use Gamescope");
    wineBoxLayout->addWidget(useGamescope);

    auto gamescopeLabel = new QLabel("Use the SteamOS compositor that uses Wayland.\n"
                                 "If you are experiencing input issues on XWayland, try this option if you have it installed.");
    gamescopeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamescopeLabel);

    connect(useGamescope, &QCheckBox::stateChanged, [this](int state) {
        getCurrentProfile().useGamescope = state;

        this->core.saveSettings();
    });

    useGamemode = new QCheckBox("Use Gamemode");
    wineBoxLayout->addWidget(useGamemode);

    auto gamemodeLabel = new QLabel("Use Feral Interactive's GameMode, which applies a couple of performance enhancements.\n"
                                     "May give a slight performance boost, but requires GameMode to be installed.\n");
    gamemodeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamemodeLabel);

    connect(useGamemode, &QCheckBox::stateChanged, [this](int state) {
        getCurrentProfile().useGamemode = state;

        this->core.saveSettings();
    });
#endif

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

    // deleting the main profile is unsupported behavior
    deleteProfileButton->setEnabled(core.profileList().size() > 1);

    ProfileSettings& profile = core.getProfile(profileWidget->currentRow());
    nameEdit->setText(profile.name);

    // game
    directXCombo->setCurrentIndex(profile.useDX9 ? 1 : 0);
    currentGameDirectory->setText(profile.gamePath);

    if(profile.installedMaxExpansion == -1) {
        expansionVersionLabel->setText("No game installed.");
    } else {
        QString expacString;
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
    wineVersionCombo->setCurrentIndex(profile.wineVersion);
    selectWineButton->setEnabled(profile.wineVersion == 1);
    winePathLabel->setText(profile.winePath);
    winePrefixDirectory->setText(profile.winePrefixPath);

#if defined(Q_OS_LINUX)
    useEsync->setChecked(profile.useEsync);
    useGamescope->setChecked(profile.useGamescope);
    useGamemode->setChecked(profile.useGamemode);
#endif

    // login
    encryptArgumentsBox->setChecked(profile.encryptArguments);
    serverType->setCurrentIndex(profile.isSapphire ? 1 : 0);
    lobbyServerURL->setEnabled(profile.isSapphire);
    lobbyServerURL->setText(profile.lobbyURL);
    rememberUsernameBox->setChecked(profile.rememberUsername);
    rememberPasswordBox->setChecked(profile.rememberPassword);

    enableDalamudBox->setChecked(profile.enableDalamud);

    window.reloadControls();

    currentlyReloadingControls = false;
}

ProfileSettings& SettingsWindow::getCurrentProfile() {
    return this->core.getProfile(profileWidget->currentRow());
}

void SettingsWindow::openPath(const QString path) {
#if defined(Q_OS_WIN)
    // for some reason, windows requires special treatment (what else is new?)
        const QFileInfo fileInfo(path);

        QProcess::startDetached("explorer.exe", QStringList(QDir::toNativeSeparators(fileInfo.canonicalFilePath())));
#else
    QDesktopServices::openUrl("file://" + path);
#endif
}