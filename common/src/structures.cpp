#include "../include/structures.h"

#include <iostream>

void SymbolList::log() {
    SymbolList *current = this;
    while (current != nullptr) {
        std::cout << current->_symbol;
        current = current->_next;
        if (current)
            std::cout << ", ";
    }
    std::cout << "\n";
}

SymbolList::SymbolList(const std::string &str, SymbolList *next) {
    _symbol = str;
    _next = next;
}

SymbolList::SymbolList(const std::string &str) {
    _symbol = str;
}

