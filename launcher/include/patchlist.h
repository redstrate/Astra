// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QString>

class Patch
{
public:
    QString name, url, repository, version;
    QList<QString> hashes;
    long hashBlockSize = 0;
    long length = 0;
};

class PatchList
{
public:
    explicit PatchList(const QString &patchList);

    [[nodiscard]] QList<Patch> patches() const;

    [[nodiscard]] bool isEmpty() const;

private:
    QList<Patch> m_patches;
};