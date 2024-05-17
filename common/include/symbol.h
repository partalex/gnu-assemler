#include <cstdint>
#include <string>
#include <vector>
#include <memory>

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
    std::vector <std::unique_ptr<SymbolTableEntry>> _table;

public:
    void addSymbol(std::unique_ptr <SymbolTableEntry>);

    bool checkSymbol(Bind::BIND, EntryType::ENTRY_TYPE, const std::string &);

    bool hasUnresolvedSymbols();

    void log();

    SymbolTableEntry *getSymbol(EntryType::ENTRY_TYPE, const std::string &);
};