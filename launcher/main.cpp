#include "launchercore.h"
#include "launcherwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <keychain.h>
#include <physis.hpp>

#include "../launcher/tablet/include/tabletinterface.h"
#include "cmdinterface.h"
#include "config.h"
#include "desktopinterface.h"
#include "gameinstaller.h"
#include "sapphirelauncher.h"
#include "squareboot.h"

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

    QCommandLineOption desktopOption("desktop", "Open a desktop interface.");
    parser.addOption(desktopOption);

    QCommandLineOption tabletOption("tablet", "Open a tablet interface.");
    parser.addOption(tabletOption);

    QCommandLineOption cliOption("cli", "Don't open a main window, and use the cli interface.");
    parser.addOption(cliOption);

    auto cmd = std::make_unique<CMDInterface>(parser);

    parser.process(app);

    if (parser.isSet(versionOption)) {
        parser.showVersion();
    }

    if (parser.isSet(helpOption)) {
        parser.showHelp();
    }

    LauncherCore c;
    if (parser.isSet(tabletOption)) {
        std::make_unique<TabletInterface>(c);
    } else if (parser.isSet(cliOption)) {
        if (!cmd->parse(parser, c))
            return -1;
    } else {
        std::make_unique<DesktopInterface>(c);
    }

    return app.exec();
}
