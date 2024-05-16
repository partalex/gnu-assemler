#include <iostream>
#include <memory>

#include "emulator.hpp"
#include "program.hpp"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cout << "Please call this program as ./emulator inputfile" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream inputFile(argv[1]);


    std::unique_ptr<Emulator> emulator(new Emulator());

    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file" << std::endl;
        return EXIT_FAILURE;
    }

    emulator->Emulate(inputFile);

    return 0;
}
