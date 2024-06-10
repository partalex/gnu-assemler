#include "../include/instruction.h"
#include "../include/symbol.h"
#include "../include/relocation.h"
#include "../include/section.h"

std::istream &operator>>(std::istream &in, enum SYMBOL &sym) {
    std::string token;
    in >> token;
    if (token == "LABEL")
        sym = SYMBOL::LABEL;
    else if (token == "SECTION")
        sym = SYMBOL::SECTION;
    else if (token == "INSTRUCTION")
        sym = SYMBOL::INSTRUCTION;
    else if (token == "SYMBOL")
        sym = SYMBOL::SYMBOL;
    return in;
}

std::ostream &operator<<(std::ostream &out, enum SYMBOL sym) {
    switch (sym) {
        case SYMBOL::LABEL:
            return out << "LABEL";
        case SYMBOL::ASCII:
            return out << "ASCII";
        case SYMBOL::SECTION:
            return out << "SECTION";
        case SYMBOL::INSTRUCTION:
            return out << "INSTRUCTION";
        case SYMBOL::SYMBOL:
            return out << "SYMBOL";
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

std::istream &operator>>(std::istream &in, SCOPE &s) {
    std::string token;
    in >> token;
    if (token == "GLOBAL")
        s = SCOPE::GLOBAL;
    if (token == "LOCAL")
        s = SCOPE::LOCAL;
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

std::ostream &operator<<(std::ostream &out, enum INSTRUCTION instr) {
    switch (instr) {
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

std::ostream &operator<<(std::ostream &out, RELOCATION rel) {
    switch (rel) {
        case RELOCATION::R_386_32:
            return out << "R_386_32";
        case RELOCATION::R_386_PC32:
            return out << "R_386_PC32";
        default:
            return out << "UNDEFINED";
    }
}
