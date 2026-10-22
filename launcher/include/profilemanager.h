// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <qqmlintegration.h>

#include "profile.h"

class ProfileManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.profileManager")

public:
    explicit ProfileManager(QObject *parent = nullptr);

    void load();

    enum CustomRoles {
        ProfileRole = Qt::UserRole,
    };

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Profile *getProfile(int index);
    Profile *getProfileByUUID(const QString &uuid);

    int getProfileIndex(const QString &name);
    Q_INVOKABLE Profile *addProfile();
    Q_INVOKABLE void deleteProfile(Profile *profile, bool deleteFiles);

    [[nodiscard]] QList<Profile *> profiles() const;

    Q_INVOKABLE bool canDelete() const;

    [[nodiscard]] Q_INVOKABLE bool hasAnyExistingInstallations() const;

    static QString getDefaultGamePath(const QString &uuid);

private:
    void insertProfile(Profile *profile);

    QList<Profile *> m_profiles;
};
