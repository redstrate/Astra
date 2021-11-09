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

#include "xivlauncher.h"

SettingsWindow::SettingsWindow(LauncherWindow& window, QWidget* parent) : window(window), QWidget(parent) {
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
        profileWidget->setCurrentRow(this->window.addProfile());

        this->window.saveSettings();
    });
    mainLayout->addWidget(addProfileButton, 2, 0);

    deleteProfileButton = new QPushButton("Delete Profile");
    connect(deleteProfileButton, &QPushButton::pressed, [=] {
        profileWidget->setCurrentRow(this->window.deleteProfile(getCurrentProfile().name));

        this->window.saveSettings();
    });
    mainLayout->addWidget(deleteProfileButton, 3, 0);

    nameEdit = new QLineEdit();
    connect(nameEdit, &QLineEdit::editingFinished, [=] {
        getCurrentProfile().name = nameEdit->text();

        reloadControls();
        this->window.saveSettings();
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

    connect(directXCombo, &QComboBox::currentIndexChanged, [=](int index) {
        getCurrentProfile().useDX9 = directXCombo->currentIndex() == 1;
        this->window.saveSettings();
    });

    currentGameDirectory = new QLabel(window.currentProfile().gamePath);
    currentGameDirectory->setWordWrap(true);
    gameBoxLayout->addRow("Game Directory", currentGameDirectory);

    auto selectDirectoryButton = new QPushButton("Select Game Directory");
    connect(selectDirectoryButton, &QPushButton::pressed, [this] {
        getCurrentProfile().gamePath = QFileDialog::getExistingDirectory(this, "Open Game Directory");

        this->reloadControls();
        this->window.saveSettings();

        this->window.readGameVersion();
    });
    gameBoxLayout->addWidget(selectDirectoryButton);

    auto gameDirectoryButton = new QPushButton("Open Game Directory");
    connect(gameDirectoryButton, &QPushButton::pressed, [this] {
        openPath(getCurrentProfile().gamePath);
    });
    gameBoxLayout->addWidget(gameDirectoryButton);

    auto loginBox = new QGroupBox("Login Options");
    auto loginBoxLayout = new QFormLayout();
    loginBox->setLayout(loginBoxLayout);

    mainLayout->addWidget(loginBox, 2, 1);

    serverType = new QComboBox();
    serverType->insertItem(0, "Square Enix");
    serverType->insertItem(1, "Sapphire");

    connect(serverType, &QComboBox::currentIndexChanged, [=](int index) {
        getCurrentProfile().isSapphire = index == 1;

        reloadControls();
        this->window.reloadControls();
        this->window.saveSettings();
    });

    loginBoxLayout->addRow("Server Lobby", serverType);

    lobbyServerURL = new QLineEdit();
    connect(lobbyServerURL, &QLineEdit::editingFinished, [=] {
        getCurrentProfile().lobbyURL = lobbyServerURL->text();
        this->window.saveSettings();
    });
    loginBoxLayout->addRow("Lobby URL", lobbyServerURL);

    rememberUsernameBox = new QCheckBox();
    connect(rememberUsernameBox, &QCheckBox::stateChanged, [=](int) {
        getCurrentProfile().rememberUsername = rememberUsernameBox->isChecked();

        this->window.reloadControls();
        this->window.saveSettings();
    });
    loginBoxLayout->addRow("Remember Username?", rememberUsernameBox);

    rememberPasswordBox = new QCheckBox();
    connect(rememberPasswordBox, &QCheckBox::stateChanged, [=](int) {
        getCurrentProfile().rememberPassword = rememberPasswordBox->isChecked();

        this->window.reloadControls();
        this->window.saveSettings();
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

    winePathLabel = new QLabel(window.currentProfile().winePath);
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

    connect(wineVersionCombo, &QComboBox::currentIndexChanged, [this](int index) {
        getCurrentProfile().wineVersion = index;

        this->window.saveSettings();
        this->reloadControls();

        // TODO: figure out the purpose of calling this before 1.0
        // this->window.readInitialInformation();
    });

    connect(selectWineButton, &QPushButton::pressed, [this] {
        getCurrentProfile().winePath = QFileDialog::getOpenFileName(this, "Open Wine Executable");

        this->window.saveSettings();
        this->reloadControls();

        // TODO: figure out the purpose of calling this before 2.0
        //this->window.readInitialInformation();
    });

    winePrefixDirectory = new QLabel(window.currentProfile().winePrefixPath);
    winePrefixDirectory->setWordWrap(true);
    wineBoxLayout->addRow("Wine Prefix", winePrefixDirectory);

    auto selectPrefixButton = new QPushButton("Select Wine Prefix");
    connect(selectPrefixButton, &QPushButton::pressed, [this] {
        getCurrentProfile().winePrefixPath = QFileDialog::getExistingDirectory(this, "Open Wine Prefix");

        this->window.saveSettings();
        this->reloadControls();

        // TODO: figure out the purpose of calling this before 3.0
        //this->window.readInitialInformation();
    });
    wineBoxLayout->addWidget(selectPrefixButton);

    auto openPrefixButton = new QPushButton("Open Wine Prefix");
    connect(openPrefixButton, &QPushButton::pressed, [this] {
        openPath(getCurrentProfile().winePrefixPath);
    });
    wineBoxLayout->addWidget(openPrefixButton);

    auto enableDXVKhud = new QCheckBox("Enable DXVK HUD");
    enableDXVKhud->setChecked(window.currentProfile().enableDXVKhud);
    wineBoxLayout->addWidget(enableDXVKhud);

    connect(enableDXVKhud, &QCheckBox::stateChanged, [this](int state) {
        this->window.currentProfile().enableDXVKhud = state;
        this->window.settings.setValue("enableDXVKhud", static_cast<bool>(state));
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

        this->window.saveSettings();
    });

    useGamescope = new QCheckBox("Use Gamescope");
    wineBoxLayout->addWidget(useGamescope);

    auto gamescopeLabel = new QLabel("Use the SteamOS compositor that uses Wayland.\n"
                                 "If you are experiencing input issues on XWayland, try this option if you have it installed.");
    gamescopeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamescopeLabel);

    connect(useGamescope, &QCheckBox::stateChanged, [this](int state) {
        getCurrentProfile().useGamescope = state;

        this->window.saveSettings();
    });

    useGamemode = new QCheckBox("Use Gamemode");
    wineBoxLayout->addWidget(useGamemode);

    auto gamemodeLabel = new QLabel("Use Feral Interactive's GameMode, which applies a couple of performance enhancements.\n"
                                     "May give a slight performance boost, but requires GameMode to be installed.\n");
    gamemodeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamemodeLabel);

    connect(useGamemode, &QCheckBox::stateChanged, [this](int state) {
        getCurrentProfile().useGamemode = state;

        this->window.saveSettings();
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

    for(auto profile : window.profileList()) {
        profileWidget->addItem(profile);
    }
    profileWidget->setCurrentRow(oldRow);

    // deleting the main profile is unsupported behavior
    deleteProfileButton->setEnabled(window.profileList().size() > 1);

    ProfileSettings& profile = window.getProfile(profileWidget->currentRow());
    nameEdit->setText(profile.name);

    // game
    directXCombo->setCurrentIndex(profile.useDX9 ? 1 : 0);
    currentGameDirectory->setText(profile.gamePath);

    // wine
    wineVersionCombo->setCurrentIndex(profile.wineVersion);
    selectWineButton->setEnabled(profile.wineVersion == 1);
    winePathLabel->setText(profile.winePath);
    winePrefixDirectory->setText(profile.winePrefixPath);

    useEsync->setChecked(profile.useEsync);
    useGamescope->setChecked(profile.useGamescope);
    useGamemode->setChecked(profile.useGamemode);

    // login
    serverType->setCurrentIndex(profile.isSapphire ? 1 : 0);
    lobbyServerURL->setEnabled(profile.isSapphire);
    lobbyServerURL->setText(profile.lobbyURL);
    rememberUsernameBox->setChecked(profile.rememberUsername);
    rememberPasswordBox->setChecked(profile.rememberPassword);

    window.reloadControls();

    currentlyReloadingControls = false;
}

ProfileSettings& SettingsWindow::getCurrentProfile() {
    return this->window.getProfile(profileWidget->currentRow());
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