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
    const auto [steamType, steamDir] = findSteamType();
    if (steamType == SteamType::NotFound) {
        Q_EMIT error(i18n("Could not find a Steam installation."));
        return;
    }

    const QDir astraToolDir = steamDir.absoluteFilePath(QStringLiteral("astra"));
    if (astraToolDir.exists()) {
        Q_EMIT error(i18n("Astra's Compatibility Tool is already installed."));
        return;
    }

    Q_UNUSED(QDir().mkpath(astraToolDir.absolutePath()))

    // we need a run script to escape the compatibility tool quirk where it runs everything in the current directory
    const auto wrapperScriptContents = QStringLiteral("#!/bin/sh\nexec \"$@\"");

    QFile wrapperScriptFile(astraToolDir.absoluteFilePath(QStringLiteral("wrapper.sh")));
    wrapperScriptFile.open(QIODevice::WriteOnly | QIODevice::Text);
    wrapperScriptFile.write(wrapperScriptContents.toUtf8());
    wrapperScriptFile.close();

    QProcess::execute(QStringLiteral("chmod"), {QStringLiteral("+x"), astraToolDir.absoluteFilePath(QStringLiteral("wrapper.sh"))});

    QString runScriptContents;
    if (steamType == SteamType::Flatpak) {
        runScriptContents = QStringLiteral(
            "$STEAM_COMPAT_CLIENT_INSTALL_PATH/compatibilitytools.d/astra/steamwrap & p1=$!\nflatpak-spawn --host -- flatpak run zone.xiv.astra --steam \"$@\" "
            "& p2=$!\nwait -n\n[ \"$?\" "
            "-gt 1 ] || kill \"$p1\" \"$p2\"\nwait");

    } else {
        runScriptContents = QStringLiteral(
            "$STEAM_COMPAT_CLIENT_INSTALL_PATH/compatibilitytools.d/astra/steamwrap & p1=$!\nflatpak run zone.xiv.astra --steam \"$@\" & p2=$!\nwait -n\n[ "
            "\"$?\" "
            "-gt 1 ] || kill \"$p1\" \"$p2\"\nwait");
    }

    QFile runScriptFile(astraToolDir.absoluteFilePath(QStringLiteral("run.sh")));
    runScriptFile.open(QIODevice::WriteOnly | QIODevice::Text);
    runScriptFile.write(runScriptContents.toUtf8());
    runScriptFile.close();

    QProcess::execute(QStringLiteral("chmod"), {QStringLiteral("+x"), astraToolDir.absoluteFilePath(QStringLiteral("run.sh"))});

    // copy required files
    const QDir homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    const QDir filesDir = homeDir.absoluteFilePath(QStringLiteral(".local/share/flatpak/app/zone.xiv.astra/current/active/files/libexec/"));

    // we want to link these so they are updated when our flatpak updates
    QFile::link(filesDir.absoluteFilePath(QStringLiteral("steamwrap")), astraToolDir.absoluteFilePath(QStringLiteral("steamwrap")));
    QFile::link(filesDir.absoluteFilePath(QStringLiteral("libsteam_api.so")), astraToolDir.absoluteFilePath(QStringLiteral("libsteam_api.so")));

    const QString toolManifestContents = QStringLiteral(
        "\"manifest\"\n"
        "{\n"
        "  \"version\" \"2\"\n"
        "  \"commandline\" \"/wrapper.sh \\\"$STEAM_RUNTIME/scripts/switch-runtime.sh\\\" --runtime=\\\"\\\" -- "
        "$STEAM_COMPAT_CLIENT_INSTALL_PATH/compatibilitytools.d/astra/run.sh %verb%\"\n"
        "}");

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

    Q_EMIT installFinished(steamType == SteamType::Flatpak);
    Q_EMIT isInstalledChanged();
}

void CompatibilityToolInstaller::removeCompatibilityTool()
{
    const auto [steamType, steamDir] = findSteamType();
    if (steamType == SteamType::NotFound) {
        Q_EMIT error(i18n("Could not find a Steam installation."));
        return;
    }

    QDir astraToolDir = steamDir.absoluteFilePath(QStringLiteral("astra"));
    if (!astraToolDir.exists()) {
        Q_EMIT error(i18n("Astra's Compatibility Tool is not installed."));
        return;
    }

    astraToolDir.removeRecursively();

    Q_EMIT removalFinished();
    Q_EMIT isInstalledChanged();
}

bool CompatibilityToolInstaller::isInstalled() const
{
    const QDir compatToolDir = findSteamType().second.absoluteFilePath(QStringLiteral("compatibilitytools.d"));
    const QDir astraToolDir = findSteamType().second.absoluteFilePath(QStringLiteral("astra"));
    return astraToolDir.exists();
}

bool CompatibilityToolInstaller::hasSteam() const
{
    return findSteamType().first != SteamType::NotFound;
}

std::pair<CompatibilityToolInstaller::SteamType, QDir> CompatibilityToolInstaller::findSteamType() const
{
    const QDir appDataDir = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::HomeLocation)[0];

    // prefer flatpak
    {
        const QDir flatpakDataDir = appDataDir.absoluteFilePath(QStringLiteral(".var/app/com.valvesoftware.Steam/data/Steam/"));
        const QDir compatToolDir = flatpakDataDir.absoluteFilePath(QStringLiteral("compatibilitytools.d"));
        if (compatToolDir.exists()) {
            return {SteamType::Flatpak, compatToolDir};
        }
    }

    // fallback to native
    {
        const QDir steamDir = appDataDir.absoluteFilePath(QStringLiteral(".steam"));
        const QDir steamDataDir = steamDir.absoluteFilePath(QStringLiteral("steam"));
        const QDir compatToolDir = steamDataDir.absoluteFilePath(QStringLiteral("compatibilitytools.d"));
        if (compatToolDir.exists()) {
            return {SteamType::Native, compatToolDir};
        }
    }

    return {SteamType::NotFound, QDir()};
}

#include "moc_compatibilitytoolinstaller.cpp"
