// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility.h"
#include "astra_http_log.h"

#include <QSslConfiguration>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

QString Utility::toWindowsPath(const QDir &dir)
{
    return QStringLiteral("Z:") + dir.absolutePath().replace('/'_L1, '\\'_L1);
}

void Utility::printRequest(const QString &type, const QNetworkRequest &request)
{
    qDebug(ASTRA_HTTP) << type.toUtf8().constData() << request.url().toDisplayString();
}

void Utility::createPathIfNeeded(const QDir &dir)
{
    if (!QDir().exists(dir.absolutePath())) {
        QDir().mkpath(dir.absolutePath());
    }
}

void Utility::setSSL(QNetworkRequest &request)
{
    QSslConfiguration config;
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(config);
}

QString Utility::readVersion(const QString &path)
{
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);

    return QString::fromUtf8(file.readAll());
}

void Utility::writeVersion(const QString &path, const QString &version)
{
    QFile verFile(path);
    verFile.open(QIODevice::WriteOnly | QIODevice::Text);
    verFile.write(version.toUtf8());
    verFile.close();
}