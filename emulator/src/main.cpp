#include "../include/emulator.h"

int main(int argc, char *argv[]) {

    auto &emulator = Emulator::singleton();
    emulator.parseArgs(argc, argv);
    emulator.execute();

    return 0;
}
