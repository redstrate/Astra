// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>

class LauncherCore;

class CompatibilityToolInstaller : public QObject
{
    Q_OBJECT
public:
    CompatibilityToolInstaller(LauncherCore &launcher, QObject *parent = nullptr);

    Q_INVOKABLE void installCompatibilityTool();
    Q_INVOKABLE void removeCompatibilityTool();

Q_SIGNALS:
    void installFinished();
    void error(QString message);

private:
    LauncherCore &m_launcher;
};