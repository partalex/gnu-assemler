#include "../include/object_file.h"
#include "../../common/include/log.h"

#include <fstream>
#include <iostream>

void ObjectFile::loadFromFile(const std::string &inputFile) {
    std::ifstream file(inputFile, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open file: " << inputFile << "\n";
        return;
    }

    // read num_symbols
    uint32_t num_symbols;
    file.read((char *) &num_symbols, sizeof(uint32_t));

    // resize _symbols to num_symbols
    _symbols.resize(num_symbols);

    // read Symbols
    for (uint32_t i = 0; i < num_symbols; ++i) {
        uint32_t name_size;
        file.read((char *) &name_size, sizeof(uint32_t));
        _symbols[i].name.resize(name_size);
        file.read(&_symbols[i].name[0], name_size);
        file.read((char *) &_symbols[i].offset, sizeof(uint64_t));
        file.read((char *) &_symbols[i].sectionIndex, sizeof(uint64_t));
        file.read((char *) &_symbols[i].flags, sizeof(uint8_t));
    }

    // read num_sections
    uint32_t num_sections;
    file.read((char *) &num_sections, sizeof(uint32_t));
    _sections.resize(num_sections);

    // read Sections
    for (uint32_t i = 0; i < num_sections; ++i) {
        uint32_t name_size;
        file.read((char *) &name_size, sizeof(uint32_t));
        _sections[i].name.resize(name_size);
        file.read(&_sections[i].name[0], name_size);
        uint64_t data_size;
        file.read((char *) &data_size, sizeof(uint64_t));
        _sections[i].data.resize(data_size);
        file.read(reinterpret_cast<char *>(_sections[i].data.data()), data_size);
    }

    // read num_relocations
    uint32_t num_relocations;
    file.read((char *) &num_relocations, sizeof(uint32_t));
    _relocations.resize(num_relocations);
    // read Relocations
    for (uint32_t i = 0; i < num_relocations; ++i)
        file.read((char *) &_relocations[i], sizeof(RelocationLink));

    file.close();
}

void ObjectFile::logRelocations(std::ostream &out) const {
    Log::tableName(out, "Relocations");
    RelocationLink::tableHeader(out);
    for (auto &rel: _relocations)
        out << rel;
    Log::tableFooter(out);
}

void ObjectFile::logSymbols(std::ostream &out) const {
    Log::tableName(out, "Symbols");
    SymbolLink::tableHeader(out);
    for (auto &sym: _symbols)
        out << sym;
    Log::tableFooter(out);
}

void ObjectFile::logSections(std::ostream &out) const {
    Log::tableName(out, "Sections");
    SectionLink::tableHeader(out);
    for (auto &sec: _sections)
        out << sec;
    Log::tableFooter(out);
}

void ObjectFile::log(std::ostream &out) const {
    logSymbols(out);
    logSections(out);
    logRelocations(out);
    Log::tableName(out, "Sections dump");
    for (auto &sec: _sections)
        sec.serialize(out);
}

