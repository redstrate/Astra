// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processwatcher.h"

#include <KProcessList>

#include "moc_processwatcher.cpp"

ProcessWatcher::ProcessWatcher(const qint64 PID, QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this, PID] {
        const auto info = KProcessList::processInfo(PID);
        // If we can't find the PID, bail!
        if (!info.isValid()) {
            m_timer->stop();
            deleteLater();
            Q_EMIT finished();
        }
    });
    m_timer->setInterval(std::chrono::seconds(5));
    m_timer->start();
}