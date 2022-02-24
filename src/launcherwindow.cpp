#include "launcherwindow.h"

#include <QMenuBar>
#include <keychain.h>
#include <QFormLayout>
#include <QApplication>

#include "settingswindow.h"
#include "squareboot.h"
#include "squarelauncher.h"
#include "sapphirelauncher.h"
#include "assetupdater.h"

LauncherWindow::LauncherWindow(LauncherCore& core, QWidget* parent) : QMainWindow(parent), core(core) {
    setWindowTitle("Astra");

    connect(&core, &LauncherCore::settingsChanged, this, &LauncherWindow::reloadControls);

    QMenu* fileMenu = menuBar()->addMenu("File");

    QAction* settingsAction = fileMenu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(*this, this->core, this);
        connect(&this->core, &LauncherCore::settingsChanged, window, &SettingsWindow::reloadControls);
        window->show();
    });

    QMenu* toolsMenu = menuBar()->addMenu("Tools");

    QAction* launchOfficial = toolsMenu->addAction("Launch Official Client...");
    connect(launchOfficial, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {currentProfile().gamePath + "/boot/ffxivboot.exe"});
    });

    QAction* launchSysInfo = toolsMenu->addAction("Launch System Info...");
    connect(launchSysInfo, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {currentProfile().gamePath + "/boot/ffxivsysinfo64.exe"});
    });

    QAction* launchCfgBackup = toolsMenu->addAction("Launch Config Backup...");
    connect(launchCfgBackup, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {currentProfile().gamePath + "/boot/ffxivconfig64.exe"});
    });

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QMenu* wineMenu = toolsMenu->addMenu("Wine");

    QAction* wineCfg = wineMenu->addAction("winecfg");
    connect(wineCfg, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {"winecfg.exe"});
    });

    QAction* controlPanel = wineMenu->addAction("Control Panel");
    connect(controlPanel, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {"control.exe"});
    });
#endif

    QMenu* helpMenu = menuBar()->addMenu("Help");
    QAction* showAbout = helpMenu->addAction("About Astra");
    connect(showAbout, &QAction::triggered, [=] {
        QMessageBox::about(this, "About Astra", "The source code is available <a href='https://github.com/redstrate/astra'>here</a>.");
    });

    QAction* showAboutQt = helpMenu->addAction("About Qt");
    connect(showAboutQt, &QAction::triggered, [=] {
        QApplication::aboutQt();
    });

    auto layout = new QFormLayout();

    profileSelect = new QComboBox();
    connect(profileSelect, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        reloadControls();
    });

    layout->addRow("Profile", profileSelect);

    usernameEdit = new QLineEdit();
    layout->addRow("Username", usernameEdit);

    rememberUsernameBox = new QCheckBox();
    connect(rememberUsernameBox, &QCheckBox::stateChanged, [=](int) {
        currentProfile().rememberUsername = rememberUsernameBox->isChecked();
        this->core.saveSettings();
    });
    layout->addRow("Remember Username?", rememberUsernameBox);

    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);
    layout->addRow("Password", passwordEdit);

    rememberPasswordBox = new QCheckBox();
    connect(rememberPasswordBox, &QCheckBox::stateChanged, [=](int) {
        currentProfile().rememberPassword = rememberPasswordBox->isChecked();
        this->core.saveSettings();
    });
    layout->addRow("Remember Password?", rememberPasswordBox);

    otpEdit = new QLineEdit();
    layout->addRow("One-Time Password", otpEdit);

    loginButton = new QPushButton("Login");
    layout->addRow(loginButton);

    registerButton = new QPushButton("Register");
    layout->addRow(registerButton);

    auto emptyWidget = new QWidget();
    emptyWidget->setLayout(layout);
    setCentralWidget(emptyWidget);

    connect(core.assetUpdater, &AssetUpdater::finishedUpdating, [=] {
        auto info = LoginInformation{&currentProfile(), usernameEdit->text(), passwordEdit->text(), otpEdit->text()};

        if(currentProfile().rememberUsername) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(usernameEdit->text());
            job->setKey(currentProfile().name + "-username");
            job->start();
        }

        if(currentProfile().rememberPassword) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(passwordEdit->text());
            job->setKey(currentProfile().name + "-password");
            job->start();
        }

        if(currentProfile().isSapphire) {
            this->core.sapphireLauncher->login(currentProfile().lobbyURL, info);
        } else {
            this->core.squareBoot->bootCheck(info);
        }
    });

    connect(loginButton, &QPushButton::released, [=] {
        // update the assets first if needed, then it calls the slot above :-)
        this->core.assetUpdater->update(currentProfile());
    });

    connect(registerButton, &QPushButton::released, [=] {
        if(currentProfile().isSapphire) {
            auto info = LoginInformation{&currentProfile(), usernameEdit->text(), passwordEdit->text(), otpEdit->text()};
            this->core.sapphireLauncher->registerAccount(currentProfile().lobbyURL, info);
        }
    });

    reloadControls();
}

ProfileSettings LauncherWindow::currentProfile() const {
    return core.getProfile(profileSelect->currentIndex());
}

ProfileSettings& LauncherWindow::currentProfile() {
    return core.getProfile(profileSelect->currentIndex());
}

void LauncherWindow::reloadControls() {
    if(currentlyReloadingControls)
        return;

    currentlyReloadingControls = true;

    const int oldIndex = profileSelect->currentIndex();

    profileSelect->clear();

    for(const auto& profile : core.profileList()) {
        profileSelect->addItem(profile);
    }

    profileSelect->setCurrentIndex(oldIndex);

    if(profileSelect->currentIndex() == -1) {
        profileSelect->setCurrentIndex(core.defaultProfileIndex);
    }

    rememberUsernameBox->setChecked(currentProfile().rememberUsername);
    if(currentProfile().rememberUsername) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey(currentProfile().name + "-username");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            usernameEdit->setText(job->textData());
        });
    }

    rememberPasswordBox->setChecked(currentProfile().rememberPassword);
    if(currentProfile().rememberPassword) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey(currentProfile().name + "-password");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            passwordEdit->setText(job->textData());
        });
    }

    const bool canLogin = currentProfile().isSapphire || (!currentProfile().isSapphire && core.squareLauncher->isGateOpen);

    if(canLogin) {
        loginButton->setText("Login");
    } else {
        loginButton->setText("Login (Maintenance is in progress)");
    }

    loginButton->setEnabled(canLogin);
    registerButton->setEnabled(currentProfile().isSapphire);
    otpEdit->setEnabled(!currentProfile().isSapphire);

    currentlyReloadingControls = false;
}