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
    connect(autologinTimer, &QTimer::timeout, [&] {
        qDebug() << "logging in!";

        // TODO: this is the second place where I have implemented this. this is a good idea to abstract, maybe? :-)
        auto loop = new QEventLoop();
        QString username, password;

        auto usernameJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        usernameJob->setKey(profile.name + "-username");
        usernameJob->start();

        core.connect(
            usernameJob, &QKeychain::ReadPasswordJob::finished, [loop, usernameJob, &username](QKeychain::Job* j) {
                username = usernameJob->textData();
                loop->quit();
            });

        loop->exec();

        auto passwordJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        passwordJob->setKey(profile.name + "-password");
        passwordJob->start();

        core.connect(
            passwordJob, &QKeychain::ReadPasswordJob::finished, [loop, passwordJob, &password](QKeychain::Job* j) {
                password = passwordJob->textData();
                loop->quit();
            });

        loop->exec();

        auto info = new LoginInformation();
        info->settings = &profile;
        info->username = username;
        info->password = password;

        if (profile.isSapphire) {
            core.sapphireLauncher->login(profile.lobbyURL, *info);
        } else {
            core.squareBoot->bootCheck(*info);
        }
    });
    connect(this, &AutoLoginWindow::loginCanceled, [autologinTimer] {
        autologinTimer->stop();
    });
    autologinTimer->start(5000);
}