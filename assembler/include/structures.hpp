#pragma once

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <memory>

class Operand {
protected:
public:
    Operand *_next = nullptr;

    explicit Operand() = default;

    virtual ~Operand() { delete _next; }

    void logOne();

    void log();
};

class WordOperand : public Operand {
public:
    explicit WordOperand(Operand *next = nullptr) {
        _next = next;
    }
};

class WordLiteral : public WordOperand {
    unsigned int _value;
public:
    explicit WordLiteral(unsigned int value, Operand *next = nullptr) : _value(value) {
        _next = next;
    }
};

class WordIdent : public WordOperand {
    std::string _ident;
public:
    explicit WordIdent(std::string ident, Operand *next = nullptr) : _ident(std::move(ident)) {
        _next = next;
    }
};

class LiteralOp : public Operand {
public:
    explicit LiteralOp(unsigned int value, Operand *next = nullptr) : _value(value) {
        _next = next;
    }

    unsigned int fromWord();

private:
    unsigned int _value;
};

class IdentOp : public Operand {
public:
    explicit IdentOp(std::string ident, Operand *next = nullptr) : _ident(std::move(ident)) {
        _next = next;
    }

    void fromWord();

private:
    std::string _ident;
};

class GprOp : public Operand {
public:
    explicit GprOp(unsigned char gpr, Operand *next = nullptr) : _gpr(gpr) {
        _next = next;
    }

private:
    unsigned char _gpr;
};

class GprLiteralOp : public Operand {
public:
    explicit GprLiteralOp(unsigned char gpr, unsigned int value, Operand *next = nullptr) : _gpr(gpr), _value(value) {
        _next = next;
    }

private:
    unsigned char _gpr;
    unsigned int _value;
};

class GprIdentOp : public Operand {
public:
    explicit GprIdentOp(unsigned char gpr, std::string ident, Operand *next = nullptr) : _gpr(gpr),
                                                                                         ident_(std::move(ident)) {
        _next = next;
    }

private:
    unsigned char _gpr;
    std::string ident_;
};

class GprCsrOp : public Operand {
public:
    explicit GprCsrOp(unsigned char gpr, unsigned char csr) : _gpr(gpr), _csr(csr) {
    }

private:
    unsigned char _gpr;
    unsigned char _csr;
};

class GprGprIdent : public Operand {
public:
    explicit GprGprIdent(unsigned char gpr1, unsigned char gpr2, std::string ident) : _gpr1(gpr1), _gpr2(gpr2),
                                                                                      _ident(std::move(ident)) {
    }

private:
    unsigned char _gpr1;
    unsigned char _gpr2;
    std::string _ident;
};

class GprGprLiteral : public Operand {
public:
    explicit GprGprLiteral(unsigned char gpr1, unsigned char gpr2, unsigned int value) : _gpr1(gpr1), _gpr2(gpr2),
                                                                                         _value(value) {
    }

private:
    unsigned char _gpr1;
    unsigned char _gpr2;
    unsigned int _value;
};

class CsrOp : public Operand {
public:
    explicit CsrOp(unsigned char csr, Operand *next = nullptr) : csr_(csr) {
        _next = next;
    }

private:
    unsigned char csr_;
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

struct SymbolList {
    std::string _symbol;
    SymbolList *_next = nullptr;

    explicit SymbolList(const std::string &);

    SymbolList(const std::string &, SymbolList *_next);

    ~SymbolList() { delete _next; }

    void log();
};

enum BIND {
    LOCAL,
    GLOBAL
};

enum ENTRY_TYPE {
    NO_TYPE,
    SECTION,
    ASCII,
    WORD
};

struct SymbolTableEntry {
    uint64_t _value;
    uint64_t _size;
    ENTRY_TYPE _type;
    BIND _bind;
    uint8_t _ndx;
    std::string _name;

    SymbolTableEntry(
            uint64_t value,
            uint64_t size,
            ENTRY_TYPE type,
            BIND bind,
            uint32_t ndx,
            std::string name
    ) : _value(value), _size(size), _type(type), _bind(bind), _ndx(ndx), _name(std::move(name)) {}

};

class SymbolTable {
    std::vector<SymbolTableEntry> _table;

public:
    void addSymbol(const SymbolTableEntry);

    bool checkSymbol(BIND, ENTRY_TYPE, const std::string &);

    bool hasUnresolvedSymbols();

    void log();

    SymbolTableEntry *getSymbol(BIND, ENTRY_TYPE, const std::string &);
};

enum RelocationType {
    R_386_32,
    R_386_PC32
};

class RelocationEntry {
    uint32_t _offset;
    RelocationType _type;
    uint32_t _value;
public:
    RelocationEntry(uint32_t offset, RelocationType type, uint32_t value) : _offset(offset), _type(type),
                                                                            _value(value) {}

    void log();
};

class RelocationTable {
    std::vector<RelocationEntry> _table;
public:
    void addRelocation(RelocationEntry &);

    void log();
};

class Instruction {
    uint8_t _byte_1;
    uint8_t _byte_2{};
    uint8_t _byte_3{};
    uint8_t _byte_4{};

public:
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
