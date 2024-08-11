#pragma once

#include "../../common/include/enum.h"
#include "../../common/include/symbol_link.h"
#include "../../common/include/section_link.h"
#include "../../common/include/relocation_link.h"

#include <utility>
#include <vector>
#include <memory>
#include <string>

class ObjectFile {
public:
    std::string name;
    std::vector<SectionLink> sections;
    std::vector<SymbolLink> symbols;
    std::vector<RelocationLink> relocations;

    explicit ObjectFile(std::string name) : name(std::move(name)) {}

    void loadFromFile(const std::string &);

    void logRelocations(std::ostream &out) const;

    void logSymbols(std::ostream &out) const;

    void logSections(std::ostream &out) const;

    void log(std::ostream &out) const;

};

