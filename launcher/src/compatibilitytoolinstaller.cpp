// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatibilitytoolinstaller.h"

#include <KLocalizedString>
#include <KSandbox>

#include "launchercore.h"

CompatibilityToolInstaller::CompatibilityToolInstaller(LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
{
}

void CompatibilityToolInstaller::installCompatibilityTool()
{
    const QDir appDataDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation)[0];
    const QDir steamDir = appDataDir.absoluteFilePath(QStringLiteral(".steam"));
    const QDir steamSteamDir = steamDir.absoluteFilePath(QStringLiteral("steam"));
    if (!steamSteamDir.exists()) {
        Q_EMIT error(i18n("Could not find a Steam installation."));
        return;
    }

    const QDir compatToolDir = steamSteamDir.absoluteFilePath(QStringLiteral("compatibilitytools.d"));
    const QDir astraToolDir = compatToolDir.absoluteFilePath(QStringLiteral("astra"));
    if (astraToolDir.exists()) {
        Q_EMIT error(i18n("The compatibility tool is already installed."));
        return;
    }

    Q_UNUSED(QDir().mkpath(astraToolDir.absolutePath()))

    QString command;
    if (KSandbox::isFlatpak()) {
        QFile::copy(QStringLiteral("/app/bin/steamwrap"), astraToolDir.absoluteFilePath(QStringLiteral("steamwrap")));
        QFile::copy(QStringLiteral("/app/bin/libsteam_api.so"), astraToolDir.absoluteFilePath(QStringLiteral("libsteam_api.so")));

        QProcess::execute(QStringLiteral("chmod"), {QStringLiteral("+x"), astraToolDir.absoluteFilePath(QStringLiteral("steamwrap"))});

        command = QStringLiteral("/steamwrap /usr/bin/flatpak run zone.xiv.astra");
    } else {
        const QString appPath = QCoreApplication::applicationFilePath();
        QFile appFile(appPath);
        appFile.link(astraToolDir.absoluteFilePath(QStringLiteral("astra")));

        command = QStringLiteral("/astra");
    }

    const QString toolManifestContents = QStringLiteral(
                                             "\"manifest\"\n"
                                             "{\n"
                                             "  \"version\" \"2\"\n"
                                             "  \"commandline\" \"%1 --steam %verb%\"\n"
                                             "}")
                                             .arg(command);

    QFile toolManifestFile(astraToolDir.absoluteFilePath(QStringLiteral("toolmanifest.vdf")));
    toolManifestFile.open(QIODevice::WriteOnly | QIODevice::Text);
    toolManifestFile.write(toolManifestContents.toUtf8());
    toolManifestFile.close();

    const auto compatibilityToolContents = QStringLiteral(
        "\"compatibilitytools\"\n"
        "{\n"
        "  \"compat_tools\"\n"
        "  {\n"
        "\t\"Proton-Astra\" // Internal name of this tool\n"
        "\t{\n"
        "\t  \"install_path\" \".\"\n"
        "\t  \"display_name\" \"Astra\"\n"
        "\n"
        "\t  \"from_oslist\"  \"windows\"\n"
        "\t  \"to_oslist\"    \"linux\"\n"
        "\t}\n"
        "  }\n"
        "}");

    QFile compatibilityToolFile(astraToolDir.absoluteFilePath(QStringLiteral("compatibilitytool.vdf")));
    compatibilityToolFile.open(QIODevice::WriteOnly | QIODevice::Text);
    compatibilityToolFile.write(compatibilityToolContents.toUtf8());
    compatibilityToolFile.close();

    Q_EMIT installFinished();
}

void CompatibilityToolInstaller::removeCompatibilityTool()
{
    const QDir appDataDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation)[0];
    const QDir steamDir = appDataDir.absoluteFilePath(QStringLiteral(".steam"));
    const QDir steamSteamDir = steamDir.absoluteFilePath(QStringLiteral("steam"));
    if (!steamSteamDir.exists()) {
        Q_EMIT error(i18n("Could not find a Steam installation."));
        return;
    }

    const QDir compatToolDir = steamSteamDir.absoluteFilePath(QStringLiteral("compatibilitytools.d"));
    QDir astraToolDir = compatToolDir.absoluteFilePath(QStringLiteral("astra"));
    if (!astraToolDir.exists()) {
        Q_EMIT error(i18n("The compatibility tool is not installed."));
        return;
    } else {
        astraToolDir.removeRecursively();
    }

    Q_EMIT installFinished();
}

#include "moc_compatibilitytoolinstaller.cpp"