#pragma once
#include "enum.h"

#include <string>
#include <cstdint>
#include <optional>
#include <sstream>

class Assembler;

class Operand {
public:

    explicit Operand() = default;

    virtual void log(std::ostream &out) {}

    virtual std::string stringValue() {
        throw std::runtime_error("Operand::stringValue() not implemented");
    }

    virtual std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &);
};

class WordOperand : public Operand {
public:
    WordOperand *_next = nullptr;

    virtual ~WordOperand() { delete _next; }

    explicit WordOperand(WordOperand *next = nullptr) : _next(next) {
    }

    virtual void logOne(std::ostream &) = 0;

    void log(std::ostream &) override;

    virtual void *getValue() = 0;

};

class WordLiteral : public WordOperand {
    int32_t _value;
public:
    explicit WordLiteral(int32_t value, WordOperand *next = nullptr) : _value(value) {
        _next = next;
    }

    void *getValue() override;

    void logOne(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;
};

class WordIdent : public WordOperand {
    std::string _ident;
public:
    explicit WordIdent(std::string ident, WordOperand *next = nullptr) : _ident(std::move(ident)) {
        _next = next;
    }

    void *getValue() override;

    void logOne(std::ostream &) override;

    std::string stringValue() override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

};

class EquOperand : public Operand {
public:
    EQU_OP _op;
    EquOperand *_next = nullptr;

    virtual ~EquOperand() { delete _next; }

    explicit EquOperand(unsigned char op = E_ADD, EquOperand *next = nullptr)
            : _op(static_cast<EQU_OP>(op)), _next(next) {}

    virtual void logOne(std::ostream &) = 0;

    void log(std::ostream &) override;

    virtual void backPatch(Assembler &) = 0;

    virtual bool isLabel() = 0;

    virtual int64_t getValue() { return 0; }
};

class EquLiteral : public EquOperand {
    int64_t _value;
public:
    explicit EquLiteral(int64_t value, unsigned char op = E_ADD, EquOperand *next = nullptr)
            : _value(value), EquOperand(op, next) {}

    int64_t getValue() override;

    void logOne(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

    void backPatch(Assembler &) override;

    bool isLabel() override { return false; }

};

class EquIdent : public EquOperand {
    std::string _ident;
public:
    explicit EquIdent(std::string ident, unsigned char op = E_ADD, EquOperand *next = nullptr)
            : _ident(std::move(ident)), EquOperand(op, next) {}

    int64_t getValue() override;

    void logOne(std::ostream &) override;

    std::string stringValue() override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

    bool isLabel() override { return true; }

    void backPatch(Assembler &) override;
};

class LiteralImm : public Operand {
public:
    explicit LiteralImm(int32_t value) : _value(value) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    int32_t _value;
};

class LiteralImmReg : public Operand {
public:
    explicit LiteralImmReg(int32_t value) : _value(value) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    int32_t _value;

};

class LiteralInDir : public Operand {
public:
    explicit LiteralInDir(int32_t value) : _value(value) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    int32_t _value;
};

class IdentImm : public Operand {
public:
    explicit IdentImm(std::string ident) : _ident(std::move(ident)) {}

    void log(std::ostream &) override;

    std::string stringValue() override { return _ident; }

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    std::string _ident;
};

class IdentAddr : public Operand {
public:
    explicit IdentAddr(std::string ident) :
            _ident(std::move(ident)) {}

    void log(std::ostream &) override;

    std::string stringValue() override { return _ident; }

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    std::string _ident;
};

class RegInDir : public Operand {
public:
    explicit RegInDir(uint8_t gpr) : _gpr(gpr) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    uint8_t _gpr;
};

class RegDir : public Operand {
public:
    explicit RegDir(uint8_t gpr) : _gpr(gpr) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    uint8_t _gpr;
};

class RegInDirOffLiteral : public Operand {
public:
    explicit RegInDirOffLiteral(uint8_t gpr, int32_t offset) : _gpr(gpr), _offset(offset) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

private:
    uint8_t _gpr;
    int32_t _offset;
};

class RegInDirOffIdent : public Operand {
public:
    explicit RegInDirOffIdent(uint8_t gpr, std::string ident) : _gpr(gpr), _ident(std::move(ident)) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;


    std::string stringValue() override { return _ident; }

private:
    uint8_t _gpr;
    std::string _ident;
};

class GprCsr : public Operand {
public:
    explicit GprCsr(uint8_t gpr, uint8_t csr) : _gpr(gpr), _csr(csr) {
    }

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

    void log(std::ostream &) override;

private:
    uint8_t _gpr;
    uint8_t _csr;
};

class TwoReg : public Operand {
public:
    explicit TwoReg(uint8_t gpr1, uint8_t gpr2) :
            _gpr1(gpr1), _gpr2(gpr2) {}

    void log(std::ostream &) override;

    uint8_t getGpr1() const { return _gpr1; }

    uint8_t getGpr2() const { return _gpr2; }

private:
    uint8_t _gpr1;
    uint8_t _gpr2;
};


class GprGprIdent : public TwoReg {
public:
    explicit GprGprIdent(uint8_t gpr1, uint8_t gpr2, std::string ident) :
            TwoReg(gpr1, gpr2), _ident(std::move(ident)) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

    std::string stringValue() override { return _ident; }

private:
    std::string _ident;
};

class GprGprLiteral : public TwoReg {
public:
    explicit GprGprLiteral(uint8_t gpr1, uint8_t gpr2, int32_t value) :
            TwoReg(gpr1, gpr2), _value(value) {}

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

    void log(std::ostream &) override;

private:
    int32_t _value;
};

class CsrOp : public Operand {
public:
    explicit CsrOp(uint8_t csr) : _csr(csr) {}

    void log(std::ostream &) override;

    std::pair<ADDRESSING, uint32_t> addRelocation(Assembler &) override;

    std::string stringValue() override;

private:
    uint8_t _csr;
};
