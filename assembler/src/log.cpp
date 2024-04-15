#include "../include/log.hpp"

void Log::LOG_STRING(std::string c) {
    fprintf(stderr, "%s", c.c_str());
}

void Log::LOG_STRING_LN(std::string c) {
    fprintf(stderr, "%s\n", c.c_str());
}