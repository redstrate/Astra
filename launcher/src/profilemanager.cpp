// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profilemanager.h"
#include "astra_log.h"

#include <KSharedConfig>
#include <QDir>
#include <QUuid>

ProfileManager::ProfileManager(LauncherCore &launcher, QObject *parent)
    : QAbstractListModel(parent)
    , m_launcher(launcher)
{
}

Profile *ProfileManager::getProfile(const int index)
{
    return m_profiles[index];
}

int ProfileManager::getProfileIndex(const QString &name)
{
    for (int i = 0; i < m_profiles.size(); i++) {
        if (m_profiles[i]->uuid() == name)
            return i;
    }

    return -1;
}

Profile *ProfileManager::getProfileByUUID(const QString &uuid)
{
    for (auto &m_profile : m_profiles) {
        if (m_profile->uuid() == uuid)
            return m_profile;
    }

    return nullptr;
}

Profile *ProfileManager::addProfile()
{
    auto newProfile = new Profile(m_launcher, QUuid::createUuid().toString(), this);
    newProfile->setName(QStringLiteral("New Profile"));

    insertProfile(newProfile);

    return newProfile;
}

void ProfileManager::deleteProfile(Profile *profile)
{
    auto config = KSharedConfig::openStateConfig();
    config->deleteGroup(QStringLiteral("profile-%1").arg(profile->uuid()));
    config->sync();

    const int row = static_cast<int>(m_profiles.indexOf(profile));
    beginRemoveRows(QModelIndex(), row, row);
    m_profiles.removeAll(profile);
    endRemoveRows();
}

QString ProfileManager::getDefaultGamePath(const QString &uuid)
{
#if defined(Q_OS_WIN)
    return "C:\\Program Files (x86)\\SquareEnix\\FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_MAC)
    return QDir::homePath() +
        "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy/drive_c/Program "
        "Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif

#if defined(Q_OS_LINUX)
    const QDir appData = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppDataLocation)[0];
    const QDir gameDir = appData.absoluteFilePath(QStringLiteral("game"));
    return gameDir.absoluteFilePath(uuid);
#endif
}

QString ProfileManager::getDefaultWinePrefixPath(const QString &uuid)
{
    const QDir appData = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppDataLocation)[0];
    const QDir prefixDir = appData.absoluteFilePath(QStringLiteral("prefix"));
    return prefixDir.absoluteFilePath(uuid);
}

void ProfileManager::load()
{
    auto config = KSharedConfig::openStateConfig();
    for (const auto &id : config->groupList()) {
        if (id.contains(QLatin1String("profile-"))) {
            const QString uuid = QString(id).remove(QLatin1String("profile-"));
            qInfo(ASTRA_LOG) << "Loading profile" << uuid;
            auto profile = new Profile(m_launcher, uuid, this);
            insertProfile(profile);
        }
    }

    // Add a dummy profile if none exist
    if (m_profiles.empty()) {
        addProfile();
    }
}

int ProfileManager::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return static_cast<int>(m_profiles.size());
}

QVariant ProfileManager::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    const int row = index.row();
    if (role == ProfileRole) {
        return QVariant::fromValue(m_profiles[row]);
    }

    return {};
}

QHash<int, QByteArray> ProfileManager::roleNames() const
{
    return {{ProfileRole, QByteArrayLiteral("profile")}};
}

void ProfileManager::insertProfile(Profile *profile)
{
    beginInsertRows(QModelIndex(), static_cast<int>(m_profiles.size()), static_cast<int>(m_profiles.size()));
    m_profiles.append(profile);
    endInsertRows();
}

QList<Profile *> ProfileManager::profiles() const
{
    return m_profiles;
}

bool ProfileManager::canDelete(Profile *account) const
{
    Q_UNUSED(account)
    return m_profiles.size() != 1;
}

bool ProfileManager::hasAnyExistingInstallations() const
{
    for (auto &profile : m_profiles) {
        if (profile->isGameInstalled()) {
            return true;
        }
    }

    return false;
}

int ProfileManager::numProfiles() const
{
    return m_profiles.count();
}

#include "moc_profilemanager.cpp"