#pragma once

#include <string>

#define DO_DEBUG

class Log
{
public:
    static void LOG_STRING(std::string);
    static void LOG_STRING_LN(std::string);
};