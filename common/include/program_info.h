#pragma once

#include "enum.h"

class ProgramInfo {
public:
    uint32_t startPoint;
    uint32_t dataAddr;
    uint32_t textSize;
    uint32_t dataSize;

    explicit ProgramInfo(uint32_t , uint32_t , uint32_t= 0, uint32_t= 0);

};
