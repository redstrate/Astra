// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QQuickStyle>
#include <QtWebEngine>

#include "astra-version.h"
#include "compatibilitytoolinstaller.h"
#include "gameinstaller.h"
#include "launchercore.h"
#include "sapphirelauncher.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QtWebEngine::initialize();

    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    KLocalizedString::setApplicationDomain("astra");

    KAboutData about(QStringLiteral("astra"),
                     i18n("Astra"),
                     QStringLiteral(ASTRA_VERSION_STRING),
                     i18n("FFXIV Launcher"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2023 Joshua Goins"));
    about.addAuthor(i18n("Joshua Goins"), i18n("Maintainer"), QStringLiteral("josh@redstrate.com"), QStringLiteral("https://redstrate.com/"));
    about.setHomepage("https://xiv.zone/astra");
    about.addComponent("physis", "Library to access FFXIV data", physis_get_physis_version(), "https://xiv.zone/physis", KAboutLicense::GPL_V3);
    about.addComponent("libphysis", "C bindings for physis", physis_get_libphysis_version(), "", KAboutLicense::GPL_V3);
    about.setDesktopFileName("zone.xiv.astra");
    about.setBugAddress("https://lists.sr.ht/~redstrate/public-inbox");
    about.setComponentName("astra");
    about.setProgramLogo("zone.xiv.astra");

    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Linux FFXIV Launcher"));

#ifdef ENABLE_STEAM
    QCommandLineOption steamOption("steam", "Used for booting the launcher from Steam.", "verb");
    steamOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(steamOption);
#endif

    about.setupCommandLine(&parser);
    parser.parse(QCoreApplication::arguments());
    about.processCommandLine(&parser);

#ifdef ENABLE_STEAM
    if (parser.isSet(steamOption)) {
        const QStringList args = parser.positionalArguments();
        // Steam tries to use as a compatibiltiy tool, running install scripts (like DirectX), so try to ignore it.
        if (!args[0].contains("ffxivboot.exe")) {
            return 0;
        }
    }

    LauncherCore c(parser.isSet(steamOption));
#else
    LauncherCore c(false);
#endif

    qmlRegisterSingletonInstance("zone.xiv.astra", 1, 0, "LauncherCore", &c);
    qmlRegisterUncreatableType<GameInstaller>("zone.xiv.astra", 1, 0, "GameInstaller", QStringLiteral("Use LauncherCore::createInstaller"));
    qmlRegisterUncreatableType<CompatibilityToolInstaller>("zone.xiv.astra",
                                                           1,
                                                           0,
                                                           "CompatibilityToolInstaller",
                                                           QStringLiteral("Use LauncherCore::createCompatInstaller"));
    qmlRegisterUncreatableType<AccountManager>("zone.xiv.astra", 1, 0, "AccountManager", QStringLiteral("Use LauncherCore::accountManager"));
    qmlRegisterUncreatableType<ProfileManager>("zone.xiv.astra", 1, 0, "ProfileManager", QStringLiteral("Use LauncherCore::profileManager"));
    qmlRegisterUncreatableType<Profile>("zone.xiv.astra", 1, 0, "Profile", QStringLiteral("Use from ProfileManager"));
    qmlRegisterUncreatableType<Account>("zone.xiv.astra", 1, 0, "Account", QStringLiteral("Use from AccountManager"));
    qmlRegisterSingletonType("zone.xiv.astra", 1, 0, "About", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(KAboutData::applicationData());
    });
    qmlRegisterUncreatableType<Headline>("zone.xiv.astra", 1, 0, "Headline", QStringLiteral("Use from AccountManager"));
    qRegisterMetaType<Banner>("Banner");
    qRegisterMetaType<QList<Banner>>("QList<Banner>");
    qRegisterMetaType<QList<News>>("QList<News>");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

    engine.load(QUrl(QStringLiteral("qrc:/ui/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QCoreApplication::exec();
}
