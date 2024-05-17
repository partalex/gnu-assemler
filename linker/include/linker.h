#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Linker {


public:
    void Link(std::ifstream &, std::vector<std::string> &inputFiles, std::ofstream &outputFile);

private:
    void LoadFile(std::string inputFile);

    void FixRelocations();

    void FillRemainingSections();

    void WriteOutputFile(std::ofstream &outputFile);

    static std::ofstream logFile;

    std::unordered_map<std::string, Section> sections;
    std::unordered_map<std::string, Symbol> symbols;
    std::vector<Relocation> relocations;
    std::unordered_map<std::string, int> sectionPositions;

    LoaderScriptFile loaderScript;

    u_int32_t locationCounter;

    void AddSymbol(Symbol &);

    int AddSection(Symbol &, Section &);

    void generateOutput();

    //u_int8_t *memory = nullptr;
    Section *outputSection = nullptr;

    u_int32_t GetSymbolVal(std::string symbolName);


};
