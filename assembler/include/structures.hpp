#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>

class Operand {
    uint8_t _selector;
    uint8_t _gpr1;
    uint32_t _literal;
    std::string _symbol;
    uint8_t _gpr2;
    uint8_t _csr;
    Operand *_next = nullptr;

public:
    explicit Operand(uint8_t selector = 0, uint8_t gpr1 = 0, uint32_t literal = 0, std::string symbol = "",
                     uint8_t csr = 0,
                     uint8_t gpr2 = 0, Operand *next = nullptr)
            : _selector(selector), _gpr1(gpr1), _literal(literal), _symbol(std::move(symbol)), _csr(csr), _gpr2(gpr2),
              _next(next) {}

    ~Operand() { delete _next; }

    void logOne();

    void log();
};

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

class Csr {
public:
    static std::string CSR[];
    enum CSR {
        STATUS,
        HANDLER,
        CAUSE
    };
};

class SymbolList {
    std::string _symbol;
    SymbolList *_next = nullptr;
public:
    explicit SymbolList(const std::string &);

    SymbolList(const std::string &, SymbolList *_next);

    ~SymbolList() { delete _next; }

    void log();
};
