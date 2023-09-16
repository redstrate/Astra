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

    int rowCount(const QModelIndex &index = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Profile *getProfile(int index);

    int getProfileIndex(const QString &name);
    Q_INVOKABLE Profile *addProfile();
    Q_INVOKABLE void deleteProfile(Profile *profile);

    QVector<Profile *> profiles() const;

    Q_INVOKABLE bool canDelete(Profile *account) const;

    static QString getDefaultGamePath(const QString &uuid);

private:
    void insertProfile(Profile *profile);

    QString getDefaultWinePrefixPath();

    QVector<Profile *> m_profiles;

    LauncherCore &m_launcher;
};