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
    std::string _name;
    std::vector<SectionLink> _sections;
    std::vector<SymbolLink> _symbols;
    std::vector<RelocationLink> _relocations;

    explicit ObjectFile(std::string name) : _name(std::move(name)) {}

    void loadFromFile(const std::string &);

    void logRelocations(std::ostream &out) const;

    void logSymbols(std::ostream &out) const;

    void logSections(std::ostream &out) const;

    void log(std::ostream &out) const;

};

