#include "../include/section_link.h"

#include <iomanip>

void SectionLink::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(20) << "Name" <<
        std::setw(15) << "Size" <<
        "\n";
}

std::ostream &operator<<(std::ostream &out, const SectionLink &sec) {
    return out << std::left <<
               std::setw(20) << sec.name <<
               std::setw(15) << sec.data.size() <<
               "\n";
}

void SectionLink::serialize(std::ostream &out, uint64_t startAddress) const {
    const int tokensByLine = 16;
    out << name << " " << std::dec << data.size() << std::right;
    for (auto i = 0; i < data.size(); ++i) {
        if (i % tokensByLine == 0)
            out << "\n" << std::setw(8) << std::setfill('0') << std::hex << (startAddress + i) << "   ";
        out << std::setfill('0') << std::setw(2) << std::hex << (uint32_t) data[i] << " ";
        if (i % 8 == 7)
            out << "  ";
        if (i % tokensByLine == tokensByLine - 1) {
            out << "|";
            for (auto j = i - tokensByLine + 1; j <= i; ++j)
                if (data[j] >= 32 && data[j] <= 126)
                    out << static_cast<char>(data[j]);
                else
                    out << ".";
            out << "|";
        }
    }
    auto remaining = data.size() % tokensByLine;
    if (remaining != 0) {
        // Fill the remaining space with zeros
        for (auto i = data.size(); i < data.size() + (tokensByLine - remaining); ++i) {
            out << "00 ";
            if (i % 8 == 7 && i != data.size() + (tokensByLine - remaining) - 1)
                out << "  ";
        }
        out << "  |";
        for (auto i = data.size() - remaining; i < data.size(); ++i)
            if (data[i] >= 32 && data[i] <= 126)
                out << static_cast<char>(data[i]);
            else
                out << ".";
        for (int i = 0; i < tokensByLine - remaining; ++i)
            out << ".";
        out << "|";
    }
    out << "\n" << ".end" << "\n" << "\n";
}
