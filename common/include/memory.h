#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

class Segment {
public:
    explicit Segment(uint32_t);

    ~Segment() = default;

    [[nodiscard]] int32_t readWord(uint32_t) const;

    void writeWord(uint32_t, uint32_t) const;

    std::vector<uint8_t> data;

};

class Memory {
public:

    uint64_t _minAddr;
    uint64_t _size;
    uint32_t _segmentSize;
    std::unordered_map<uint32_t, std::unique_ptr<Segment>> _segments;

    explicit Memory(uint64_t, uint64_t, uint32_t);

    int32_t readWord(uint32_t);

    void writeWord(uint32_t, uint32_t);

    [[nodiscard]] bool isAddrValid(uint32_t) const;

    Segment &getSegment(uint32_t);

    void loadMemory(uint32_t, std::vector<uint8_t> &);

    [[nodiscard]] uint32_t getSegmentIndex(uint32_t) const;

};