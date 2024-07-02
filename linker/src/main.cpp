#include "../include/object_file.h"
#include "../include/linker.h"

#include <iostream>

int main(int argc, char *argv[]) {

    Linker &linker = Linker::singleton();
    linker.parseArgs(argc, argv);

    linker.loadObjects();
    linker.resolveSymbols();
    linker.placeSection();
    linker.link();

    linker.log();

    if (linker.options.relocatable)
        linker.writeRelocatable();
    else
        linker.writeExe();

    auto programFile =
            std::make_unique<ProgramFile>(linker.options.textAddr, linker.options.dataAddr);

//    linker.writeRelocatable(*programFile);

    return 0;
}