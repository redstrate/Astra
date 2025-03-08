// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launchersettings.h"

LauncherSettings::LauncherSettings(QObject *parent)
    : QObject(parent)
{
    m_config = new Config(KSharedConfig::openConfig(QStringLiteral("astrarc"), KConfig::SimpleConfig, QStandardPaths::AppConfigLocation));
}

bool LauncherSettings::closeWhenLaunched() const
{
    return m_config->closeWhenLaunched();
}

void LauncherSettings::setCloseWhenLaunched(const bool value)
{
    if (value != m_config->closeWhenLaunched()) {
        m_config->setCloseWhenLaunched(value);
        m_config->save();
        Q_EMIT closeWhenLaunchedChanged();
    }
}

bool LauncherSettings::showDevTools() const
{
    return m_config->showDevTools();
}

void LauncherSettings::setShowDevTools(const bool value)
{
    if (value != m_config->showDevTools()) {
        m_config->setShowDevTools(value);
        m_config->save();
        Q_EMIT showDevToolsChanged();
    }
}

bool LauncherSettings::keepPatches() const
{
    return m_config->keepPatches();
}

void LauncherSettings::setKeepPatches(const bool value)
{
    if (value != m_config->keepPatches()) {
        m_config->setKeepPatches(value);
        m_config->save();
        Q_EMIT keepPatchesChanged();
    }
}

QString LauncherSettings::dalamudDistribServer() const
{
    return m_config->dalamudDistribServer();
}

void LauncherSettings::setDalamudDistribServer(const QString &value)
{
    if (value != m_config->dalamudDistribServer()) {
        m_config->setDalamudDistribServer(value);
        m_config->save();
        Q_EMIT dalamudDistribServerChanged();
    }
}

QString LauncherSettings::defaultDalamudDistribServer() const
{
    return m_config->defaultDalamudDistribServerValue();
}

QString LauncherSettings::squareEnixServer() const
{
    return m_config->squareEnixServer();
}

void LauncherSettings::setSquareEnixServer(const QString &value)
{
    if (value != m_config->squareEnixServer()) {
        m_config->setSquareEnixServer(value);
        m_config->save();
        Q_EMIT squareEnixServerChanged();
    }
}

QString LauncherSettings::defaultSquareEnixServer() const
{
    return m_config->defaultSquareEnixServerValue();
}

QString LauncherSettings::squareEnixLoginServer() const
{
    return m_config->squareEnixLoginServer();
}

void LauncherSettings::setSquareEnixLoginServer(const QString &value)
{
    if (value != m_config->squareEnixLoginServer()) {
        m_config->setSquareEnixLoginServer(value);
        m_config->save();
        Q_EMIT squareEnixLoginServerChanged();
    }
}

QString LauncherSettings::defaultSquareEnixLoginServer() const
{
    return m_config->defaultSquareEnixLoginServerValue();
}

QString LauncherSettings::mainServer() const
{
    return m_config->mainServer();
}

void LauncherSettings::setMainServer(const QString &value)
{
    if (value != m_config->mainServer()) {
        m_config->setMainServer(value);
        m_config->save();
        Q_EMIT mainServerChanged();
    }
}

QString LauncherSettings::defaultMainServer() const
{
    return m_config->defaultMainServerValue();
}

QString LauncherSettings::preferredProtocol() const
{
    return m_config->preferredProtocol();
}

void LauncherSettings::setPreferredProtocol(const QString &value)
{
    if (value != m_config->preferredProtocol()) {
        m_config->setPreferredProtocol(value);
        m_config->save();
        Q_EMIT preferredProtocolChanged();
    }
}

QString LauncherSettings::defaultPreferredProtocol() const
{
    return m_config->defaultPreferredProtocolValue();
}

QString LauncherSettings::screenshotDir() const
{
    return m_config->screenshotDir();
}

void LauncherSettings::setScreenshotDir(const QString &value)
{
    if (value != m_config->screenshotDir()) {
        m_config->setScreenshotDir(value);
        m_config->save();
        Q_EMIT screenshotDirChanged();
    }
}

bool LauncherSettings::argumentsEncrypted() const
{
    return m_config->encryptArguments();
}

void LauncherSettings::setArgumentsEncrypted(const bool value)
{
    if (m_config->encryptArguments() != value) {
        m_config->setEncryptArguments(value);
        m_config->save();
        Q_EMIT encryptedArgumentsChanged();
    }
}

bool LauncherSettings::enableRenderDocCapture() const
{
    return m_config->enableRenderDocCapture();
}

void LauncherSettings::setEnableRenderDocCapture(const bool value)
{
    if (m_config->enableRenderDocCapture() != value) {
        m_config->setEnableRenderDocCapture(value);
        m_config->save();
        Q_EMIT enableRenderDocCaptureChanged();
    }
}

QString LauncherSettings::currentProfile() const
{
    return KSharedConfig::openStateConfig()->group(QStringLiteral("General")).readEntry(QStringLiteral("CurrentProfile"));
}

void LauncherSettings::setCurrentProfile(const QString &value)
{
    auto stateConfig = KSharedConfig::openStateConfig();
    stateConfig->group(QStringLiteral("General")).writeEntry(QStringLiteral("CurrentProfile"), value);
    stateConfig->sync();
}

bool LauncherSettings::enableSync() const
{
    return m_config->enableSync();
}

void LauncherSettings::setEnableSync(const bool enabled)
{
    if (m_config->enableSync() != enabled) {
        m_config->setEnableSync(enabled);
        m_config->save();
        Q_EMIT enableSyncChanged();
    }
}

QString LauncherSettings::customGameServer() const
{
    return m_config->customGameServer();
}

void LauncherSettings::setCustomGameServer(const QString &value)
{
    if (m_config->customGameServer() != value) {
        m_config->setCustomGameServer(value);
        m_config->save();
        Q_EMIT customGameServerChanged();
    }
}

int LauncherSettings::customGameServerPort() const
{
    return m_config->customGameServerPort();
}

void LauncherSettings::setCustomGameServerPort(const int port)
{
    if (m_config->customGameServerPort() != port) {
        m_config->setCustomGameServerPort(port);
        m_config->save();
        Q_EMIT customGameServerPortChanged();
    }
}

Config *LauncherSettings::config()
{
    return m_config;
}

#include "moc_launchersettings.cpp"
