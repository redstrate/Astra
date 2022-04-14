#pragma once

#include <QString>
#include <functional>

class LauncherCore;
class ProfileSettings;

// TODO: convert to a nice signal/slots class like assetupdater
void installDXVK(LauncherCore& launcher, ProfileSettings& profile);