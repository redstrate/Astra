// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

class LauncherCore;

class CompatibilityToolInstaller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.createCompatInstaller")

    Q_PROPERTY(bool isInstalled READ isInstalled NOTIFY isInstalledChanged)

public:
    explicit CompatibilityToolInstaller(LauncherCore &launcher, QObject *parent = nullptr);

    Q_INVOKABLE void installCompatibilityTool();
    Q_INVOKABLE void removeCompatibilityTool();

    bool isInstalled() const;

Q_SIGNALS:
    void installFinished();
    void error(QString message);
    void removalFinished();
    void isInstalledChanged();

private:
    LauncherCore &m_launcher;
};