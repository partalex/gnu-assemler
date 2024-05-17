#include "../include/instruction.h"
#include "../include/symbol.h"
#include "../include/relocation.h"
#include "../include/section.h"

#include <iostream>
#include <iomanip>

std::ostream &operator<<(std::ostream &out, SECTION_TYPE sec) {
    switch (sec) {
        case SECTION_TYPE::TEXT:
            return out << "TEXT";
        case SECTION_TYPE::DATA:
            return out << "DATA";
        case SECTION_TYPE::BSS:
            return out << "BSS";
        default:
            return out << "UNDEFINED";
    }
}

std::istream &operator>>(std::istream &in, SECTION_TYPE &sec) {
    std::string token;
    in >> token;
    if (token == "TEXT")
        sec = SECTION_TYPE::TEXT;
    if (token == "DATA")
        sec = SECTION_TYPE::DATA;
    if (token == "BSS")
        sec = SECTION_TYPE::BSS;
    return in;
}


std::ostream &operator<<(std::ostream &out, SCOPE s) {
    switch (s) {
        case SCOPE::GLOBAL:
            return out << "GLOBAL";
        case SCOPE::LOCAL:
            return out << "LOCAL";
        default:
            return out << "UNDEFINED";
    }
}

std::istream &operator>>(std::istream &in, SCOPE &s) {
    std::string token;
    in >> token;
    if (token == "GLOBAL")
        s = SCOPE::GLOBAL;
    if (token == "LOCAL")
        s = SCOPE::LOCAL;
    return in;
}

std::ostream &operator<<(std::ostream &out, RELOCATION s) {
    switch (s) {
        case RELOCATION::R_386_32:
            return out << "R_386_32";
        case RELOCATION::R_386_PC32:
            return out << "R_386_PC32";
        default:
            return out << "UNDEFINED";
    }
}

std::istream &operator>>(std::istream &in, RELOCATION &s) {
    std::string token;
    in >> token;
    if (token == "R_386_32")
        s = RELOCATION::R_386_32;
    if (token == "R_386_PC32")
        s = RELOCATION::R_386_PC32;
    return in;
}

std::ostream &operator<<(std::ostream &out, enum INSTRUCTION instr) {
    switch (instr) {
        case INSTRUCTION::HALT:
            return out << "INT";
        case INSTRUCTION::INT:
            return out << "INT";
        case INSTRUCTION::CALL:
            return out << "CALL";
        case INSTRUCTION::CALL_MEM:
            return out << "CALL_MEM";
        case INSTRUCTION::JMP:
            return out << "JMP";
        case INSTRUCTION::BEQ:
            return out << "BEQ";
        case INSTRUCTION::BNE:
            return out << "BNE";
        case INSTRUCTION::BGT:
            return out << "BGT";
        case INSTRUCTION::JMP_MEM:
            return out << "JMP_MEM";
        case INSTRUCTION::BEQ_MEM:
            return out << "BEQ_MEM";
        case INSTRUCTION::BNE_MEM:
            return out << "BNE_MEM";
        case INSTRUCTION::BGT_MEM:
            return out << "BGT_MEM";
        case INSTRUCTION::XCHG:
            return out << "XCHG";
        case INSTRUCTION::ADD:
            return out << "ADD";
        case INSTRUCTION::SUB:
            return out << "SUB";
        case INSTRUCTION::MUL:
            return out << "MUL";
        case INSTRUCTION::DIV:
            return out << "DIV";
        case INSTRUCTION::NOT:
            return out << "NOT";
        case INSTRUCTION::AND:
            return out << "AND";
        case INSTRUCTION::OR:
            return out << "OR";
        case INSTRUCTION::XOR:
            return out << "XOR";
        case INSTRUCTION::SHL:
            return out << "SHL";
        case INSTRUCTION::SHR:
            return out << "SHR";
        case INSTRUCTION::ST:
            return out << "ST";
        case INSTRUCTION::ST_IND:
            return out << "ST_IND";
        case INSTRUCTION::ST_POST_INC:
            return out << "ST_POST_INC";
        case INSTRUCTION::LD_CSR:
            return out << "LD_CSR";
        case INSTRUCTION::LD:
            return out << "LD";
        case INSTRUCTION::LD_IND:
            return out << "LD_IND";
        case INSTRUCTION::LD_POST_INC:
            return out << "LD_POST_INC";
        case INSTRUCTION::CSR_LD:
            return out << "CSR_LD";
        case INSTRUCTION::CSR_LD_OR:
            return out << "CSR_LD_OR";
        case INSTRUCTION::CSR_LD_IND:
            return out << "CSR_LD_IND";
        case INSTRUCTION::CSR_LD_POST_INC:
            return out << "CSR_LD_POST_INC";
        default:
            return out << "UNDEFINED";
    }
}

std::ostream &operator<<(std::ostream &out, Instruction &instr) {
    out << "";
//    out << "Instruction: " << instr.instructionSymbol << "\n";
//    out << "Condition: " << instr.instructionCondition << "\n";
//    out << "setflags: " << instr.setFlags << "\n";
//    for (auto &temp: instr.parameters)
//        out << "\t" << temp << "\n";
//    out << "\tcode:" << std::hex << instr.instrCode.binaryCode << "\n";
//    out << "\n";
    return out;
}

std::ostream &operator<<(std::ostream &out, Symbol &symbol) {
    out << "Symbol: " << symbol._name << "\n";
    out << "\tDefined:\t" << symbol._defined << "\n";
    out << "\tSectionName:\t" << symbol._sectionName << "\n";
    out << "\tOffset:\t" << symbol._offset << "\n";
    out << "\tType:\t" << symbol._scope << "\n" << "\n";
    out << "\tSize:\t" << symbol._size << "\n" << "\n";
    return out;
}

std::ostream &operator<<(std::ostream &out, Relocation &rel) {
    out << "Relocation: " << "\n";
    out << "\tSymbol:\t" << rel._symbolName << "\n";
    out << "\tSection:\t" << rel._section << "\n";
    out << "\tOffset:\t" << rel._offset << "\n";
    out << "\tRelocationType:\t" << rel._relocationType << "\n" << "\n";
    return out;
}

std::ostream &operator<<(std::ostream &out, Section &section) {
    const int tokensByLine = 8;
    out << "Section: " << section._name << "\n";
    out << "Size: " << std::dec << section._size;
    for (int i = 0; i < section._size; ++i) {
        if (i % tokensByLine == 0) out << "\n";
        out << std::setfill('0') << std::setw(2) << std::hex
            << (u_int32_t) section._memory[i] << " ";
    }
    out << "\n" << "\n";
    return out;
}

