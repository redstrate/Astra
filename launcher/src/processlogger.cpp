// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processlogger.h"
#include "astra_log.h"
#include "utility.h"

ProcessLogger::ProcessLogger(QProcess *process)
{
    const QDir logDirectory = Utility::stateDirectory().absoluteFilePath("log");

    m_file.setFileName(logDirectory.absoluteFilePath(QStringLiteral("ffxiv.log")));
    m_file.open(QIODevice::WriteOnly | QIODevice::Unbuffered);

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process] {
        m_file.write(process->readAllStandardOutput());
        m_file.flush();
    });

    connect(process, &QProcess::readyReadStandardError, this, [this, process] {
        m_file.write(process->readAllStandardError());
        m_file.flush();
    });

    connect(process, &QProcess::finished, this, [this] {
        deleteLater();
    });

    qInfo(ASTRA_LOG) << "Client logs are being written to" << m_file.fileName().toUtf8().constData();
}
