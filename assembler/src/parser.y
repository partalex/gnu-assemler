%{
//    #include <cstdint>
    #include <stdint.h> // Izmenite ovu liniju
	#include <cstdio>
	#include "../include/log.hpp"
	#include "../include/assembler.hpp"
	#include "../include/structures.hpp"

	int yylex(void);
	void yyerror(const char*);

    #ifdef YYDEBUG
        yydebug = 1;
    #endif

%}

%output "./src/parser.cpp"
%defines "./include/parser.hpp"

%union {
	char*               ident;
    unsigned char       num_u8;
    unsigned int        num_32;
    class SymbolList*   symbolList;
    class Operand*      operand;
}

%token            T_COMMA
%token            T_COLON
%token            T_DOLLAR
%token            T_PERCENT
%token            T_OBRACKET
%token            T_CBRACKET
%token            T_PLUS
%token            T_MINUS
%token            T_TIMES
%token            T_SLASH
%token            T_GLOBAL
%token            T_EXTERN
%token            T_SECTION
%token            T_WORD
%token            T_SKIP
%token            T_ASCII
%token            T_END
%token<ident>     T_STRING
%token            I_HALT
%token            I_INT
%token            I_IRET
%token            I_CALL
%token            I_RET
%token            I_JMP
%token            I_BEQ
%token            I_BNE
%token            I_BGT
%token            I_PUSH
%token            I_POP
%token            I_XCHG
%token            I_ADD
%token            I_SUB
%token            I_MUL
%token            I_DIV
%token            I_NOT
%token            I_AND
%token            I_OR
%token            I_XOR
%token            I_SHL
%token            I_SHR
%token            I_LD
%token            I_ST
%token            I_CSRRD
%token            I_CSRWR
%token            CSR_STATUS
%token            CSR_HANDLER
%token            CSR_CAUSE

%token <num_u8>   REG
%token <num_u8>   SP
%token <num_u8>   PC
%token <num_32>   T_LITERAL
%token <ident>    T_IDENT

%type <num_u8>      gpr
%type <num_u8>      csr
%type <num_u8>      noadr
%type <num_u8>      jmp
%type <num_u8>      tworeg
%type <operand>     jmpOperand
%type <operand>     intOperand
%type <operand>     oneRegOperand
%type <operand>     wordOperand
%type <symbolList>  symbolList

%%

prog
  :
  | instr prog;

instr
  : label
  | directive
  | instruction;

label
  : T_IDENT T_COLON
  { Assembler::singleton().parseLabel($1);}

  | csr T_COLON
  { Assembler::singleton().parseLabel(Csr::CSR[$1]);};
  
directive
  : T_GLOBAL symbolList
  { Assembler::singleton().parseGlobal($2); }

  | T_EXTERN symbolList
  { Assembler::singleton().parseExtern($2); }

  | T_SECTION T_IDENT
  { Assembler::singleton().parseSection($2); }

  | T_WORD wordOperand
  { Assembler::singleton().parseWord($2); }

  | T_SKIP T_LITERAL
  { Assembler::singleton().parseSkip($2); }

  | T_ASCII T_STRING
  { Assembler::singleton().parseAscii($2); }

  | T_END
  { Assembler::singleton().parseEnd(); };

wordOperand
  : T_LITERAL
  { $$ = new Operand(0b01000000, 0, $1); }

  | T_LITERAL T_COMMA wordOperand
  { $$ = new Operand(0b01000100, 0, $1, "", 0, 0, $3); }

  | T_IDENT
  { $$ = new Operand(0b00100000, 0, 0, $1); }

  | T_IDENT T_COMMA wordOperand
  { $$ = new Operand(0b00100100, 0, 0, $1, 0, 0, $3); };

instruction
  : I_HALT
  { Assembler::singleton().parseHalt(); }

  | noadr
  { Assembler::singleton().parseNoAdr($1); }

  | jmp jmpOperand
  { Assembler::singleton().parseJmp($1, $2); }

  | I_PUSH T_PERCENT REG
  { Assembler::singleton().parsePush($3); }

  | I_POP T_PERCENT REG 
  { Assembler::singleton().parsePop($3); }

  | I_NOT T_PERCENT REG 
  { Assembler::singleton().parseNot($3); }

  | I_INT T_PERCENT intOperand
  { Assembler::singleton().parseInt($3); }

  | I_XCHG T_PERCENT REG T_COMMA T_PERCENT REG
  { Assembler::singleton().parseXchg($3, $6); }

  | tworeg T_PERCENT REG T_COMMA T_PERCENT REG
  { Assembler::singleton().parseTwoReg($1, $3, $6); }

  | I_CSRRD T_PERCENT csr T_COMMA T_PERCENT REG
  { Assembler::singleton().parseCsrrd($3, $6); }

  | I_CSRWR T_PERCENT REG T_COMMA T_PERCENT csr
  { Assembler::singleton().parseCsrwr($3, $6); }

  | I_LD oneRegOperand T_COMMA T_PERCENT gpr
  { Assembler::singleton().parseLoad($2, $5); }

  | I_ST T_PERCENT gpr T_COMMA oneRegOperand
  { Assembler::singleton().parseStore($3, $5); };

symbolList
  : T_IDENT
  { $$ = new SymbolList($1); }

  | T_IDENT T_COMMA symbolList
  { $$ = new SymbolList(std::string($1), $3); }

  | csr
  { $$ = new SymbolList(Csr::CSR[$1]); }

  | csr T_COMMA symbolList
  { $$ = new SymbolList(Csr::CSR[$1], $3); };

// Operand(mode, gpr1, literal, ident, csr, gpr2, * next)
// Operand(   1,    2,       3,     4,    5,    6,    7 )
oneRegOperand
  : T_DOLLAR T_LITERAL
  { $$ = new Operand(0b10000000, $2); }

  | T_DOLLAR T_IDENT
  { $$ = new Operand(0b00100000, 0, 0, $2); }

  | T_DOLLAR csr
  { $$ = new Operand(0b00010000, 0, 0, "", $2); }

  | T_LITERAL
  { $$ = new Operand(0b01000000, 0, $1); }

  | T_IDENT
  { $$ = new Operand(0b00100000, 0, 0, $1); }

  | gpr
  { $$ = new Operand(0b10000000, $1); }

  | T_OBRACKET gpr T_CBRACKET
  { $$ = new Operand(0b10000000, $2); }

  | T_OBRACKET gpr T_PLUS T_LITERAL T_CBRACKET
  { $$ = new Operand(0b11000000, $2, $4); }

  | T_OBRACKET gpr T_PLUS T_IDENT T_CBRACKET
  { $$ = new Operand(0b10100000, $2, 0, $4); }

  | T_OBRACKET gpr T_PLUS T_DOLLAR csr T_CBRACKET
  { $$ = new Operand(0b10010000, $2, 0, 0, $5); };

csr
  : CSR_STATUS
  { $$ = Csr::CSR::STATUS; }

  | CSR_HANDLER
  { $$ = Csr::CSR::HANDLER; }

  | CSR_CAUSE
  { $$ = Csr::CSR::CAUSE; };

gpr
  : REG
  | SP
  | PC;

intOperand
  : REG
  { $$ = new Operand(0b10000000, $1); }

  | T_LITERAL
  { $$ = new Operand(0b01000000, 0, $1); }

  | T_IDENT
  { $$ = new Operand(0b00100000, 0, 0, $1); };

noadr
  : I_IRET
  { $$ = I::INSTRUCTION::LD; }

  | I_RET
  { $$ = I::INSTRUCTION::LD_POST_INC; };

tworeg
  : I_ADD
  { $$ = I::INSTRUCTION::ADD; }

  | I_SUB
  { $$ = I::INSTRUCTION::SUB; }

  | I_MUL
  { $$ = I::INSTRUCTION::MUL; }

  | I_DIV
  { $$ = I::INSTRUCTION::DIV; }

  | I_AND
  { $$ = I::INSTRUCTION::AND; }

  | I_OR
  { $$ = I::INSTRUCTION::OR; }

  | I_XOR
  { $$ = I::INSTRUCTION::XOR; }

  | I_SHL
  { $$ = I::INSTRUCTION::SHL; }

  | I_SHR
  { $$ = I::INSTRUCTION::SHR; };

jmp
  : I_CALL
  { $$ = I::INSTRUCTION::CALL; }

  | I_JMP
  { $$ = I::INSTRUCTION::JMP; }

  | I_BEQ
  { $$ = I::INSTRUCTION::BEQ; }

  | I_BNE
  { $$ = I::INSTRUCTION::BNE; }

  | I_BGT
  { $$ = I::INSTRUCTION::BGT; };

jmpOperand
  : T_LITERAL
  { $$ = new Operand(0b01000000, 0, $1); }

  | T_IDENT
  { $$ = new Operand(0b00100000, 0, 0, $1); }

  | T_PERCENT gpr T_COMMA T_PERCENT gpr T_COMMA T_IDENT
  { $$ = new Operand(0b10101000, $2, 0, $7, 0, $5); }

  | T_PERCENT gpr T_COMMA T_PERCENT gpr T_COMMA T_LITERAL
  { $$ = new Operand(0b11001000, $2, $7, "", 0, $5); };

%%