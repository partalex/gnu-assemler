#include "../include/linker.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <cstdlib>

std::unique_ptr<Linker> Linker::_instance = nullptr;

Linker &Linker::singleton() {
    if (!_instance)
        _instance = std::make_unique<Linker>();
    return *_instance;
}

void Linker::log() const {
    for (auto &inputFile: inputFiles) {
        std::ofstream output(inputFile._name + ".txt");
        if (!output) {
            std::cerr << "Failed to open file: " << inputFile._name << ".txt" << "\n";
            return;
        }
        inputFile.log(output);
        output.close();
    }
}

void Linker::parseArgs(int argc, char **argv) {

    if (argc < 4) {
        std::cout << "Please call this program as ./linker scriptfile outputfile [inputfiles]+" << "\n";
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-hex") == 0)
            options.hex = true;
        else if (strcmp(argv[i], "-relocatable") == 0)
            options.relocatable = true;
        else if (strncmp(argv[i], "-place", 6) == 0) {
            if (i + 1 < argc) {
                uint64_t temp;
                if (sscanf(argv[i], "-place=data@%lx", &temp) == 1)
                    options.dataPlace = temp;
                else if (sscanf(argv[i], "-place=text@%lx", &temp) == 1)
                    options.textPlace = temp;
            }
        } else if (strcmp(argv[i], "-o") == 0)
            outputFile = argv[++i];
        else
            inputNames.emplace_back(argv[i]);
    }

    testConditions();
}

void Linker::testConditions() {
    if (options.hex && options.relocatable) {
        std::cerr << "Both -hex and -relocatable flags are set" << "\n";
        exit(EXIT_FAILURE);
    }

    if (!options.hex && !options.relocatable) {
        std::cerr << "None of -hex and -relocatable flags are set" << "\n";
        exit(EXIT_FAILURE);
    }

    if (outputFile.empty()) {
        std::cerr << "Output file not specified" << "\n";
        exit(EXIT_FAILURE);
    }

    if (options.relocatable) {
        options.dataPlace = UNDEFINED;
        options.dataPlace = UNDEFINED;
    }

}

void Linker::loadObjects() {
    inputFiles.reserve(inputNames.size());
    for (auto &inputName: inputNames) {
        auto objFile = ObjectFile(inputName);
        objFile.loadFromFile(inputName);
        inputFiles.push_back(std::move(objFile));
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
            sections.push_back(&section);
        }
        // Iterate through all symbols in section
        for (auto &symbol: file._symbols)
            // Look for Global symbols
            if (symbol.flags.scope == GLOBAL) {
                auto it = _globSymMapSymbol.find(symbol.name);
//                std::cout << symbol.name << " " << symbol.sectionIndex << std::endl;
                if (it == _globSymMapSymbol.end())
                    if (symbol.sectionIndex == UNDEFINED)
                        _globSymMapSymbol[symbol.name] = nullptr;
                    else {
                        // If defined, add it to the map with value &symbol
                        _globSymMapSymbol[symbol.name] = &symbol;
                        _globSymMapSection[symbol.name] = &file._sections[symbol.sectionIndex];
                    }
                else if (it->second != nullptr) {
                    if (symbol.sectionIndex < UNDEFINED) {
                        std::cerr << "Symbol " << symbol.name << " already defined" << "\n";
                        exit(EXIT_FAILURE);
                    }
                } else if (symbol.sectionIndex == UNDEFINED)
                    _globSymMapSymbol[symbol.name] = nullptr;
                else {
                    // If defined, add it to the map with value &symbol
                    _globSymMapSymbol[symbol.name] = &symbol;
                    _globSymMapSection[symbol.name] = &file._sections[symbol.sectionIndex];
                }
            }
    }

    // Check if all symbols are defined
    bool allDefined = true;
    for (auto &symbol: _globSymMapSymbol)
        if (symbol.second == nullptr){
            std::cerr << "Symbol " << symbol.first << " not defined" << "\n";
            allDefined = false;
        }
    if (!allDefined)
        exit(EXIT_FAILURE);
}


void Linker::placeSection() {

    uint64_t testLC = options.textPlace;
    uint64_t dataLC = options.dataPlace;

    if (sections.empty())
        return;
    std::string next_section = sections[0]->name;

    // While sections not empty
    while (!next_section.empty()) {

        // Find first section with name next_section
        auto exist = std::find_if(sections.begin(), sections.end(), [&next_section](SectionLink *section) {
            return section->name == next_section;
        });
        while (exist != sections.end()) {
            // Set section position and address; add to textOrder or dataOrder
            if (next_section == ".text") {
                _sectionAddr[*exist] = testLC;
                testLC += (*exist)->data.size();
                textOrder.push_back(*exist);
            } else {
                _sectionAddr[*exist] = dataLC;
                dataLC += (*exist)->data.size();
                dataOrder.push_back(*exist);
            }

            // Remove section from sections
            sections.erase(exist);

            // Find next section with name next_section
            exist = std::find_if(sections.begin(), sections.end(), [&next_section](SectionLink *section) {
                return section->name == next_section;
            });
        }
        if (sections.empty())
            break;
        next_section = sections[0]->name;
    }

}

void Linker::link() {

    // Iterate through inputFiles
    for (auto &file: inputFiles)

        // Iterate through relocations in section
        for (auto &rel: file._relocations) {
            auto &destSect = file._sections[rel.sectionIndex]; // write to this section
            auto &symbol = file._symbols[rel.symbolIndex];
            SectionLink *srcSect; // read from this section
            if (file._symbols[rel.symbolIndex].sectionIndex == UNDEFINED) {
                symbol = *_globSymMapSymbol[symbol.name];
                srcSect = _globSymMapSection[symbol.name];
            } else
                srcSect = &file._sections[symbol.sectionIndex];
            fixRelocation(rel, destSect, srcSect);
        }

}

void Linker::fixRelocation(const RelocationLink &rel, SectionLink &destSect, SectionLink *srcSect) {
    if (rel.type == R_2B_EXC_4b) {
        // Copy 2 bytes as relative displacement
        // Displacement = AAAABBBB CCCCDDDD; Keep AAAA in destSect.data at rel.offset but overwrite other bits (12 bits)
        auto mask = 0xF000;
        auto placeSrc = _sectionAddr[srcSect];
        auto placeDest = _sectionAddr[&destSect] + rel.offset;
        // Instruction is 4B long so subtract 2B from the address
        uint64_t relativeAddr = placeSrc - placeDest - 2;;
        // Max 12 bits signed, if relativeAddr is greater than 0xFFF or less than -0xFFF, error
        if ((short) relativeAddr > 0xFFF || (short) relativeAddr < -0xFFF) {
            std::cerr << "Relative address too big" << "\n";
            exit(EXIT_FAILURE);
        }
        auto *ptrDest = (uint16_t *) placeDest;
        *ptrDest = (*ptrDest & mask) | (relativeAddr & ~mask);
    } else if (rel.type == R_PC32) {


    }
}

