// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>

#include "processwatcher.h"

class ProcessWatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testWatcher()
    {
        QProcess process;
        process.setProgram(QStringLiteral("echo"));
        process.start();

        const auto watcher = new ProcessWatcher(process.processId());

        const QSignalSpy spy(watcher, &ProcessWatcher::finished);

        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        process.kill();

        QTRY_COMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(ProcessWatcherTest)
#include "processwatchertest.moc"
