#pragma once

#include "../../common/include/section.h"
#include "../../common/include/relocation.h"
#include "../../common/include/symbol.h"


#include <vector>
#include <unordered_map>
#include <string>

class ObjectFile {
public:
    std::unordered_map<std::string, Section> _sections;
    std::unordered_map<std::string, Symbol> _symbols;
    std::vector<Relocation> _relocations;
    void loadFromFile(std::string);
};
