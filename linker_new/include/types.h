#include <vector>
#include <memory>

struct Section {
    std::vector<char> data;
    std::unique_ptr<char[]> name;
};

struct Symbol {
    std::unique_ptr<char[]> name;
    uint32_t section;
    struct {
        uint8_t symbolType: 2, defined: 1, scope: 1;
    } flags;
};

struct Relocation {
    uint32_t offset;
    uint32_t symbol;
    uint32_t section;
};

struct Program {
    std::vector<Section> sections;
    std::vector<Symbol> symbols;
};