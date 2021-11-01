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

void LauncherWindow::launch(const LoginAuth auth) {
    auto process = new QProcess(this);
    process->setProcessChannelMode(QProcess::ForwardedChannels);

    bool isWine = false;
    QString winePath;
    QString ffxivPath;

#if defined(Q_OS_WIN)
    ffxivPath = gamePath + "\\game\\ffxiv_dx11.exe";
#endif

#if defined(Q_OS_MACOS)
    isWine = true;
   // TODO: this is assuming FFXIV is installed in /Applications
   winePath = "/Applications/FINAL FANTASY XIV ONLINE.app/Contents/SharedSupport/finalfantasyxiv/FINAL FANTASY XIV ONLINE/wine";
   ffxivPath = QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn/game/ffxiv_dx11.exe";
#endif

#if defined(Q_OS_LINUX)
    isWine = true;
    // TODO: this is assuming you want to use the wine in your PATH, which isn't always the case
    winePath = "wine";

    // TODO: this is assuming it's in your default WINEPREFIX
    ffxivPath = gamePath + "/game/ffxiv_dx11.exe";
    process->setWorkingDirectory(gamePath + "/game/");
#endif

    QList<QString> arguments;
    if (isWine) {
        arguments.push_back(ffxivPath);
    }

    // i wonder what these mean...
    arguments.push_back("DEV.DataPathType=1");
    arguments.push_back("DEV.UseSqPack=1");
    // by the way, it looks like setting graphics options is possible via these too, i wonder what
    // else is hiding :-)))

    arguments.push_back(QString("DEV.MaxEntitledExpansionID=%1").arg(auth.maxExpansion));
    arguments.push_back(QString("DEV.TestSID=%1").arg(auth.SID));
    arguments.push_back(QString("SYS.Region=%1").arg(auth.region));
    arguments.push_back(QString("language=%1").arg(language));
    arguments.push_back(QString("ver=%1").arg(gameVersion));

    if(!auth.lobbyhost.isEmpty()) {
        arguments.push_back(QString("DEV.GMServerHost=%1").arg(auth.frontierHost));
        for(int i = 1; i < 9; i++)
            arguments.push_back(QString("DEV.LobbyHost0%1=%2 DEV.LobbyPort0%1=54994").arg(QString::number(i), auth.lobbyhost));
    }

    if (isWine) {
        QStringList env = QProcess::systemEnvironment();
        //env << "DXVK_FILTER_DEVICE_NAME=AMD";
        process->setEnvironment(env);
        process->start(winePath, arguments);
    } else {
        process->start(ffxivPath, arguments);
    }
}

QString LauncherWindow::readVersion(QString path) {
    QFile file(path);
    file.open(QFile::OpenModeFlag::ReadOnly);

    return file.readAll();
}

void LauncherWindow::readInitialInformation() {
#if defined(Q_OS_WIN)
    gamePath = "C:\\Program Files (x86\\SquareEnix\\FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_MACOS)
    gamePath = QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_LINUX)
    // TODO: this is assuming it's in your default WINEPREFIX
    gamePath = QDir::homePath() + "/.wine/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif

    bootVersion = readVersion(gamePath + "/boot/ffxivboot.ver");
    gameVersion = readVersion(gamePath + "/game/ffxivgame.ver");
}

LauncherWindow::LauncherWindow(QWidget* parent) :
        QMainWindow(parent) {
    mgr = new QNetworkAccessManager();
    sapphireLauncher = new SapphireLauncher(*this);
    squareLauncher = new SquareLauncher(*this);
    squareBoot = new SquareBoot(*this, *squareLauncher);

    QMenu* fileMenu = menuBar()->addMenu("File");

    QAction* settingsAction = fileMenu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(*this);
        window->show();
    });

    const auto savedServerType = settings.value("serverType", 0).toInt();
    const auto savedLobbyURL = settings.value("lobbyURL", "127.0.0.1").toString();
    const auto shouldRememberUsername = settings.value("rememberUsername", false).toBool();
    const auto shouldRememberPassword = settings.value("rememberPassword", false).toBool();

    auto layout = new QFormLayout();

    auto serverType = new QComboBox();
    serverType->insertItem(0, "Square Enix");
    serverType->insertItem(1, "Sapphire");
    serverType->setCurrentIndex(savedServerType);

    layout->addRow("Server Lobby", serverType);

    auto lobbyServerURL = new QLineEdit();
    lobbyServerURL->setText(savedLobbyURL);
    layout->addRow("Lobby URL", lobbyServerURL);

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

    const auto refreshControls = [=](int index) {
        lobbyServerURL->setEnabled(index == 1);
        registerButton->setEnabled(index == 1);
        otpEdit->setEnabled(index == 0);
    };
    refreshControls(serverType->currentIndex());

    connect(serverType, &QComboBox::currentIndexChanged, [=](int index) {
        refreshControls(index);
    });

    auto emptyWidget = new QWidget();
    emptyWidget->setLayout(layout);
    setCentralWidget(emptyWidget);

    readInitialInformation();

    connect(loginButton, &QPushButton::released, [=] {
        auto info = LoginInformation{usernameEdit->text(), passwordEdit->text(), otpEdit->text()};

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

        settings.setValue("serverType", serverType->currentIndex());
        settings.setValue("lobbyURL", lobbyServerURL->text());

        if(serverType->currentIndex() == 0) {
            // begin se's booting process
            squareBoot->bootCheck(info);
        } else {
            sapphireLauncher->login(lobbyServerURL->text(), info);
        }
    });

    connect(registerButton, &QPushButton::released, [=] {
        if(serverType->currentIndex() == 1) {
            auto info = LoginInformation{usernameEdit->text(), passwordEdit->text(), otpEdit->text()};
            sapphireLauncher->registerAccount(lobbyServerURL->text(), info);
        }
    });
}

LauncherWindow::~LauncherWindow() = default;
