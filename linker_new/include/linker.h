#pragma once

#include "../include/object_file.h"

typedef struct {
    uint8_t hex: 1, relocatable: 1, place: 1;
    uint64_t dataPlace;
    uint64_t textPlace;
} Options;

class Linker {
    Linker() = default;

    ~Linker() = default;

public:
    static std::vector<Section> sections;
    static std::vector<Symbol> symbols;
    static std::vector<Relocation> relocations;
    static Options options;
    static std::string outputFile;
    static std::vector<std::string> inputNames;
    static std::vector<std::unique_ptr<ObjectFile>> inputFiles;

    static void linkFiles();

    static void readObject();

    // ./linker
    // -hex -relocatable -place=data@0x4000F000 -place=text@0x40000000 -o mem_content.hex test1.o test2.o
    // must be -hex or -relocatable
    // if -relocatable ignore all -place arguments

};

