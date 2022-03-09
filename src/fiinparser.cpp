#include "fiinparser.h"

#include <cstdio>
#include <QDebug>

FileInfo readFileInfo(const std::string_view path) {
    FILE* file = fopen(path.data(), "rb");
    if(!file) {
        qInfo() << "Failed to read file info " << path.data();
        return {};
    }

    FileInfo info;
    fread(&info.header, sizeof info.header, 1, file);

    char magic[9] = "FileInfo";
    if(strcmp(info.header.magic, magic) != 0)
        qInfo() << "Invalid magic for fileinfo.";
    else
        qInfo() << "Got matching magic:" << info.header.magic;

    qInfo() << "unknown (version?) = " << info.header.unknown;
    qInfo() << "unknown1 = " << info.header.unknown1;
    qInfo() << "unknown2 = " << info.header.unknown2;

    int overflow = info.header.unknown2;
    int extra = overflow * 256;
    int first = info.header.unknown1 / 96;
    int first2 = extra / 96;
    int actualEntries = first + first2 + 1; // is this 1 really needed? lol

    qInfo() << "Guessed number of entries: " << actualEntries;

    int numEntries = actualEntries;
    for(int i = 0; i < numEntries; i++) {
        FileInfoEntry entry;
        fread(&entry, sizeof entry, 1, file);

        info.entries.push_back(entry);

        qDebug() << entry.str;
    }

    fclose(file);

    return info;
}