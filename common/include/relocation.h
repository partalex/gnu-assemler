#include "enum.h"

#include <cstdint>
#include <vector>
#include <string>

class Relocation {
public:
    std::string _section;
    std::string _symbolName;
    uint32_t _offset;
    RELOCATION _relocationType;

    Relocation(std::string, std::string, u_int32_t, RELOCATION);

    std::string serialize();

    static Relocation deserialize(std::string);

    friend std::ostream &operator<<(std::ostream &, Relocation &);

    friend std::istream &operator>>(std::istream &, Relocation &);

};

