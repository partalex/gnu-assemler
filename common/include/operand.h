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

    virtual Addressing addRelocation(Assembler *);
};

class WordOperand : public Operand {
public:
    WordOperand *next = nullptr;

    virtual ~WordOperand() { delete next; }

    explicit WordOperand(WordOperand *next = nullptr) : next(next) {
    }

    virtual void logOne(std::ostream &) = 0;

    void log(std::ostream &) override;

    virtual void *getValue() = 0;

};

class WordLiteral : public WordOperand {
    uint32_t value;
public:
    explicit WordLiteral(uint32_t value, WordOperand *next = nullptr)
            : value(value), WordOperand(next) {}

    void *getValue() override;

    void logOne(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;
};

class WordIdent : public WordOperand {
    std::string ident;
public:
    explicit WordIdent(std::string ident, WordOperand *next = nullptr)
            : ident(std::move(ident)), WordOperand(next) {}

    void *getValue() override;

    void logOne(std::ostream &) override;

    std::string stringValue() override;

    Addressing addRelocation(Assembler *) override;

};

class EquOperand : public Operand {
public:
    EQU_OP op;
    EquOperand *next = nullptr;

    virtual ~EquOperand() { delete next; }

    explicit EquOperand(unsigned char op = E_ADD, EquOperand *next = nullptr)
            : op(static_cast<EQU_OP>(op)), next(next) {}

    virtual void logOne(std::ostream &) = 0;

    void log(std::ostream &) override;

    virtual bool isLabel() = 0;

    virtual std::string getIdent() = 0;

    virtual bool isResolved(Assembler *) = 0;

    virtual uint32_t getValue() = 0;
};

class EquLiteral : public EquOperand {
    uint32_t value;
public:
    explicit EquLiteral(uint32_t value, unsigned char op = E_ADD, EquOperand *next = nullptr)
            : value(value), EquOperand(op, next) {}

    uint32_t getValue() override;

    void logOne(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

    bool isResolved(Assembler *) override { return true; }

    bool isLabel() override;

    std::string getIdent() override;

};

class EquIdent : public EquOperand {
    std::string ident;
public:
    explicit EquIdent(std::string ident, unsigned char op = E_ADD, EquOperand *next = nullptr)
            : ident(std::move(ident)), EquOperand(op, next) {}

    uint32_t getValue() override;

    void logOne(std::ostream &) override;

    std::string stringValue() override;

    Addressing addRelocation(Assembler *) override;

    bool isLabel() override;

    bool isResolved(Assembler *) override;

    std::string getIdent() override;
};

class LiteralImm : public Operand {
public:
    explicit LiteralImm(uint32_t value) : value(value) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint32_t value;
};

class LiteralInDir : public Operand {
public:
    explicit LiteralInDir(uint32_t value) : value(value) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint32_t value;
};

class IdentImm : public Operand {
public:
    explicit IdentImm(std::string ident) :
            ident(std::move(ident)) {}

    void log(std::ostream &) override;

    std::string stringValue() override { return ident; }

    Addressing addRelocation(Assembler *) override;

private:
    std::string ident;
};

class IdentInDir : public Operand {
public:
    explicit IdentInDir(std::string ident) :
            ident(std::move(ident)) {}

    void log(std::ostream &) override;

    std::string stringValue() override { return ident; }

    Addressing addRelocation(Assembler *) override;

private:
    std::string ident;
};

class RegInDir : public Operand {
public:
    explicit RegInDir(uint8_t gpr) : gpr(gpr) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint8_t gpr;
};

class RegDir : public Operand {
public:
    explicit RegDir(uint8_t gpr) : gpr(gpr) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint8_t gpr;
};

class RegInDirOffLiteral : public Operand {
public:
    explicit RegInDirOffLiteral(uint8_t gpr, uint32_t offset) : gpr(gpr), offset(offset) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

private:
    uint8_t gpr;
    uint32_t offset;
};

class RegInDirOffIdent : public Operand {
public:
    explicit RegInDirOffIdent(uint8_t gpr, std::string ident) : gpr(gpr), ident(std::move(ident)) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;


    std::string stringValue() override { return ident; }

private:
    uint8_t gpr;
    std::string ident;
};

class CsrOp : public Operand {
public:
    explicit CsrOp(uint8_t csr) : csr(csr) {}

    void log(std::ostream &) override;

    Addressing addRelocation(Assembler *) override;

    std::string stringValue() override;

private:
    uint8_t csr;
};
