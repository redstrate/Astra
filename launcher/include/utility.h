#pragma once

#include <QDir>

namespace Utility
{
QDir stateDirectory();
QString toWindowsPath(const QDir &dir);
}