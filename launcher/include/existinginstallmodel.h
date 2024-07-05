// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

#include <physis.hpp>

class ExistingInstallModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum CustomRoles {
        TypeRole = Qt::UserRole,
        PathRole,
    };

    explicit ExistingInstallModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void fill();

    struct ExistingInstall {
        ExistingInstallType type;
        QString path;
    };

    QList<ExistingInstall> m_existingInstalls;
};
