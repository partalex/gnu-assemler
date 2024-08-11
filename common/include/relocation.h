#pragma once

#include "enum.h"
#include "relocation_link.h"

#include <cstdint>
#include <vector>
#include <string>

class Relocation {
public:
    std::string symbol;
    RelocationLink core;

    Relocation(std::string, uint32_t, uint32_t, int32_t, RELOCATION);

    void static tableHeader(std::ostream &);

    friend std::ostream &operator<<(std::ostream &, Relocation &);

};

