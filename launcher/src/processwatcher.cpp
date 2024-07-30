// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processwatcher.h"

#include <KProcessList>

#include "moc_processwatcher.cpp"

ProcessWatcher::ProcessWatcher(const int PID)
{
    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, [this, PID] {
        const auto info = KProcessList::processInfo(PID);
        // If we can't find the PID, bail!
        if (!info.isValid()) {
            m_timer->stop();
            deleteLater();
            Q_EMIT finished();
        }
    });
    m_timer->setInterval(std::chrono::seconds(30));
    m_timer->start();
}