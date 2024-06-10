#pragma once

#include <string>
#include <cstdint>
#include <memory>

class Section {
    static const uint64_t MIN_SIZE;
    static const uint64_t MULTIPLIER;
public:

    std::string _name;
    uint64_t _size = 0;
    uint64_t _locationCounter = 0;
    std::unique_ptr<uint8_t[]> _memory = nullptr;

    explicit Section(std::string);


    void write(void *, uint32_t, uint64_t);

    void writeZeros(uint32_t, uint64_t);

    Section &operator+=(Section &);

    [[nodiscard]] std::string serialize() const;

    static Section deserialize(const std::string &);

    friend std::ostream &operator<<(std::ostream &, Section &);

    void static tableHeader(std::ostream &);

    void reallocateMemory(uint64_t size);
};

