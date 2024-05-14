#include <iostream>
#include <algorithm>
#include <iomanip>
#include <memory>

#include "structures.hpp"
#include "log.hpp"

std::string Bind::BIND_STR[] = {"LOC", "GLB"};

std::string EntryType::ENTRY_TYPE_STR[] = {"SYM","LBL", "SEC"};

SymbolList::SymbolList(const std::string &str, SymbolList *next) {
    _symbol = str;
    _next = next;
}

SymbolList::SymbolList(const std::string &str) {
    _symbol = str;
}

void SymbolList::log() {
#ifdef LOG_PARSER
    SymbolList *current = this;
    while (current != nullptr) {
        Log::STRING(current->_symbol);
        current = current->_next;
        if (current)
            Log::STRING(", ");
    }
    Log::STRING_LN("");
#endif
}

bool SymbolTable::hasUnresolvedSymbols() {
    return false;
}

bool SymbolTable::checkSymbol(Bind::BIND bind, EntryType::ENTRY_TYPE type, const std::string &name) {
    auto res = std::find_if(
            _table.begin(),
            _table.end(),
            [&name, bind, type](const std::unique_ptr<SymbolTableEntry> &entry) {
                return entry->_name == name && entry->_bind == bind && entry->_type == type;
            });
    return res != _table.end();

}

SymbolTableEntry *SymbolTable::getSymbol(Bind::BIND bind, EntryType::ENTRY_TYPE type, const std::string &name) {
    auto res = std::find_if(
            _table.begin(),
            _table.end(),
            [&name, bind, type](const std::unique_ptr<SymbolTableEntry> &entry) {
                return entry->_name == name && entry->_bind == bind && entry->_type == type;
            });
    if (res != _table.end()) return res->get();
    return nullptr;
}

void SymbolTable::log() {
    std::cerr << std::left;
    std::cerr << std::setw(7) << "Index"
              << std::setw(8) << "Offset"
              << std::setw(6) << "Type"
              << std::setw(6) << "Bind"
              << std::setw(10) << "Section"
              << std::setw(10) << "Resolved"
              << std::setw(17) << "Name"
              << std::endl;
    for (int i = 0; i < _table.size(); ++i) {
        std::cerr << std::setw(7) << i
                  << std::setw(8) << _table[i]->_offset
                  << std::setw(6) << EntryType::ENTRY_TYPE_STR[_table[i]->_type]
                  << std::setw(6) << Bind::BIND_STR[_table[i]->_bind]
                  << std::setw(10) << _table[i]->_section
                  << std::setw(10) << _table[i]->_resolved
                  << std::setw(10) << _table[i]->_name
                  << std::endl;
    }
}

void SymbolTable::addSymbol(std::unique_ptr<SymbolTableEntry> entry) {
    _table.push_back(std::move(entry));
}

void RelocationEntry::log() {
#ifdef LOG_PARSER
#endif
}

void RelocationTable::addRelocation(RelocationEntry &entry) {
    _table.push_back(entry);
}

void RelocationTable::log() {
#ifdef LOG_PARSER
    for (auto &entry: _table)
        entry.log();
#endif
}

