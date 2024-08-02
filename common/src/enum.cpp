#include "../include/instruction.h"

#include <iomanip>

std::ostream &operator<<(std::ostream &out, MARKER marker) {
    switch (marker) {
        case MARKER::LITERALS:
            return out << "LITERALS";
        case MARKER::UNDEFINED:
            return out << "UND";
        default:
            return out << (uint32_t) marker;
    }
}

std::ostream &operator<<(std::ostream &out, enum DEFINED defined) {
    switch (defined) {
        case DEFINED::NOT_DEFINED:
            return out << "NO";
        case DEFINED::DEFINED:
            return out << "YES";
        default:
            throw std::runtime_error("DEFINED operator<<: unknown " + std::to_string((uint32_t) defined));
    }
}

std::ostream &operator<<(std::ostream &out, SOURCE source) {
    switch (source) {
        case SOURCE::OTHER:
            return out << "OTHER";
        case SOURCE::THIS:
            return out << "THIS";
        default:
            throw std::runtime_error("SOURCE operator<<: unknown " + std::to_string((uint32_t) source));
    }
}

std::ostream &operator<<(std::ostream &out, EQU_OP op) {
    switch (op) {
        case E_ADD:
            return out << "+";
        case E_SUB:
            return out << "-";
        default:
            throw std::runtime_error("EQU_OP operator<<: unknown " + std::to_string((uint32_t) op));
    }
}

std::ostream &operator<<(std::ostream &out, enum REG_GPR gpr) {
    switch (gpr) {
        case GPR_R0:
            return out << "r0";
        case GPR_R1:
            return out << "r1";
        case GPR_R2:
            return out << "r2";
        case GPR_R3:
            return out << "r3";
        case GPR_R4:
            return out << "r4";
        case GPR_R5:
            return out << "r5";
        case GPR_R6:
            return out << "r6";
        case GPR_R7:
            return out << "r7";
        case GPR_R8:
            return out << "r8";
        case GPR_R9:
            return out << "r9";
        case GPR_R10:
            return out << "r10";
        case GPR_R11:
            return out << "r11";
        case GPR_R12:
            return out << "r12";
        case GPR_R13:
            return out << "r13";
        case GPR_R14:
            return out << "SP";
        case GPR_R15:
            return out << "PC";
        default:
            throw std::runtime_error("REG_GPR operator<<: unknown " + std::to_string((uint32_t) gpr));
    }
}

std::ostream &operator<<(std::ostream &out, enum REG_CSR csr) {
    switch (csr) {
        case CSR_STATUS:
            return out << "status";
        case CSR_HANDLER:
            return out << "handler";
        case CSR_CAUSE:
            return out << "cause";
        default:
            throw std::runtime_error("REG_CSR operator<<: unknown " + std::to_string((uint32_t) csr));
    }
}

std::ostream &operator<<(std::ostream &out, enum SYMBOL sym) {
    switch (sym) {
        case SYMBOL::NO_TYPE:
            return out << "NO_TYPE";
        case SYMBOL::ASCII:
            return out << "ASCII";
        case SYMBOL::LABEL:
            return out << "LABEL";
        case SYMBOL::EQU:
            return out << "EQU";
        default:
            throw std::runtime_error("SYMBOL operator<<: unknown " + std::to_string((uint32_t) sym));
    }
}

std::ostream &operator<<(std::ostream &out, SCOPE scope) {
    switch (scope) {
        case SCOPE::GLOBAL:
            return out << "GLOBAL";
        case SCOPE::LOCAL:
            return out << "LOCAL";
        default:
            throw std::runtime_error("SCOPE operator<<: unknown " + std::to_string((uint32_t) scope));
    }
}

std::ostream &operator<<(std::ostream &out, enum INSTRUCTION instr) {
    switch (instr) {
        case INSTRUCTION::HALT:
            return out << "HALT";
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
            throw std::runtime_error("INSTRUCTION operator<<: unknown " + std::to_string((uint32_t) instr));
    }
}

std::ostream &operator<<(std::ostream &out, RELOCATION rel) {
    switch (rel) {
        case RELOCATION::R_12b:
            return out << "R_12b";
        case RELOCATION::R_32b:
            return out << "R_32b";
        default:
            throw std::runtime_error("RELOCATION operator<<: unknown " + std::to_string((uint32_t) rel));
    }
}
