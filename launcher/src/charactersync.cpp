// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "charactersync.h"

#include <KLocalizedString>
#include <KZip>
#include <qcorosignal.h>
#include <qcorotask.h>

#include "astra_log.h"
#include "syncmanager.h"

const auto gearsetFilename = QStringLiteral("GEARSET.DAT");

CharacterSync::CharacterSync(Account &account, LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , launcher(launcher)
    , m_account(account)
{
}

QCoro::Task<bool> CharacterSync::sync(const bool initialSync)
{
    if (!launcher.settings()->enableSync()) {
        co_return true;
    }

    const auto syncManager = launcher.syncManager();
    if (!syncManager->connected()) {
        // TODO: provide an option to continue in the UI
        Q_EMIT launcher.loginError(i18n("Failed to connect to sync server! Please check your sync settings."));
        co_return false;
    }

    if (!syncManager->isReady()) {
        Q_EMIT launcher.stageChanged(i18n("Waiting for sync connection..."));

        // NOTE: probably does not handle errors well?
        co_await qCoro(syncManager, &SyncManager::isReadyChanged);
    }

    Q_EMIT launcher.stageChanged(i18n("Synchronizing character data..."));

    // Perform a manual sync just in case
    co_await syncManager->sync();

    // On game boot, check if we need the lock. Otherwise break it when we clean up.
    if (initialSync) {
        if (const auto hostname = co_await syncManager->checkLock(); hostname.has_value()) {
            // Don't warn about our own failures
            if (hostname != QSysInfo::machineHostName()) {
                Q_EMIT launcher.loginError(i18n("Device %1 has not yet uploaded it's character data. Astra will not continue until that device is re-synced."));
                co_return false;
            }
        }

        syncManager->setLock();
    } else {
        syncManager->breakLock();
    }

    // so first, we need to list the character folders
    // we sync each one separately
    QList<QFileInfo> characterDirs;

    const QDir configPath = m_account.getConfigPath();
    qCDebug(ASTRA_LOG) << "Searching for characters in" << configPath;

    QDirIterator configIterator(configPath.absolutePath());
    while (configIterator.hasNext()) {
        const auto fileInfo = configIterator.nextFileInfo();
        if (fileInfo.isDir() && fileInfo.fileName().startsWith(QStringLiteral("FFXIV_"))) {
            characterDirs.append(fileInfo);
        }
    }

    qCDebug(ASTRA_LOG) << "Character directories:" << characterDirs;

    for (const auto &dir : characterDirs) {
        const QString id = dir.fileName(); // FFXIV_CHR0040000001000001 for example
        const auto previousData = co_await syncManager->getUploadedCharacterData(id);

        // The files are packed into an archive. So if only one of the files doesn't exist or fails the hash check, download the whole thing and overwrite.
        bool areFilesDifferent = false;
        for (const auto &[file, hash] : previousData->fileHashes.asKeyValueRange()) {
            QFile existingFile(QDir(dir.absoluteFilePath()).absoluteFilePath(file));
            if (!existingFile.exists()) {
                areFilesDifferent = true;
                qCDebug(ASTRA_LOG) << id << "does not match locally, reason:" << existingFile.fileName() << "does not exist";
                break;
            }

            existingFile.open(QIODevice::ReadOnly);
            const auto existingHash = QString::fromUtf8(QCryptographicHash::hash(existingFile.readAll(), QCryptographicHash::Algorithm::Sha256).toHex());
            if (existingHash != hash) {
                areFilesDifferent = true;
                qCDebug(ASTRA_LOG) << id << "does not match locally, reason: hashes do not match for" << file;
                break;
            }
        }

        const bool hasPreviousUpload = !previousData.has_value();
        const bool isGameClosing = !initialSync;

        // We want to upload if the files are truly different, or there is no existing data on the server.
        const bool needsUpload = (areFilesDifferent && isGameClosing) || hasPreviousUpload;

        // We want to download if the files are different.
        const bool needsDownload = areFilesDifferent;

        if (needsUpload) {
            // if we didn't upload character data yet, upload it now
            co_await uploadCharacterData(dir.absoluteFilePath(), id);
        } else if (needsDownload) {
            co_await downloadCharacterData(dir.absoluteFilePath(), id, previousData->mxcUri);
        }
    }

    co_return true;
}

QCoro::Task<void> CharacterSync::uploadCharacterData(const QDir &dir, const QString &id)
{
    qCDebug(ASTRA_LOG) << "Uploading" << dir << id;
    const QTemporaryDir tempDir;

    const auto tempZipPath = tempDir.filePath(QStringLiteral("%1.zip").arg(id));

    const auto zip = new KZip(tempZipPath);
    zip->setCompression(KZip::DeflateCompression);
    zip->open(QIODevice::WriteOnly);

    QFile gearsetFile(dir.filePath(gearsetFilename));
    gearsetFile.open(QFile::ReadOnly);

    const auto data = gearsetFile.readAll();

    zip->writeFile(gearsetFilename, data);
    zip->close();

    QMap<QString, QString> fileHashes;
    fileHashes[gearsetFilename] = QString::fromUtf8(QCryptographicHash::hash(data, QCryptographicHash::Algorithm::Sha256).toHex());

    co_await launcher.syncManager()->uploadCharacterArchive(id, tempZipPath, fileHashes);
    // TODO: error handling

    co_return;
}

QCoro::Task<void> CharacterSync::downloadCharacterData(const QDir &dir, const QString &id, const QString &contentUri)
{
    const QTemporaryDir tempDir;

    const auto tempZipPath = tempDir.filePath(QStringLiteral("%1.zip").arg(id));

    co_await launcher.syncManager()->downloadCharacterArchive(contentUri, tempZipPath);

    auto zip = new KZip(tempZipPath);
    zip->setCompression(KZip::DeflateCompression);
    zip->open(QIODevice::ReadOnly);

    qCDebug(ASTRA_LOG) << "contents:" << zip->directory()->entries();

    Q_UNUSED(zip->directory()->file(gearsetFilename)->copyTo(dir.absolutePath()))

    qCDebug(ASTRA_LOG) << "Extracted character data!";

    zip->close();

    co_return;
}

#include "moc_charactersync.cpp"