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
        std::ofstream output(inputFile.name + ".txt");
        if (!output)
            throw std::runtime_error("Failed to open file: " + inputFile.name + ".txt");
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
        for (auto &section: file.sections) {
            // Map section to file
            sectionMapFile[&section] = &file;
            // Add section to sections
            sections.emplace_back(&section);
            // Map section with same name
            mapSameSections[section.name].push_back(&section);
        }
        // Iterate through all symbols in section
        for (auto &symbol: file.symbols)
            // Look for Global symbols
            if (symbol.flags.scope == GLOBAL) {
                auto it = globSymMapSymbol.find(symbol.name);
                if (it == globSymMapSymbol.end())
                    // Not found
                    if (!symbol.flags.defined)
                        // If undefined, add it to the map with value nullptr
                        globSymMapSymbol[symbol.name] = nullptr;
                    else {
                        // If defined, add it to the map with value &symbol
                        globSymMapSymbol[symbol.name] = &symbol;
                        globSymMapSection[symbol.name] = &file.sections[symbol.sectionIndex];
                    }
                else if (it->second != nullptr) {
                    if (symbol.flags.defined)
                        throw std::runtime_error("Symbol " + symbol.name + " already defined");
                } else if (!symbol.flags.defined)
                    globSymMapSymbol[symbol.name] = nullptr;
                else {
                    // If defined, add it to the map with value &symbol
                    globSymMapSymbol[symbol.name] = &symbol;
                    globSymMapSection[symbol.name] = &file.sections[symbol.sectionIndex];
                }
            }
    }

    // Check if all symbols are defined
    for (auto &symbol: globSymMapSymbol)
        if (symbol.second == nullptr)
            throw std::runtime_error("Symbol " + symbol.first + " not defined");
}

void Linker::testSectionSpace() {
    // start from second elem
    for (auto it = _willingSectionMapAddr.begin(); it != _willingSectionMapAddr.end(); ++it) {
        auto next = std::next(it);
        if (next == _willingSectionMapAddr.end())
            break;
        auto list = mapSameSections[it->name];
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

    uint32_t addr = 0x40000000;
    uint32_t lastFreeAddr = addr;

    // get one by one from _sectionMapAddr, its primary purpose is to set the address of the section
    // when section is placed in memory remove it from sectionNames
    // when _sectionMapAddr is empty, continue with the rest of the sections from sectionsNames
    for (auto &section: _willingSectionMapAddr) {
        addr = section.addr;
        // map sections in _mapSameSections
        auto list = mapSameSections[section.name];
        // write to all sections with the same name
        for (auto sect: list) {
            sectionAddr[sect] = addr;
            addr += sect->data.size();
        }
        lastFreeAddr = lastFreeAddr > addr ? lastFreeAddr : addr;
        // remove from sectionNames
        sectionNames.erase(section.name);
        resultSectionMapAddr.insert({section.name, section.addr});
    }

    addr = lastFreeAddr;

    // iterate though the rest of the sections -> sectionNames
    for (auto &sectionName: sectionNames) {
        auto list = mapSameSections[sectionName];
        // add section name and address in  _resultMapAddr
        resultSectionMapAddr.insert({sectionName, addr});
        for (auto sect: list) {
            sectionAddr[sect] = addr;
            addr += sect->data.size();
        }
    }
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
        for (const auto &symbol: file.symbols) {
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
        for (const auto &relocation: file.relocations) {
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
        for (auto &rel: file.relocations) {

            // write to this section
            auto &destSect = file.sections[rel.sectionIndex];

            auto &symbol = file.symbols[rel.symbolIndex];
            auto ptrValue = std::make_unique<uint32_t>();
            symbol = *globSymMapSymbol[file.symbols[rel.symbolIndex].name];
            if (symbol.flags.symbolType == EQU)
                *ptrValue.get() = symbol.offset;
            else {
                auto srcAddr = sectionAddr[globSymMapSection[file.symbols[rel.symbolIndex].name]];
                *ptrValue.get() = srcAddr + symbol.offset;
            }
            switch (rel.type) {
                case R_32_IMMEDIATE:
                case R_32_IN_DIR:
                    destSect.fixWord(ptrValue.get(), rel.offset);
                    break;
                default:
                    throw std::runtime_error("Unknown relocation type");
            }
        }

}

void Linker::writeHex() const {
    std::ofstream out(emulatorPath + outputFile, std::ios::binary);
    if (!out)
        throw std::runtime_error("Failed to open file: " + emulatorPath + outputFile);

    out << "# Sections: \n";
    for (const auto &sect: resultSectionMapAddr)
        out << "# \t 0x" << std::hex << sect.addr << ": " << sect.name << "\n";

    for (const auto &sect: resultSectionMapAddr) {
        auto name = sect.name;
        auto *section = mapMergedSections.at(name).get();
        section->serializeClean(out, sect.addr);
    }
    out.close();
}

void Linker::writeExe() const {
    auto exeName = outputFile;
    exeName.erase(exeName.end() - 4, exeName.end());
    std::ofstream exeOutput(emulatorPath + exeName);
    if (!exeOutput)
        throw std::runtime_error("Failed to open file: " + emulatorPath + exeName);

    uint32_t numSections = resultSectionMapAddr.size();
    // write number of section
    exeOutput.write(reinterpret_cast<const char *>(&numSections), sizeof(numSections));

    // write sections using _sectionAddr
    for (const auto &sect: resultSectionMapAddr) {

        uint32_t address = sect.addr;
        exeOutput.write(reinterpret_cast<const char *>(&address), sizeof(address));

        auto *section = mapMergedSections.at(sect.name).get();

        uint32_t sectionSize = section->data.size();
        // write sectionSize
        exeOutput.write(reinterpret_cast<const char *>(&sectionSize), sizeof(sectionSize));

        // write sections
        exeOutput.write(reinterpret_cast<const char *>(section->data.data()), section->data.size());
    }
    exeOutput.close();
}

void Linker::mergeSections() {
    // iterate through mapSameSections, merge sections with the same name inside mapMergedSections
    // use operator += for merge
    //    SectionLink &operator=(const SectionLink &);
    for (auto &section: mapSameSections) {
        auto &list = section.second;
        mapMergedSections[section.first] = std::make_unique<SectionLink>(section.first);
        for (auto &sect: list)
            *mapMergedSections[section.first].get() += *sect;
    }
}
