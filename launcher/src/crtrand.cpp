// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "crtrand.h"

CrtRand::CrtRand(const uint32_t seed)
{
    this->seed = seed;
}

uint32_t CrtRand::next()
{
    this->seed = 0x343FD * this->seed + 0x269EC3;
    return ((this->seed >> 16) & 0xFFFF) & 0x7FFF;
}
