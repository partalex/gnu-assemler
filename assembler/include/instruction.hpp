#pragma once

#include <map>
#include <memory>
#include <vector>

#include "operand.hpp"
#include "log.hpp"

class I {
public:
    enum INSTRUCTION {
        HALT = 0b00000000,              // halt
        INT = 0b00010000,               // push status; push pc; cause<=4; status<=status&(~0x1); pc<=handler;
        CALL = 0b00100000,              // push pc; pc<=gpr[A]+gpr[B]+D
        CALL_MEM = 0b00100001,          // push pc; pc<=mem32[gpr[A]+gpr[B]+D]

        JMP = 0b00110000,               // pc<=gpr[A]+D
        BEQ = 0b00110001,               // if (gpr[B] == gpr[C]) pc<=gpr[A]+D
        BNE = 0b00110010,               // if (gpr[B] != gpr[C]) pc<=gpr[A]+D
        BGT = 0b00110011,               // if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D
        JMP_MEM = 0b00111000,           // pc<=mem32[gpr[A]+D]
        BEQ_MEM = 0b00111001,           // if (gpr[B] == gpr[C]) pc<=mem32[gpr[A]+D]
        BNE_MEM = 0b00111010,           // if (gpr[B] != gpr[C]) pc<=mem32[gpr[A]+D]
        BGT_MEM = 0b00111011,           // if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A]+D]

        XCHG = 0b01000000,              // temp<=gpr[B]; gpr[B]<=gpr[C]; gpr[C]<=temp;
        ADD = 0b01010000,               // gpr[A]<=gpr[B]+gpr[C]
        SUB = 0b01010001,               // gpr[A]<=gpr[B]-gpr[C]
        MUL = 0b01010010,               // gpr[A]<=gpr[B] * gpr[C]
        DIV = 0b01010011,               // gpr[A]<=gpr[B] / gpr[C]

        NOT = 0b01100000,               // gpr[A]<=~gpr[B]
        AND = 0b01100001,               // gpr[A]<=gpr[B] & gpr[C]
        OR = 0b01100010,                // gpr[A]<=gpr[B] | gpr[C]
        XOR = 0b01100011,               // gpr[A]<=gpr[B] ^ gpr[C]
        SHL = 0b01110000,               // gpr[A]<=gpr[B] << gpr[C]
        SHR = 0b01110001,               // gpr[A]<=gpr[B] >> gpr[C]

        ST = 0b10000000,                // mem32[gpr[A]+gpr[B]+D]<=gpr[C]
        ST_IND = 0b10000010,            // mem32[mem32[gpr[A]+gpr[B]+D]]<=gpr[C]
        ST_POST_INC = 0b10000001,       // gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C] // PUSH

        LD_CSR = 0b10010000,            // gpr[A]<=csr[B] ## CSRRD
        LD = 0b10010001,                // gpr[A]<=gpr[B]+D // IRET -> pop status; pop pc;
        LD_IND = 0b10010010,            // gpr[A]<=mem32[gpr[B]+gpr[C]+D]
        LD_POST_INC = 0b10010011,       // gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D ## POP, RET

        CSR_LD = 0b10010100,            // csr[A]<=gpr[B] ## CSRWR
        CSR_LD_OR = 0b10010101,         // csr[A]<=csr[B]|D
        CSR_LD_IND = 0b10010110,        // csr[A]<=mem32[gpr[B]+gpr[C]+D]
        CSR_LD_POST_INC = 0b10010111,   // csr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D
    };

    static std::map<I::INSTRUCTION, std::string> NAMES;
};

class Instruction {
public:

    uint8_t _byte_1;
    uint8_t _byte_2{};
    uint8_t _byte_3{};
    uint8_t _byte_4{};

    explicit Instruction(I::INSTRUCTION byte_1, uint8_t regA = 0, uint8_t regB = 0, uint8_t regC = 0, int offset = 0)
            : _byte_1(byte_1), _byte_2(regA << 4 | regB), _byte_3(regC << 4) {
        if (offset < -2048 || offset > 2047)
            throw std::runtime_error("Displacement out of range.");
        _byte_3 |= (offset & 0xF00) >> 8;
        _byte_4 |= offset & 0xFF;
    }

    virtual void log();

    virtual void setRegA(uint8_t regA) final { _byte_2 |= regA << 4; }

    virtual void setRegB(uint8_t regB) final { _byte_2 |= regB; }

    virtual void setRegC(uint8_t regC) final { _byte_3 |= regC << 4; }

    virtual void setDisplacement(int16_t offset) final {
        if (offset < -2048 || offset > 2047)
            throw std::runtime_error("Displacement out of range.");
        _byte_3 |= (offset & 0xF00) >> 8;
        _byte_4 |= offset & 0xFF;
    }
};

class Halt_Instr : public Instruction {
public:
    explicit Halt_Instr() : Instruction(I::HALT) {}
};

class Push_Instr : public Instruction {
public:
    explicit Push_Instr(uint8_t gpr)
            : Instruction(I::ST_POST_INC, gpr) {}
};

class Pop_Instr : public Instruction {
public:
    explicit Pop_Instr(uint8_t grp)
            : Instruction(I::LD_POST_INC, grp) {}
};

class Not_Instr : public Instruction {
public:
    explicit Not_Instr(uint8_t gpr)
            : Instruction(I::NOT, gpr) {}
};

class Int_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Int_Instr(std::unique_ptr<Operand> operand)
            : Instruction(I::INT), _operand(std::move(operand)) {}
};

class Xchg_Instr : public Instruction {
public:
    explicit Xchg_Instr(uint8_t regA, uint8_t regB)
            : Instruction(I::XCHG, regA, regB) {}
};

class Csrrd_Instr : public Instruction {
public:
    explicit Csrrd_Instr(unsigned char csr, unsigned char gpr)
            : Instruction(I::LD_CSR, csr, gpr) {}
};

class Csrwr_Instr : public Instruction {
public:
    explicit Csrwr_Instr(unsigned char gpr, unsigned char csr)
            : Instruction(I::CSR_LD, gpr, csr) {}
};

class Load_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Load_Instr(std::unique_ptr<Operand> operand, unsigned char gpr)
            : Instruction(I::LD, gpr), _operand(std::move(operand)) {}
};

class Store_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Store_Instr(unsigned char gpr, std::unique_ptr<Operand> operand)
            : Instruction(I::ST, gpr), _operand(std::move(operand)) {}
};

class TwoReg_Instr : public Instruction {
public:
    explicit TwoReg_Instr(I::INSTRUCTION instruction, unsigned char regD, unsigned char regS)
            : Instruction(instruction, regD, regS) {}
};

class Jmp_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Jmp_Instr(I::INSTRUCTION instruction, std::unique_ptr<Operand> operand)
            : Instruction(instruction), _operand(std::move(operand)) {}
};

class Call_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Call_Instr(I::INSTRUCTION instruction, std::unique_ptr<Operand> operand)
            : Instruction(instruction), _operand(std::move(operand)) {}
};

class JmpCond_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit JmpCond_Instr(I::INSTRUCTION instruction, std::unique_ptr<Operand> operand)
            : Instruction(instruction), _operand(std::move(operand)) {}
};

class NoAdr_Instr : public Instruction {
public:
    explicit NoAdr_Instr(I::INSTRUCTION instruction) : Instruction(instruction) {}
};

class Instructions {
    std::vector<std::unique_ptr<Instruction>> _table;
public:
    void addInstruction(std::unique_ptr<Instruction>);

    void log();
};


