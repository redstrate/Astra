// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processlogger.h"
#include "astra_log.h"
#include "utility.h"

ProcessLogger::ProcessLogger(QProcess *process)
{
    const QDir logDirectory = Utility::stateDirectory().absoluteFilePath("log");

    file.setFileName(logDirectory.absoluteFilePath(QStringLiteral("ffxiv.log")));
    file.open(QIODevice::WriteOnly | QIODevice::Unbuffered);

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process] {
        file.write(process->readAllStandardOutput());
        file.flush();
    });

    connect(process, &QProcess::readyReadStandardError, this, [this, process] {
        file.write(process->readAllStandardError());
        file.flush();
    });

    connect(process, &QProcess::finished, this, [this] {
        deleteLater();
    });

    qInfo(ASTRA_LOG) << "Client logs are being written to" << file.fileName().toUtf8().constData();
}
