#pragma once

#include "../include/types.h"

#include <vector>
#include <memory>
#include <string>

class ObjectFile {
public:
    std::vector<Section> _sections;
    std::vector<Symbol> _symbols;

    void loadFromFile(const std::string &);
};

