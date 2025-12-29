// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "patcher.h"

#include <KFormat>
#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QNetworkRequest>
#include <QtConcurrent>
#include <physis.hpp>
#include <qcorofuture.h>

#include "astra_patcher_log.h"
#include "launchercore.h"
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

Patcher::Patcher(LauncherCore &launcher, const QString &baseDirectory, SqPackResource &gameData, QObject *parent)
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

QCoro::Task<bool> Patcher::patch(const physis_PatchList &patchList)
{
    if (patchList.num_entries == 0) {
        co_return false;
    }

    // First, let's check if we have enough space to even download the patches
    const qint64 neededSpace = patchList.total_size_downloaded - m_patchesDirStorageInfo.bytesAvailable();
    if (neededSpace > 0) {
        KFormat format;
        QString neededSpaceStr = format.formatByteSize(neededSpace);
        Q_EMIT m_launcher.miscError(
            i18n("There isn't enough space available on disk to update the game. You need %1 more free space to continue.", neededSpaceStr));
        co_return false;
    }

    // Calculate the existing repository sizes, which will be used to have an idea of how much space it will grow by.
    QMap<QString, int64_t> localRepositorySizes;
    for (int i = 0; i < patchList.num_entries; i++) {
        const auto &patch = patchList.entries[i];

        // The "game" repository is written as "ffxiv" locally.
        QString key = Utility::repositoryFromPatchUrl(QLatin1String(patch.url));
        QString folder = Utility::repositoryFromPatchUrl(QLatin1String(patch.url));
        if (key == QStringLiteral("game")) {
            folder = QStringLiteral("ffxiv");
        }

        // Take the movie directory
        const QDir movieDir = m_baseDirectory.absoluteFilePath(QStringLiteral("movie"));
        const QDir repoMovieDir = movieDir.absoluteFilePath(folder);

        // Take the sqpack directory
        const QDir sqPackDir = m_baseDirectory.absoluteFilePath(QStringLiteral("sqpack"));
        const QDir repoSqPackDir = sqPackDir.absoluteFilePath(folder);

        // TODO: other files should be accounted for the game repository, such as the game executable. This isn't too important as it's only ~100 MiB

        auto totalSize = Utility::getDirectorySize(repoSqPackDir.absolutePath()) + Utility::getDirectorySize(repoMovieDir.absolutePath());
        if (totalSize > 0) {
            localRepositorySizes[key] = totalSize;
        } else {
            qWarning() << "Failed to calculate size for" << key << ", ignoring...";
        }
    }

    // If we do, we want to make sure we have enough space for all the repositories we download
    for (int i = 0; i < patchList.num_entries; i++) {
        // Record the largest byte size for the repository
        const auto &patch = patchList.entries[i];
        const auto &key = Utility::repositoryFromPatchUrl(QLatin1String(patch.url));
        if (localRepositorySizes.contains(key)) {
            // Now calculate how much space we need, and cull anything that would be "negative space" which makes no sense.
            auto newSize = localRepositorySizes[key] - patch.size_on_disk;
            if (newSize > 0) {
                localRepositorySizes[key] = newSize;
            } else {
                localRepositorySizes.remove(key);
            }
        }
    }

    int64_t requiredInstallSize = 0;
    for (const auto &[_, value] : localRepositorySizes.asKeyValueRange()) {
        requiredInstallSize += value;
    }
    const qint64 neededInstallSpace = requiredInstallSize - m_baseDirStorageInfo.bytesAvailable();
    if (neededInstallSpace > 0) {
        KFormat format;
        QString neededSpaceStr = format.formatByteSize(neededInstallSpace);
        Q_EMIT m_launcher.miscError(
            i18n("There is not enough space available on disk to update the game. You need %1 more free space to continue.", neededSpaceStr));
        co_return false;
    }

    Q_EMIT m_launcher.stageIndeterminate();
    Q_EMIT m_launcher.stageChanged(i18n("Updating %1", getBaseString()));

    m_remainingPatches = patchList.num_entries;
    m_patchQueue.resize(m_remainingPatches);

    QFutureSynchronizer<void> synchronizer;

    int patchIndex = 0;

    for (int i = 0; i < patchList.num_entries; i++) {
        const auto &patch = patchList.entries[i];

        const int ourIndex = patchIndex++;

        const QString filename = QStringLiteral("%1.patch").arg(QLatin1String(patch.version));
        const QString tempFilename = QStringLiteral("%1.patch~").arg(QLatin1String(patch.version)); // tilde afterward to hide it easily

        const QString repository = Utility::repositoryFromPatchUrl(QLatin1String(patch.url));
        const QDir repositoryDir = m_patchesDir.absoluteFilePath(repository);
        Utility::createPathIfNeeded(repositoryDir);

        const QString patchPath = repositoryDir.absoluteFilePath(filename);
        const QString tempPatchPath = repositoryDir.absoluteFilePath(tempFilename);

        QStringList convertedHashes;
        for (uint64_t i = 0; i < patch.hash_count; i++) {
            convertedHashes.push_back(QLatin1String(patch.hashes[i]));
        }

        const QueuedPatch queuedPatch{.name = QLatin1String(patch.version),
                                      .repository = repository,
                                      .version = QLatin1String(patch.version),
                                      .path = patchPath,
                                      .hashes = convertedHashes,
                                      .hashBlockSize = static_cast<long>(patch.hash_block_size),
                                      .length = static_cast<long>(patch.length),
                                      .isBoot = isBoot()};

        qDebug(ASTRA_PATCHER) << "Adding a queued patch:";
        qDebug(ASTRA_PATCHER) << "- Repository or is boot:" << (isBoot() ? QStringLiteral("boot") : repository);
        qDebug(ASTRA_PATCHER) << "- Version:" << patch.version;
        qDebug(ASTRA_PATCHER) << "- Downloaded Path:" << patchPath;
        qDebug(ASTRA_PATCHER) << "- Hashes:" << patch.hashes;
        qDebug(ASTRA_PATCHER) << "- Hash Block Size:" << patch.hash_block_size;
        qDebug(ASTRA_PATCHER) << "- Length:" << patch.length;

        m_patchQueue[ourIndex] = queuedPatch;

        if (!QFile::exists(patchPath)) {
            // make sure to remove any previous attempts
            if (QFile::exists(tempPatchPath)) {
                QFile::remove(tempPatchPath);
            }

            const auto patchRequest = QNetworkRequest(QUrl(QLatin1String(patch.url)));
            Utility::printRequest(QStringLiteral("GET"), patchRequest);

            auto patchReply = m_launcher.mgr()->get(patchRequest);

            connect(patchReply, &QNetworkReply::downloadProgress, this, [this, ourIndex](const int received, const int total) {
                Q_UNUSED(total)
                updateDownloadProgress(ourIndex, received);
            });

            connect(patchReply, &QNetworkReply::readyRead, this, [tempPatchPath, patchReply] {
                // TODO: don't open the file each time we receive data
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
            qDebug(ASTRA_PATCHER) << "Found existing patch: " << patch.version;
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

        const bool success = co_await QtConcurrent::run([this, patch] {
            return processPatch(patch);
        });
        if (!success) {
            co_return false;
        }
    }

    co_return true;
}

bool Patcher::processPatch(const QueuedPatch &patch)
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
            return false;
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
                return false;
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
        return false;
    }

    qDebug(ASTRA_PATCHER) << "Installed" << patch.path << "to" << (isBoot() ? QStringLiteral("boot") : patch.repository);

    QString verFilePath;
    if (isBoot()) {
        verFilePath = m_baseDirectory.absoluteFilePath(QStringLiteral("ffxivboot.ver"));
    } else {
        if (patch.repository == "game"_L1) {
            verFilePath = m_baseDirectory.absoluteFilePath(QStringLiteral("ffxivgame.ver"));
        } else {
            const QDir sqPackDir = m_baseDirectory.absoluteFilePath(QStringLiteral("sqpack/%1").arg(patch.repository));
            Utility::createPathIfNeeded(sqPackDir);
            verFilePath = sqPackDir.absoluteFilePath(patch.repository + QStringLiteral(".ver"));
        }
    }

    Utility::writeVersion(verFilePath, patch.version);

    if (!m_launcher.config()->keepPatches()) {
        QFile::remove(patch.path);
    }

    return true;
}

void Patcher::setupDirectories()
{
    QDir dataDir;
    dataDir.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    m_patchesDir.setPath(dataDir.absoluteFilePath(QStringLiteral("patch")));
    if (!m_patchesDir.exists()) {
        QDir().mkpath(m_patchesDir.path());
    }

    m_patchesDirStorageInfo = QStorageInfo(m_patchesDir);
    m_baseDirStorageInfo = QStorageInfo(m_baseDirectory);
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
