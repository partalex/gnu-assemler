#include "../include/program_info.h"

ProgramInfo::ProgramInfo(uint32_t startPoint, uint32_t dataAddr, uint32_t textSize, uint32_t dataSize)
        : startPoint(startPoint), dataAddr(dataAddr), textSize(textSize), dataSize(dataSize) {}
