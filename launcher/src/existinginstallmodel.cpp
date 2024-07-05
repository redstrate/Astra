// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "existinginstallmodel.h"

#include <KLocalizedString>

ExistingInstallModel::ExistingInstallModel(QObject *parent)
    : QAbstractListModel(parent)
{
    fill();
}

QVariant ExistingInstallModel::data(const QModelIndex &index, const int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const auto &install = m_existingInstalls[index.row()];

    switch (role) {
    case TypeRole: {
        switch (install.type) {
        case ExistingInstallType::OfficialLauncher:
            return i18n("Official Launcher");
        case ExistingInstallType::XIVLauncherCore:
            return QStringLiteral("XIVLauncher.Core");
        case ExistingInstallType::XIVOnMac:
            return QStringLiteral("XIV on Mac");
        case ExistingInstallType::XIVQuickLauncher:
            return QStringLiteral("XIVQuickLauncher");
        default:
            return i18n("Unknown");
        }
    }
    case PathRole:
        return install.path;
    default:
        return {};
    }
}

int ExistingInstallModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_existingInstalls.size();
}

QHash<int, QByteArray> ExistingInstallModel::roleNames() const
{
    return {
        {TypeRole, "type"},
        {PathRole, "path"},
    };
}

void ExistingInstallModel::fill()
{
    const auto dirs = physis_find_existing_game_dirs();
    for (uint32_t i = 0; i < dirs.count; i++) {
        // We shouldn't be able to import our own game installs, that's handled elsewhere in the UI
        if (dirs.entries[i].install_type != ExistingInstallType::Astra) {
            beginInsertRows({}, m_existingInstalls.size(), m_existingInstalls.size());
            m_existingInstalls.push_back(ExistingInstall{.type = dirs.entries[i].install_type, .path = QString::fromUtf8(dirs.entries[i].path)});
            endInsertRows();
        }
    }
}

#include "moc_existinginstallmodel.cpp"