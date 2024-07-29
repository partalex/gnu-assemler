#pragma once

#include<cstdint>
#include<vector>
#include<set>

class Segment {
public:
    uint32_t address;
    uint32_t size;
    std::vector<uint8_t> data;

    Segment(uint32_t, uint32_t);

    bool operator<(const Segment &other) const {
        return address < other.address;
    }

    bool overlaps(const Segment &other) const {
        return address < other.address + other.size && address + size > other.address;
    }

    bool isAdjacent(const Segment &other) const {
        return address + size == other.address || other.address + other.size == address;
    }
};

class Memory {
public:
    std::set<Segment> segments;

    void insertSegment(Segment &);

    [[nodiscard]] uint32_t readWord(uint32_t) const;

    void writeWord(uint32_t, uint32_t);

    void mergeAdjacent();

};