// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "charactersync.h"

#include <KLocalizedString>
#include <KZip>
#include <QCoro>

#include "astra_log.h"
#include "syncmanager.h"

CharacterSync::CharacterSync(Account &account, LauncherCore &launcher, QObject *parent)
    : launcher(launcher)
    , m_account(account)
{
}

QCoro::Task<bool> CharacterSync::sync(const bool initialSync)
{
    if (!launcher.settings()->enableSync()) {
        co_return true;
    }

    auto syncManager = launcher.syncManager();
    if (!syncManager->connected()) {
        qInfo() << "B";
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

        // TODO: make this a little bit smarter. We shouldn't waste time re-uploading data that's exactly the same.
        if (!initialSync || !previousData.has_value()) {
            // if we didn't upload character data yet, upload it now
            co_await uploadCharacterData(dir.absoluteFilePath(), id);
        } else {
            // otherwise, download it

            const bool exists = QFile::exists(dir.absoluteFilePath() + QStringLiteral("/GEARSET.DAT"));

            // but check first if it's our hostname. only skip if it exists
            if (exists && QSysInfo::machineHostName() == previousData->hostname) {
                qCDebug(ASTRA_LOG) << "Skipping" << id << "We uploaded this data.";
                continue;
            }

            co_await downloadCharacterData(dir.absoluteFilePath(), id, previousData->mxcUri);
        }
    }

    co_return true;
}

QCoro::Task<void> CharacterSync::uploadCharacterData(const QDir &dir, const QString &id)
{
    qCDebug(ASTRA_LOG) << "Uploading" << dir << id;
    QTemporaryDir tempDir;

    auto tempZipPath = tempDir.filePath(QStringLiteral("%1.zip").arg(id));

    KZip *zip = new KZip(tempZipPath);
    zip->setCompression(KZip::DeflateCompression);
    zip->open(QIODevice::WriteOnly);

    QFile gearsetFile(dir.filePath(QStringLiteral("GEARSET.DAT")));
    gearsetFile.open(QFile::ReadOnly);

    zip->writeFile(QStringLiteral("GEARSET.DAT"), gearsetFile.readAll());
    zip->close();

    co_await launcher.syncManager()->uploadedCharacterData(id, tempZipPath);
    // TODO: error handling

    co_return;
}

QCoro::Task<void> CharacterSync::downloadCharacterData(const QDir &dir, const QString &id, const QString &contentUri)
{
    QTemporaryDir tempDir;

    auto tempZipPath = tempDir.filePath(QStringLiteral("%1.zip").arg(id));

    co_await launcher.syncManager()->downloadCharacterData(contentUri, tempZipPath);

    KZip *zip = new KZip(tempZipPath);
    zip->setCompression(KZip::DeflateCompression);
    zip->open(QIODevice::ReadOnly);

    qCDebug(ASTRA_LOG) << "contents:" << zip->directory()->entries();

    zip->directory()->file(QStringLiteral("GEARSET.DAT"))->copyTo(dir.absolutePath());

    qCDebug(ASTRA_LOG) << "Extracted character data!";

    zip->close();

    co_return;
}

#include "moc_charactersync.cpp"