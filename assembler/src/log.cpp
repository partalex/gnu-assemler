#include "log.hpp"

void Log::STRING(std::string const& str) {
    fprintf(stderr, "%s", str.c_str());
}

void Log::STRING_LN(std::string const& str) {
    fprintf(stderr, "%s\n", str.c_str());
}