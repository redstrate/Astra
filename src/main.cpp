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
#include "desktopinterface.h"
#include "cmdinterface.h"

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

    // we want to decide which interface to use. this is decided by the
    // -cli, -desktop, or -tablet
    // the default is -desktop
    // cli is a special case where it's always "enabled"

    QCommandLineParser parser;
    parser.setApplicationDescription("Cross-platform FFXIV Launcher");

    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();

    QCommandLineOption noguiOption("nogui", "Don't open a main window.");
    parser.addOption(noguiOption);

    auto cmd = new CMDInterface(parser);

    parser.process(app);

    if(parser.isSet(versionOption)) {
        parser.showVersion();
    }

    if(parser.isSet(helpOption)) {
        parser.showHelp();
    }

    LauncherCore c;
    LauncherWindow w(c);
    if(!parser.isSet(noguiOption)) {
        new DesktopInterface(c);
    } else {
        if(!cmd->parse(parser, c))
            return -1;
    }

    return app.exec();
}
