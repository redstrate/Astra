#include "accountmanager.h"

#include <KSharedConfig>

AccountManager::AccountManager(LauncherCore &launcher, QObject *parent)
    : QAbstractListModel(parent)
    , m_launcher(launcher)
{
}

void AccountManager::load()
{
    auto config = KSharedConfig::openStateConfig();
    for (const auto &id : config->groupList()) {
        if (id.contains("account-")) {
            auto account = new Account(m_launcher, QString(id).remove("account-"), this);
            m_accounts.append(account);
        }
    }
}

int AccountManager::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_accounts.size();
}

QVariant AccountManager::data(const QModelIndex &index, int role) const
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

Account *AccountManager::createSquareEnixAccount(const QString &username, int licenseType, bool isFreeTrial)
{
    auto account = new Account(m_launcher, QUuid::createUuid().toString(), this);
    account->setIsSapphire(false);
    account->setLicense(static_cast<Account::GameLicense>(licenseType));
    account->setIsFreeTrial(isFreeTrial);
    account->setName(username);

    insertAccount(account);

    return account;
}

Account *AccountManager::createSapphireAccount(const QString &lobbyUrl, const QString &username)
{
    auto account = new Account(m_launcher, QUuid::createUuid().toString(), this);
    account->setIsSapphire(true);
    account->setName(username);
    account->setLobbyUrl(lobbyUrl);

    insertAccount(account);

    return account;
}

Account *AccountManager::getByUuid(const QString &uuid) const
{
    for (auto &account : m_accounts) {
        if (account->uuid() == uuid) {
            return account;
        }
    }

    return nullptr;
}

bool AccountManager::canDelete(Account *account) const
{
    Q_UNUSED(account)
    return m_accounts.size() != 1;
}

void AccountManager::deleteAccount(Account *account)
{
    auto config = KSharedConfig::openStateConfig();
    config->deleteGroup(QString("account-%1").arg(account->uuid()));
    config->sync();

    const int row = m_accounts.indexOf(account);
    beginRemoveRows(QModelIndex(), row, row);
    m_accounts.removeAll(account);
    endRemoveRows();
}

void AccountManager::insertAccount(Account *account)
{
    beginInsertRows(QModelIndex(), m_accounts.size(), m_accounts.size());
    m_accounts.append(account);
    endInsertRows();
}