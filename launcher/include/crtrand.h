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
