#include "../include/object_file.h"
#include "../include/linker.h"

#include <iostream>
#include <cstring>

int main(int argc, char *argv[]) {

    if (argc < 4) {
        std::cout << "Please call this program as ./linker scriptfile outputfile [inputfiles]+" << "\n";
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-hex") == 0)
            Linker::options.hex = 1;
        else if (strcmp(argv[i], "-relocatable") == 0)
            Linker::options.relocatable = 1;
        else if (strncmp(argv[i], "-place", 6) == 0) {
            Linker::options.place = 1;
            if (i + 1 < argc) {
                uint64_t temp;
                if (sscanf(argv[i], "-place=data@%lx", &temp) == 1)
                    Linker::options.dataPlace = temp;
                else if (sscanf(argv[i], "-place=text@%lx", &temp) == 1)
                    Linker::options.textPlace = temp;
            }
        } else if (strcmp(argv[i], "-o") == 0)
            Linker::outputFile = argv[++i];
        else
            Linker::inputNames.emplace_back(argv[i]);
    }

    Linker::readObject();
    Linker::linkFiles();

    return 0;
}