#include "enum.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

class Symbol {
public:
    std::string _name;
    bool _defined;
    uint32_t _offset;
    uint32_t _sectionIndex;
    SCOPE _scope;
//    int32_t _size;
    enum SYMBOL _symbolType;

    friend std::ostream &operator<<(std::ostream &, Symbol &);

    friend std::istream &operator>>(std::istream &, Symbol &);

    Symbol(std::string, bool, uint32_t, SCOPE, uint32_t, enum SYMBOL, int32_t size = 0);

    [[nodiscard]] std::string serialize() const;

    static Symbol deserialize(const std::string &);

    static void tableHeader(std::ostream &);

};
