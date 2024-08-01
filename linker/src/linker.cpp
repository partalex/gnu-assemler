#include "../include/linker.h"

#include <fstream>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <unordered_set>

std::unique_ptr<Linker> Linker::_instance = nullptr;

Linker &Linker::singleton() {
    if (!_instance)
        _instance = std::make_unique<Linker>();
    return *_instance;
}

void Linker::log() const {
    for (auto &inputFile: inputFiles) {
        std::ofstream output(inputFile._name + ".txt");
        if (!output)
            throw std::runtime_error("Failed to open file: " + inputFile._name + ".txt");
        inputFile.log(output);
        output.close();
    }
}

void Linker::parseArgs(int argc, char **argv) {

    if (argc < 4)
        throw std::runtime_error("Please call this program as ./linker scriptfile outputfile [inputfiles]+");

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-hex") == 0)
            options.hex = true;
        else if (strcmp(argv[i], "-relocatable") == 0)
            options.relocatable = true;
        else if (strncmp(argv[i], "-place", 6) == 0) {
            std::string arg = argv[i];
            auto pos = arg.find('@');
            if (pos != std::string::npos) {
                std::string sectionName = arg.substr(7, pos - 7);
                std::string addrStr = arg.substr(pos + 1);
                std::istringstream iss(addrStr);
                uint32_t addr;
                iss >> std::hex >> addr;
                _willingSectionMapAddr.insert({sectionName, addr});
            }

        } else if (strcmp(argv[i], "-o") == 0)
            outputFile = argv[++i];
        else
            inputNames.emplace_back(argv[i]);
    }

    testConditions();
}

void Linker::testConditions() {
    if (options.hex && options.relocatable)
        throw std::runtime_error("Both -hex and -relocatable flags are set");

    if (!options.hex && !options.relocatable)
        throw std::runtime_error("None of -hex and -relocatable flags are set");

    if (outputFile.empty())
        throw std::runtime_error("Output file not specified");

    if (options.relocatable)
        _willingSectionMapAddr.clear();

}

void Linker::loadObjects() {
    inputFiles.reserve(inputNames.size());
    for (auto &inputName: inputNames) {
        inputFiles.emplace_back(inputName);
        inputFiles.back().loadFromFile(inputName);
    }
}

void Linker::resolveSymbols() {
    // Iterate through all inputFiles
    for (auto &file: inputFiles) {
        // Iterate through all sections in file
        for (auto &section: file._sections) {
            // Map section to file
            _sectionMapFile[&section] = &file;
            // Add section to sections
            sections.emplace_back(&section);
            // Map section with same name
            _mapSameSections[section.name].push_back(&section);
        }
        // Iterate through all symbols in section
        for (auto &symbol: file._symbols)
            // Look for Global symbols
            if (symbol.flags.scope == GLOBAL) {
                auto it = _globSymMapSymbol.find(symbol.name);
                if (it == _globSymMapSymbol.end())
                    // Not found
                    if (!symbol.flags.defined)
                        // If undefined, add it to the map with value nullptr
                        _globSymMapSymbol[symbol.name] = nullptr;
                    else {
                        // If defined, add it to the map with value &symbol
                        _globSymMapSymbol[symbol.name] = &symbol;
                        _globSymMapSection[symbol.name] = &file._sections[symbol.sectionIndex];
                    }
                else if (it->second != nullptr) {
                    if (symbol.flags.defined)
                        throw std::runtime_error("Symbol " + symbol.name + " already defined");
                } else if (!symbol.flags.defined)
                    _globSymMapSymbol[symbol.name] = nullptr;
                else {
                    // If defined, add it to the map with value &symbol
                    _globSymMapSymbol[symbol.name] = &symbol;
                    _globSymMapSection[symbol.name] = &file._sections[symbol.sectionIndex];
                }
            }
    }

    // Check if all symbols are defined
    for (auto &symbol: _globSymMapSymbol)
        if (symbol.second == nullptr)
            throw std::runtime_error("Symbol " + symbol.first + " not defined");
}

void Linker::testSectionSpace() {
    // start from second elem
    for (auto it = _willingSectionMapAddr.begin(); it != _willingSectionMapAddr.end(); ++it) {
        auto next = std::next(it);
        if (next == _willingSectionMapAddr.end())
            break;
        auto list = _mapSameSections[it->name];
        uint32_t size = 0;
        for (auto &section: list)
            size += section->data.size();
        if (it->addr + size > next->addr)
            throw std::runtime_error("Section " + it->name + " overlaps with " + next->name);
    }
}

void Linker::placeSection() {

    if (sections.empty())
        return;

    testSectionSpace();

    std::unordered_set<std::string> sectionNames;
    for (auto &section: sections)
        sectionNames.insert(section->name);

    // TODO
    uint32_t addr = 0x40000000;
    uint32_t lastFreeAddr = addr;

    // get one by one from _sectionMapAddr, its primary purpose is to set the address of the section
    // when section is placed in memory remove it from sectionNames
    // when _sectionMapAddr is empty, continue with the rest of the sections from sectionsNames
    for (auto &section: _willingSectionMapAddr) {
        addr = section.addr;
        // map sections in _mapSameSections
        auto list = _mapSameSections[section.name];
        // write to all sections with the same name
        for (auto &sect: list) {
            _sectionAddr[sect] = reinterpret_cast<char *>(addr);
            addr += sect->data.size();
        }
        lastFreeAddr = lastFreeAddr > addr ? lastFreeAddr : addr;
        // remove from sectionNames
        sectionNames.erase(section.name);
        _resultSectionMapAddr.insert({section.name, section.addr});
    }

    addr = lastFreeAddr;

    // iterate though the rest of the sections -> sectionNames
    for (auto &sectionName: sectionNames) {
        auto list = _mapSameSections[sectionName];
        // add section name and address in  _resultMapAddr
        _resultSectionMapAddr.insert({sectionName, addr});
        for (auto &sect: list) {
            _sectionAddr[sect] = reinterpret_cast<char *>(addr);
            addr += sect->data.size();
        }
    }
}

void Linker::checkDisplacement(int32_t value) const {
    if (value > 2047 || value < -2048)
        throw std::runtime_error("Displacement out of bounds: " + std::to_string(value));
}

void Linker::writeDisplacement(void *ptrDest, int32_t value) const {
    auto *ptr = reinterpret_cast<uint16_t *>(ptrDest);
    *ptr = value & 0x0FFF;
}

void Linker::writeRelocatable() const {
    std::ofstream output(outputFile, std::ios::binary);
    if (!output)
        throw std::runtime_error("Failed to open file: " + outputFile);

    // Write all sections to the output file
    for (const auto &section: sections) {
        // Write section name
        uint32_t name_size = section->name.size();
        output.write(reinterpret_cast<const char *>(&name_size), sizeof(uint32_t));
        output.write(section->name.c_str(), name_size);

        // Write section data size
        uint64_t data_size = section->data.size();
        output.write(reinterpret_cast<const char *>(&data_size), sizeof(uint64_t));

        // Write section data
        output.write(reinterpret_cast<const char *>(section->data.data()), data_size);
    }

    // Write all symbols to the output file
    for (const auto &file: inputFiles) {
        for (const auto &symbol: file._symbols) {
            // Write symbol name
            uint32_t name_size = symbol.name.size();
            output.write(reinterpret_cast<const char *>(&name_size), sizeof(uint32_t));
            output.write(symbol.name.c_str(), name_size);

            // Write symbol offset
            output.write(reinterpret_cast<const char *>(&symbol.offset), sizeof(uint64_t));

            // Write symbol section index
            output.write(reinterpret_cast<const char *>(&symbol.sectionIndex), sizeof(uint64_t));

            // Write symbol flags
            output.write(reinterpret_cast<const char *>(&symbol.flags), sizeof(uint8_t));
        }
    }

    // Write all relocations to the output file
    for (const auto &file: inputFiles) {
        for (const auto &relocation: file._relocations) {
            // Write relocation data
            output.write(reinterpret_cast<const char *>(&relocation), sizeof(RelocationLink));
        }
    }

    output.close();
}

void Linker::link() {

    // Iterate through inputFiles
    for (auto &file: inputFiles)

        // Iterate through relocations in section
        for (auto &rel: file._relocations) {

            // write to this section
            auto &destSect = file._sections[rel.sectionIndex];

            // find symbol
            auto &symbol = *_globSymMapSymbol[file._symbols[rel.symbolIndex].name];

            auto srcSect = _globSymMapSection[symbol.name];
            auto srcAddr = _sectionAddr[srcSect] + symbol.offset;
            auto destAddr = _sectionAddr[&destSect] + rel.offset;
            auto diff = (int32_t) (srcAddr - destAddr);
            auto final = destSect.data.data() + rel.offset;
            writeDisplacement(final, diff);
            switch (rel.type) {
                case R_WORD:
                    std::memcpy(final, &diff, 4);
                    break;
                case R_PC_12Bits:
                    checkDisplacement(diff);
                    writeDisplacement(final, diff);
                    break;
                default:
                    throw std::runtime_error("Unknown relocation type");
            }
        }

}

void Linker::writeHex() const {
    std::ofstream out(outputFile, std::ios::binary);
    if (!out)
        throw std::runtime_error("Failed to open file: " + outputFile);

    out << "# Sections: \n";
    for (const auto &sect: _resultSectionMapAddr)
        out << "# \t 0x" << std::hex << sect.addr << ": " << sect.name << "\n";

    for (const auto &sect: _resultSectionMapAddr) {
        auto name = sect.name;
        auto &list = _mapSameSections.at(sect.name);
        auto address = sect.addr;
        for (auto &section: list) {
            section->serializeClean(out, address);
            address += section->data.size();
        }
    }
    out.close();
}

void Linker::writeExe() const {
    auto exeName = outputFile;
    exeName.erase(exeName.end() - 4, exeName.end());
    std::ofstream exeOutput(exeName);
    if (!exeOutput)
        throw std::runtime_error("Failed to open file: " + exeName);

    uint32_t numSections = _resultSectionMapAddr.size();
    // write numFields
    exeOutput.write(reinterpret_cast<const char *>(&numSections), sizeof(numSections));

    // write sections using _sectionAddr
    for (const auto &sect: _resultSectionMapAddr) {

        uint32_t address = sect.addr;
        exeOutput.write(reinterpret_cast<const char *>(&address), sizeof(address));

        auto &list = _mapSameSections.at(sect.name);
        uint32_t sectionSize = 0;
        // calculate sectionSize
        for (auto &section: list)
            sectionSize += section->data.size();
        // write sectionSize
        exeOutput.write(reinterpret_cast<const char *>(&sectionSize), sizeof(sectionSize));

        // write sections
        for (auto &section: list)
            // write data
            exeOutput.write(reinterpret_cast<const char *>(section->data.data()), section->data.size());
    }
    exeOutput.close();
}
