#pragma once

#include <string>
#include <vector>

class Operand {
    std::string _value;
public:
    explicit Operand(const std::string &);
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

};
