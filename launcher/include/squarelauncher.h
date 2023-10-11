// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qcorotask.h>

#include "launchercore.h"
#include "patcher.h"

class SquareLauncher : public QObject
{
    Q_OBJECT

public:
    explicit SquareLauncher(LauncherCore &window, QObject *parent = nullptr);

    QCoro::Task<> login(const LoginInformation &info);

private:
    using StoredInfo = std::pair<QString, QUrl>;
    QCoro::Task<std::optional<StoredInfo>> getStored(const LoginInformation &info);

    QCoro::Task<> registerSession(const LoginInformation &info);

    static QCoro::Task<QString> getBootHash(const LoginInformation &info);

    Patcher *m_patcher = nullptr;

    QString m_SID, m_username;
    LoginAuth m_auth;

    LauncherCore &m_launcher;
};
