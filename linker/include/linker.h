#pragma once

#include "./loader_script_file.h"
#include "../../common/include/section.h"
#include "../../common/include/relocation.h"
#include "../../common/include/symbol.h"

#include <string>
#include <vector>
#include <unordered_map>

class Linker {
    static std::ofstream _logFile;
    std::unordered_map<std::string, Section> _sections;
    std::unordered_map<std::string, Symbol> _symbols;
    std::vector<Relocation> _relocations;
    std::unordered_map<std::string, int> _sectionPositions;
    Section *outputSection = nullptr;
    LoaderScriptFile _loaderScript;
    uint32_t _locationCounter;

    void loadFile(const std::string &inputFile);

    void fixRelocations();

    void fillRemainingSections();

    void writeOutputFile(std::ofstream &);

    void addSymbol(Symbol &);

    int addSection(Symbol &, Section &);

    void generateOutput();

    uint32_t getSymbolVal(const std::string &);

public:
    void link(std::ifstream &, std::vector<std::string> &, std::ofstream &);

};
