// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

class CrtRand
{
public:
    explicit CrtRand(uint32_t seed);

    uint32_t next();

private:
    uint32_t seed;
};
