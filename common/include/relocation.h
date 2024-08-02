#pragma once

#include "enum.h"
#include "relocation_link.h"

#include <cstdint>
#include <vector>
#include <string>

class Relocation {
public:
    std::string _symbol;
    RelocationLink _core;

    Relocation(std::string, uint64_t, uint64_t, uint64_t, RELOCATION);

    void static tableHeader(std::ostream &);

    friend std::ostream &operator<<(std::ostream &, Relocation &);

};

