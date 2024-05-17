#include "../include/relocation.h"

void RelocationEntry::log() {
}

void RelocationTable::addRelocation(RelocationEntry &entry) {
    _table.push_back(entry);
}

void RelocationTable::log() {
    for (auto &entry: _table)
        entry.log();
}