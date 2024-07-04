#include "../include/operand.h"
#include "../include/program.h"

#include <iostream>
#include <iomanip>

std::ostream &operator<<(std::ostream &out, Instruction &instr) {
    out << std::left <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_3 <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_2 <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_1 <<
        std::setw(15) << std::hex << (short) instr.bytes.byte_0 << "\n";
//    out << "Instruction: " << instr.instructionSymbol << "\n";
//    out << "Condition: " << instr.instructionCondition << "\n";
//    out << "setflags: " << instr.setFlags << "\n";
//    for (auto &temp: instr.parameters)
//        out << "\t" << temp << "\n";
//    out << "\tcode:" << std::hex << instr.instrCode.binaryCode << "\n";
//    out << "\n";
    return out;
}

void Instruction::setDisplacement(int16_t offset) {
    if (offset < -2048 || offset > 2047)
        throw std::runtime_error("Displacement out of range.");
    bytes.DISPLACEMENT |= offset & 0x7FF;
}

Instruction::Instruction(enum INSTRUCTION instr, uint8_t regA, uint8_t regB, uint8_t regC) {
    setInstr(instr);
    setRegA(regA);
    setRegB(regB);
    setRegC(regC);
}


void Instruction::setInstr(uint8_t instr) {
    bytes.byte_0 = instr;
}

void Instruction::setRegA(uint8_t regA) {
    bytes.REG_A |= regA << 4;
}

void Instruction::setRegB(uint8_t regB) {
    bytes.REG_B |= regB;
}

void Instruction::setRegC(uint8_t regC) {
    bytes.REG_C |= regC << 4;
}

Push_Instr::Push_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::ST_POST_INC, gpr) {}

Pop_Instr::Pop_Instr(uint8_t grp)
        : Instruction(INSTRUCTION::LD_POST_INC, grp) {}

Not_Instr::Not_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::NOT, gpr) {}

Int_Instr::Int_Instr(std::unique_ptr<Operand> operand)
        : Instruction(INSTRUCTION::INT), _operand(std::move(operand)) {}

Load_Instr::Load_Instr(std::unique_ptr<Operand> operand, uint8_t gpr)
        : Instruction(INSTRUCTION::LD, gpr), _operand(std::move(operand)) {
    setRegB(15);
}

Load_Instr::Load_Instr(uint8_t gprD, uint8_t gprS, int16_t offset)
        : Instruction(INSTRUCTION::LD, gprD, gprS) {
    setDisplacement(offset);
}

Csrwr_Instr::Csrwr_Instr(uint8_t gpr, uint8_t csr)
        : Instruction(INSTRUCTION::CSR_LD, gpr, csr) {}

Csrrd_Instr::Csrrd_Instr(uint8_t csr, uint8_t gpr)
        : Instruction(INSTRUCTION::LD_CSR, csr, gpr) {}

Xchg_Instr::Xchg_Instr(uint8_t regA, uint8_t regB)
        : Instruction(INSTRUCTION::XCHG, regA, regB) {}

NoAdr_Instr::NoAdr_Instr(enum INSTRUCTION instruction) :
        Instruction(instruction) {}

JmpCond_Instr::JmpCond_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}

Call_Instr::Call_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {}


Jmp_Instr::Jmp_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand> operand)
        : Instruction(instruction), _operand(std::move(operand)) {
    setRegB(15);
}


TwoReg_Instr::TwoReg_Instr(enum INSTRUCTION instruction, uint8_t regD, uint8_t regS)
        : Instruction(instruction, regD, regS, regS) {}

void Instruction::execute(Mnemonic bytes, Program &prog) {
    auto instr = static_cast<INSTRUCTION>(bytes.byte_0);
    static int counter = 0;
    *Program::LOG << "Executing " << ++counter << ". " << instr << '\n';
    uint32_t temp;
    switch (instr) {
        case INSTRUCTION::HALT:
            prog.isEnd = true;
            break;
        case INSTRUCTION::INT:
            prog.push(prog.STATUS());
            prog.push(prog.PC());
            prog.CAUSE() = STATUS::SOFTWARE;
            prog.STATUS() &= ~0x1;
            prog.PC() = prog.HANDLER();
            prog.PC() = prog.getMemoryOffset(
                    prog.registers[bytes.REG_A] + prog.registers[bytes.REG_B] + bytes.DISPLACEMENT);
            break;
        case INSTRUCTION::CALL:
            prog.push(prog.PC());
            prog.PC() = prog.registers[bytes.REG_A] + prog.registers[bytes.REG_B] + bytes.DISPLACEMENT;
            break;
        case INSTRUCTION::CALL_MEM:
            prog.push(prog.PC());
            prog.PC() = prog.getMemory(
                    prog.registers[bytes.REG_A] + prog.registers[bytes.REG_B] + bytes.DISPLACEMENT);
            break;
        case INSTRUCTION::JMP:
            prog.PC() = prog.registers[bytes.REG_A] + bytes.DISPLACEMENT;
            break;
        case INSTRUCTION::BEQ:
            if (prog.registers[bytes.REG_B] == prog.registers[bytes.REG_C])
                prog.PC() = prog.registers[bytes.REG_A] + bytes.DISPLACEMENT;
            break;
        case INSTRUCTION::BNE:
            if (prog.registers[bytes.REG_B] != prog.registers[bytes.REG_C])
                prog.PC() = prog.registers[bytes.REG_A] + bytes.DISPLACEMENT;
            break;
        case INSTRUCTION::JMP_MEM:
            prog.PC() = prog.getMemory(prog.registers[bytes.REG_A] + bytes.DISPLACEMENT);
            break;
        case INSTRUCTION::BEQ_MEM:
            if (prog.registers[bytes.REG_B] == prog.registers[bytes.REG_C])
                prog.PC() = prog.getMemory(prog.registers[bytes.REG_A] + bytes.DISPLACEMENT);
            break;
        case INSTRUCTION::BNE_MEM:
            if (prog.registers[bytes.REG_B] != prog.registers[bytes.REG_C])
                prog.PC() = prog.getMemory(prog.registers[bytes.REG_A] + bytes.DISPLACEMENT);
            break;
        case INSTRUCTION::BGT_MEM:
            if (prog.registers[bytes.REG_B] > prog.registers[bytes.REG_C])
                prog.PC() = prog.getMemory(prog.registers[bytes.REG_A] + bytes.DISPLACEMENT);
            break;
        case INSTRUCTION::XCHG:
            temp = prog.registers[bytes.REG_B];
            prog.registers[bytes.REG_B] = prog.registers[bytes.REG_C];
            prog.registers[bytes.REG_C] = temp;
            break;
        case INSTRUCTION::ADD:
            prog.registers[bytes.REG_A] = prog.sum(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::SUB:
            prog.registers[bytes.REG_A] = prog.sub(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::MUL:
            prog.registers[bytes.REG_A] = prog.mul(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::DIV:
            prog.registers[bytes.REG_A] = prog.div(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::NOT:
            prog.registers[bytes.REG_A] = prog.not_(prog.registers[bytes.REG_B]);
            break;
        case INSTRUCTION::AND:
            prog.registers[bytes.REG_A] = prog.registers[bytes.REG_B] & prog.registers[bytes.REG_C];
            break;
        case INSTRUCTION::OR:
            prog.registers[bytes.REG_A] = prog.or_(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::XOR:
            prog.registers[bytes.REG_A] = prog.xor_(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::SHL:
            prog.registers[bytes.REG_A] = prog.shl(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::SHR:
            prog.registers[bytes.REG_A] = prog.shr(prog.registers[bytes.REG_B], prog.registers[bytes.REG_C]);
            break;
        case INSTRUCTION::ST:
            prog.registers[prog.registers[bytes.REG_A] + prog.registers[bytes.REG_B] + bytes.DISPLACEMENT] =
                    prog.registers[bytes.REG_C];
            break;
        case INSTRUCTION::ST_IND:
            prog.registers[prog.registers[bytes.REG_A] + prog.registers[bytes.REG_B] + bytes.DISPLACEMENT] =
                    prog.registers[prog.registers[bytes.REG_C]];
            break;
        case INSTRUCTION::ST_POST_INC:
            prog.registers[bytes.REG_A] = prog.registers[bytes.REG_A] + bytes.DISPLACEMENT;
            prog.registers[prog.registers[bytes.REG_A]] = prog.registers[bytes.REG_C];
            break;
        case INSTRUCTION::LD_CSR:
            prog.registers[bytes.REG_A] = prog.registers[bytes.REG_B];
            break;
        case INSTRUCTION::LD:
            prog.registers[bytes.REG_A] = prog.registers[bytes.REG_B] + bytes.DISPLACEMENT;
            break;
        case INSTRUCTION::LD_POST_INC:
            prog.registers[bytes.REG_A] = prog.getMemory(
                    prog.registers[bytes.REG_B] + prog.registers[bytes.REG_C] + bytes.DISPLACEMENT);
            break;
        case INSTRUCTION::CSR_LD:
            prog.registers[bytes.REG_A] = prog.getMemory(prog.registers[bytes.REG_B] + bytes.DISPLACEMENT);
            break;
        default:
            throw std::runtime_error("Unknown instruction: " + std::to_string(bytes.byte_0));
    }
}

Store_Instr::Store_Instr(uint8_t gpr, std::unique_ptr<Operand> operand)
        : Instruction(INSTRUCTION::ST, gpr), _operand(std::move(operand)) {}

void Instruction::tableHeader(std::ostream &out) {
    out << std::left <<
        std::setw(15) << "BYTE[3]" <<
        std::setw(15) << "BYTE[2]" <<
        std::setw(15) << "BYTE[1]" <<
        std::setw(15) << "BYTE[0]" << "\n";
}

Instruction::Instruction(uint32_t value) {
    bytes.value = value;
}

std::ostream &Instruction::logExecute(std::ostream &out) const {
    return out << std::hex << std::setfill('0') << std::setw(8) << bytes.byte_0 << " "
               << std::setw(8) << bytes.byte_1 << " "
               << std::setw(8) << bytes.byte_2 << " "
               << std::setw(8) << bytes.byte_3 << " ";
}
