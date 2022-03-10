#include "indexparser.h"

#include <cstdio>
#include <QDebug>

template<class T>
void commonParseSqPack(FILE* file, IndexFile<T> index) {
    fread(&index.packHeader, sizeof index.packHeader, 1, file);

    // data starts at size
    fseek(file, index.packHeader.size, SEEK_SET);

    // read index header
    fread(&index.indexHeader, sizeof index.indexHeader, 1, file);

    // version should be 1?
    qInfo() << index.packHeader.version;

    fseek(file, index.indexHeader.indexDataOffset, SEEK_SET);

    qInfo() << "size: " << index.indexHeader.indexDataSize;
}

IndexFile<IndexHashTableEntry> readIndexFile(const std::string_view path) {
    FILE* file = fopen(path.data(), "rb");
    if(!file) {
        qInfo() << "Failed to read file info " << path.data();
        return {};
    }

    IndexFile<IndexHashTableEntry> index;
    commonParseSqPack(file, index);

    for(int i = 0; i < index.indexHeader.indexDataSize; i++) {
        IndexHashTableEntry entry;
        fread(&entry, sizeof entry, 1, file);

        qInfo() << entry.hash;
        qInfo() << entry.dataFileId;
        qInfo() << entry.offset;
    }

    return index;
}

IndexFile<Index2HashTableEntry> readIndex2File(const std::string_view path) {
    FILE* file = fopen(path.data(), "rb");
    if(!file) {
        qInfo() << "Failed to read file info " << path.data();
        return {};
    }

    IndexFile<Index2HashTableEntry> index;
    commonParseSqPack(file, index);

    for(int i = 0; i < index.indexHeader.indexDataSize; i++) {
        Index2HashTableEntry entry;
        fread(&entry, sizeof entry, 1, file);

        qInfo() << entry.hash;
        qInfo() << entry.dataFileId;
        qInfo() << entry.offset;
    }

    return index;
}