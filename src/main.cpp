#include "xivlauncher.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    LauncherWindow w;
    w.show();

    return app.exec();
}
