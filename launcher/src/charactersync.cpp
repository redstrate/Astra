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

QCoro::Task<bool> CharacterSync::sync()
{
    if (!launcher.settings()->enableSync()) {
        co_return true;
    }

    qInfo() << "A";

    auto syncManager = launcher.syncManager();
    if (!syncManager->connected()) {
        qInfo() << "B";
        // TODO: provide an option to continue in the UI
        Q_EMIT launcher.loginError(i18n("Failed to connect to sync server! Please check your sync settings."));
        co_return false;
    }

    qInfo() << "C";

    if (!syncManager->isReady()) {
        Q_EMIT launcher.stageChanged(i18n("Waiting for sync connection..."));

        // NOTE: probably does not handle errors well?
        co_await qCoro(syncManager, &SyncManager::isReadyChanged);
    }

    Q_EMIT launcher.stageChanged(i18n("Synchronizing character data..."));

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
        if (!previousData.has_value()) {
            // if we didn't upload character data yet, upload it now
            co_await uploadCharacterData(dir.absoluteFilePath(), id);
        } else {
            // otherwise, download it

            // but check first if it's our hostname
            if (QSysInfo::machineHostName() == previousData->hostname) {
                qCDebug(ASTRA_LOG) << "Skipping! We uploaded this data.";
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