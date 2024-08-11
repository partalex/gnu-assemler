#include "../include/linker.h"

int main(int argc, char *argv[]) {

    Linker &linker = Linker::singleton();
    linker.parseArgs(argc, argv);

    linker.loadObjects();
    linker.resolveSymbols();
    linker.placeSection();
    linker.link();

    linker.mergeSections();

    linker.log();

    linker.writeHex();

    if (linker.options.relocatable)
        linker.writeRelocatable();
    else
        linker.writeExe();

//    auto programFile =
//            std::make_unique<ProgramFile>();
//    linker.writeRelocatable(*programFile);

    return 0;
}