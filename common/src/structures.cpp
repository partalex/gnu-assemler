#include "../include/structures.h"

#include <iostream>

void SymbolList::log(std::ostream &out) {
    SymbolList *current = this;
    while (current != nullptr) {
        out << current->_symbol;
        current = current->_next;
        if (current)
            out << ", ";
    }
    out << "\n";
}

SymbolList::SymbolList(const std::string &str, SymbolList *next) {
    _symbol = str;
    _next = next;
}

SymbolList::SymbolList(const std::string &str) {
    _symbol = str;
}

