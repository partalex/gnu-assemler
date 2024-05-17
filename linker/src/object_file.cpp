#include "../include/object_file.h"

#include <fstream>
#include <iostream>

void ObjectFile::loadFromFile(std::string inputFile) {

    std::ifstream inFile(inputFile);

    if (!inFile.is_open())
        std::cerr << "Error opening input file " << inputFile << std::endl;

    std::string line;
    std::string sectionCumulate;

    bool symbolsSection = false;
    bool relocationSection = false;
    bool sectionsSection = false;

    while (getline(inFile, line)) {
        if (!symbolsSection && !relocationSection && !sectionsSection) {
            if (line == "%SYMBOLS SECTION%") {
                symbolsSection = true;
                getline(inFile, line);
            }
            if (line == "%RELOCATIONS SECTION%") {
                relocationSection = true;
                getline(inFile, line);
            }
            if (line == "%SECTIONS SECTION%")
                sectionsSection = true;
            continue;
        }
        if (line == "%END%") {
            symbolsSection = relocationSection = sectionsSection = false;
            continue;
        }
        if (symbolsSection) {
            Symbol sym = Symbol::deserialize(line);
            _symbols.insert({sym._name, sym});
        }
        if (relocationSection) {
            Relocation rel = Relocation::deserialize(line);
            _relocations.push_back(rel);
        }
        if (sectionsSection) {
            if (line == ".end") {
                Section section = Section::deserialize(sectionCumulate);
                _sections.insert({section._name, section});
                sectionCumulate.clear();
            } else
                sectionCumulate += line + "\n";
        }
    }
}