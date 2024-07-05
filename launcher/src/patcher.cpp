// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "patcher.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QNetworkRequest>
#include <QtConcurrent>
#include <physis.hpp>
#include <qcorofuture.h>

#include "astra_patcher_log.h"
#include "launchercore.h"
#include "patchlist.h"
#include "utility.h"

using namespace Qt::StringLiterals;

Patcher::Patcher(LauncherCore &launcher, const QString &baseDirectory, BootData &bootData, QObject *parent)
    : QObject(parent)
    , m_baseDirectory(baseDirectory)
    , m_bootData(&bootData)
    , m_launcher(launcher)
{
    m_launcher.m_isPatching = true;

    setupDirectories();

    Q_EMIT m_launcher.stageChanged(i18n("Checking %1 version", getBaseString()));
}

Patcher::Patcher(LauncherCore &launcher, const QString &baseDirectory, GameData &gameData, QObject *parent)
    : QObject(parent)
    , m_baseDirectory(baseDirectory)
    , m_gameData(&gameData)
    , m_launcher(launcher)
{
    m_launcher.m_isPatching = true;

    setupDirectories();

    Q_EMIT m_launcher.stageChanged(i18n("Checking %1 version", getBaseString()));
}

Patcher::~Patcher()
{
    m_launcher.m_isPatching = false;
}

QCoro::Task<bool> Patcher::patch(const PatchList &patchList)
{
    if (patchList.isEmpty()) {
        co_return false;
    }

    Q_EMIT m_launcher.stageIndeterminate();
    Q_EMIT m_launcher.stageChanged(i18n("Updating %1", getBaseString()));

    m_remainingPatches = static_cast<int>(patchList.patches().size());
    m_patchQueue.resize(m_remainingPatches);

    QFutureSynchronizer<void> synchronizer;

    int patchIndex = 0;

    for (auto &patch : patchList.patches()) {
        const int ourIndex = patchIndex++;

        const QString filename = QStringLiteral("%1.patch").arg(patch.name);
        const QString tempFilename = QStringLiteral("%1.patch~").arg(patch.name); // tilde afterwards to hide it easily

        const QDir repositoryDir = m_patchesDir.absoluteFilePath(patch.repository);
        Utility::createPathIfNeeded(repositoryDir);

        const QString patchPath = repositoryDir.absoluteFilePath(filename);
        const QString tempPatchPath = repositoryDir.absoluteFilePath(tempFilename);

        const QueuedPatch queuedPatch{.name = patch.name,
                                      .repository = patch.repository,
                                      .version = patch.version,
                                      .path = patchPath,
                                      .hashes = patch.hashes,
                                      .hashBlockSize = patch.hashBlockSize,
                                      .length = patch.length,
                                      .isBoot = isBoot()};

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
            // make sure to remove any previous attempts
            if (QFile::exists(tempPatchPath)) {
                QFile::remove(tempPatchPath);
            }

            const auto patchRequest = QNetworkRequest(QUrl(patch.url));
            Utility::printRequest(QStringLiteral("GET"), patchRequest);

            auto patchReply = m_launcher.mgr()->get(patchRequest);

            connect(patchReply, &QNetworkReply::downloadProgress, this, [this, ourIndex](const int received, const int total) {
                Q_UNUSED(total)
                updateDownloadProgress(ourIndex, received);
            });

            connect(patchReply, &QNetworkReply::readyRead, this, [this, tempPatchPath, patchReply] {
                // TODO: don't open the file each time we recieve data
                QFile file(tempPatchPath);
                file.open(QIODevice::WriteOnly | QIODevice::Append);
                file.write(patchReply->readAll());
                file.close();
            });

            synchronizer.addFuture(QtFuture::connect(patchReply, &QNetworkReply::finished).then([this, ourIndex, patchPath, tempPatchPath] {
                qDebug(ASTRA_PATCHER) << "Downloaded to" << patchPath;

                QDir().rename(tempPatchPath, patchPath);

                QMutexLocker locker(&m_finishedPatchesMutex);
                m_finishedPatches++;
                m_patchQueue[ourIndex].downloaded = true;

                updateMessage();
            }));
        } else {
            m_patchQueue[ourIndex].downloaded = true;
            m_finishedPatches++;
            qDebug(ASTRA_PATCHER) << "Found existing patch: " << patch.name;
        }
    }

    co_await QtConcurrent::run([&synchronizer] {
        synchronizer.waitForFinished();
    });

    // This must happen synchronously
    int i = 0;
    for (const auto &patch : m_patchQueue) {
        QString repositoryName = patch.repository;
        if (repositoryName == QStringLiteral("game")) {
            repositoryName = QStringLiteral("ffxiv");
        }

        Q_EMIT m_launcher.stageChanged(i18n("Installing %1 - %2 [%3/%4]", repositoryName, patch.version, i++, m_remainingPatches));
        Q_EMIT m_launcher.stageDeterminate(0, static_cast<int>(m_patchQueue.size()), i);

        co_await QtConcurrent::run([this, patch] {
            processPatch(patch);
        });
    }

    co_return true;
}

void Patcher::processPatch(const QueuedPatch &patch)
{
    // Perform hash checking
    if (!patch.hashes.isEmpty()) {
        auto f = QFile(patch.path);
        f.open(QIODevice::ReadOnly);

        qDebug(ASTRA_PATCHER) << "Installing" << patch.path;

        if (patch.length != f.size()) {
            f.remove();
            qCritical(ASTRA_PATCHER) << patch.path << "has the wrong size.";
            Q_EMIT m_launcher.miscError(i18n("Patch %1 is the wrong size. The downloaded patch has been discarded, please log in again.", patch.name));
            return;
        }

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

            if (QString::fromUtf8(hash.result().toHex()) != patch.hashes[i]) {
                f.remove();
                qCritical(ASTRA_PATCHER) << patch.path << "failed the hash check.";
                Q_EMIT m_launcher.miscError(i18n("Patch %1 failed the hash check. The downloaded patch has been discarded, please log in again.", patch.name));
                return;
            }
        }
    }

    bool res;
    if (isBoot()) {
        res = physis_bootdata_apply_patch(m_bootData, patch.path.toStdString().c_str());
    } else {
        res = physis_gamedata_apply_patch(m_gameData, patch.path.toStdString().c_str());
    }

    if (!res) {
        qCritical(ASTRA_PATCHER) << "Failed to install" << patch.path << "to" << (isBoot() ? QStringLiteral("boot") : patch.repository);
        Q_EMIT m_launcher.miscError(i18n("Patch %1 failed to apply. The game is now in an invalid state and must be immediately repaired.", patch.name));
        return;
    }

    qDebug(ASTRA_PATCHER) << "Installed" << patch.path << "to" << (isBoot() ? QStringLiteral("boot") : patch.repository);

    QString verFilePath;
    if (isBoot()) {
        verFilePath = m_baseDirectory + QStringLiteral("/ffxivboot.ver");
    } else {
        if (patch.repository == "game"_L1) {
            verFilePath = m_baseDirectory + QStringLiteral("/ffxivgame.ver");
        } else {
            const QString sqPackDir = m_baseDirectory + QStringLiteral("/sqpack/") + patch.repository + QStringLiteral("/");
            Utility::createPathIfNeeded(sqPackDir);
            verFilePath = sqPackDir + patch.repository + QStringLiteral(".ver");
        }
    }

    Utility::writeVersion(verFilePath, patch.version);

    if (!m_launcher.settings()->keepPatches()) {
        QFile::remove(patch.path);
    }
}

void Patcher::setupDirectories()
{
    QDir dataDir;
    dataDir.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    m_patchesDir.setPath(dataDir.absoluteFilePath(QStringLiteral("patch")));
}

QString Patcher::getBaseString() const
{
    if (isBoot()) {
        return i18n("FINAL FANTASY XIV Update/Launcher");
    } else {
        return i18n("FINAL FANTASY XIV Game");
    }
}

void Patcher::updateDownloadProgress(const int index, const int received)
{
    QMutexLocker locker(&m_finishedPatchesMutex);

    m_patchQueue[index].bytesDownloaded = received;

    updateMessage();
}

void Patcher::updateMessage()
{
    // Find first not-downloaded patch
    for (const auto &patch : m_patchQueue) {
        if (!patch.downloaded) {
            QString repositoryName = patch.repository;
            if (repositoryName == QStringLiteral("game")) {
                repositoryName = QStringLiteral("ffxiv");
            }

            const float progress = (static_cast<float>(patch.bytesDownloaded) / static_cast<float>(patch.length)) * 100.0f;
            const QString progressStr = QStringLiteral("%1").arg(progress, 1, 'f', 1, QLatin1Char('0'));

            Q_EMIT m_launcher.stageChanged(i18n("Downloading %1 - %2 [%3/%4]", repositoryName, patch.version, m_finishedPatches, m_remainingPatches),
                                           i18n("%1%", progressStr));
            Q_EMIT m_launcher.stageDeterminate(0, static_cast<int>(patch.length), static_cast<int>(patch.bytesDownloaded));
            return;
        }
    }
}

#include "moc_patcher.cpp"