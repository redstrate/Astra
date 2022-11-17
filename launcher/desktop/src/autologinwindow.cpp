#include "autologinwindow.h"

#include <QDesktopServices>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QToolTip>

#include "launchercore.h"
#include "launcherwindow.h"
#include "sapphirelauncher.h"

AutoLoginWindow::AutoLoginWindow(DesktopInterface& interface, ProfileSettings& profile, LauncherCore& core, QWidget* parent)
    : VirtualDialog(interface, parent) {
    setWindowTitle("Auto Login");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QFormLayout();
    setLayout(mainLayout);

    auto label = new QLabel("Currently logging in...");
    mainLayout->addWidget(label);

    auto cancelButton = new QPushButton("Cancel");
    connect(cancelButton, &QPushButton::clicked, this, &AutoLoginWindow::loginCanceled);
    mainLayout->addWidget(cancelButton);

    auto autologinTimer = new QTimer();

    connect(autologinTimer, &QTimer::timeout, [&, this, autologinTimer] {
        core.autoLogin(profile);
    });

    connect(this, &AutoLoginWindow::loginCanceled, [autologinTimer] {
        autologinTimer->stop();
    });

    connect(&core, &LauncherCore::successfulLaunch, [this, autologinTimer] {
        close();
        autologinTimer->stop();
    });

    autologinTimer->start(5000);
}