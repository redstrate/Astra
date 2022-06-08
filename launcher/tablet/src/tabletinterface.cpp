#include "tabletinterface.h"

#include <QQuickView>
#include <QQmlContext>

TabletInterface::TabletInterface(LauncherCore &core) {
    qmlRegisterType<ProfileSettings>("Astra", 1, 0, "ProfileSettings");
    qmlRegisterType<LoginInformation>("Astra", 1, 0, "LoginInformation");

    applicationEngine = new QQmlApplicationEngine();

    applicationEngine->rootContext()->setContextProperty("core", &core);
    applicationEngine->load("qrc:/main.qml");
}
