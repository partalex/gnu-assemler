#include "../include/operand.h"
#include "../include/program.h"
#include "../../assembler/include/assembler.h"

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

void Instruction::setDisplacement(uint32_t offset) {
    if (!fitIn12Bits(offset))
        throw std::runtime_error("Displacement out of range.");
    bytes.DISPLACEMENT = offset;
}

Instruction::Instruction(enum INSTRUCTION instr, uint8_t regA, uint8_t regB, uint8_t regC, int32_t disp) {
    setInstr(instr);
    setRegA(regA);
    setRegB(regB);
    setRegC(regC);
    setDisplacement(disp);
}

void Instruction::setInstr(uint8_t instr) {
    bytes.byte_0 = instr;
}

void Instruction::setMode(uint8_t mode) {
    bytes.MODE = mode;
}

void Instruction::andMode(uint8_t value) {
    bytes.MODE |= value;
}

void Instruction::orMode(uint8_t value) {
    bytes.MODE |= value;
}

void Instruction::setRegA(uint8_t regA) {
    bytes.REG_A = regA;
}

void Instruction::setRegB(uint8_t regB) {
    bytes.REG_B = regB;
}

void Instruction::setRegC(uint8_t regC) {
    bytes.REG_C = regC;
}

Push_Instr::Push_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::ST_POST_INC, REG_SP, 0, gpr, -4) {
}

Pop_Instr::Pop_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::LD_POST_INC, gpr, REG_SP, 0, 4) {
}

Not_Instr::Not_Instr(uint8_t gpr)
        : Instruction(INSTRUCTION::NOT, gpr) {}

Int_Instr::Int_Instr()
        : Instruction(INSTRUCTION::INT) {}

Load_Instr::Load_Instr(Operand *operand, uint8_t gpr, Assembler *as)
        : Instruction(INSTRUCTION::LD, gpr) {
    auto addressing = operand->addRelocation(as);
    delete operand;
    // MMMM==0b0000: gpr[A]<=csr[B];
    // MMMM==0b0001: gpr[A]<=gpr[B]+D;
    // MMMM==0b0010: gpr[A]<=mem32[gpr[B]+gpr[C=0]+D];
    // MMMM==0b0011: gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;
    setRegB(addressing.reg);
    setDisplacement(addressing.value);
    switch (addressing.addressing) {
        case CSR_OP: // CsrOp
            setMode(0b0000);
            break;
        case REG_DIR: // RegDir, LiteralInDir, IdentImm
            setMode(0b0001);
            break;
        case IN_DIR_OFFSET: // RegInDirOffIdent, RegInDir, LiteralInDir, IdentInDir
            setMode(0b0010);
            break;
        case IN_DIR_INDEX: // RegInDirOffLiteral
            setMode(0b0011);
            break;
        case IN_DIR_IN_DIR:
            isInDirInDir = true;
            setMode(0b0010);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Load instruction.");
    }
}

Load_Instr::Load_Instr(uint8_t gprD, uint8_t gprS, uint32_t offset)
        : Instruction(INSTRUCTION::LD, gprD, gprS) {
    setDisplacement(offset);
}

Csrwr_Instr::Csrwr_Instr(uint8_t gpr, uint8_t csr)
        : Instruction(INSTRUCTION::CSR_LD, gpr, csr) {}

Csrrd_Instr::Csrrd_Instr(uint8_t csr, uint8_t gpr)
        : Instruction(INSTRUCTION::LD_CSR, csr, gpr) {}

Xchg_Instr::Xchg_Instr(uint8_t regA, uint8_t regB)
        : Instruction(INSTRUCTION::XCHG, regA, regB) {}

JmpCond_Instr::JmpCond_Instr(INSTRUCTION instr, uint8_t regS, uint8_t regD, Operand *operand, Assembler *as)
        : Instruction(instr) {
    setRegA(REG_PC);
    setRegB(regS);
    setRegC(regD);
    auto addressing = operand->addRelocation(as);
    delete operand;
    // MMMM==0b0001: if (gpr[B] == gpr[C]) pc<=gpr[A=PC]+D;
    // MMMM==0b0010: if (gpr[B] != gpr[C]) pc<=gpr[A=PC]+D;
    // MMMM==0b0011: if (gpr[B] signed> gpr[C]) pc<=gpr[A=PC]+D;
    // MMMM==0b1001: if (gpr[B] == gpr[C]) pc<=mem32[gpr[A=PC]+D];
    // MMMM==0b1010: if (gpr[B] != gpr[C]) pc<=mem32[gpr[A=PC]+D];
    // MMMM==0b1011: if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A=PC]+D];
    switch (addressing.addressing) {
        case REG_DIR:     // LiteralInDir, IdentInDir
            setDisplacement((int32_t) addressing.value);
            andMode(0b0111);
            break;
        case IN_DIR_OFFSET:   // LiteralInDir
        case IN_DIR_IN_DIR:   // LiteralInDir
            orMode(0b1000);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for JmpCond instruction.");
    }
}

Call_Instr::Call_Instr(enum INSTRUCTION instruction, Operand *operand, Assembler *as)
        : Instruction(instruction) {
    setRegA(REG_PC);
    setRegB(GPR_R0);
    auto addressing = operand->addRelocation(as);
    delete operand;
    setDisplacement((int32_t) addressing.value);
    // MMMM==0b0000: pc<=gpr[A=PC]+gpr[B=0]+D;
    // MMMM==0b0001: pc<=mem32[gpr[A=PC]+gpr[B=0]+D]; // not used
    switch (addressing.addressing) {
        case REG_DIR:         // LiteralImm
            setMode(0b0000);
            break;
        case IN_DIR_OFFSET:// LiteralImm, IdentInDir
            setMode(0b0001);
            // displacement will be set in by relocation
            break;
        case IN_DIR_IN_DIR:
            isInDirInDir = true;
            setMode(0b0001);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Jmp instruction.");
    }
}

Jmp_Instr::Jmp_Instr(enum INSTRUCTION instruction, Operand *operand, Assembler *as)
        : Instruction(instruction) {
    setRegA(15);
    auto addressing = operand->addRelocation(as);
    delete operand;
    setDisplacement((int32_t) addressing.value);
    // MMMM==0b0000: pc<=gpr[A=PC]+D;
    // MMMM==0b1000: pc<=mem32[gpr[A=PC]+D];
    switch (addressing.addressing) {
        // will line below be executed?
        case REG_DIR:         // LiteralImm, IdentInDir
            setMode(0b0000);
            break;
        case IN_DIR_OFFSET:   // LiteralImm
        case IN_DIR_IN_DIR:
            setMode(0b1000);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Jmp instruction.");
    }
}

TwoReg_Instr::TwoReg_Instr(enum INSTRUCTION instruction, uint8_t regD, uint8_t regS)
        : Instruction(instruction, regD, regS, regS) {}

Store_Instr::Store_Instr(uint8_t gpr, Operand *operand, Assembler *as)
        : Instruction(INSTRUCTION::ST) {
    setRegC(gpr);
    auto addressing = operand->addRelocation(as);
    delete operand;
    // MMMM==0b0000: mem32[gpr[A=PC]+gpr[B=0]+D]<=gpr[C=gpr];
    // MMMM==0b0010: mem32[mem32[gpr[A=PC]+gpr[B=0]+D]]<=gpr[C];
    // MMMM==0b0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];
    setRegA(addressing.reg);
    setDisplacement((int32_t) addressing.value);
    switch (addressing.addressing) {
        case REG_DIR:           // IdentImm, RegDir, LiteralInDir
            // MMMM==0b0000: mem32[gpr[A=reg]+gpr[B=0]+D]<=gpr[C=gpr];
            setMode(0b0000);
            break;
        case IN_DIR_OFFSET:     // RegInDirOffIdent, RegInDir, LiteralInDir, IdentInDir
            // MMMM==0b0010: mem32[mem32[gpr[A=PC]+gpr[B=0]+D]]<=gpr[C];
            // st [PC+off], %r1;
            setMode(0b0010);
            break;
        case IN_DIR_INDEX:      // RegInDirOffLiteral: // st [%r1+0], %r2
            // MMMM==0b0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];
            setMode(0b0001);
            break;
        case IN_DIR_IN_DIR:
            isInDirInDir = true;
            setMode(0b0010);
            break;
        default:
            throw std::runtime_error("Error: Invalid addressing mode for Store instruction.");
    }
}

Store_Instr::Store_Instr(uint8_t gprD, int16_t offset, uint8_t gprS)
        : Instruction(INSTRUCTION::ST, gprD, gprS) {
    setDisplacement(offset);
}

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

void Instruction::insertInstr(Assembler *as) {
    if (isInDirInDir) {
        // push GPR_TEMP
        as->insertInstr(std::make_unique<Push_Instr>((uint8_t) GPR_TEMP).get());

        // MMMM==0b0010: gpr[A=GPR_TEMP]<=mem32[gpr[B=PC]+gpr[C=0]+D];
        auto loadInstr = std::make_unique<Load_Instr>((uint8_t) GPR_TEMP, REG_PC, 0);
        loadInstr->setMode(0b0010);
        as->insertInstr(this);

        // insert instr
        as->insertInstr(this);

        // pop GPR_TEMP
        as->insertInstr(std::make_unique<Pop_Instr>((uint8_t) GPR_TEMP).get());
        return;
    }
    as->insertInstr(this);
}

std::ostream &Instruction::logExecute(std::ostream &out) const {
    return out << std::hex << std::setfill('0') << std::setw(8) << bytes.byte_0 << " "
               << std::setw(8) << bytes.byte_1 << " "
               << std::setw(8) << bytes.byte_2 << " "
               << std::setw(8) << bytes.byte_3 << " ";
}

IRet_Instr::IRet_Instr()
        : Instruction(INSTRUCTION::LD_POST_INC, REG_PC, REG_SP, 0, 4) {}

void IRet_Instr::insertInstr(Assembler *as) {
    as->insertInstr(this);
    as->insertInstr(std::make_unique<Pop_Instr>((uint8_t) CSR_STATUS).get());
}
