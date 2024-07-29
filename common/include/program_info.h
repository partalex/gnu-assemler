#pragma once

#include "enum.h"

// numFields: num
// fields: address numBytes


class ProgramInfo {
    typedef struct {
        uint32_t address;
        uint32_t numBytes;
        void* data;
    } SectionInfo;
public:
    uint32_t numFields;
    SectionInfo *fields;
};
