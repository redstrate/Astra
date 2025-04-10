// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accountmanager.h"
#include "accountconfig.h"
#include "astra_log.h"

#include <KSharedConfig>

using namespace Qt::StringLiterals;

AccountManager::AccountManager(QObject *parent)
    : QAbstractListModel(parent)
{
}

void AccountManager::load()
{
    auto config = KSharedConfig::openStateConfig();
    for (const auto &id : config->groupList()) {
        if (id.contains("account-"_L1)) {
            const QString uuid = QString(id).remove("account-"_L1);
            qInfo(ASTRA_LOG) << "Loading account" << uuid;

            const auto account = new Account(uuid, this);
            m_accounts.append(account);
            Q_EMIT accountsChanged();
            Q_EMIT accountAdded(account);
        }
    }
}

int AccountManager::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return static_cast<int>(m_accounts.size());
}

QVariant AccountManager::data(const QModelIndex &index, const int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    const int row = index.row();
    if (role == AccountRole) {
        return QVariant::fromValue(m_accounts[row]);
    }

    return {};
}

QHash<int, QByteArray> AccountManager::roleNames() const
{
    return {{AccountRole, QByteArrayLiteral("account")}};
}

Account *AccountManager::createSquareEnixAccount(const QString &username, const int licenseType, const bool isFreeTrial)
{
    const auto account = new Account(QUuid::createUuid().toString(), this);
    account->config()->setLicense(static_cast<Account::GameLicense>(licenseType));
    account->config()->setIsFreeTrial(isFreeTrial);
    account->config()->setName(username);
    account->config()->save();

    insertAccount(account);

    return account;
}

Account *AccountManager::getByUuid(const QString &uuid) const
{
    for (const auto &account : m_accounts) {
        if (account->uuid() == uuid) {
            return account;
        }
    }

    return nullptr;
}

bool AccountManager::canDelete(const Account *account) const
{
    Q_UNUSED(account)
    return m_accounts.size() > 1;
}

void AccountManager::deleteAccount(Account *account)
{
    auto config = KSharedConfig::openStateConfig();
    config->deleteGroup(QStringLiteral("account-%1").arg(account->uuid()));
    config->sync();

    const int row = static_cast<int>(m_accounts.indexOf(account));
    beginRemoveRows(QModelIndex(), row, row);
    m_accounts.removeAll(account);
    endRemoveRows();
    Q_EMIT accountsChanged();
}

void AccountManager::insertAccount(Account *account)
{
    connect(account, &Account::avatarUrlChanged, this, [this, account] {
        Q_EMIT accountLodestoneIdChanged(account);
    });
    beginInsertRows(QModelIndex(), static_cast<int>(m_accounts.size()), static_cast<int>(m_accounts.size()));
    m_accounts.append(account);
    endInsertRows();
    Q_EMIT accountsChanged();
    Q_EMIT accountAdded(account);
}

bool AccountManager::hasAnyAccounts() const
{
    return !m_accounts.empty();
}

int AccountManager::numAccounts() const
{
    return static_cast<int>(m_accounts.count());
}

#include "moc_accountmanager.cpp"
