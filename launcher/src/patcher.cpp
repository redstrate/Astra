#include "patcher.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QStandardPaths>
#include <physis.hpp>
#include <utility>

#include "launchercore.h"

Patcher::Patcher(LauncherCore &launcher, QString baseDirectory, BootData *boot_data, QObject *parent)
    : QObject(parent)
    , baseDirectory(std::move(baseDirectory))
    , boot_data(boot_data)
    , m_launcher(launcher)
{
    Q_EMIT m_launcher.stageChanged(i18n("Checking the FINAL FANTASY XIV Updater/Launcher version."));
}

Patcher::Patcher(LauncherCore &launcher, QString baseDirectory, GameData *game_data, QObject *parent)
    : QObject(parent)
    , baseDirectory(std::move(baseDirectory))
    , game_data(game_data)
    , m_launcher(launcher)
{
    Q_EMIT m_launcher.stageChanged(i18n("Checking the FINAL FANTASY XIV Game version."));
}

void Patcher::processPatchList(QNetworkAccessManager &mgr, const QString &patchList)
{
    if (patchList.isEmpty()) {
        emit done();
    } else {
        if (isBoot()) {
            Q_EMIT m_launcher.stageIndeterminate();
            Q_EMIT m_launcher.stageChanged(i18n("Checking the FINAL FANTASY XIV Update/Launcher version."));
        } else {
            Q_EMIT m_launcher.stageIndeterminate();
            Q_EMIT m_launcher.stageChanged(i18n("Checking the FINAL FANTASY XIV Game version."));
        }

        const QStringList parts = patchList.split("\r\n");

        remainingPatches = parts.size() - 7;
        patchQueue.resize(remainingPatches);

        int patchIndex = 0;

        for (int i = 5; i < parts.size() - 2; i++) {
            const QStringList patchParts = parts[i].split("\t");

            const int length = patchParts[0].toInt();
            int ourIndex = patchIndex++;

            const QString version = patchParts[4];
            const long hashBlockSize = patchParts.size() == 9 ? patchParts[6].toLong() : 0;

            const QString name = version;
            const QStringList hashes = patchParts.size() == 9 ? (patchParts[7].split(',')) : QStringList();
            const QString url = patchParts[patchParts.size() == 9 ? 8 : 5];

            qDebug() << "Parsed patch name: " << name;

            auto url_parts = url.split('/');
            const QString repository = url_parts[url_parts.size() - 3];

            const QString patchesDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/patches/" + repository;

            if (!QDir().exists(patchesDir))
                QDir().mkpath(patchesDir);

            if (!QFile::exists(patchesDir + "/" + name + ".patch")) {
                qDebug() << "Need to download " + name;

                QNetworkRequest patchRequest(url);
                auto patchReply = mgr.get(patchRequest);
                connect(patchReply, &QNetworkReply::downloadProgress, [this, repository, version, length](int recieved, int total) {
                    Q_UNUSED(total)

                    if (isBoot()) {
                        Q_EMIT m_launcher.stageChanged(i18n("Updating the FINAL FANTASY XIV Updater/Launcher version.\nDownloading ffxivboot - %1", version));
                    } else {
                        Q_EMIT m_launcher.stageChanged(i18n("Updating the FINAL FANTASY XIV Game version.\nDownloading %1 - %2", repository, version));
                    }

                    Q_EMIT m_launcher.stageDeterminate(0, length, recieved);
                });

                connect(patchReply,
                        &QNetworkReply::finished,
                        [this, ourIndex, patchesDir, name, patchReply, repository, version, hashes, hashBlockSize, length] {
                            QFile file(patchesDir + "/" + name + ".patch");
                            file.open(QIODevice::WriteOnly);
                            file.write(patchReply->readAll());
                            file.close();

                            auto patch_path = patchesDir + "/" + name + ".patch";

                            patchQueue[ourIndex] = {name, repository, version, patch_path, hashes, hashBlockSize, length};

                            remainingPatches--;
                            checkIfDone();
                        });
            } else {
                qDebug() << "Found existing patch: " << name;

                patchQueue[ourIndex] = {name, repository, version, patchesDir + "/" + name + ".patch", hashes, hashBlockSize, length};

                remainingPatches--;
                checkIfDone();
            }
        }
    }
}

void Patcher::checkIfDone()
{
    if (remainingPatches <= 0) {
        if (isBoot()) {
            Q_EMIT m_launcher.stageChanged(i18n("Applying updates to the FINAL FANTASY XIV Updater/Launcher."));
        } else {
            Q_EMIT m_launcher.stageChanged(i18n("Applying updates to the FINAL FANTASY XIV Game."));
        }

        int i = 0;
        for (const auto &patch : patchQueue) {
            Q_EMIT m_launcher.stageDeterminate(0, patchQueue.size(), i++);
            processPatch(patch);
        }

        patchQueue.clear();

        emit done();
    }
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
        physis_bootdata_apply_patch(boot_data, patch.path.toStdString().c_str());
    } else {
        physis_gamedata_apply_patch(game_data, patch.path.toStdString().c_str());
    }

    QString verFilePath;
    if (isBoot()) {
        verFilePath = baseDirectory + "/ffxivboot.ver";
    } else {
        if (patch.repository == "game") {
            verFilePath = baseDirectory + "/ffxivgame.ver";
        } else {
            verFilePath = baseDirectory + "/sqpack/" + patch.repository + "/" + patch.repository + ".ver";
        }
    }

    QFile verFile(verFilePath);
    verFile.open(QIODevice::WriteOnly | QIODevice::Text);
    verFile.write(patch.version.toUtf8());
    verFile.close();
}
