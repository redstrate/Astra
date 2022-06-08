#include "tabletinterface.h"

TabletInterface::TabletInterface(LauncherCore &core) {
    applicationEngine = new QQmlApplicationEngine();

    applicationEngine->load("qrc:/main.qml");
}
