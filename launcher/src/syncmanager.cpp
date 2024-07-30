// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syncmanager.h"
#include "astra_log.h"

#include <Quotient/accountregistry.h>
#include <Quotient/csapi/content-repo.h>
#include <Quotient/csapi/room_state.h>
#include <Quotient/events/stateevent.h>
#include <Quotient/jobs/downloadfilejob.h>
#include <Quotient/room.h>
#include <Quotient/settings.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QCoreApplication>
#include <QCoro>
#include <QTemporaryFile>

const QString roomType = QStringLiteral("zone.xiv.astra-sync");
const QString syncEventType = QStringLiteral("zone.xiv.astra.sync");
const QString lockEventType = QStringLiteral("zone.xiv.astra.lock");

using namespace Quotient;

SyncManager::SyncManager(QObject *parent)
    : QObject(parent)
{
    m_accountRegistry.invokeLogin(); // TODO: port from invokeLogin
    connect(&m_accountRegistry, &AccountRegistry::rowsInserted, this, [this]() {
        connection()->setCacheState(false);
        connection()->setLazyLoading(false);
        connection()->setDirectChatEncryptionDefault(false);
        connection()->setEncryptionDefault(false);

        Q_EMIT connectedChanged();
        Q_EMIT userIdChanged();
        Q_EMIT connectionChanged();
        sync();
    });
    connect(&m_accountRegistry, &AccountRegistry::rowsRemoved, this, [this]() {
        Q_EMIT connectedChanged();
        Q_EMIT userIdChanged();
        Q_EMIT connectionChanged();
    });
}

void SyncManager::login(const QString &matrixId, const QString &password)
{
    auto connection = new Connection(this);
    connection->resolveServer(matrixId);
    connect(
        connection,
        &Connection::loginFlowsChanged,
        this,
        [connection, matrixId, password]() {
            connection->loginWithPassword(matrixId, password, qAppName(), {});
        },
        Qt::SingleShotConnection);

    connect(connection, &Connection::connected, this, [this, connection] {
        qCDebug(ASTRA_LOG) << "Connected!";

        // TODO: store somewhere else, not their QSettings
        AccountSettings account(connection->userId());
        account.setKeepLoggedIn(true);
        account.setHomeserver(connection->homeserver());
        account.setDeviceId(connection->deviceId());
        account.setDeviceName(qAppName());
        account.sync();

        m_accountRegistry.add(connection);
    });

    connect(connection, &Connection::loginError, this, &SyncManager::loginError);
    connect(connection, &Connection::resolveError, this, &SyncManager::loginError);
}

bool SyncManager::connected() const
{
    return !m_accountRegistry.empty();
}

QString SyncManager::userId() const
{
    return !m_accountRegistry.empty() ? m_accountRegistry.accounts().first()->userId() : QString();
}

void SyncManager::logout()
{
    m_accountRegistry.accounts().first()->logout();
}

Quotient::Connection *SyncManager::connection() const
{
    if (!m_accountRegistry.empty()) {
        return m_accountRegistry.accounts().first();
    }
    return nullptr;
}

void SyncManager::sync()
{
    auto connection = m_accountRegistry.accounts().first();
    connection->sync();
    connect(
        connection,
        &Connection::syncDone,
        this,
        [this]() {
            m_accountRegistry.accounts().first()->stopSync();

            qCDebug(ASTRA_LOG) << "Done with sync.";

            // Find the room we need to sync with
            findRoom();
        },
        Qt::SingleShotConnection);
}

QCoro::Task<void> SyncManager::findRoom()
{
    qCDebug(ASTRA_LOG) << "Time to find the sync room!";

    const QString roomId = this->roomId();

    qCDebug(ASTRA_LOG) << "Stored room id:" << roomId;

    // If we have no room id set, we need to find the correct room type
    const bool needsFirstTimeRoom = roomId.isEmpty();

    // Try to find our room
    auto rooms = m_accountRegistry.accounts().first()->rooms(Quotient::JoinState::Join);
    for (auto room : rooms) {
        if (!room->currentState().contains<RoomCreateEvent>()) {
            co_await qCoro(room, &Room::baseStateLoaded);
            qCDebug(ASTRA_LOG) << "Loaded base state...";
        }

        if (needsFirstTimeRoom) {
            const QJsonObject createEvent = room->currentState().eventsOfType(QStringLiteral("m.room.create")).first()->fullJson();
            auto contentJson = createEvent[QStringLiteral("content")].toObject();
            if (contentJson.contains(QStringLiteral("type"))) {
                if (contentJson[QStringLiteral("type")] == roomType) {
                    qCDebug(ASTRA_LOG) << room << "matches!";
                    m_currentRoom = room;
                    setRoomId(room->id());
                    Q_EMIT isReadyChanged();
                }
            }
        } else {
            if (room->id() == roomId) {
                qCDebug(ASTRA_LOG) << "Found pre-existing room!";

                m_currentRoom = room;
                Q_EMIT isReadyChanged();

                co_return;
            }
        }
    }

    // We failed to find a room, and we need to create one
    if (needsFirstTimeRoom && !m_currentRoom) {
        qCDebug(ASTRA_LOG) << "Need to create room!";

        auto job = connection()->createRoom(Quotient::Connection::RoomVisibility::UnpublishRoom,
                                            QString{},
                                            i18n("Astra Sync"),
                                            i18n("Room used to sync Astra between devices"),
                                            QStringList{},
                                            QString{},
                                            QString::number(10),
                                            false,
                                            {},
                                            {},
                                            QJsonObject{{QStringLiteral("type"), roomType}});
        co_await qCoro(job, &BaseJob::finished);

        setRoomId(job->roomId());
        qCDebug(ASTRA_LOG) << "Created sync room at" << job->roomId();

        // re-run sync to get the new room
        sync();
    }

    co_return;
}

QString SyncManager::roomId() const
{
    return KSharedConfig::openStateConfig()->group(QStringLiteral("Sync")).readEntry(QStringLiteral("RoomId"));
}

void SyncManager::setRoomId(const QString &roomId)
{
    auto stateConfig = KSharedConfig::openStateConfig();
    stateConfig->group(QStringLiteral("Sync")).writeEntry(QStringLiteral("RoomId"), roomId);
    stateConfig->sync();
}

bool SyncManager::isReady() const
{
    return connected() && m_currentRoom;
}

QCoro::Task<std::optional<SyncManager::PreviousCharacterData>> SyncManager::getUploadedCharacterData(const QString &id)
{
    Q_ASSERT(m_currentRoom);

    const auto syncEvent = m_currentRoom->currentState().contentJson(syncEventType, id);
    if (syncEvent.isEmpty()) {
        qCDebug(ASTRA_LOG) << "No previous sync for" << id;
        co_return std::nullopt;
    } else {
        qCDebug(ASTRA_LOG) << "previous sync event:" << syncEvent;
        co_return PreviousCharacterData{.mxcUri = syncEvent[QStringLiteral("content-uri")].toString(),
                                        .hostname = syncEvent[QStringLiteral("hostname")].toString()};
    }
}

QCoro::Task<bool> SyncManager::uploadedCharacterData(const QString &id, const QString &path)
{
    Q_ASSERT(m_currentRoom);

    auto uploadFileJob = connection()->uploadFile(path);
    co_await qCoro(uploadFileJob, &BaseJob::finished);

    // TODO: error handling

    const QUrl contentUri = uploadFileJob->contentUri();

    auto syncSetState = m_currentRoom->setState(
        syncEventType,
        id,
        QJsonObject{{{QStringLiteral("content-uri"), contentUri.toString()}, {QStringLiteral("hostname"), QSysInfo::machineHostName()}}});
    co_await qCoro(syncSetState, &BaseJob::finished);

    co_return true;
}

QCoro::Task<bool> SyncManager::downloadCharacterData(const QString &mxcUri, const QString &destPath)
{
    auto job = connection()->downloadFile(QUrl::fromUserInput(mxcUri), destPath);
    co_await qCoro(job, &BaseJob::finished);

    // TODO: error handling

    co_return true;
}

QCoro::Task<std::optional<QString>> SyncManager::checkLock()
{
    const auto lockEvent = m_currentRoom->currentState().contentJson(syncEventType, QStringLiteral("latest"));
    if (lockEvent.isEmpty()) {
        co_return std::nullopt;
    }

    qCDebug(ASTRA_LOG) << "previous lock event:" << lockEvent;
    const QString hostname = lockEvent[QStringLiteral("hostname")].toString();
    if (hostname == QStringLiteral("none")) {
        co_return std::nullopt;
    }

    co_return hostname;
}

QCoro::Task<> SyncManager::setLock()
{
    auto lockSetState =
        m_currentRoom->setState(syncEventType, QStringLiteral("latest"), QJsonObject{{QStringLiteral("hostname"), QSysInfo::machineHostName()}});
    co_await qCoro(lockSetState, &BaseJob::finished);
    co_return;
}

QCoro::Task<> SyncManager::breakLock()
{
    auto lockSetState = m_currentRoom->setState(syncEventType, QStringLiteral("latest"), QJsonObject{{QStringLiteral("hostname"), QStringLiteral("none")}});
    co_await qCoro(lockSetState, &BaseJob::finished);
    co_return;
}

#include "moc_syncmanager.cpp"
