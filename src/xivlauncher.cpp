#include <QPushButton>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDir>
#include <QFormLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QComboBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCheckBox>
#include <keychain.h>
#include <QMessageBox>
#include <QMenuBar>

#include "xivlauncher.h"
#include "sapphirelauncher.h"
#include "squarelauncher.h"
#include "squareboot.h"
#include "settingswindow.h"

void LauncherWindow::setSSL(QNetworkRequest& request) {
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

void LauncherWindow::buildRequest(QNetworkRequest& request) {
    setSSL(request);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QString("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QSysInfo::bootUniqueId()));
    request.setRawHeader("Accept",
                         "image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, application/x-ms-xbap, */*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept-Language", "en-us");
}

void LauncherWindow::launchGame(const LoginAuth auth) {
    QList<QString> arguments;

    // now for the actual game...
    if(currentProfile().useDX9) {
        arguments.push_back(currentProfile().gamePath + "\\game\\ffxiv.exe");
    } else {
        arguments.push_back(currentProfile().gamePath + "\\game\\ffxiv_dx11.exe");
    }

    arguments.push_back("DEV.DataPathType=1");
    arguments.push_back("DEV.UseSqPack=1");

    arguments.push_back(QString("DEV.MaxEntitledExpansionID=%1").arg(auth.maxExpansion));
    arguments.push_back(QString("DEV.TestSID=%1").arg(auth.SID));
    arguments.push_back(QString("SYS.Region=%1").arg(auth.region));
    arguments.push_back(QString("language=%1").arg(currentProfile().language));
    arguments.push_back(QString("ver=%1").arg(currentProfile().gameVersion));

    if(!auth.lobbyhost.isEmpty()) {
        arguments.push_back(QString("DEV.GMServerHost=%1").arg(auth.frontierHost));
        for(int i = 1; i < 9; i++)
            arguments.push_back(QString("DEV.LobbyHost0%1=%2 DEV.LobbyPort0%1=54994").arg(QString::number(i), auth.lobbyhost));
    }

    launchExecutable(arguments);
}

void LauncherWindow::launchExecutable(const QStringList args) {
    auto process = new QProcess(this);
    process->setProcessChannelMode(QProcess::ForwardedChannels);

    QList<QString> arguments;
    QStringList env = QProcess::systemEnvironment();

#if defined(Q_OS_LINUX)
    if(currentProfile().useGamescope) {
        arguments.push_back("gamescope");
        arguments.push_back("-f");
        arguments.push_back("-b");
    }

    if(currentProfile().useGamemode)
        arguments.push_back("gamemoderun");

    if(currentProfile().useEsync) {
        env << "WINEESYNC=1";
    }
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env << "WINEPREFIX=" + currentProfile().winePrefixPath;

    if(currentProfile().enableDXVKhud)
        env << "DXVK_HUD=full";

    arguments.push_back(currentProfile().winePath);
#endif

    arguments.append(args);

    auto executable = arguments[0];
    arguments.removeFirst();

    process->setWorkingDirectory(currentProfile().gamePath + "/game/");
    process->setEnvironment(env);
    process->start(executable, arguments);
}

QString LauncherWindow::readVersion(QString path) {
    QFile file(path);
    file.open(QFile::OpenModeFlag::ReadOnly);

    return file.readAll();
}

void LauncherWindow::readInitialInformation() {
    const int wineVersion = settings.value("wineVersion", 0).toInt();
#if defined(Q_OS_MAC)
    switch(wineVersion) {
        case 0: // system wine
            winePath = "/usr/local/bin/wine64";
            break;
        case 1: // custom path
            winePath = settings.value("winePath").toString();
            break;
        case 2: // ffxiv built-in (for mac users)
            winePath = "/Applications/FINAL FANTASY XIV ONLINE.app/Contents/SharedSupport/finalfantasyxiv/FINAL FANTASY XIV ONLINE/wine";
            break;
    }
#endif

#if defined(Q_OS_LINUX)
    switch(wineVersion) {
        case 0: // system wine (should be in $PATH)
            currentProfile().winePath = "wine";
            break;
        case 1: // custom pth
            currentProfile().winePath = settings.value("winePath").toString();
            break;
    }
#endif

    if(settings.contains("gamePath") && settings.value("gamePath").canConvert<QString>() && !settings.value("gamePath").toString().isEmpty()) {
        currentProfile().gamePath = settings.value("gamePath").toString();
    } else {
#if defined(Q_OS_WIN)
        gamePath = "C:\\Program Files (x86)\\SquareEnix\\FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_MACOS)
        gamePath = QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_LINUX)
        currentProfile().gamePath = QDir::homePath() + "/.wine/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif
    }

    if(settings.contains("winePrefix") && settings.value("winePrefix").canConvert<QString>() && !settings.value("winePrefix").toString().isEmpty()) {
        currentProfile().winePrefixPath = settings.value("winePrefix").toString();
    } else {
#if defined(Q_OS_MACOS)
        winePrefixPath = QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy";
#endif

#if defined(Q_OS_LINUX)
        currentProfile().winePrefixPath = QDir::homePath() + "/.wine";
#endif
    }

    currentProfile().bootVersion = readVersion(currentProfile().gamePath + "/boot/ffxivboot.ver");
    currentProfile().gameVersion = readVersion(currentProfile().gamePath + "/game/ffxivgame.ver");

    currentProfile().useEsync = settings.value("useEsync", false).toBool();
    currentProfile().useGamemode = settings.value("useGamemode", false).toBool();
    currentProfile().useGamescope = settings.value("useGamescope", false).toBool();
    currentProfile().enableDXVKhud = settings.value("enableDXVKhud", false).toBool();
}

LauncherWindow::LauncherWindow(QWidget* parent) :
        QMainWindow(parent) {
    mgr = new QNetworkAccessManager();
    sapphireLauncher = new SapphireLauncher(*this);
    squareLauncher = new SquareLauncher(*this);
    squareBoot = new SquareBoot(*this, *squareLauncher);

    profileSettings.append(ProfileSettings());

    QMenu* fileMenu = menuBar()->addMenu("File");
    // sorry linux users, for some reason my global menu does not like qt6 apps right now
#if defined(Q_OS_LINUX)
    menuBar()->setNativeMenuBar(false);
#endif

    QAction* settingsAction = fileMenu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(*this);
        window->show();
    });

    QMenu* toolsMenu = menuBar()->addMenu("Tools");

    QAction* launchOfficial = toolsMenu->addAction("Launch Official Client...");
    connect(launchOfficial, &QAction::triggered, [=] {
        launchExecutable({currentProfile().gamePath + "/boot/ffxivboot64.exe"});
    });

    QAction* launchSysInfo = toolsMenu->addAction("Launch System Info...");
    connect(launchSysInfo, &QAction::triggered, [=] {
        launchExecutable({currentProfile().gamePath + "/boot/ffxivsysinfo64.exe"});
    });

    QAction* launchCfgBackup = toolsMenu->addAction("Launch Config Backup...");
    connect(launchCfgBackup, &QAction::triggered, [=] {
        launchExecutable({currentProfile().gamePath + "/boot/ffxivconfig64.exe"});
    });

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QMenu* wineMenu = toolsMenu->addMenu("Wine");

    QAction* wineCfg = wineMenu->addAction("winecfg");
    connect(wineCfg, &QAction::triggered, [=] {
        launchExecutable({"winecfg.exe"});
    });

    QAction* controlPanel = wineMenu->addAction("Control Panel");
    connect(controlPanel, &QAction::triggered, [=] {
        launchExecutable({"control.exe"});
    });
#endif

    const auto savedServerType = settings.value("serverType", 0).toInt();
    const auto savedLobbyURL = settings.value("lobbyURL", "127.0.0.1").toString();
    const auto shouldRememberUsername = settings.value("rememberUsername", false).toBool();
    const auto shouldRememberPassword = settings.value("rememberPassword", false).toBool();

    auto layout = new QFormLayout();

    auto profileSelect = new QComboBox();
    profileSelect->addItem("Default");
    layout->addRow("Profile", profileSelect);

    auto usernameEdit = new QLineEdit();
    layout->addRow("Username", usernameEdit);

    if(shouldRememberUsername) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey("username");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            usernameEdit->setText(job->textData());
        });
    }

    auto rememberUsernameBox = new QCheckBox();
    rememberUsernameBox->setChecked(shouldRememberUsername);
    layout->addRow("Remember Username?", rememberUsernameBox);

    auto passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);
    layout->addRow("Password", passwordEdit);

    if(shouldRememberPassword) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey("password");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            passwordEdit->setText(job->textData());
        });
    }

    auto rememberPasswordBox = new QCheckBox();
    rememberPasswordBox->setChecked(shouldRememberPassword);
    layout->addRow("Remember Password?", rememberPasswordBox);

    auto otpEdit = new QLineEdit();
    layout->addRow("One-Time Password", otpEdit);

    auto loginButton = new QPushButton("Login");
    layout->addRow(loginButton);

    auto registerButton = new QPushButton("Register");
    layout->addRow(registerButton);

    /*const auto refreshControls = [=](int index) {
        lobbyServerURL->setEnabled(index == 1);
        registerButton->setEnabled(index == 1);
        otpEdit->setEnabled(index == 0);
    };
    refreshControls(serverType->currentIndex());

    connect(serverType, &QComboBox::currentIndexChanged, [=](int index) {
        refreshControls(index);
    });*/

    auto emptyWidget = new QWidget();
    emptyWidget->setLayout(layout);
    setCentralWidget(emptyWidget);

    readInitialInformation();

    connect(loginButton, &QPushButton::released, [=] {
        auto info = LoginInformation{usernameEdit->text(), passwordEdit->text(), otpEdit->text()};

        settings.setValue("gamePath", currentProfile().gamePath);
        settings.setValue("winePrefix", currentProfile().winePrefixPath);

        settings.setValue("rememberUsername", rememberUsernameBox->checkState() == Qt::CheckState::Checked);
        if(rememberUsernameBox->checkState() == Qt::CheckState::Checked) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(usernameEdit->text());
            job->setKey("username");
            job->start();
        }

        settings.setValue("rememberPassword", rememberPasswordBox->checkState() == Qt::CheckState::Checked);
        if(rememberPasswordBox->checkState() == Qt::CheckState::Checked) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(passwordEdit->text());
            job->setKey("password");
            job->start();
        }

        //settings.setValue("serverType", serverType->currentIndex());
        //settings.setValue("lobbyURL", lobbyServerURL->text());

        //if(serverType->currentIndex() == 0) {
            // begin se's booting process
            squareBoot->bootCheck(info);
        //} else {
        //    sapphireLauncher->login(lobbyServerURL->text(), info);
        //}
    });

    /*connect(registerButton, &QPushButton::released, [=] {
        if(serverType->currentIndex() == 1) {
            auto info = LoginInformation{usernameEdit->text(), passwordEdit->text(), otpEdit->text()};
            sapphireLauncher->registerAccount(lobbyServerURL->text(), info);
        }
    });*/
}

LauncherWindow::~LauncherWindow() = default;

ProfileSettings LauncherWindow::currentProfile() const {
    return profileSettings[currentProfileIndex];
}

ProfileSettings& LauncherWindow::currentProfile() {
    return profileSettings[currentProfileIndex];
}

void LauncherWindow::setProfile(QString name) {
    for(int i = 0; i < profileSettings.size(); i++) {
        currentProfileIndex = 0;
    }

    currentProfileIndex = -1;
}

ProfileSettings LauncherWindow::getProfile(QString name) {
    for(auto profile : profileSettings) {
        return profile;
    }
}

QList<QString> LauncherWindow::profileList() const {
    QList<QString> list;
    for(auto profile : profileSettings) {
        list.append("Default");
    }

    return list;
}