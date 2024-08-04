#include "../include/section_link.h"

#include <iomanip>
#include <cstring>

void SectionLink::tableHeader(std::ostream &out) {
    out << std::left
        << std::setw(20) << "Name"
        << std::setw(15) << "Size"
        << "\n";
}

std::ostream &operator<<(std::ostream &out, const SectionLink &sec) {
    return out << std::left
               << std::setw(20) << sec._name
               << std::setw(15) << sec._data.size()
               << "\n";
}

void SectionLink::serialize(std::ostream &out, uint64_t startAddress, char fillChar) const {
    out << _name << " " << std::dec << _data.size() << std::right;
    serializeClean(out, startAddress, fillChar);
    out << "\n" << ".end" << "\n" << "\n";
}

void SectionLink::serializeClean(std::ostream &out, uint64_t startAddress, char fillChar) const {
    const int tokensByLine = 16;
    for (auto i = 0; i < _data.size(); ++i) {
        if (i % tokensByLine == 0)
            out << "\n" << std::setw(8) << std::setfill('0') << std::hex << (startAddress + i) << ":  ";
        out << std::setfill('0') << std::setw(2) << std::hex << (uint32_t) _data[i] << " ";
        if (i % 8 == 7)
            out << "  ";
        if (i % tokensByLine == tokensByLine - 1) {
            out << "|";
            for (auto j = i - tokensByLine + 1; j <= i; ++j)
                if (_data[j] >= 32 && _data[j] <= 126)
                    out << static_cast<char>(_data[j]);
                else
                    out << ".";
            out << "|";
        }
    }
    auto remaining = _data.size() % tokensByLine;
    if (remaining != 0) {
        for (auto i = _data.size(); i < _data.size() + (tokensByLine - remaining); ++i) {
            out << fillChar << fillChar << " ";
            if (i % 8 == 7 && i != _data.size() + (tokensByLine - remaining) - 1)
                out << "  ";
        }
        out << "  |";
        for (auto i = _data.size() - remaining; i < _data.size(); ++i)
            if (_data[i] >= 32 && _data[i] <= 126)
                out << static_cast<char>(_data[i]);
            else
                out << ".";
        for (int i = 0; i < tokensByLine - remaining; ++i)
            out << ".";
        out << "|";
    }
}

void SectionLink::reallocateIfNeeded(uint32_t pos, uint64_t length) {
    if (pos + length <= _data.size())
        return;
    auto newSize = pos + length;
    _data.resize(newSize);
}

void SectionLink::addToLocCounter(uint32_t offset) {
    reallocateIfNeeded(locationCnt(), offset);
}

int32_t SectionLink::readWord(uint32_t offset) {
    if (offset > _data.size() - 4)
        throw std::runtime_error("Reading outside of section bounds");
    int32_t word;
    std::memcpy(&word, _data.data() + offset, 4);
    return word;
}

void SectionLink::write(void *src, uint32_t pos, uint32_t length) {
    reallocateIfNeeded(pos, length);
    std::memcpy(_data.data() + pos, src, length);
}

void SectionLink::writeAndIncr(void *src, uint32_t pos, uint32_t length) {
    write(src, pos, length);
    addToLocCounter(length);
}

void SectionLink::writeInstr(void *src, uint32_t pos) {
    writeAndIncr(src, pos, 4);
}

uint32_t SectionLink::locationCnt() const {
    return (uint32_t) _data.size();
}
