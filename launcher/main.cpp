#include "launchercore.h"

#include <QApplication>
#include <QCommandLineParser>

#include "config.h"
#include "desktopinterface.h"
#include "sapphirelauncher.h"
#include "squareboot.h"

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("astra");
    QCoreApplication::setApplicationVersion(version);

    QCommandLineParser parser;
    parser.setApplicationDescription("Cross-platform FFXIV Launcher");

    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();

    QCommandLineOption steamOption("steam", "Simulate booting the launcher via Steam.");
#ifdef ENABLE_STEAM
    parser.addOption(steamOption);
#endif

    parser.process(app);

    if (parser.isSet(versionOption)) {
        parser.showVersion();
    }

    if (parser.isSet(helpOption)) {
        parser.showHelp();
    }

#ifdef ENABLE_STEAM
    LauncherCore c(parser.isSet(steamOption));
#else
    LauncherCore c(false);
#endif
    std::make_unique<DesktopInterface>(c);

    return QApplication::exec();
}
