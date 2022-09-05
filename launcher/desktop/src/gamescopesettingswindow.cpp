#include "gamescopesettingswindow.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QFormLayout>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolTip>

#include "launchercore.h"
#include "launcherwindow.h"

GamescopeSettingsWindow::GamescopeSettingsWindow(ProfileSettings& settings, LauncherCore& core, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Gamescope Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QFormLayout(this);
    setLayout(mainLayout);

    auto fullscreenBox = new QCheckBox("Fullscreen");
    fullscreenBox->setChecked(settings.gamescope.fullscreen);
    connect(fullscreenBox, &QCheckBox::clicked, [&](bool checked) {
        settings.gamescope.fullscreen = checked;

        core.saveSettings();
    });
    mainLayout->addWidget(fullscreenBox);

    auto borderlessBox = new QCheckBox("Borderless");
    borderlessBox->setChecked(settings.gamescope.fullscreen);
    connect(borderlessBox, &QCheckBox::clicked, [&](bool checked) {
        settings.gamescope.borderless = checked;

        core.saveSettings();
    });
    mainLayout->addWidget(borderlessBox);

    auto widthBox = new QSpinBox();
    widthBox->setValue(settings.gamescope.width);
    widthBox->setSpecialValueText("Default");
    connect(widthBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        settings.gamescope.width = value;

        core.saveSettings();
    });
    mainLayout->addRow("Width", widthBox);

    auto heightBox = new QSpinBox();
    heightBox->setValue(settings.gamescope.height);
    heightBox->setSpecialValueText("Default");
    connect(heightBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        settings.gamescope.height = value;

        core.saveSettings();
    });
    mainLayout->addRow("Height", heightBox);

    auto refreshRateBox = new QSpinBox();
    refreshRateBox->setValue(settings.gamescope.refreshRate);
    refreshRateBox->setSpecialValueText("Default");
    connect(refreshRateBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        settings.gamescope.refreshRate = value;

        core.saveSettings();
    });
    mainLayout->addRow("Refresh Rate", refreshRateBox);
}