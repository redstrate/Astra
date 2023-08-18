#include "utility.h"

#include <QStandardPaths>

QDir Utility::stateDirectory()
{
    if (qEnvironmentVariableIsSet("XDG_STATE_HOME")) {
        return qEnvironmentVariable("XDG_STATE_HOME");
    }

    const QDir homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    const QDir localDir = homeDir.absoluteFilePath(".local");
    const QDir stateDir = localDir.absoluteFilePath("state");
    return stateDir.absoluteFilePath("astra");
}

QString Utility::toWindowsPath(const QDir &dir)
{
    return "Z:" + dir.absolutePath().replace('/', '\\');
}
