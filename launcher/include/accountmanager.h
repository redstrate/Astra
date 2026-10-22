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

public:
    explicit AccountManager(QObject *parent = nullptr);

    void load();

    enum CustomRoles {
        AccountRole = Qt::UserRole,
    };

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Account *createSquareEnixAccount(const QString &username, int licenseType, bool isFreeTrial);

    [[nodiscard]] Account *getByUuid(const QString &uuid) const;

    Q_INVOKABLE bool canDelete() const;
    Q_INVOKABLE void deleteAccount(Account *account);

    static QString getDefaultWinePrefixPath(const QString &uuid);

Q_SIGNALS:
    void accountAdded(Account *account);
    void accountLodestoneIdChanged(Account *account);

private:
    void insertAccount(Account *account);

    QList<Account *> m_accounts;
};
