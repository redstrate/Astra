// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "patchlist.h"
#include "astra_patcher_log.h"

#include <QDebug>

PatchList::PatchList(const QString &patchList)
{
    const QStringList parts = patchList.split("\r\n");

    for (int i = 5; i < parts.size() - 2; i++) {
        const QStringList patchParts = parts[i].split(QLatin1Char('\t'));

        const int length = patchParts[0].toInt();

        const QString &version = patchParts[4];
        const long hashBlockSize = patchParts.size() == 9 ? patchParts[6].toLong() : 0;

        const QString &name = version;
        const QStringList hashes = patchParts.size() == 9 ? (patchParts[7].split(QLatin1Char(','))) : QStringList();
        const QString &url = patchParts[patchParts.size() == 9 ? 8 : 5];

        auto url_parts = url.split(QLatin1Char('/'));
        const QString repository = url_parts[url_parts.size() - 3];

        m_patches.push_back(
            {.name = name, .url = url, .repository = repository, .version = version, .hashes = hashes, .hashBlockSize = hashBlockSize, .length = length});
    }

    Q_ASSERT_X(m_patches.isEmpty() ? patchList.isEmpty() : true,
               "PatchList",
               "Patch parsing failed, we were given an non-empty patchlist body but nothing were parsed.");

    qDebug(ASTRA_PATCHER) << "Finished parsing patch list. # of patches:" << m_patches.size();
}

QVector<Patch> PatchList::patches() const
{
    return m_patches;
}

bool PatchList::isEmpty() const
{
    return m_patches.isEmpty();
}
