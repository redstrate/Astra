// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qcorotask.h>

#include "launchercore.h"

class LauncherCore;
class QNetworkReply;

/**
 * @brief Works in tandem with @c SyncManager to synchronizes character data.
 */
class CharacterSync : public QObject
{
    Q_OBJECT

public:
    explicit CharacterSync(Account &account, LauncherCore &launcher, QObject *parent = nullptr);

    /// Checks and synchronizes character files as necessary.
    /// \param initialSync Whether this is the initial sync on game start.
    /// \return False if the synchronization failed.
    QCoro::Task<bool> sync(bool initialSync = true);

private:
    QCoro::Task<void> uploadCharacterData(const QDir &dir, const QString &id);
    QCoro::Task<void> downloadCharacterData(const QDir &dir, const QString &id, const QString &contentUri);

    LauncherCore &launcher;
    Account &m_account;
};
