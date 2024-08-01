#pragma once

#include "enum.h"
#include "operand.h"

#include <map>
#include <memory>
#include <vector>
#include <stdexcept>

class Operand;

class Program;

class Instruction {
public:

    Mnemonic bytes;

    explicit Instruction(enum INSTRUCTION, uint8_t regA = 0, uint8_t regB = 0, uint8_t regC = 0);

    explicit Instruction(uint32_t);

    virtual void setInstr(uint8_t) final;

    virtual void setMode(uint8_t) final;

    virtual void andMode(uint8_t) final;

    virtual void setRegA(uint8_t) final;

    virtual void setRegB(uint8_t) final;

    virtual void setRegC(uint8_t) final;

    virtual void setDisplacement(int32_t) final;

    static void execute(Mnemonic , Program &);

    void static tableHeader(std::ostream &);

    friend std::ostream &operator<<(std::ostream &, Instruction &);

    virtual std::ostream &logExecute(std::ostream &) const final;

    static bool fitIn12Bits(int32_t);

};

class Halt_Instr : public Instruction {
public:
    explicit Halt_Instr() : Instruction(INSTRUCTION::HALT) {}

    explicit Halt_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Push_Instr : public Instruction {
public:
    explicit Push_Instr(uint8_t);

    explicit Push_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Pop_Instr : public Instruction {
public:
    explicit Pop_Instr(uint8_t);

    explicit Pop_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Not_Instr : public Instruction {
public:
    explicit Not_Instr(uint8_t gpr);

    explicit Not_Instr(uint32_t bytes) : Instruction(bytes) {}


};

class Int_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Int_Instr();

    explicit Int_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Xchg_Instr : public Instruction {
public:
    explicit Xchg_Instr(uint8_t, uint8_t);

    explicit Xchg_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Csrrd_Instr : public Instruction {
public:
    explicit Csrrd_Instr(uint8_t, uint8_t);

    explicit Csrrd_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Csrwr_Instr : public Instruction {
public:
    explicit Csrwr_Instr(uint8_t, uint8_t);

    explicit Csrwr_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Load_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Load_Instr(std::unique_ptr<Operand>, uint8_t);

    explicit Load_Instr(uint32_t bytes) : Instruction(bytes) {}

    explicit Load_Instr(uint8_t, uint8_t, int16_t);

};

class Store_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Store_Instr(uint8_t, std::unique_ptr<Operand>);

    explicit Store_Instr(uint32_t bytes) : Instruction(bytes) {}


};

class TwoReg_Instr : public Instruction {
public:
    explicit TwoReg_Instr(enum INSTRUCTION, uint8_t, uint8_t);

    explicit TwoReg_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Jmp_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Jmp_Instr(enum INSTRUCTION instruction, std::unique_ptr<Operand>);

    explicit Jmp_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Call_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit Call_Instr(enum INSTRUCTION, std::unique_ptr<Operand>);

    explicit Call_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class JmpCond_Instr : public Instruction {
    std::unique_ptr<Operand> _operand;
public:
    explicit JmpCond_Instr(enum INSTRUCTION, std::unique_ptr<Operand>);

    explicit JmpCond_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class NoAdr_Instr : public Instruction {
public:
    explicit NoAdr_Instr(enum INSTRUCTION);

    explicit NoAdr_Instr(uint32_t bytes) : Instruction(bytes) {}

};
