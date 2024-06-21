#include "../include/object_file.h"
#include "../include/linker.h"

#include <iostream>

int main(int argc, char *argv[]) {

    Linker &linker = linker.singleton();
    linker.parseArgs(argc, argv);

    linker.loadObjects();
    linker.resolveSymbols();
    linker.placeSection();
    linker.link();

//    linker.log();

    auto programFile =
            std::make_unique<ProgramFile>(linker.options.textPlace, linker.options.dataPlace);

//    linker.writeRelocatable(*programFile);

    return 0;
}