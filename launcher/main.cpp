#include "launchercore.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>

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

    QCoreApplication::setApplicationName("astra");
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
#ifdef ENABLE_DESKTOP
    parser.addOption(desktopOption);
#endif

    QCommandLineOption tabletOption("tablet", "Open a tablet interface.");
#ifdef ENABLE_TABLET
    parser.addOption(tabletOption);
#endif

    QCommandLineOption cliOption("cli", "Don't open a main window, and use the cli interface.");
#ifdef ENABLE_CLI
    parser.addOption(cliOption);
#endif

    QCommandLineOption steamOption("steam", "Simulate booting the launcher via Steam.");
#ifdef ENABLE_STEAM
    parser.addOption(steamOption);
#endif

    auto cmd = std::make_unique<CMDInterface>(parser);

    parser.process(app);

    if (parser.isSet(versionOption)) {
        parser.showVersion();
    }

    if (parser.isSet(helpOption)) {
        parser.showHelp();
    }

    LauncherCore c(parser.isSet(steamOption));
    std::unique_ptr<DesktopInterface> desktopInterface;
    std::unique_ptr<TabletInterface> tabletInterface;

    if (parser.isSet(tabletOption)) {
#ifdef ENABLE_TABLET
        tabletInterface = std::make_unique<TabletInterface>(c);
#endif
    } else if (parser.isSet(cliOption)) {
#ifdef ENABLE_CLI
        if (!cmd->parse(parser, c))
            return -1;
#endif
    } else {
#ifdef ENABLE_DESKTOP
        desktopInterface = std::make_unique<DesktopInterface>(c);
#endif
    }

    return app.exec();
}
