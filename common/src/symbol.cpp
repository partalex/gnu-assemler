#include "../include/symbol.h"

#include <algorithm>
#include <iostream>
#include <iomanip>

std::string Bind::BIND_STR[] = {"LOC", "GLB"};

std::string EntryType::ENTRY_TYPE_STR[] = {"SYM", "SEC"};

bool SymbolTable::hasUnresolvedSymbols() {
    return false;
}

bool SymbolTable::checkSymbol(Bind::BIND bind, EntryType::ENTRY_TYPE type, const std::string &name) {
    auto res = std::find_if(
            _table.begin(),
            _table.end(),
            [&name, bind, type](const std::unique_ptr <SymbolTableEntry> &entry) {
                return entry->_name == name && entry->_bind == bind && entry->_type == type;
            });
    return res != _table.end();

}

SymbolTableEntry *SymbolTable::getSymbol(EntryType::ENTRY_TYPE type, const std::string &name) {
    auto res = std::find_if(
            _table.begin(),
            _table.end(),
            [&name, type](const std::unique_ptr <SymbolTableEntry> &entry) {
                return entry->_name == name && entry->_type == type;
            });
    if (res != _table.end()) return res->get();
    return nullptr;
}

void SymbolTable::log() {
    std::cout << std::left
              << std::setw(7) << "Index"
              << std::setw(8) << "Offset"
              << std::setw(6) << "Type"
              << std::setw(6) << "Bind"
              << std::setw(10) << "Section"
              << std::setw(10) << "Resolved"
              << std::setw(17) << "Name"
              << "\n";
    for (int i = 0; i < _table.size(); ++i) {
        std::cout << std::setw(7) << i
                  << std::setw(8) << _table[i]->_offset
                  << std::setw(6) << EntryType::ENTRY_TYPE_STR[_table[i]->_type]
                  << std::setw(6) << Bind::BIND_STR[_table[i]->_bind]
                  << std::setw(10) << _table[i]->_section
                  << std::setw(10) << _table[i]->_resolved
                  << std::setw(10) << _table[i]->_name
                  << "\n";
    }
}

void SymbolTable::addSymbol(std::unique_ptr <SymbolTableEntry> entry) {
    _table.push_back(std::move(entry));
}