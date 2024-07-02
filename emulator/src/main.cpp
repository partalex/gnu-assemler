#include "../include/emulator.h"

int main(int argc, char *argv[]) {

    Emulator &emulator = Emulator::singleton();
    emulator.parseArgs(argc, argv);
    emulator.execute();

    return 0;
}
