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

    Symbol(std::string, uint64_t, SCOPE, uint64_t, enum SYMBOL);

    static void tableHeader(std::ostream &);

};
