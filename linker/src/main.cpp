#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include "linker.h"

int main(int argc, char *argv[]) {

    if (argc < 4) {
        std::cout << "Please call this program as ./linker scriptfile outputfile [inputfiles]+" << endl;
        return 1;
    }

    std::ifstream loaderFile(argv[1]);
    std::ofstream outputFile(argv[2]);

    std::unique_ptr<Linker> linker(new Linker());

    if (!loaderFile.is_open()) {
        std::cerr << "Error opening loader file" << std::endl;
        return 1;
    }

    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return 1;
    }

    std::vector<std::string> inputFiles;

    for (int i = 3; i < argc; i++) {
        inputFiles.push_back(argv[i]);
    }

    bool err = false;
    for (auto &inputFile: inputFiles) {
//        if (!inputFile.is_open())
//        {
//            err = true;
//            cerr << "Error opening input file " << endl;
//        }
    }
//    if (err) return 1;

    linker->Link(loaderFile, inputFiles, outputFile);
    return 0;
}