#include "gamescopesettingswindow.h"

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
#include <QSpinBox>

#include "launchercore.h"
#include "launcherwindow.h"

GamescopeSettingsWindow::GamescopeSettingsWindow(ProfileSettings& settings, QWidget* parent) :  QDialog(parent) {
    setWindowTitle("Gamescope Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QFormLayout(this);
    setLayout(mainLayout);

    auto fullscreenBox = new QCheckBox("Fullscreen");
    fullscreenBox->setChecked(settings.gamescope.fullscreen);
    connect(fullscreenBox, &QCheckBox::clicked, [&](bool checked) {
        settings.gamescope.fullscreen = checked;
    });
    mainLayout->addWidget(fullscreenBox);

    auto borderlessBox = new QCheckBox("Borderless");
    borderlessBox->setChecked(settings.gamescope.fullscreen);
    connect(borderlessBox, &QCheckBox::clicked, [&](bool checked) {
        settings.gamescope.borderless = checked;
    });
    mainLayout->addWidget(borderlessBox);

    auto widthBox = new QSpinBox();
    widthBox->setValue(settings.gamescope.width);
    widthBox->setSpecialValueText("Default");
    connect(widthBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        settings.gamescope.width = value;
    });
    mainLayout->addRow("Width", widthBox);

    auto heightBox = new QSpinBox();
    heightBox->setValue(settings.gamescope.height);
    heightBox->setSpecialValueText("Default");
    connect(heightBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        settings.gamescope.height = value;
    });
    mainLayout->addRow("Height", heightBox);

    auto refreshRateBox = new QSpinBox();
    refreshRateBox->setValue(settings.gamescope.refreshRate);
    refreshRateBox->setSpecialValueText("Default");
    connect(refreshRateBox, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value) {
        settings.gamescope.refreshRate = value;
    });
    mainLayout->addRow("Refresh Rate", refreshRateBox);
}