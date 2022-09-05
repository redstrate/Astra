#pragma once

#include "launchercore.h"
#include <QCommandLineParser>

/*
 * The CLI interface for Astra, driven primarily by the command-line.
 */
class CMDInterface {
public:
    explicit CMDInterface(QCommandLineParser& parser);

    bool parse(QCommandLineParser& parser, LauncherCore& core);

private:
    QCommandLineOption profileOption = {
        "default-profile",
        "Profile to use for default profile and autologin.",
        "profile"};
    QCommandLineOption autologinOption = {
        "autologin",
        "Auto-login with the default profile. This requires the profile to have remember username/password enabled!"};
};