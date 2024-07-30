// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <Quotient/accountregistry.h>
#include <Quotient/connection.h>
#include <Task>

/**
 * @brief Handles setting up the connection to Matrix and all of the fun things needed to do for that.
 * Does NOT handle the actual synchronization process, see @c CharacterSync. That handles determining the files to sync and whatnot.
 */
class SyncManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)
    Q_PROPERTY(Quotient::Connection *connection READ connection NOTIFY connectionChanged)

public:
    explicit SyncManager(QObject *parent = nullptr);

    /**
     * Log in to a connection
     * @param matrixId user id in the form @user:server.tld
     * @param password
     */
    Q_INVOKABLE void login(const QString &matrixId, const QString &password);

    /**
     * Log out of the connection
     */
    Q_INVOKABLE void logout();

    /**
     * Run a single sync. We're not syncing constantly, since we typically don't need it and it consumes a lot of data
     */
    Q_INVOKABLE void sync();

    bool connected() const;
    QString userId() const;
    Quotient::Connection *connection() const;

    /// If we're ready to begin downloading or uploading data
    bool isReady() const;

    struct PreviousCharacterData {
        QString mxcUri;
        QString hostname;
    };

    /// Returns a content repo URI, or nullopt if there's existing character data or not respectively
    QCoro::Task<std::optional<PreviousCharacterData>> getUploadedCharacterData(const QString &id);

    /// Uploads character data for @p id from @p path (a file)
    QCoro::Task<bool> uploadedCharacterData(const QString &id, const QString &path);

    /// Downloads character data
    QCoro::Task<bool> downloadCharacterData(const QString &mxcUri, const QString &destPath);

    /// Checks the lock on the sync
    QCoro::Task<std::optional<QString>> checkLock();

    /// Sets the sync lock to the device's hostname
    QCoro::Task<> setLock();

    /// Breaks the sync lock
    QCoro::Task<> breakLock();

Q_SIGNALS:
    void connectedChanged();
    void userIdChanged();
    void connectionChanged();
    void isReadyChanged();
    void loginError(const QString &message);

private:
    QString roomId() const;
    void setRoomId(const QString &roomId);
    QCoro::Task<void> findRoom();

    Quotient::AccountRegistry m_accountRegistry;

    Quotient::Room *m_currentRoom = nullptr;
};