// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QVector>

class Patch
{
public:
    QString name, url, repository, version;
    QVector<QString> hashes;
    long hashBlockSize = 0;
    long length = 0;
};

class PatchList
{
public:
    explicit PatchList(const QString &patchList);

    [[nodiscard]] QVector<Patch> patches() const;

    [[nodiscard]] bool isEmpty() const;

private:
    QVector<Patch> m_patches;
};