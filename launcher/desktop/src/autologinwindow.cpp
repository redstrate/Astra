#include "autologinwindow.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QToolTip>
#include <keychain.h>

#include "launchercore.h"
#include "launcherwindow.h"
#include "sapphirelauncher.h"

AutoLoginWindow::AutoLoginWindow(ProfileSettings& profile, LauncherCore& core, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Auto Login");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QFormLayout(this);
    setLayout(mainLayout);

    auto label = new QLabel("Currently logging in...");
    mainLayout->addWidget(label);

    auto cancelButton = new QPushButton("Cancel");
    connect(cancelButton, &QPushButton::clicked, this, &AutoLoginWindow::loginCanceled);
    mainLayout->addWidget(cancelButton);

    auto autologinTimer = new QTimer();
    connect(autologinTimer, &QTimer::timeout, [&, this, autologinTimer] {
        core.autoLogin(profile);

        close();
        autologinTimer->stop();
    });
    connect(this, &AutoLoginWindow::loginCanceled, [autologinTimer] {
        autologinTimer->stop();
    });
    autologinTimer->start(5000);
}