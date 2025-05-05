// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profilemanager.h"
#include "astra_log.h"
#include "profileconfig.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <QDir>
#include <QUuid>

using namespace Qt::StringLiterals;

ProfileManager::ProfileManager(QObject *parent)
    : QAbstractListModel(parent)
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
    for (const auto &m_profile : m_profiles) {
        if (m_profile->uuid() == uuid)
            return m_profile;
    }

    return nullptr;
}

Profile *ProfileManager::addProfile()
{
    QString newProfileName;
    if (m_profiles.size() == 0) {
        newProfileName = i18n("Default");
    } else {
        newProfileName = i18n("New Profile (%1)", m_profiles.size());
    }

    const auto newProfile = new Profile(QUuid::createUuid().toString(), this);
    newProfile->config()->setName(newProfileName);
    newProfile->config()->save();

    insertProfile(newProfile);

    return newProfile;
}

void ProfileManager::deleteProfile(Profile *profile, const bool deleteFiles)
{
    auto config = KSharedConfig::openStateConfig();
    config->deleteGroup(QStringLiteral("profile-%1").arg(profile->uuid()));
    config->sync();

    // delete files if requested
    if (deleteFiles) {
        QDir(profile->config()->gamePath()).removeRecursively();
    }

    const int row = static_cast<int>(m_profiles.indexOf(profile));
    beginRemoveRows(QModelIndex(), row, row);
    m_profiles.removeAll(profile);
    endRemoveRows();
    Q_EMIT profilesChanged();
}

QString ProfileManager::getDefaultGamePath(const QString &uuid)
{
    const QDir appData = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::AppDataLocation)[0];
    const QDir gameDir = appData.absoluteFilePath(QStringLiteral("game"));
    return gameDir.absoluteFilePath(uuid);
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
        if (id.contains("profile-"_L1)) {
            const QString uuid = QString(id).remove("profile-"_L1);
            qInfo(ASTRA_LOG) << "Loading profile" << uuid;
            const auto profile = new Profile(uuid, this);
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

QVariant ProfileManager::data(const QModelIndex &index, const int role) const
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
    Q_EMIT profilesChanged();
}

QList<Profile *> ProfileManager::profiles() const
{
    return m_profiles;
}

bool ProfileManager::canDelete(const Profile *account) const
{
    Q_UNUSED(account)
    return m_profiles.size() != 1;
}

bool ProfileManager::hasAnyExistingInstallations() const
{
    return std::ranges::any_of(m_profiles, [](const auto &profile) {
        return profile->isGameInstalled();
    });
}

int ProfileManager::numProfiles() const
{
    return static_cast<int>(m_profiles.count());
}

#include "moc_profilemanager.cpp"
