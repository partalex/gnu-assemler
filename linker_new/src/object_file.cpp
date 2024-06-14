#include "../include/object_file.h"

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
    _symbols.resize(num_symbols);

    // read Symbols
    for (uint32_t i = 0; i < num_symbols; ++i) {
        uint32_t name_size;
        file.read((char *) &name_size, sizeof(uint32_t));
        _symbols[i].name = std::make_unique<char[]>(name_size);
        file.read(_symbols[i].name.get(), name_size);
        file.read((char *) &_symbols[i].section, sizeof(uint32_t));
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
        _sections[i].name = std::make_unique<char[]>(name_size);
        file.read(_sections[i].name.get(), name_size);
        uint32_t data_size;
        file.read((char *) &data_size, sizeof(uint32_t));
        _sections[i].data.resize(data_size);
        file.read(_sections[i].data.data(), data_size);
    }

    file.close();
}

