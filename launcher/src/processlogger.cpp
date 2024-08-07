// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processlogger.h"
#include "astra_log.h"
#include "utility.h"

#include <QStandardPaths>

ProcessLogger::ProcessLogger(const QString &baseName, QProcess *process)
{
    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QDir logDirectory = dataDir.absoluteFilePath(QStringLiteral("log"));

    m_file.setFileName(logDirectory.absoluteFilePath(QStringLiteral("%1.log").arg(baseName)));
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
