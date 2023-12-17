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

bool LauncherSettings::showNews() const
{
    return m_config->showNews();
}

void LauncherSettings::setShowNews(const bool value)
{
    if (value != m_config->showNews()) {
        m_config->setShowNews(value);
        m_config->save();
        Q_EMIT showNewsChanged();
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

QString LauncherSettings::xivApiServer() const
{
    return m_config->xivApiServer();
}

void LauncherSettings::setXivApiServer(const QString &value)
{
    if (value != m_config->xivApiServer()) {
        m_config->setXivApiServer(value);
        m_config->save();
        Q_EMIT xivApiServerChanged();
    }
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

Config *LauncherSettings::config()
{
    return m_config;
}
