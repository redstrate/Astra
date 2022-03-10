#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

// these are methods dedicated to reading ".index" and ".index2" files
// major thanks to xiv.dev for providing the struct definitions

enum PlatformId : uint8_t
{
    Win32,
    PS3,
    PS4
};

// https://github.com/SapphireServer/Sapphire/blob/develop/deps/datReader/SqPack.cpp#L5
struct SqPackHeader
{
    char magic[0x8];
    PlatformId platformId;
    uint8_t padding0[3];
    uint32_t size;
    uint32_t version;
    uint32_t type;
};

struct SqPackIndexHeader
{
    uint32_t size;
    uint32_t type;
    uint32_t indexDataOffset;
    uint32_t indexDataSize;
};

struct IndexHashTableEntry
{
    uint64_t hash;
    uint32_t unknown : 1;
    uint32_t dataFileId : 3;
    uint32_t offset : 28;
    uint32_t _padding;
};

struct Index2HashTableEntry
{
    uint32_t hash;
    uint32_t unknown : 1;
    uint32_t dataFileId : 3;
    uint32_t offset : 28;
};

template<class Entry>
struct IndexFile {
    SqPackHeader packHeader;
    SqPackIndexHeader indexHeader;

    std::vector<Entry> entries;
};

IndexFile<IndexHashTableEntry> readIndexFile(const std::string_view path);
IndexFile<Index2HashTableEntry> readIndex2File(const std::string_view path);