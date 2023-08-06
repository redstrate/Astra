// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>

#include "account.h"

class AccountManager : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AccountManager(LauncherCore &launcher, QObject *parent = nullptr);

    void load();

    enum CustomRoles {
        AccountRole = Qt::UserRole,
    };

    int rowCount(const QModelIndex &index = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Account *createSquareEnixAccount(const QString &username, int licenseType, bool isFreeTrial);
    Q_INVOKABLE Account *createSapphireAccount(const QString &lobbyUrl, const QString &username);

    Account *getByUuid(const QString &uuid) const;

    Q_INVOKABLE bool canDelete(Account *account) const;
    Q_INVOKABLE void deleteAccount(Account *account);

Q_SIGNALS:

private:
    void insertAccount(Account *account);

    QVector<Account *> m_accounts;

    LauncherCore &m_launcher;
};