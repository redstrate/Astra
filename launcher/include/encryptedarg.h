// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

QString encryptGameArg(const QString &arg);
QString encryptSteamTicket(const QByteArray &ticket, uint32_t time);
