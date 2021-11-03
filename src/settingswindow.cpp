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
#include <QComboBox>

#include "xivlauncher.h"

SettingsWindow::SettingsWindow(LauncherWindow& window, QWidget* parent) : window(window), QWidget(parent) {
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto layout = new QFormLayout(this);
    setLayout(layout);

    auto directXCombo = new QComboBox();
    directXCombo->setCurrentIndex(window.settings.value("directx", 0).toInt());
    directXCombo->addItem("DirectX 11");
    directXCombo->addItem("DirectX 9");
    layout->addRow("DirectX Version", directXCombo);

    connect(directXCombo, &QComboBox::currentIndexChanged, [=](int index) {
        this->window.settings.setValue("directx", directXCombo->currentIndex());
        this->window.useDX9 = directXCombo->currentIndex() == 1;
    });

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    auto wineBox = new QGroupBox("Wine Options");
    auto wineBoxLayout = new QFormLayout();
    wineBox->setLayout(wineBoxLayout);

    layout->addRow(wineBox);

    auto infoLabel = new QLabel("This is a list of possible enhancements you can make to your Wine gaming experience.\n"
                                "This is all stuff you can do outside of the launcher, but we can take care of it for you.");
    infoLabel->setWordWrap(true);
    wineBoxLayout->addWidget(infoLabel);

    auto winePathLabel = new QLabel(window.winePath);
    winePathLabel->setWordWrap(true);
    wineBoxLayout->addRow("Wine Executable", winePathLabel);

    auto wineVersionCombo = new QComboBox();

#if defined(Q_OS_MAC)
    wineVersionCombo->insertItem(2, "FFXIV Built-In");
#endif

    wineVersionCombo->insertItem(0, "System Wine");
    wineVersionCombo->insertItem(1, "Custom Path...");
    wineVersionCombo->setCurrentIndex(window.settings.value("wineVersion", 0).toInt());
    wineBoxLayout->addWidget(wineVersionCombo);

    auto selectWineButton = new QPushButton("Select Wine Executable");
    selectWineButton->setEnabled(window.settings.value("wineVersion", 0).toInt() == 2);
    wineBoxLayout->addWidget(selectWineButton);

    connect(wineVersionCombo, &QComboBox::currentIndexChanged, [this, selectWineButton, winePathLabel](int index) {
        this->window.settings.setValue("wineVersion", index);
        selectWineButton->setEnabled(index == 1);

        this->window.readInitialInformation();
        winePathLabel->setText(this->window.winePath);
    });

    connect(selectWineButton, &QPushButton::pressed, [this, winePathLabel] {
        this->window.winePath = QFileDialog::getOpenFileName(this, "Open Wine Executable");
        this->window.settings.setValue("winePath", this->window.winePath);

        this->window.readInitialInformation();
        winePathLabel->setText(this->window.winePath);
    });

    auto winePrefixDirectory = new QLabel(window.winePrefixPath);
    winePrefixDirectory->setWordWrap(true);
    wineBoxLayout->addRow("Wine Prefix", winePrefixDirectory);

    auto selectPrefixButton = new QPushButton("Select Wine Prefix");
    connect(selectPrefixButton, &QPushButton::pressed, [this, winePrefixDirectory] {
        this->window.winePrefixPath = QFileDialog::getExistingDirectory(this, "Open Wine Prefix");
        winePrefixDirectory->setText(this->window.winePrefixPath);

        this->window.readInitialInformation();
    });
    wineBoxLayout->addWidget(selectPrefixButton);

    auto openPrefixButton = new QPushButton("Open Wine Prefix");
    connect(openPrefixButton, &QPushButton::pressed, [this] {
        openPath(this->window.winePrefixPath);
    });
    wineBoxLayout->addWidget(openPrefixButton);

    auto enableDXVKhud = new QCheckBox("Enable DXVK HUD");
    enableDXVKhud->setChecked(window.enableDXVKhud);
    wineBoxLayout->addWidget(enableDXVKhud);

    connect(enableDXVKhud, &QCheckBox::stateChanged, [this](int state) {
        this->window.enableDXVKhud = state;
        this->window.settings.setValue("enableDXVKhud", static_cast<bool>(state));
    });
#endif

#if defined(Q_OS_LINUX)
    auto useEsync = new QCheckBox("Use Esync");
    useEsync->setChecked(window.useEsync);
    wineBoxLayout->addWidget(useEsync);

    auto esyncLabel = new QLabel("Improves general game performance, but requires a Wine built with the Esync patches.\n"
                                 "If you use the latest Wine staging, it should work.");
    esyncLabel->setWordWrap(true);
    wineBoxLayout->addWidget(esyncLabel);

    connect(useEsync, &QCheckBox::stateChanged, [this](int state) {
        this->window.useEsync = state;
        this->window.settings.setValue("useEsync", static_cast<bool>(state));
    });

    auto useGamescope = new QCheckBox("Use Gamescope");
    useGamescope->setChecked(window.useGamescope);
    wineBoxLayout->addWidget(useGamescope);

    auto gamescopeLabel = new QLabel("Use the SteamOS compositor that uses Wayland.\n"
                                 "If you are experiencing input issues on XWayland, try this option if you have it installed.");
    gamescopeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamescopeLabel);

    connect(useGamescope, &QCheckBox::stateChanged, [this](int state) {
        this->window.useGamescope = state;
        this->window.settings.setValue("useGamescope", static_cast<bool>(state));
    });

    auto useGamemode = new QCheckBox("Use Gamemode");
    useGamemode->setChecked(window.useGamemode);
    wineBoxLayout->addWidget(useGamemode);

    auto gamemodeLabel = new QLabel("Use Feral Interactive's GameMode, which applies a couple of performance enhancements.\n"
                                     "May give a slight performance boost, but requires GameMode to be installed.\n");
    gamemodeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamemodeLabel);

    connect(useGamemode, &QCheckBox::stateChanged, [this](int state) {
        this->window.useGamemode = state;
        this->window.settings.setValue("useGamemode", static_cast<bool>(state));
    });
#endif

    auto currentGameDirectory = new QLabel(window.gamePath);
    currentGameDirectory->setWordWrap(true);
    layout->addRow("Game Directory", currentGameDirectory);

    auto selectDirectoryButton = new QPushButton("Select Game Directory");
    connect(selectDirectoryButton, &QPushButton::pressed, [this, currentGameDirectory] {
        this->window.gamePath = QFileDialog::getExistingDirectory(this, "Open Game Directory");
        currentGameDirectory->setText(this->window.gamePath);

        this->window.readInitialInformation();
    });
    layout->addWidget(selectDirectoryButton);

    auto gameDirectoryButton = new QPushButton("Open Game Directory");
    connect(gameDirectoryButton, &QPushButton::pressed, [this] {
        openPath(this->window.gamePath);
    });
    layout->addWidget(gameDirectoryButton);
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