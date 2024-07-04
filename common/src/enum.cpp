#include <iomanip>
#include "../include/instruction.h"

uint64_t UNDEFINED = 0xFFFFFFFFFFFFFFFF;

std::ostream &operator<<(std::ostream &out, enum CSR csr) {
    switch (csr) {
        case CSR::STATUS:
            return out << "status";
        case CSR::HANDLER:
            return out << "handler";
        case CSR::CAUSE:
            return out << "cause";
        default:
            return out << "UNDEFINED";
    }
}

std::ostream &operator<<(std::ostream &out, enum SYMBOL sym) {
    switch (sym) {
        case SYMBOL::ASCII:
            return out << "ASCII";
        case SYMBOL::LABEL:
            return out << "LABEL";
        default:
            return out << "UNDEFINED";
    }
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
        case RELOCATION::R_2B_EXC_4b:
            return out << "R_2B_EXC_4b";
        case RELOCATION::R_PC32:
            return out << "R_PC32";
        default:
            return out << "UNDEFINED";
    }
}
