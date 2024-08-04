#include "../include/memory.h"

#include <cstring>
#include <algorithm>
#include <stdexcept>

Segment::Segment(uint32_t size) {
    data.resize(size);
}

int32_t Segment::readWord(uint32_t offset) const {
    uint32_t ret;
    std::memcpy(&ret, data.data() + offset, sizeof(ret));
    return ret;
}

void Segment::writeWord(uint32_t offset, uint32_t value) const {
    std::memcpy((void *) (data.data() + offset), &value, sizeof(value));
}

Memory::Memory(uint64_t minAddr, uint64_t size, uint32_t segmentSize)
        : _minAddr(minAddr), _size(size), _segmentSize(segmentSize) {
    if (size % segmentSize != 0)
        throw std::runtime_error("Memory size must be a multiple of segment size!");
}

bool Memory::isAddrValid(uint32_t addr) const {
    return addr >= _minAddr && addr < _minAddr + _size;
}

Segment &Memory::getSegment(uint32_t index) {
    if (_segments.find(index) == _segments.end())
        _segments[index] = std::make_unique<Segment>(_segmentSize);
    return *_segments[index];
}

void Memory::writeWord(uint32_t addr, uint32_t value) {
    auto index = getSegmentIndex(addr);
    auto &segment = getSegment(index);
    auto offset = addr % _segmentSize;

    // Check if the word spans across two segments
    if (offset + 4 > _segmentSize) {
        auto &nextSegment = getSegment(index + 1);

        // Calculate the number of bytes to write in the current segment
        auto bytesInCurrentSegment = _segmentSize - offset;

        // Write the part to the current segment
        segment.writeWord(offset, value & ((1 << (8 * bytesInCurrentSegment)) - 1));

        // Write the part to the next segment
        nextSegment.writeWord(0, value >> (8 * bytesInCurrentSegment));
    } else
        // Write the word within a single segment
        segment.writeWord(offset, value);
}

uint32_t Memory::getSegmentIndex(uint32_t addr) const {
    if (!isAddrValid(addr))
        throw std::runtime_error("Invalid address!");
    auto index = (addr - _minAddr) / _segmentSize;
    return index;
}

int32_t Memory::readWord(uint32_t address) {
    auto index = getSegmentIndex(address);
    auto &segment = getSegment(index);
    auto offset = address % _segmentSize;

    // Check if the word spans across two segments
    if (offset + 4 > _segmentSize) {
        auto &nextSegment = getSegment(index + 1);
        auto nextOffset = 4 - (_segmentSize - offset);

        // Read the part from the current segment
        uint32_t part1;
        std::memcpy(&part1, segment.data.data() + offset, _segmentSize - offset);

        // Read the part from the next segment
        uint32_t part2;
        std::memcpy(&part2, nextSegment.data.data(), nextOffset);

        // Combine the parts
        return part1 | (part2 << (8 * (_segmentSize - offset)));
    }

    // Read the word within a single segment
    return segment.readWord(offset);
}

void Memory::loadMemory(uint32_t startAddr, std::vector<uint8_t> &data) {
    uint32_t memorySize = data.size();
    uint32_t remainingBytes = memorySize;
    uint32_t currentAddr = startAddr;
    while (remainingBytes > 0) {
        auto index = getSegmentIndex(currentAddr);
        auto &segment = getSegment(index);
        auto offset = currentAddr % _segmentSize;
        auto bytesToCopy = std::min(_segmentSize - offset, remainingBytes);
        std::memcpy(segment.data.data() + offset, data.data() + (memorySize - remainingBytes), bytesToCopy);
        remainingBytes -= bytesToCopy;
        currentAddr += bytesToCopy;
    }
}