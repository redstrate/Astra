// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatibilitytoolinstaller.h"

#include "launchercore.h"

CompatibilityToolInstaller::CompatibilityToolInstaller(LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
{
}

void CompatibilityToolInstaller::installCompatibilityTool()
{
    const QDir appDataDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::GenericDataLocation)[0];
    const QDir steamDir = appDataDir.absoluteFilePath("Steam");
    if (!steamDir.exists()) {
        Q_EMIT error("Could not find a Steam installation.");
        return;
    }

    const QDir compatToolDir = steamDir.absoluteFilePath("compatibilitytools.d");
    const QDir astraToolDir = compatToolDir.absoluteFilePath("astra");
    if (astraToolDir.exists()) {
        Q_EMIT error("The compatibility tool is already installed.");
        return;
    } else {
        QDir().mkpath(astraToolDir.absolutePath());
    }

    const QString appPath = QCoreApplication::applicationFilePath();
    QFile appFile(appPath);
    appFile.link(astraToolDir.absoluteFilePath("astra"));

    const QString toolManifestContents = QStringLiteral(
        "\"manifest\"\n"
        "{\n"
        "  \"version\" \"2\"\n"
        "  \"commandline\" \"/astra --steam %verb%\"\n"
        "}");

    QFile toolManifestFile(astraToolDir.absoluteFilePath("toolmanifest.vdf"));
    toolManifestFile.open(QIODevice::WriteOnly);
    toolManifestFile.write(toolManifestContents.toUtf8());
    toolManifestFile.close();

    const QString compatibilityToolContents = QStringLiteral(
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

    QFile compatibilityToolFile(astraToolDir.absoluteFilePath("compatibilitytool.vdf"));
    compatibilityToolFile.open(QIODevice::WriteOnly);
    compatibilityToolFile.write(compatibilityToolContents.toUtf8());
    compatibilityToolFile.close();

    Q_EMIT installFinished();
}

void CompatibilityToolInstaller::removeCompatibilityTool()
{
    const QDir appDataDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::GenericDataLocation)[0];
    const QDir steamDir = appDataDir.absoluteFilePath("Steam");
    if (!steamDir.exists()) {
        Q_EMIT error("Could not find a Steam installation.");
        return;
    }

    const QDir compatToolDir = steamDir.absoluteFilePath("compatibilitytools.d");
    QDir astraToolDir = compatToolDir.absoluteFilePath("astra");
    if (!astraToolDir.exists()) {
        Q_EMIT error("The compatibility tool is not installed.");
        return;
    } else {
        astraToolDir.removeRecursively();
    }

    Q_EMIT installFinished();
}
