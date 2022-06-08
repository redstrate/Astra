#include <keychain.h>
#include "cmdinterface.h"
#include "squareboot.h"
#include "sapphirelauncher.h"

CMDInterface::CMDInterface(QCommandLineParser &parser) {
    parser.addOption(autologinOption);
    parser.addOption(profileOption);
}

bool CMDInterface::parse(QCommandLineParser &parser, LauncherCore &core) {
    if(parser.isSet(profileOption)) {
        core.defaultProfileIndex = core.getProfileIndex(parser.value(profileOption));

        if(core.defaultProfileIndex == -1) {
            qInfo() << "The profile \"" << parser.value(profileOption) << "\" does not exist!";
            return false;
        }
    }

    if(parser.isSet(autologinOption)) {
        auto& profile = core.getProfile(core.defaultProfileIndex);

        if(!profile.rememberUsername || !profile.rememberPassword) {
            qInfo() << "Profile does not have a username and/or password saved, autologin disabled.";

            return false;
        }

        auto loop = new QEventLoop();
        QString username, password;

        auto usernameJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        usernameJob->setKey(profile.name + "-username");
        usernameJob->start();

        core.connect(usernameJob, &QKeychain::ReadPasswordJob::finished, [loop, usernameJob, &username](QKeychain::Job* j) {
            username = usernameJob->textData();
            loop->quit();
        });

        loop->exec();

        auto passwordJob = new QKeychain::ReadPasswordJob("LauncherWindow");
        passwordJob->setKey(profile.name + "-password");
        passwordJob->start();

        core.connect(passwordJob, &QKeychain::ReadPasswordJob::finished, [loop, passwordJob, &password](QKeychain::Job* j) {
            password = passwordJob->textData();
            loop->quit();
        });

        loop->exec();

        auto info = new LoginInformation();
        info->settings = &profile;
        info->username = username;
        info->password = password;

        if(profile.isSapphire) {
            core.sapphireLauncher->login(profile.lobbyURL, *info);
        } else {
            core.squareBoot->bootCheck(*info);
        }
    }

    return true;
}
