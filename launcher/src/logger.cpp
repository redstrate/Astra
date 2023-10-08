// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logger.h"
#include "utility.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QtLogging>
#include <iostream>

class Logger
{
public:
    void log(const QtMsgType type, const QMessageLogContext &context, const QString &message)
    {
        const QString formattedMsg = qFormatLogMessage(type, context, message);

        if (file.isOpen()) {
            QMutexLocker locker(&mutex);
            QByteArray buf;
            QTextStream str(&buf);

            str << formattedMsg << QStringLiteral("\n");
            str.flush();
            file.write(buf.constData(), buf.size());
            file.flush();
        }

        std::cout << formattedMsg.toStdString() << std::endl;
    }

    void initialize()
    {
        const QString filename{QStringLiteral("astra.log")};

        const QDir logDirectory = Utility::stateDirectory().absoluteFilePath("log");

        if (!logDirectory.exists()) {
            QDir().mkpath(logDirectory.absolutePath());
        }

        // Sort them from highest to lowest (4, 3, 2, 1, 0)
        auto existingLogEntries = logDirectory.entryList({filename + QStringLiteral(".*")});
        std::sort(existingLogEntries.begin(), existingLogEntries.end(), [](const auto &left, const auto &right) {
            auto leftIndex = left.split(QStringLiteral(".")).last().toInt();
            auto rightIndex = right.split(QStringLiteral(".")).last().toInt();
            return leftIndex > rightIndex;
        });

        // Iterate through the existing logs, prune them and move the oldest up
        for (const auto &entry : existingLogEntries) {
            bool valid = false;
            const auto index = entry.split(QStringLiteral(".")).last().toInt(&valid);
            if (!valid) {
                continue;
            }

            const QString entryFullPath = logDirectory.absoluteFilePath(entry);
            const QFileInfo info(entryFullPath);
            if (info.exists()) {
                QFile existingFile(entryFullPath);
                if (index > 3) {
                    existingFile.remove();
                    continue;
                }
                const auto &newName = logDirectory.absoluteFilePath(QStringLiteral("%1.%2").arg(filename, QString::number(index + 1)));
                const auto success = existingFile.copy(newName);
                if (success) {
                    existingFile.remove();
                } else {
                    qFatal("Cannot move log file '%s' to '%s': %s",
                           qUtf8Printable(existingFile.fileName()),
                           qUtf8Printable(newName),
                           qUtf8Printable(existingFile.errorString()));
                }
            }
        }

        file.setFileName(logDirectory.absoluteFilePath(filename + QStringLiteral(".0")));
        file.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    }

private:
    QMutex mutex;
    QFile file;
};

Q_GLOBAL_STATIC(Logger, logger)

void handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        logger()->log(type, context, message);
        break;
    case QtFatalMsg:
        logger()->log(QtCriticalMsg, context, message);
        abort();
    }
}

void initializeLogging()
{
    logger()->initialize();
    qInstallMessageHandler(handler);
}