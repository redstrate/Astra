#include "settingswindow.h"

#include <QFormLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QLabel>
#include <QFileDialog>

#include "xivlauncher.h"

SettingsWindow::SettingsWindow(LauncherWindow& window, QWidget* parent) : window(window), QWidget(parent) {
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto layout = new QFormLayout(this);
    setLayout(layout);

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