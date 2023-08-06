// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profilemanager.h"

#include <QDir>

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
        if (m_profiles[i]->name() == name)
            return i;
    }

    return -1;
}

Profile *ProfileManager::addProfile()
{
    auto newProfile = new Profile(m_launcher, QUuid::createUuid().toString(), this);
    newProfile->setName("New Profile");

    newProfile->readWineInfo();

    newProfile->setGamePath(getDefaultGamePath());
    newProfile->setWinePrefixPath(getDefaultWinePrefixPath());

    insertProfile(newProfile);

    return newProfile;
}

void ProfileManager::deleteProfile(Profile *profile)
{
    auto config = KSharedConfig::openStateConfig();
    config->deleteGroup(QString("profile-%1").arg(profile->uuid()));
    config->sync();

    const int row = m_profiles.indexOf(profile);
    beginRemoveRows(QModelIndex(), row, row);
    m_profiles.removeAll(profile);
    endRemoveRows();
}

QString ProfileManager::getDefaultWinePrefixPath()
{
#if defined(Q_OS_MACOS)
    return QDir::homePath() + "/Library/Application Support/FINAL FANTASY XIV ONLINE/Bottles/published_Final_Fantasy";
#endif

#if defined(Q_OS_LINUX)
    return QDir::homePath() + "/.wine";
#endif

    return "";
}

QString ProfileManager::getDefaultGamePath()
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
    return QDir::homePath() + "/.wine/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV - A Realm Reborn";
#endif
}

void ProfileManager::load()
{
    auto config = KSharedConfig::openStateConfig();
    for (const auto &id : config->groupList()) {
        if (id.contains("profile-")) {
            auto profile = new Profile(m_launcher, QString(id).remove("profile-"), this);
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
    Q_UNUSED(index);
    return m_profiles.size();
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
    beginInsertRows(QModelIndex(), m_profiles.size(), m_profiles.size());
    m_profiles.append(profile);
    endInsertRows();
}

QVector<Profile *> ProfileManager::profiles() const
{
    return m_profiles;
}

bool ProfileManager::canDelete(Profile *account) const
{
    Q_UNUSED(account)
    return m_profiles.size() != 1;
}
