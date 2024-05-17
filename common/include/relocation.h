#include <cstdint>
#include <vector>

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
    std::vector <RelocationEntry> _table;
public:
    void addRelocation(RelocationEntry &);

    void log();
};
