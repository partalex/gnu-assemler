#pragma once

#include <vector>
#include <memory>
#include <iostream>

enum MARKER {
    UNDEFINED = 0xFFFFFFFF,
    LITERALS = 0xFFFFFFFE
};

enum EQU_OP {
    E_ADD, E_SUB
};

enum ADDRESSING {
    ADDR_UND,
    REG_DIR,            // reg = reg[B=gpr] + D=value                   // noRel
    IN_DIR_OFFSET,      // reg = mem[reg[B=PC] + reg[C=0] + offset] // noRel or R_PC_12Bits
    IN_DIR_INDEX,       // reg = mem[reg[B]]   reg[B]=reg[B]+index; // noRel
    CSR_OP              // reg = csr[B] + D                         // noRel
};

// gpr[B] = PC, cause PC is gpr[15]
//1. $<literal> - vrednost <literal>
// literal fitIn12Bits ?
// D=literal; gpr[A]<=gpr[0]+D; IMMEDIATE_LITERAL_RO:
// : Need relocation R_PC32; D=literal; gpr[A]<=gpr[15]+D; IMMEDIATE_LITERAL_PC

//2. $<simbol> - vrednost <simbol>
// IMMEDIATE_SYMBOL:            gpr[A]<=gpr[15]+D; Need relocation

//3. <literal> - vrednost iz memorije na adresi <literal>
// literal fitIn12Bits ?
// INDIRECT_LITERAL_12bits:            gpr[A]<=mem32[gpr[15]+gpr[0]+D]; D<=literal no relocation
// INDIRECT_LITERAL:                   gpr[A]<=mem32[gpr[15]+gpr[0]+D]; Need relocation R_PC32 for D

//4. <simbol> - vrednost iz memorije na adresi <simbol>
// literal fitIn12Bits ?
// INDIRECT_SYMBOL:             gpr[A]<=mem32[gpr[15]+gpr[0]+D]; D=literal; Need relocation R_PC32
// : :                          gpr[A]<=mem32[gpr[B]+gpr[0]+D]; D=mem32[symbol] <= 12bits ? mem32[symbol]: relocation

//5. %<reg> - vrednost u registru <reg>
// DIRECT_REGISTER:             gpr[A]<=gpr[B]+D, D<=0

//6. [%<reg>] - vrednost iz memorije na adresi <reg>
// INDIRECT_REGISTER:           gpr[A]<=mem32[gpr[B]+gpr[0]+D]; D<=0

//7. [%<reg> + <literal>] - vrednost iz memorije na adresi <reg> + <literal>1
// INDIRECT_REG_LITERAL:        gpr[A]<=mem32[gpr[B]+gpr[0]+D]; D= literal <= 12bits ? literal: error

//8. [%<reg> + <simbol>] - vrednost iz memorije na adresi <reg> + <simbol>2
// INDIRECT_REG_SYMBOL:         gpr[A]<=mem32[gpr[B]+gpr[0]+D]; D= mem32[symbol] <= 12bits ? mem32[symbol]: error

enum SOURCE {
    THIS,
    OTHER
};

enum DEFINED {
    NOT_DEFINED, DEFINED
};

enum REG_GPR {
    GPR_R0,
    GPR_R1,
    GPR_R2,
    GPR_R3,
    GPR_R4,
    GPR_R5,
    GPR_R6,
    GPR_R7,
    GPR_R8,
    GPR_R9,
    GPR_R10,
    GPR_R11,
    GPR_R12,
    GPR_R13,
    GPR_R14,
    REG_SP = GPR_R14,
    GPR_R15,
    REG_PC = GPR_R15,
};

enum REG_CSR {
    CSR_STATUS,
    CSR_HANDLER,
    CSR_CAUSE
};

enum RELOCATION {
    R_PC_12Bits,
    R_WORD
};

enum STATUS {
    FAULT = 1, TIMER, TERMINAL, SOFTWARE
};

enum SYMBOL {
    NO_TYPE, // from: .extern, .global or equExpr
    ASCII,
    LABEL,
    EQU
};

enum SCOPE {
    LOCAL, GLOBAL
};

typedef union {
    uint32_t val;
    struct {
        uint32_t Tr: 1, Tl: 1, I: 1, : 24, Z: 1, O: 1, C: 1, N: 1;
    };
} PSW;

struct Addressing {
    ADDRESSING addressing = ADDR_UND;
    int32_t value = 0;
    uint8_t reg = 0;
};

struct EquResolved {
    bool resolved = false;
    int32_t value = 0;
};

enum INSTRUCTION {
    HALT = 0b00000000,              // halt
    INT = 0b00010000,               // push status; push pc; cause<=4; status<=status&(~0x1); pc<=handler;
    CALL = 0b00100000,              // push pc; pc<=gpr[A]+gpr[B]+D
    CALL_MEM = 0b00100001,          // push pc; pc<=memory[gpr[A]+gpr[B]+D]

    JMP = 0b00110000,               // pc<=gpr[A]+D
    BEQ = 0b00110001,               // if (gpr[B] == gpr[C]) pc<=gpr[A]+D
    BNE = 0b00110010,               // if (gpr[B] != gpr[C]) pc<=gpr[A]+D
    BGT = 0b00110011,               // if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D
    JMP_MEM = 0b00111000,           // pc<=memory[gpr[A]+D]
    BEQ_MEM = 0b00111001,           // if (gpr[B] == gpr[C]) pc<=memory[gpr[A]+D]
    BNE_MEM = 0b00111010,           // if (gpr[B] != gpr[C]) pc<=memory[gpr[A]+D]
    BGT_MEM = 0b00111011,           // if (gpr[B] signed> gpr[C]) pc<=memory[gpr[A]+D]

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

    ST = 0b10000000,                // memory[gpr[A]+gpr[B]+D]<=gpr[C]
    ST_IND = 0b10000010,            // memory[memory[gpr[A]+gpr[B]+D]]<=gpr[C]
    ST_POST_INC = 0b10000001,       // gpr[A]<=gpr[A]+D; memory[gpr[A]]<=gpr[C] // PUSH

    LD_CSR = 0b10010000,            // gpr[A]<=csr[B] ## CSRRD
    LD = 0b10010001,                // gpr[A]<=gpr[B]+D // IRET -> pop status; pop pc;
    LD_IND = 0b10010010,            // gpr[A]<=memory[gpr[B]+gpr[C]+D]
    LD_POST_INC = 0b10010011,       // gpr[A]<=memory[gpr[B]]; gpr[B]<=gpr[B]+D ## POP, RET

    CSR_LD = 0b10010100,            // csr[A]<=gpr[B] ## CSRWR
    CSR_LD_OR = 0b10010101,         // csr[A]<=csr[B]|D
    CSR_LD_IND = 0b10010110,        // csr[A]<=memory[gpr[B]+gpr[C]+D]
    CSR_LD_POST_INC = 0b10010111,   // csr[A]<=memory[gpr[B]]; gpr[B]<=gpr[B]+D
};

typedef union {
    struct {
        uint32_t byte_0: 8, byte_1: 8, byte_2: 8, byte_3: 8;
    };
    struct {
        uint32_t MODE: 4, OC: 4, REG_B: 4, REG_A: 4, REG_C: 4, DISPLACEMENT: 12;
    };
    uint32_t value;
} Mnemonic;


std::ostream &operator<<(std::ostream &, MARKER);

std::ostream &operator<<(std::ostream &, enum DEFINED);

std::ostream &operator<<(std::ostream &, SOURCE);

std::ostream &operator<<(std::ostream &, EQU_OP);

std::ostream &operator<<(std::ostream &, enum REG_GPR);

std::ostream &operator<<(std::ostream &, enum REG_CSR);

std::ostream &operator<<(std::ostream &, enum SYMBOL);

std::ostream &operator<<(std::ostream &, SCOPE);

std::ostream &operator<<(std::ostream &, enum INSTRUCTION);

std::ostream &operator<<(std::ostream &, RELOCATION);

