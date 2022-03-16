#pragma once

#include <QString>

class LauncherCore;

// TODO: convert to a nice signal/slots class like assetupdater
void installGame(LauncherCore& launcher, std::function<void()> returnFunc);