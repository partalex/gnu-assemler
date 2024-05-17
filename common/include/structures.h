#pragma once

#include "instruction.h"

#include <string>
#include <utility>
#include <vector>
#include <memory>

struct SymbolList {
    std::string _symbol;
    SymbolList *_next = nullptr;

    explicit SymbolList(const std::string &);

    SymbolList(const std::string &, SymbolList *_next);

    ~SymbolList() { delete _next; }

    void log();
};

class Bind {
public:
    static std::string BIND_STR[];
    enum BIND {
        LOCAL,
        GLOBAL
    };
};

class EntryType {
public:
    static std::string ENTRY_TYPE_STR[];
    enum ENTRY_TYPE {
        SYMBOL, // GLOBAL, EXTERN
        SECTION,
    };
};

struct SymbolTableEntry {
    uint64_t _offset;
    EntryType::ENTRY_TYPE _type;
    Bind::BIND _bind;
    uint32_t _section;
    std::string _name;
    bool _resolved;

    SymbolTableEntry(
            uint64_t offset,
            EntryType::ENTRY_TYPE type,
            Bind::BIND bind,
            uint32_t section,
            std::string name,
            bool defined
    ) : _offset(offset), _type(type), _bind(bind), _section(section), _name(std::move(name)), _resolved(defined) {}

};

class SymbolTable {
    std::vector<std::unique_ptr<SymbolTableEntry>> _table;

public:
    void addSymbol(std::unique_ptr<SymbolTableEntry>);

    bool checkSymbol(Bind::BIND, EntryType::ENTRY_TYPE, const std::string &);

    bool hasUnresolvedSymbols();

    void log();

    SymbolTableEntry *getSymbol(EntryType::ENTRY_TYPE, const std::string &);
};

enum RelocationType {
    R_386_32,
    R_386_PC32
};

class RelocationEntry {
    uint32_t _offset;
    RelocationType _type;
    uint32_t _value;
public:
    RelocationEntry(uint32_t offset, RelocationType type, uint32_t value) : _offset(offset), _type(type),
                                                                            _value(value) {}

    void log();
};

class RelocationTable {
    std::vector<RelocationEntry> _table;
public:
    void addRelocation(RelocationEntry &);

    void log();
};
