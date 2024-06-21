#pragma once

#include "../../common/include/symbol_link.h"
#include "../../common/include/section_link.h"

#include <vector>
#include <memory>
#include <string>

class ProgramFile {
public:
    ProgramFile(uint64_t textPlace, uint64_t dataPlace);

    uint64_t _dataPlace;
    uint64_t _textPlace;

    SectionLink _dataSections;
    SectionLink _textSections;

};

