#pragma once

#include "../include/object_file.h"

#include <set>
#include <list>
#include <unordered_map>

typedef struct {
    bool hex = false;
    bool relocatable = false;
} LinkerOptions;

struct SortedMapSection {
    std::string name;
    uintptr_t addr;

    bool operator<(const SortedMapSection &other) const {
        return addr < other.addr;
    }

    bool operator==(const SortedMapSection &other) const {
        return name == other.name;
    }
};

class Linker {
    static std::unique_ptr<Linker> _instance;

public:
    LinkerOptions options;

    std::string emulatorPath = "../../emulator/bin/";
    std::string outputFile;
    std::vector<std::string> inputNames;
    std::vector<ObjectFile> inputFiles;

    std::set<SortedMapSection> _willingSectionMapAddr; // in parseArgs()

    std::vector<SectionLink *> sections; // in resolveSymbols()
    std::unordered_map<std::string, SymbolLink *> globSymMapSymbol; // in resolveSymbols()
    std::unordered_map<std::string, SectionLink *> globSymMapSection; // in resolveSymbols()
    std::unordered_map<SectionLink *, ObjectFile *> sectionMapFile; // in resolveSymbols()
    std::unordered_map<std::string, std::list<SectionLink *>> mapSameSections; // in resolveSymbols()
    std::unordered_map<std::string, std::unique_ptr<SectionLink>> mapMergedSections; // mergeSections()

    std::set<SortedMapSection> resultSectionMapAddr; // in placeSection()
    std::unordered_map<SectionLink *, uintptr_t> sectionAddr; // in placeSection()

    ~Linker() = default;

    void operator=(Linker const &) = delete;

    static Linker &singleton();

    void log() const;

    void mergeSections();

    void parseArgs(int argc, char *argv[]);

    void testConditions();

    void loadObjects();

    void resolveSymbols();

    void testSectionSpace();

    void placeSection();

    void link();

    void writeRelocatable() const;

    void writeExe() const;

    void writeHex() const;

    // must be -hex or -relocatable
    // if -relocatable ignore all -place arguments
    // -hex -place=data@0x4000F000 -place=text@0x40000000 -o program.hex main.o handler.o isr_terminal.o isr_timer.o
    // -relocatable -o mem_content.hex test1.o test2.o
};
