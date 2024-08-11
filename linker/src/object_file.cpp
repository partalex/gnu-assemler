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
    symbols.resize(num_symbols);

    // read Symbols
    for (uint32_t i = 0; i < num_symbols; ++i) {
        uint32_t name_size;
        file.read((char *) &name_size, sizeof(name_size));
        symbols[i].name.resize(name_size);
        file.read(&symbols[i].name[0], name_size);
        file.read((char *) &symbols[i].offset, sizeof(symbols[i].offset));
        file.read((char *) &symbols[i].sectionIndex, sizeof(symbols[i].sectionIndex));
        file.read((char *) &symbols[i].flags, sizeof(symbols[i].flags));
    }

    // read num_sections
    uint32_t num_sections;
    file.read((char *) &num_sections, sizeof(num_sections));
    sections.resize(num_sections);

    // read Sections
    for (uint32_t i = 0; i < num_sections; ++i) {
        uint32_t name_size;
        file.read((char *) &name_size, sizeof(name_size));
        sections[i].name.resize(name_size);
        file.read(&sections[i].name[0], name_size);
        uint32_t data_size;
        file.read((char *) &data_size, sizeof(data_size));
        sections[i].data.resize(data_size);
        file.read(reinterpret_cast<char *>(sections[i].data.data()), data_size);
    }

    // read num_relocations
    uint32_t num_relocations;
    file.read((char *) &num_relocations, sizeof(uint32_t));
    relocations.resize(num_relocations);
    // read Relocations
    for (uint32_t i = 0; i < num_relocations; ++i)
        file.read((char *) &relocations[i], sizeof(RelocationLink));

    file.close();
}

void ObjectFile::logRelocations(std::ostream &out) const {
    Log::tableName(out, "Relocations");
    RelocationLink::tableHeader(out);
    for (auto &rel: relocations)
        out << rel;
    Log::tableFooter(out);
}

void ObjectFile::logSymbols(std::ostream &out) const {
    Log::tableName(out, "Symbols");
    SymbolLink::tableHeader(out);
    for (auto &sym: symbols)
        out << sym;
    Log::tableFooter(out);
}

void ObjectFile::logSections(std::ostream &out) const {
    Log::tableName(out, "Sections");
    SectionLink::tableHeader(out);
    for (auto &sec: sections)
        out << sec;
    Log::tableFooter(out);
}

void ObjectFile::log(std::ostream &out) const {
    logSymbols(out);
    logRelocations(out);
    logSections(out);
    Log::tableName(out, "Sections dump");
    for (auto &sec: sections)
        sec.serialize(out);
}

