// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

#include "profile.h"

class ProfileManager : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.profileManager")

public:
    explicit ProfileManager(LauncherCore &launcher, QObject *parent = nullptr);

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
    Q_INVOKABLE void deleteProfile(Profile *profile);

    [[nodiscard]] QVector<Profile *> profiles() const;

    Q_INVOKABLE bool canDelete(Profile *account) const;

    Q_INVOKABLE bool hasAnyExistingInstallations() const;

    static QString getDefaultGamePath(const QString &uuid);
    static QString getDefaultWinePrefixPath(const QString &uuid);

private:
    void insertProfile(Profile *profile);

    QVector<Profile *> m_profiles;

    LauncherCore &m_launcher;
};