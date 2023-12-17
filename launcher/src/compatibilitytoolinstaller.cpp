// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatibilitytoolinstaller.h"

#include <KLocalizedString>

#include "launchercore.h"

CompatibilityToolInstaller::CompatibilityToolInstaller(LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
{
}

void CompatibilityToolInstaller::installCompatibilityTool()
{
    const QDir appDataDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::GenericDataLocation)[0];
    const QDir steamDir = appDataDir.absoluteFilePath(QStringLiteral("Steam"));
    if (!steamDir.exists()) {
        Q_EMIT error(i18n("Could not find a Steam installation."));
        return;
    }

    const QDir compatToolDir = steamDir.absoluteFilePath(QStringLiteral("compatibilitytools.d"));
    const QDir astraToolDir = compatToolDir.absoluteFilePath(QStringLiteral("astra"));
    if (astraToolDir.exists()) {
        Q_EMIT error(i18n("The compatibility tool is already installed."));
        return;
    } else {
        QDir().mkpath(astraToolDir.absolutePath());
    }

    const QString appPath = QCoreApplication::applicationFilePath();
    QFile appFile(appPath);
    appFile.link(astraToolDir.absoluteFilePath(QStringLiteral("astra")));

    const QString toolManifestContents = QStringLiteral(
        "\"manifest\"\n"
        "{\n"
        "  \"version\" \"2\"\n"
        "  \"commandline\" \"/astra --steam %verb%\"\n"
        "}");

    QFile toolManifestFile(astraToolDir.absoluteFilePath(QStringLiteral("toolmanifest.vdf")));
    toolManifestFile.open(QIODevice::WriteOnly | QIODevice::Text);
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

    QFile compatibilityToolFile(astraToolDir.absoluteFilePath(QStringLiteral("compatibilitytool.vdf")));
    compatibilityToolFile.open(QIODevice::WriteOnly | QIODevice::Text);
    compatibilityToolFile.write(compatibilityToolContents.toUtf8());
    compatibilityToolFile.close();

    Q_EMIT installFinished();
}

void CompatibilityToolInstaller::removeCompatibilityTool()
{
    const QDir appDataDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::GenericDataLocation)[0];
    const QDir steamDir = appDataDir.absoluteFilePath(QStringLiteral("Steam"));
    if (!steamDir.exists()) {
        Q_EMIT error(i18n("Could not find a Steam installation."));
        return;
    }

    const QDir compatToolDir = steamDir.absoluteFilePath(QStringLiteral("compatibilitytools.d"));
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