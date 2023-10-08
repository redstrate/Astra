// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFile>
#include <QObject>
#include <QProcess>

class ProcessLogger : public QObject
{
public:
    explicit ProcessLogger(QProcess *process);

private:
    QFile file;
};