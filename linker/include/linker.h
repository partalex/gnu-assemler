#pragma once

#include "program_file.h"
#include "../include/object_file.h"

#include <unordered_map>


typedef struct {
    bool hex = false;
    bool relocatable = false;
    uint32_t dataAddr = 0;
    uint32_t textAddr = 0x40000000;
} LinkerOptions;


class Linker {
    static std::unique_ptr<Linker> _instance;

public:
    LinkerOptions options;

    std::string outputFile;
    std::vector<std::string> inputNames;
    std::vector<ObjectFile> inputFiles;

    std::vector<SectionLink *> sections; // in resolveSymbols()
    std::unordered_map<std::string, SymbolLink *> _globSymMapSymbol; // in resolveSymbols()
    std::unordered_map<std::string, SectionLink *> _globSymMapSection; // in resolveSymbols()
    std::unordered_map<SectionLink *, ObjectFile *> _sectionMapFile; // in resolveSymbols()
    std::unordered_map<SectionLink *, uint64_t> _sectionAddr;

    std::vector<SectionLink *> textOrder;
    std::vector<SectionLink *> dataOrder;

    ~Linker() = default;

    void operator=(Linker const &) = delete;

    static Linker &singleton();

    void log() const;

    void parseArgs(int argc, char *argv[]);

    void testConditions();

    void loadObjects();

    void resolveSymbols();

    void placeSection();

    void link();

    void fixRelocation(const RelocationLink &, SectionLink &, SectionLink *);

    void writeRelocatable() const;

    void writeExe() const;


    // must be -hex or -relocatable
    // if -relocatable ignore all -place arguments
    // -hex -place=data@0x4000F000 -place=text@0x40000000 -o mem_content.hex test1.o test2.o
    // -relocatable -o mem_content.hex test1.o test2.o
};

