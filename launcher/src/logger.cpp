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
        const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir logDirectory = dataDir.absoluteFilePath(QStringLiteral("log"));
        Utility::createPathIfNeeded(logDirectory);

        // Sort them from highest to lowest (4, 3, 2, 1, 0)
        auto existingLogEntries = logDirectory.entryList({QStringLiteral("astra.*.log")});
        std::sort(existingLogEntries.begin(), existingLogEntries.end(), [](const auto &left, const auto &right) {
            auto leftIndex = left.split(QStringLiteral("."))[1].toInt();
            auto rightIndex = right.split(QStringLiteral("."))[1].toInt();
            return leftIndex > rightIndex;
        });

        // Iterate through the existing logs, prune them and move the oldest up
        for (const auto &entry : existingLogEntries) {
            bool valid = false;
            const auto index = entry.split(QStringLiteral("."))[1].toInt(&valid);
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
                const auto &newName = logDirectory.absoluteFilePath(QStringLiteral("astra.%1.log").arg(QString::number(index + 1)));
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

        file.setFileName(logDirectory.absoluteFilePath(QStringLiteral("astra.0.log")));
        file.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    }

private:
    QMutex mutex;
    QFile file;
};

Q_GLOBAL_STATIC(Logger, logger)

void handler(const QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (!logger.exists()) {
        return;
    }
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
