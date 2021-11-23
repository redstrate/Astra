#include "launchercore.h"
#include "launcherwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("redstrate");
    QCoreApplication::setOrganizationDomain("redstrate.com");

#ifdef NDEBUG
    QCoreApplication::setApplicationName("xivlauncher");
#else
    QCoreApplication::setApplicationName("xivlauncher-debug");
#endif

    LauncherCore c;

    QCommandLineParser parser;
    parser.setApplicationDescription("Cross-platform FFXIV Launcher");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    LauncherWindow w(c);
    w.show();

    return app.exec();
}
