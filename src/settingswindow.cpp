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

    auto infoLabel = new QLabel("This is a list of possible enhancements you can make to your Wine gaming experience.\n"
                                "This is all stuff you can do outside of the launcher, but we can take care of it for you.");
    infoLabel->setWordWrap(true);

#if defined(Q_OS_MAC)
    auto wineBox = new QGroupBox("Wine Options");
    auto wineBoxLayout = new QFormLayout();
    wineBox->setLayout(wineBoxLayout);

    wineBoxLayout->addWidget(infoLabel);

    auto useSystemWine = new QCheckBox("Use System Wine");
    useSystemWine->setChecked(window.settings.value("useSystemWine", false).toBool());
    wineBoxLayout->addWidget(useSystemWine);

    connect(useSystemWine, &QCheckBox::stateChanged, [this](int state) {
        this->window.useSystemWine = state;
        this->window.settings.setValue("useSystemWine", static_cast<bool>(state));
    });

    auto systemWineLabel = new QLabel("Use the system wine instead of the one packaged with the macOS version of FFXIV.\n"
                                     "You can easily install wine through homebrew, but please note that the game will not run out of the box\n"
                                     "on DX11 without DXVK installed.");
    systemWineLabel->setWordWrap(true);
    wineBoxLayout->addWidget(systemWineLabel);

    layout->addRow(wineBox);
#endif

#if defined(Q_OS_LINUX)
    auto wineBox = new QGroupBox("Wine Options");
    auto wineBoxLayout = new QFormLayout();
    wineBox->setLayout(wineBoxLayout);

    wineBoxLayout->addWidget(infoLabel);

    auto useEsync = new QCheckBox("Use Esync");
    useEsync->setChecked(window.settings.value("useEsync", false).toBool());
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
    useGamescope->setChecked(window.settings.value("useGamescope", false).toBool());
    wineBoxLayout->addWidget( useGamescope);

    auto gamescopeLabel = new QLabel("Use the SteamOS compositor that uses Wayland.\n"
                                 "If you are experiencing input issues on XWayland, try this option if you have it installed.");
    gamescopeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamescopeLabel);

    connect(useGamescope, &QCheckBox::stateChanged, [this](int state) {
        this->window.useGamescope = state;
        this->window.settings.setValue("useGamescope", static_cast<bool>(state));
    });

    auto useGamemode = new QCheckBox("Use Gamemode");
    useGamemode->setChecked(window.settings.value("useGamemode", false).toBool());
    wineBoxLayout->addWidget(useGamemode);

    auto gamemodeLabel = new QLabel("Use Feral Interactive's GameMode, which applies a couple of performance enhancements.\n"
                                     "May give a slight performance boost, but requires GameMode to be installed.\n");
    gamemodeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamemodeLabel);

    connect(useGamemode, &QCheckBox::stateChanged, [this](int state) {
        this->window.useGamemode = state;
        this->window.settings.setValue("useGamemode", static_cast<bool>(state));
    });

    layout->addRow(wineBox);
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
#if defined(Q_OS_WIN)
        // for some reason, windows requires special treatment (what else is new?)
        const QFileInfo fileInfo(this->window.gamePath);

        QProcess::startDetached("explorer.exe", QStringList(QDir::toNativeSeparators(fileInfo.canonicalFilePath())));
#else
        QDesktopServices::openUrl("file://" + this->window.gamePath);
#endif
    });
    layout->addWidget(gameDirectoryButton);
}