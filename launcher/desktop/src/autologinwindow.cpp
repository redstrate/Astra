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
#include <cotp.h>

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
        // TODO: this is the second place where I have implemented this. this is a good idea to abstract, maybe? :-)
        auto loop = new QEventLoop();

        QString username, password;
        QString otpSecret;

        auto usernameJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        usernameJob->setKey(profile.name + "-username");
        usernameJob->start();

        QObject::connect(
            usernameJob, &QKeychain::ReadPasswordJob::finished, [loop, usernameJob, &username](QKeychain::Job* j) {
                username = usernameJob->textData();
                loop->quit();
            });

        loop->exec();

        auto passwordJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        passwordJob->setKey(profile.name + "-password");
        passwordJob->start();

        QObject::connect(
            passwordJob, &QKeychain::ReadPasswordJob::finished, [loop, passwordJob, &password](QKeychain::Job* j) {
                password = passwordJob->textData();
                loop->quit();
            });

        loop->exec();

        // TODO: handle cases where the user doesn't want to store their OTP secret, so we have to manually prompt them
        if(profile.useOneTimePassword && profile.rememberOTPSecret) {
            auto otpJob = new QKeychain::ReadPasswordJob("LauncherWindow");
            otpJob->setKey(profile.name + "-otpsecret");
            otpJob->start();

            QObject::connect(
                otpJob, &QKeychain::ReadPasswordJob::finished, [loop, otpJob, &otpSecret](QKeychain::Job* j) {
                    otpSecret = otpJob->textData();
                    loop->quit();
                });

            loop->exec();
        }

        auto info = new LoginInformation();
        info->settings = &profile;
        info->username = username;
        info->password = password;

        if(profile.useOneTimePassword && profile.rememberOTPSecret) {
            // generate otp
            char *totp = get_totp (otpSecret.toStdString().c_str(), 6, 30, SHA1, nullptr);
            info->oneTimePassword = totp;
            free (totp);
        }

        if (profile.isSapphire) {
            core.sapphireLauncher->login(profile.lobbyURL, *info);
        } else {
            core.squareBoot->bootCheck(*info);
        }

        close();
        autologinTimer->stop();
    });
    connect(this, &AutoLoginWindow::loginCanceled, [autologinTimer] {
        autologinTimer->stop();
    });
    autologinTimer->start(5000);
}