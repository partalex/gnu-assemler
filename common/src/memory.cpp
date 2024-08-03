#include "../include/memory.h"

#include <memory>
#include <cstring>
#include <stdexcept>

Segment::Segment(uint32_t addr, uint32_t size) : address(addr), size(size) {
    data.resize(size);
}

void Memory::insertSegment(Segment &segment) {
    segments.insert(segment);
}

uint32_t Memory::readWord(uint32_t addr) const {
    for (auto &segment: segments)
        if (addr >= segment.address && addr < segment.address + segment.size) {
            uint32_t offset = addr - segment.address;
            uint32_t ret;
            std::memcpy(&ret, segment.data.data() + offset, sizeof(ret));
            return ret;
        }
    throw std::runtime_error("Address not found in any segment!");
}

void Memory::writeWord(uint32_t addr, uint32_t value) {
    for (auto &segment: segments) {
        if (addr >= segment.address && addr < segment.address + segment.size) {
            uint32_t offset = addr - segment.address;
            auto destAddr = segment.data.data() + offset;
            std::memcpy((void *) destAddr, &value, sizeof(value));
            return;
        }
    }
    throw std::runtime_error("Address not found in any segment!");
}

void Memory::mergeAdjacent() {
    for (auto it = segments.begin(); it != segments.end(); ++it) {
        auto next = it;
        ++next;
        if (next != segments.end() && it->isAdjacent(*next)) {
            Segment merged = {it->address, it->size + next->size};
            merged.data.resize(merged.size);
            std::memcpy(merged.data.data(), it->data.data(), it->size);
            std::memcpy(merged.data.data() + it->size, next->data.data(), next->size);
            segments.erase(it);
            segments.erase(next);
            segments.insert(merged);
            mergeAdjacent();
            return;
        }
    }
}

