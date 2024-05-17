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
    enum SYMBOL _symbolType;

    friend std::ostream &operator<<(std::ostream &, Symbol &);

    friend std::istream &operator>>(std::istream &, Symbol &);

    Symbol(std::string, bool, std::string, SCOPE, uint64_t, enum SYMBOL, int32_t size = 0);

    std::string serialize();

    static Symbol deserialize(std::string);

};
