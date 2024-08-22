// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>

#include "account.h"

class AccountManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.accountManager")

    Q_PROPERTY(int numAccounts READ numAccounts NOTIFY accountsChanged)

public:
    explicit AccountManager(LauncherCore &launcher, QObject *parent = nullptr);

    void load();

    enum CustomRoles {
        AccountRole = Qt::UserRole,
    };

    [[nodiscard]] int rowCount(const QModelIndex &index) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Account *createSquareEnixAccount(const QString &username, int licenseType, bool isFreeTrial);
    Q_INVOKABLE Account *createSapphireAccount(const QString &lobbyUrl, const QString &username);

    [[nodiscard]] Account *getByUuid(const QString &uuid) const;

    Q_INVOKABLE bool canDelete(const Account *account) const;
    Q_INVOKABLE void deleteAccount(Account *account);

    Q_INVOKABLE [[nodiscard]] bool hasAnyAccounts() const;
    Q_INVOKABLE [[nodiscard]] int numAccounts() const;

Q_SIGNALS:
    void accountsChanged();
    void accountAdded(Account *account);
    void accountLodestoneIdChanged(Account *account);

private:
    void insertAccount(Account *account);

    QList<Account *> m_accounts;

    LauncherCore &m_launcher;
};