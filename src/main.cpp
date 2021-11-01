#include "xivlauncher.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("redstrate");
    QCoreApplication::setOrganizationDomain("com.redstrate");

#ifdef NDEBUG
    QCoreApplication::setApplicationName("xivlauncher");
#else
    QCoreApplication::setApplicationName("xivlauncher-debug");
#endif

    LauncherWindow w;
    w.show();

    return app.exec();
}
