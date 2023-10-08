// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "patcher.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QtConcurrent>
#include <physis.hpp>
#include <qcorofuture.h>

#include "astra_patcher_log.h"
#include "launchercore.h"
#include "patchlist.h"
#include "utility.h"

Patcher::Patcher(LauncherCore &launcher, const QString &baseDirectory, BootData &bootData, QObject *parent)
    : QObject(parent)
    , m_baseDirectory(baseDirectory)
    , m_bootData(&bootData)
    , m_launcher(launcher)
{
    setupDirectories();

    Q_EMIT m_launcher.stageChanged(i18n("Checking %1 version.", getBaseString()));
}

Patcher::Patcher(LauncherCore &launcher, const QString &baseDirectory, GameData &gameData, QObject *parent)
    : QObject(parent)
    , m_baseDirectory(baseDirectory)
    , m_gameData(&gameData)
    , m_launcher(launcher)
{
    setupDirectories();

    Q_EMIT m_launcher.stageChanged(i18n("Checking %1 version.", getBaseString()));
}

QCoro::Task<bool> Patcher::patch(const PatchList &patchList)
{
    if (patchList.isEmpty()) {
        co_return false;
    }

    Q_EMIT m_launcher.stageIndeterminate();
    Q_EMIT m_launcher.stageChanged(i18n("Checking %1 version.", getBaseString()));

    m_remainingPatches = patchList.patches().size();
    m_patchQueue.resize(m_remainingPatches);

    QFutureSynchronizer<void> synchronizer;

    int patchIndex = 0;

    for (auto &patch : patchList.patches()) {
        const int ourIndex = patchIndex++;

        const QString filename = QStringLiteral("%1.patch").arg(patch.name);
        const QDir repositoryDir = m_patchesDir.absoluteFilePath(patch.repository);

        if (!QDir().exists(repositoryDir.absolutePath()))
            QDir().mkpath(repositoryDir.absolutePath());

        const QString patchPath = repositoryDir.absoluteFilePath(filename);

        const QueuedPatch queuedPatch{patch.name, patch.repository, patch.version, patchPath, patch.hashes, patch.hashBlockSize, patch.length, isBoot()};

        qDebug(ASTRA_PATCHER) << "Adding a queued patch:";
        qDebug(ASTRA_PATCHER) << "- Name:" << patch.name;
        qDebug(ASTRA_PATCHER) << "- Repository or is boot:" << (isBoot() ? QStringLiteral("boot") : patch.repository);
        qDebug(ASTRA_PATCHER) << "- Version:" << patch.version;
        qDebug(ASTRA_PATCHER) << "- Downloaded Path:" << patchPath;
        qDebug(ASTRA_PATCHER) << "- Hashes:" << patch.hashes;
        qDebug(ASTRA_PATCHER) << "- Hash Block Size:" << patch.hashBlockSize;
        qDebug(ASTRA_PATCHER) << "- Length:" << patch.length;

        m_patchQueue[ourIndex] = queuedPatch;

        if (!QFile::exists(patchPath)) {
            const auto patchRequest = QNetworkRequest(patch.url);
            Utility::printRequest(QStringLiteral("GET"), patchRequest);

            auto patchReply = m_launcher.mgr->get(patchRequest);

            connect(patchReply, &QNetworkReply::downloadProgress, this, [this, queuedPatch](int received, int total) {
                Q_EMIT m_launcher.stageChanged(i18n("Updating %1.\nDownloading %2", getBaseString(), queuedPatch.getVersion()));
                Q_EMIT m_launcher.stageDeterminate(0, total, received);
            });

            synchronizer.addFuture(QtFuture::connect(patchReply, &QNetworkReply::finished).then([patchPath, patchReply] {
                QFile file(patchPath);
                file.open(QIODevice::WriteOnly);
                file.write(patchReply->readAll());
                file.close();
            }));
        } else {
            qDebug(ASTRA_PATCHER) << "Found existing patch: " << patch.name;
        }
    }

    co_await QtConcurrent::run([&synchronizer] {
        synchronizer.waitForFinished();
    });

    // This must happen synchronously
    size_t i = 0;
    for (const auto &patch : m_patchQueue) {
        Q_EMIT m_launcher.stageChanged(i18n("Updating %1.\nInstalling %2", getBaseString(), patch.getVersion()));
        Q_EMIT m_launcher.stageDeterminate(0, m_patchQueue.size(), i++);

        processPatch(patch);
    }

    co_return true;
}

void Patcher::processPatch(const QueuedPatch &patch)
{
    // Perform hash checking
    if (!patch.hashes.isEmpty()) {
        auto f = QFile(patch.path);
        f.open(QIODevice::ReadOnly);

        Q_ASSERT(patch.length == f.size());

        const int parts = std::ceil(static_cast<double>(patch.length) / static_cast<double>(patch.hashBlockSize));

        QByteArray block;
        block.resize(patch.hashBlockSize);

        for (int i = 0; i < parts; i++) {
            const auto read = f.read(patch.hashBlockSize);

            if (read.length() <= patch.hashBlockSize) {
                block = read;
            }

            QCryptographicHash hash(QCryptographicHash::Sha1);
            hash.addData(block);

            Q_ASSERT(hash.result().toHex() == patch.hashes[i]);
        }
    }

    if (isBoot()) {
        physis_bootdata_apply_patch(m_bootData, patch.path.toStdString().c_str());
    } else {
        physis_gamedata_apply_patch(m_gameData, patch.path.toStdString().c_str());
    }

    qDebug(ASTRA_PATCHER) << "Installed" << patch.path << "to" << (isBoot() ? QStringLiteral("boot") : patch.repository);

    QString verFilePath;
    if (isBoot()) {
        verFilePath = m_baseDirectory + QStringLiteral("/ffxivboot.ver");
    } else {
        if (patch.repository == QLatin1String("game")) {
            verFilePath = m_baseDirectory + QStringLiteral("/ffxivgame.ver");
        } else {
            verFilePath = m_baseDirectory + QStringLiteral("/sqpack/") + patch.repository + QStringLiteral("/") + patch.repository + QStringLiteral(".ver");
        }
    }

    QFile verFile(verFilePath);
    verFile.open(QIODevice::WriteOnly | QIODevice::Text);
    verFile.write(patch.version.toUtf8());
    verFile.close();
}

void Patcher::setupDirectories()
{
    QDir dataDir;
    if (m_launcher.keepPatches()) {
        dataDir.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    } else {
        dataDir.setPath(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    }

    m_patchesDir.setPath(dataDir.absoluteFilePath(QStringLiteral("patches")));
}

QString Patcher::getBaseString() const
{
    if (isBoot()) {
        return i18n("FINAL FANTASY XIV Update/Launcher");
    } else {
        return i18n("FINAL FANTASY XIV Game");
    }
}
