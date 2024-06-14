#include <fstream>
#include <iostream>
#include <cstring>
#include "../include/linker.h"

std::vector<Section> Linker::sections = std::vector<Section>();
std::vector<Symbol> Linker::symbols = std::vector<Symbol>();
std::vector<Relocation> Linker::relocations = std::vector<Relocation>();
Options Linker::options = {};
std::string Linker::outputFile = "";
std::vector<std::string> Linker::inputNames = std::vector<std::string>();
std::vector<std::unique_ptr<ObjectFile>> Linker::inputFiles = std::vector<std::unique_ptr<ObjectFile>>();


void Linker::linkFiles() {
    std::ofstream file(outputFile, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open file: " << outputFile << "\n";
        return;
    }

    // write num_symbols
    uint32_t num_symbols = 0;
    for (auto &inputFile: inputFiles) {
        num_symbols += inputFile->_symbols.size();
    }
    file.write((char *) &num_symbols, sizeof(uint32_t));

    // write Symbols
    for (auto &inputFile: inputFiles) {
        for (auto &symbol: inputFile->_symbols) {
            uint32_t name_size = strlen(symbol.name.get()) + 1;
            file.write((char *) &name_size, sizeof(uint32_t));
            file.write(symbol.name.get(), name_size);
            file.write((char *) &symbol.section, sizeof(uint32_t));
            file.write((char *) &symbol.flags, sizeof(uint8_t));
        }
    }

    // write num_sections
    uint32_t num_sections = 0;
    for (auto &inputFile: inputFiles)
        num_sections += inputFile->_sections.size();

    file.write((char *) &num_sections, sizeof(uint32_t));

    // write Sections
    for (auto &inputFile: inputFiles) {
        for (auto &section: inputFile->_sections) {
            uint32_t name_size = strlen(section.name.get()) + 1;
            file.write((char *) &name_size, sizeof(uint32_t));
            file.write(section.name.get(), name_size);
            uint32_t data_size = section.data.size();
            file.write((char *) &data_size, sizeof(uint32_t));
            file.write(section.data.data(), data_size);
        }
    }

    file.close();

}

void Linker::readObject() {
    for (auto &inputName: inputNames) {
        auto objFile = std::make_unique<ObjectFile>();
        objFile->loadFromFile(inputName);
        inputFiles.push_back(std::move(objFile));
    }
}
