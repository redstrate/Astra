#pragma once

#include <QString>
#include <functional>

class LauncherCore;
class ProfileSettings;

// TODO: convert to a nice signal/slots class like assetupdater
void installGame(LauncherCore& launcher, ProfileSettings& profile, const std::function<void()>& returnFunc);