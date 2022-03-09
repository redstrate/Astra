#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

// this is methods dedicated to parsing "fiin" files, commonly shown as "fileinfo.fiin"

// header is 1024 bytes
// for some reason, they store unknown1 and unknown 2 in this weird format,
// unknown1 is capped at 256 (in decimal) and will overflow into unknown 2
// for example, 1 is equal to unknown1 = 96 and unknown2 = 0
// 96 / 1 == 1
// if you have say, 14 entries, then unknown1 = 64 and unknown2 = 5
// 5 (unknown2) * 256 = 1280 + 64 (unknown1) = 1344
// 1344 / 96 = 14
// i could've made a mistake and this is actually really common but i don't know
struct FileInfoHeader {
    char magic[9];
    uint8_t dummy1[16];
    uint8_t unknown; // version? always seems to be 4
    uint8_t dummy2[2];
    uint8_t unknown1;
    uint8_t unknown2;
    uint8_t dummy[994];
};

// each entry is 96 bytes
struct FileInfoEntry {
    uint8_t dummy[8]; // length of file name in some format
    char str[64]; // simple \0 encoded string
    uint8_t dummy2[24]; // sha1
};

struct FileInfo {
    FileInfoHeader header;
    std::vector<FileInfoEntry> entries;
};

FileInfo readFileInfo(const std::string_view path);