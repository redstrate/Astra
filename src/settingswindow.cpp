#include "settingswindow.h"

#include <QFormLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QLabel>
#include <QFileDialog>
#include <QCheckBox>
#include <QGroupBox>

#include "xivlauncher.h"

SettingsWindow::SettingsWindow(LauncherWindow& window, QWidget* parent) : window(window), QWidget(parent) {
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto layout = new QFormLayout(this);
    setLayout(layout);

#if defined(Q_OS_LINUX)
    auto wineBox = new QGroupBox("Wine Options");
    auto wineBoxLayout = new QFormLayout();
    wineBox->setLayout(wineBoxLayout);

    auto infoLabel = new QLabel("This is a list of possible enhancements you can make to your Wine gaming experience.\n"
                                "This is all stuff you can do outside of the launcher, but we can take care of it for you.");
    wineBoxLayout->addWidget(infoLabel);

    auto useEsync = new QCheckBox("Use Esync");
    useEsync->setChecked(window.settings.value("useEsync", false).toBool());
    wineBoxLayout->addWidget(useEsync);

    auto esyncLabel = new QLabel("Improves general game performance, but requires a Wine built with the Esync patches.\n"
                                 "If you use the latest Wine staging, it should work.");
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
    wineBoxLayout->addWidget(gamemodeLabel);

    connect(useGamemode, &QCheckBox::stateChanged, [this](int state) {
        this->window.useGamemode = state;
        this->window.settings.setValue("useGamemode", static_cast<bool>(state));
    });

    layout->addRow(wineBox);
#endif

    auto currentGameDirectory = new QLabel(window.gamePath);
    layout->addRow("Game Directory", currentGameDirectory);

    auto selectDirectoryButton = new QPushButton("Select Game Directory");
    connect(selectDirectoryButton, &QPushButton::pressed, [this, currentGameDirectory] {
        this->window.gamePath = QFileDialog::getExistingDirectory(this, "Open Game Directory");
        currentGameDirectory->setText(this->window.gamePath);
    });
    layout->addWidget(selectDirectoryButton);

    auto gameDirectoryButton = new QPushButton("Open Game Directory");
    connect(gameDirectoryButton, &QPushButton::pressed, [this] {
        QDesktopServices::openUrl("file://" + this->window.gamePath);
    });
    layout->addWidget(gameDirectoryButton);
}