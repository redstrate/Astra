// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KAboutData>
#include <KLocalizedString>
#include <KirigamiApp>
#include <QApplication> // NOTE: do not remove this, if your IDE suggests to do so
#include <QQuickStyle>
#include <kdsingleapplication.h>
#include <qcoroqml.h>

#ifdef HAVE_WEBVIEW
#include <QtWebView>
#endif

#ifdef Q_OS_WINDOWS
#include <QStyleFactory>
#endif

#include "astra-version.h"
#include "launchercore.h"
#include "logger.h"
#include "utility.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
#ifdef HAVE_WEBVIEW
    QtWebView::initialize();
#endif

    KirigamiApp::App app(argc, argv);
    KirigamiApp kapp;

    // TODO: remove once https://invent.kde.org/libraries/kirigami-addons/-/merge_requests/399 is merged
#ifdef Q_OS_WINDOWS
    app.setStyle(QStyleFactory::create(QStringLiteral("Breeze")));
#endif

    const KDSingleApplication singleApplication;
    if (!singleApplication.isPrimaryInstance()) {
        return 0;
    }

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    KLocalizedString::setApplicationDomain("astra");

    KAboutData about(QStringLiteral("astra"),
                     i18n("Astra"),
                     QStringLiteral(ASTRA_VERSION_STRING),
                     i18n("FFXIV Launcher"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2021-2025 Joshua Goins"));
    about.setOtherText(
        i18n("This software requires that you have legitimate access to FINAL FANTASY XIV. By using this software, you may be in violation "
             "of your User Agreement.\n\nFINAL FANTASY, FINAL FANTASY XIV, FFXIV, SQUARE ENIX, and the SQUARE ENIX logo are registered trademarks or "
             "trademarks of Square Enix Holdings Co., Ltd."));
    about.addAuthor(i18n("Joshua Goins"),
                    i18n("Maintainer"),
                    QStringLiteral("josh@redstrate.com"),
                    QStringLiteral("https://redstrate.com/"),
                    QUrl(QStringLiteral("https://redstrate.com/rss-image.png")));
    about.setHomepage(QStringLiteral("https://xiv.zone/astra"));
    about.addComponent(QStringLiteral("physis"),
                       i18n("Library for reading and writing FFXIV data."),
                       QString::fromLatin1(physis_get_physis_version()),
                       QStringLiteral("https://xiv.zone/physis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("libphysis"),
                       i18n("C/C++ bindings for Physis."),
                       QString::fromLatin1(physis_get_libphysis_version()),
                       QStringLiteral("https://github.com/redstrate/libphysis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("KDSingleApplication"),
                       i18n("Helper class for single-instance policy applications."),
                       QStringLiteral("1.2.0"),
                       QStringLiteral("https://github.com/KDAB/KDSingleApplication"),
                       KAboutLicense::MIT);
    about.addComponent(QStringLiteral("libcotp"),
                       i18n(" C library that generates TOTP and HOTP."),
                       QStringLiteral("3.1.1"),
                       QStringLiteral("https://github.com/paolostivanin/libcotp"),
                       KAboutLicense::Unknown);
    about.setDesktopFileName(QStringLiteral("zone.xiv.astra"));
    about.setBugAddress(QByteArrayLiteral("https://github.com/redstrate/astra/issues"));
    about.setComponentName(QStringLiteral("astra"));
    about.setProgramLogo(QStringLiteral("zone.xiv.astra"));
    about.setOrganizationDomain(QByteArrayLiteral("xiv.zone"));

    KAboutData::setApplicationData(about);

    initializeLogging();

    QCommandLineParser parser;
    about.setupCommandLine(&parser);

    QCommandLineOption steamOption(QStringLiteral("steam"), QString(), QStringLiteral("verb"));
    steamOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(steamOption);

    parser.parse(QCoreApplication::arguments());
    about.processCommandLine(&parser);

    // We must handle these manually, since we use parse() and not process()
    if (parser.isSet(QStringLiteral("help")) || parser.isSet(QStringLiteral("help-all"))) {
        parser.showHelp();
    }

    if (parser.isSet(QStringLiteral("version"))) {
        parser.showVersion();
    }

    if (parser.isSet(steamOption)) {
        const QStringList args = parser.positionalArguments();
        // Steam tries to use as a compatibility tool, running installation scripts (like DirectX), so try to ignore it.
        if (!args.empty() && !args.join(QLatin1Char(';')).contains("ffxivboot.exe"_L1)) {
            return 0;
        }
    }

    QCoro::Qml::registerTypes();

    QQmlApplicationEngine engine;

    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

    kapp.start(QStringLiteral("zone.xiv.astra"), QStringLiteral("Main"), &engine);
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QCoreApplication::exec();
}
