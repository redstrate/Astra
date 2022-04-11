#include "launchercore.h"
#include "launcherwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <keychain.h>
#include <QDir>

#include "sapphirelauncher.h"
#include "squareboot.h"
#include "gameinstaller.h"
#include "config.h"

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

#ifdef NDEBUG
    QCoreApplication::setApplicationName("astra");
#else
    QCoreApplication::setApplicationName("astra-debug");
#endif

    QCoreApplication::setApplicationVersion(version);

    QCommandLineParser parser;
    parser.setApplicationDescription("Cross-platform FFXIV Launcher");

    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();

    QCommandLineOption noguiOption("nogui", "Don't open a main window.");
    parser.addOption(noguiOption);

    QCommandLineOption autologinOption("autologin", "Auto-login with the default profile. This requires the profile to have remember username/password enabled!");
    parser.addOption(autologinOption);

    QCommandLineOption profileOption("default-profile", "Profile to use for default profile and autologin.", "profile");
    parser.addOption(profileOption);

    parser.process(app);

    if(parser.isSet(versionOption)) {
        parser.showVersion();
    }

    if(parser.isSet(helpOption)) {
        parser.showHelp();
    }

    LauncherCore c;

    if(parser.isSet(profileOption)) {
        c.defaultProfileIndex = c.getProfileIndex(parser.value(profileOption));

        if(c.defaultProfileIndex == -1) {
            qInfo() << "The profile \"" << parser.value(profileOption) << "\" does not exist!";
            return 0;
        }
    }

    if(parser.isSet(autologinOption)) {
        auto profile = c.getProfile(c.defaultProfileIndex);

        if(!profile.rememberUsername || !profile.rememberPassword) {
            qInfo() << "Profile does not have a username and/or password saved, autologin disabled.";

            return 0;
        }

        auto loop = new QEventLoop();
        QString username, password;

        auto usernameJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        usernameJob->setKey(profile.name + "-username");
        usernameJob->start();

        c.connect(usernameJob, &QKeychain::ReadPasswordJob::finished, [loop, usernameJob, &username](QKeychain::Job* j) {
            username = usernameJob->textData();
            loop->quit();
        });

        loop->exec();

        auto passwordJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        passwordJob->setKey(profile.name + "-password");
        passwordJob->start();

        c.connect(passwordJob, &QKeychain::ReadPasswordJob::finished, [loop, passwordJob, &password](QKeychain::Job* j) {
            password = passwordJob->textData();
            loop->quit();
        });

        loop->exec();

        auto info = LoginInformation{&profile, username, password, ""};

        if(profile.isSapphire) {
            c.sapphireLauncher->login(profile.lobbyURL, info);
        } else {
            c.squareBoot->bootCheck(info);
        }
    }

    LauncherWindow w(c);
    if(!parser.isSet(noguiOption)) {
        w.show();

        auto defaultProfile = c.getProfile(c.defaultProfileIndex);

        if(!defaultProfile.isGameInstalled()) {
            auto messageBox = new QMessageBox(&w);
            messageBox->setIcon(QMessageBox::Icon::Question);
            messageBox->setText("No Game Found");
            messageBox->setInformativeText("FFXIV is not installed. Would you like to install it now?");

            QString detailedText = QString("Astra will install FFXIV for you at '%1'").arg(c.getProfile(c.defaultProfileIndex).gamePath);
            detailedText.append("\n\nIf you do not wish to install it to this location, please set it in your default profile first.");

            messageBox->setDetailedText(detailedText);
            messageBox->setWindowModality(Qt::WindowModal);

            auto installButton = messageBox->addButton("Install Game", QMessageBox::YesRole);
            c.connect(installButton, &QPushButton::clicked, [&c, messageBox] {
                installGame(c, [messageBox, &c] {
                    c.readGameVersion();

                    messageBox->close();
                });
            });

            messageBox->addButton(QMessageBox::StandardButton::No);
            messageBox->setDefaultButton(installButton);

            messageBox->exec();
        }

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        if(!defaultProfile.isWineInstalled()) {
            auto messageBox = new QMessageBox(&w);
            messageBox->setIcon(QMessageBox::Icon::Critical);
            messageBox->setText("No Wine Found");
            messageBox->setInformativeText("Wine is not installed but is required to FFXIV on this operating system.");
            messageBox->setWindowModality(Qt::WindowModal);

            messageBox->addButton(QMessageBox::StandardButton::Ok);
            messageBox->setDefaultButton(QMessageBox::StandardButton::Ok);

            messageBox->exec();
        }
#endif
    }

    return app.exec();
}
