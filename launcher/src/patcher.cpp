#include "patcher.h"

#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QStandardPaths>
#include <physis.hpp>
#include <utility>

Patcher::Patcher(QString baseDirectory, BootData *boot_data, QObject *parent)
    : QObject(parent)
    , baseDirectory(std::move(baseDirectory))
    , boot_data(boot_data)
{
    /*dialog = new QProgressDialog();
    dialog->setLabelText("Checking the FINAL FANTASY XIV Updater/Launcher version.");

    dialog->show();*/
}

Patcher::Patcher(QString baseDirectory, GameData *game_data, QObject *parent)
    : QObject(parent)
    , baseDirectory(std::move(baseDirectory))
    , game_data(game_data)
{
    /*dialog = new QProgressDialog();
    dialog->setLabelText("Checking the FINAL FANTASY XIV Game version.");

    dialog->show();*/
}

void Patcher::processPatchList(QNetworkAccessManager &mgr, const QString &patchList)
{
    if (patchList.isEmpty()) {
        // dialog->hide();

        emit done();
    } else {
        if (isBoot()) {
            // dialog->setLabelText("Updating the FINAL FANTASY XIV Updater/Launcher version.");
        } else {
            // dialog->setLabelText("Updating the FINAL FANTASY XIV Game version.");
        }

        const QStringList parts = patchList.split("\r\n");

        remainingPatches = parts.size() - 7;

        for (int i = 5; i < parts.size() - 2; i++) {
            const QStringList patchParts = parts[i].split("\t");

            const int length = patchParts[0].toInt();
            Q_UNUSED(length)

            QString name, url, version, repository;

            if (isBoot()) {
                name = patchParts[4];
                url = patchParts[5];
                version = name;
            } else {
                url = patchParts[8];
                version = patchParts[4];
                name = url.split('/').last().remove(".patch");
            }

            qDebug() << "Parsed patch name: " << name;

            auto url_parts = url.split('/');
            repository = url_parts[url_parts.size() - 3];

            /*if (isBoot()) {
                dialog->setLabelText(
                    "Updating the FINAL FANTASY XIV Updater/Launcher version.\nDownloading ffxivboot - " + version);
            } else {
                dialog->setLabelText(
                    "Updating the FINAL FANTASY XIV Game version.\nDownloading " + repository + " - " + version);
            }

            dialog->setMinimum(0);
            dialog->setMaximum(length);*/

            const QString patchesDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/patches/" + repository;

            if (!QDir().exists(patchesDir))
                QDir().mkpath(patchesDir);

            if (!QFile::exists(patchesDir + "/" + name + ".patch")) {
                qDebug() << "Need to download " + name;

                QNetworkRequest patchRequest(url);
                auto patchReply = mgr.get(patchRequest);
                connect(patchReply, &QNetworkReply::downloadProgress, [=](int recieved, int total) {
                    Q_UNUSED(recieved)
                    Q_UNUSED(total)
                    // dialog->setValue(recieved);
                });

                connect(patchReply, &QNetworkReply::finished, [this, patchesDir, name, patchReply, repository, version] {
                    QFile file(patchesDir + "/" + name + ".patch");
                    file.open(QIODevice::WriteOnly);
                    file.write(patchReply->readAll());
                    file.close();

                    auto patch_path = patchesDir + "/" + name + ".patch";

                    patchQueue.push_back({name, repository, version, patch_path});

                    remainingPatches--;
                    checkIfDone();
                });
            } else {
                qDebug() << "Found existing patch: " << name;

                patchQueue.push_back({name, repository, version, patchesDir + "/" + name + ".patch"});

                remainingPatches--;
                checkIfDone();
            }
        }
    }
}

void Patcher::checkIfDone()
{
    if (remainingPatches <= 0) {
        for (const auto &patch : patchQueue) {
            processPatch(patch);
        }

        patchQueue.clear();

        // dialog->hide();

        emit done();
    }
}

void Patcher::processPatch(const QueuedPatch &patch)
{
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
