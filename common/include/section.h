#pragma once

#include <string>
#include <cstdint>
#include <memory>

class Section {
    static const uint64_t MIN_SIZE;
    static const uint8_t MULTIPLIER;
    uint64_t _locationCounter = 0;
    uint64_t _size = 0;
public:

    std::string _name;
    std::unique_ptr<uint8_t[]> _memory = nullptr;

    void addToLocCounter(uint32_t offset);

    [[nodiscard]] uint64_t getLocCounter() const;

    explicit Section(std::string);

    void write(void *, uint32_t, uint32_t);

    void writeInstr(void *, uint32_t);

    void writeZeros(uint32_t, uint64_t);

    Section &operator+=(Section &);

    void serialize(std::ostream &out) const;

    friend std::ostream &operator<<(std::ostream &, Section &);

    void static tableHeader(std::ostream &);

    void reallocateMemory(uint64_t size);

    uint64_t getSize() const;


};

