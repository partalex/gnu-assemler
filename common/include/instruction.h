#pragma once

#include "enum.h"
#include "operand.h"

#include <map>
#include <memory>
#include <vector>
#include <stdexcept>

class Operand;

class Assembler;

class Instruction {
protected:
    bool isInDirInDir = false;
public:

    Mnemonic bytes;

    explicit Instruction(enum INSTRUCTION, uint8_t  = 0, uint8_t  = 0, uint8_t  = 0, int32_t = 0);

    explicit Instruction(uint32_t);

    virtual void setInstr(uint8_t) final;

    virtual void setMode(uint8_t) final;

    virtual void andMode(uint8_t) final;

    virtual void orMode(uint8_t) final;

    virtual void setRegA(uint8_t) final;

    virtual void setRegB(uint8_t) final;

    virtual void setRegC(uint8_t) final;

    virtual void setDisplacement(uint32_t) final;

    virtual void insertInstr(Assembler *);

    void static tableHeader(std::ostream &);

    friend std::ostream &operator<<(std::ostream &, Instruction &);

    virtual std::ostream &logExecute(std::ostream &) const final;

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

class IRet_Instr : public Instruction {
public:
    explicit IRet_Instr();

    explicit IRet_Instr(uint32_t bytes) : Instruction(bytes) {}

    void insertInstr(Assembler *) override;

};

class Not_Instr : public Instruction {
public:
    explicit Not_Instr(uint8_t gpr);

    explicit Not_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Int_Instr : public Instruction {
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
public:
    explicit Load_Instr(Operand *, uint8_t, Assembler *);

    explicit Load_Instr(uint32_t bytes) : Instruction(bytes) {}

    explicit Load_Instr(uint8_t, uint8_t, uint32_t);
};

class Store_Instr : public Instruction {
public:
    explicit Store_Instr(uint8_t, Operand *, Assembler *);

    explicit Store_Instr(uint32_t bytes) : Instruction(bytes) {}

    explicit Store_Instr(uint8_t, int16_t, uint8_t);
};

class TwoReg_Instr : public Instruction {
public:
    explicit TwoReg_Instr(enum INSTRUCTION, uint8_t, uint8_t);

    explicit TwoReg_Instr(uint32_t bytes) : Instruction(bytes) {}

};

class Jmp_Instr : public Instruction {
public:
    explicit Jmp_Instr(enum INSTRUCTION instruction, Operand *, Assembler *);

    explicit Jmp_Instr(uint32_t bytes) : Instruction(bytes) {}
};

class Call_Instr : public Instruction {
public:
    explicit Call_Instr(enum INSTRUCTION, Operand *, Assembler *);

    explicit Call_Instr(uint32_t bytes) : Instruction(bytes) {}
};

class JmpCond_Instr : public Instruction {
public:
    explicit JmpCond_Instr(uint32_t bytes) : Instruction(bytes) {}

    explicit JmpCond_Instr(INSTRUCTION, uint8_t, uint8_t, Operand *, Assembler *);
};
