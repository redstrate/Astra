// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <Quotient/accountregistry.h>
#include <Quotient/connection.h>
#include <Task>

/**
 * @brief Handles setting up the connection to Matrix and all of the fun things needed to do for that.
 * Does NOT handle the actual synchronization process, see @c CharacterSync.
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
     * @brief Log in to a connection
     * @param matrixId user id in the form @user:server.tld
     * @param password
     */
    Q_INVOKABLE void login(const QString &matrixId, const QString &password);

    /**
     * @brief Log out of the connection
     */
    Q_INVOKABLE void logout();

    /**
     * @brief Run a single sync. We're not syncing constantly, since we typically don't need it and it consumes a lot of data.
     */
    Q_INVOKABLE QCoro::Task<> sync();

    /**
     * @return Whether there is a connection to the server.
     */
    bool connected() const;

    /**
     * @return The currently logged in user.
     */
    QString userId() const;

    /**
     * @return The LibQuotient connection.
     */
    Quotient::Connection *connection() const;

    /**
     * @return If we're ready to begin downloading or uploading data
     */
    bool isReady() const;

    struct PreviousCharacterData {
        QString mxcUri;
        QString hostname;
    };

    /**
     * @return The currently uploaded character data, or nullopt if there's none for @p id.
     */
    QCoro::Task<std::optional<PreviousCharacterData>> getUploadedCharacterData(const QString &id);

    /**
     * @brief Uploads character data for @p id from @p path.
     * @return True if uploaded successfuly, false otherwise.
     */
    QCoro::Task<bool> uploadedCharacterData(const QString &id, const QString &path);

    /**
     * @brief Downloads the character data archive from @p mxcUri and extracts it in @p destPath.
     */
    QCoro::Task<bool> downloadCharacterData(const QString &mxcUri, const QString &destPath);

    /**
     * @brief Checks if there's a lock.
     */
    QCoro::Task<std::optional<QString>> checkLock();

    /**
     * @brief Sets the sync to the device's hostname.
     */
    QCoro::Task<> setLock();

    /**
     * @brief Breaks the current sync lock.
     */
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
    QCoro::Task<> findRoom();
    QCoro::Task<> beginInitialSync();

    Quotient::AccountRegistry m_accountRegistry;

    Quotient::Room *m_currentRoom = nullptr;
};