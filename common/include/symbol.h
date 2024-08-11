#pragma once

#include "enum.h"
#include "symbol_link.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

class Symbol {
public:
    SymbolLink core;

    friend std::ostream &operator<<(std::ostream &, Symbol &);

    Symbol(std::string, uint32_t, SCOPE, uint32_t, enum SYMBOL, SOURCE, enum DEFINED);

    static void tableHeader(std::ostream &);

};
