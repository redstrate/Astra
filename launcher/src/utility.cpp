// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility.h"

#include <QStandardPaths>

QDir Utility::stateDirectory()
{
    if (qEnvironmentVariableIsSet("XDG_STATE_HOME")) {
        return qEnvironmentVariable("XDG_STATE_HOME");
    }

    const QDir homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    const QDir localDir = homeDir.absoluteFilePath(QStringLiteral(".local"));
    const QDir stateDir = localDir.absoluteFilePath(QStringLiteral("state"));
    return stateDir.absoluteFilePath(QStringLiteral("astra"));
}

QString Utility::toWindowsPath(const QDir &dir)
{
    return QStringLiteral("Z:") + dir.absolutePath().replace(QLatin1Char('/'), QLatin1Char('\\'));
}
