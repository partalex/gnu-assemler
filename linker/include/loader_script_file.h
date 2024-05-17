#pragma once

#include "../../common/include/symbol.h"

#include <iostream>
#include <vector>
#include <unordered_map>


class LoaderScriptFile {
    std::vector<std::string> scriptInput;
    //unordered_map<string, Symbol> symbols;
public:

    void loadFromFile(std::istream &);

    void fillSymbolsAndSectPos(std::unordered_map<std::string, Symbol> &, std::unordered_map<std::string, int> &);


};

