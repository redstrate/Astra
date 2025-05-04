// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCoroTask>
#include <QNetworkAccessManager>
#include <QObject>

class LauncherCore;

class SteamAPI : public QObject
{
public:
    explicit SteamAPI(QObject *parent = nullptr);

    QCoro::Task<> initialize();
    QCoro::Task<> shutdown();
    QCoro::Task<QString> getTicket();

private:
    QNetworkAccessManager m_qnam;
};