#include "enum.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

class Symbol {
public:
    std::string _name;
    bool _defined;
    uint64_t _offset;
    std::string _sectionName;
    SCOPE _scope;
    int32_t _size;
    SECTION_TYPE _symbolType;

    friend std::ostream &operator<<(std::ostream &, Symbol &);

    Symbol(std::string, bool, std::string, SCOPE, uint64_t, SECTION_TYPE, int32_t size = 0);

    std::string serialize();

    static Symbol deserialize(std::string);

};
