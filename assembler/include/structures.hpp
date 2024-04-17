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

    void logOne();

public:
    explicit Operand(uint8_t selector = 0, uint8_t gpr1 = 0, uint32_t literal = 0, std::string symbol = "",
                     uint8_t csr = 0,
                     uint8_t gpr2 = 0, Operand *next = nullptr)
            : _selector(selector), _gpr1(gpr1), _literal(literal), _symbol(std::move(symbol)), _csr(csr), _gpr2(gpr2),
              _next(next) {}

    ~Operand() { delete _next; }

    void log();
};

class I {
public:
    enum INSTRUCTION {
        HALT = 0b00000000,
        IRET = 0b10010001,
        RET = 0b10010011,
        INT = 0b00010000,
        CALL = 0b00100000,
        JMP = 0x30, // oruje se
        POP = 0b10010011,
        PUSH = 0b10000001,
        NOT = 0b01100000,
        XCHG = 0b01000000,
        ADD = 0b01010000,
        SUB = 0b01010001,
        MUL = 0b01010010,
        DIV = 0b01010011,
        AND = 0b01100001,
        OR = 0b01100010,
        XOR = 0b01100011,
        SHL = 0b01110000,
        SHR = 0b01110001,
        CSRRD = 0b10010000,
        CSRWR = 0b10010100,
        LD = 0x90, // oruje se
        ST = 0x80, // oruje se
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
