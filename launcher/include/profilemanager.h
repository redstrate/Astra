// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

#include "profile.h"

class ProfileManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.profileManager")

    Q_PROPERTY(int numProfiles READ numProfiles NOTIFY profilesChanged)

public:
    explicit ProfileManager(QObject *parent = nullptr);

    void load();

    enum CustomRoles {
        ProfileRole = Qt::UserRole,
    };

    [[nodiscard]] int rowCount(const QModelIndex &index) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Profile *getProfile(int index);
    Profile *getProfileByUUID(const QString &uuid);

    int getProfileIndex(const QString &name);
    Q_INVOKABLE Profile *addProfile();
    Q_INVOKABLE void deleteProfile(Profile *profile, bool deleteFiles);

    [[nodiscard]] QList<Profile *> profiles() const;
    [[nodiscard]] int numProfiles() const;

    Q_INVOKABLE bool canDelete(const Profile *account) const;

    [[nodiscard]] Q_INVOKABLE bool hasAnyExistingInstallations() const;

    static QString getDefaultGamePath(const QString &uuid);
    static QString getDefaultWinePrefixPath(const QString &uuid);

Q_SIGNALS:
    void profilesChanged();

private:
    void insertProfile(Profile *profile);

    QList<Profile *> m_profiles;
};
