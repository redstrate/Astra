// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTimer>

/// Listens and waits for a process to finish.
class ProcessWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWatcher(qint64 PID, QObject *parent = nullptr);

Q_SIGNALS:
    void finished();

private:
    QTimer *m_timer = nullptr;
};