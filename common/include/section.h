#pragma once

#include <string>
#include <cstdint>

class Section {
public:

    std::string _name;
    uint64_t _size;
    uint8_t *_memory;

    Section(std::string, int);

    ~Section();

    void Write(void *, int, uint64_t);

    void WriteZeros(int, size_t);

    Section &operator+=(Section &);

    friend std::ostream &operator<<(std::ostream &, Section &);

    std::string serialize();

    static Section deserialize(std::string);

};

