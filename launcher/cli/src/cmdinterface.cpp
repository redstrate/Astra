#include "cmdinterface.h"
#include "sapphirelauncher.h"
#include <keychain.h>

CMDInterface::CMDInterface(QCommandLineParser& parser) {
    parser.addOption(autologinOption);
    parser.addOption(profileOption);
}

bool CMDInterface::parse(QCommandLineParser& parser, LauncherCore& core) {
    if (parser.isSet(profileOption)) {
        core.defaultProfileIndex = core.getProfileIndex(parser.value(profileOption));

        if (core.defaultProfileIndex == -1) {
            qInfo() << "The profile \"" << parser.value(profileOption) << "\" does not exist!";
            return false;
        }
    }

    if (parser.isSet(autologinOption)) {
        auto& profile = core.getProfile(core.defaultProfileIndex);

        return core.autoLogin(profile);
    }

    return true;
}
