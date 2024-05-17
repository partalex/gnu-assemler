#pragma once

#include "instruction.h"
#include "enum.h"

#include <map>
#include <memory>
#include <vector>
#include <stdexcept>

class Operand;

class Instruction {
public:

    uint8_t _byte_1;
    uint8_t _byte_2{};
    uint8_t _byte_3{};
    uint8_t _byte_4{};

    explicit Instruction(enum INSTRUCTION byte_1, uint8_t regA = 0, uint8_t regB = 0, uint8_t regC = 0, int offset = 0)
            : _byte_1(byte_1), _byte_2(regA << 4 | regB), _byte_3(regC << 4) {
        if (offset < -2048 || offset > 2047)
            throw std::runtime_error("Displacement out of range.");
        _byte_3 |= (offset & 0xF00) >> 8;
        _byte_4 |= offset & 0xFF;
    }

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
    explicit Halt_Instr() : Instruction(INSTRUCTION::HALT) {}
};

class Push_Instr : public Instruction {
public:
    explicit Push_Instr(uint8_t);
};

class Pop_Instr : public Instruction {
public:
    explicit Pop_Instr(uint8_t);
};

class Not_Instr : public Instruction {
public:
    explicit Not_Instr(uint8_t gpr);
};

class Int_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Int_Instr(std::unique_ptr<Operand>);
};

class Xchg_Instr : public Instruction {
public:
    explicit Xchg_Instr(uint8_t, uint8_t);
};

class Csrrd_Instr : public Instruction {
public:
    explicit Csrrd_Instr(uint8_t, uint8_t);
};

class Csrwr_Instr : public Instruction {
public:
    explicit Csrwr_Instr(uint8_t, uint8_t);
};

class Load_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Load_Instr(std::unique_ptr<Operand>, uint8_t);
};

class Store_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Store_Instr(uint8_t, std::unique_ptr<Operand>);
};

class TwoReg_Instr : public Instruction {
public:
    explicit TwoReg_Instr(enum INSTRUCTION, uint8_t, uint8_t);
};

class Jmp_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Jmp_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand>);
};

class Call_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Call_Instr(enum INSTRUCTION, std::unique_ptr<Operand>);
};

class JmpCond_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit JmpCond_Instr(enum INSTRUCTION, std::unique_ptr<Operand>);
};

class NoAdr_Instr : public Instruction {
public:
    explicit NoAdr_Instr(enum INSTRUCTION);
};

class Instructions {
    std::vector<std::unique_ptr<Instruction>> _table;
public:
    void addInstruction(std::unique_ptr<Instruction>);
};
