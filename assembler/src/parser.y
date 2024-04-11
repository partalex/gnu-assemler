%{
	#include "../include/log.hpp"
	#include "../include/assembler.hpp"

	#include <cstdio>

	int yylex(void);
	void yyerror(const char*);

  #ifdef YYDEBUG
    yydebug = 1;
  #endif

%}

%output "./src/parser.cpp"
%defines "./include/parser.hpp"
/* %define parse.trace */

%union {
	char*               ident;
}

%token            TOKEN_COMMA
%token            TOKEN_COLON
%token            TOKEN_DOLLAR
%token            TOKEN_PERCENT
%token            TOKEN_OBRACKET
%token            TOKEN_CBRACKET
%token            TOKEN_PLUS
%token            TOKEN_MINUS
%token            TOKEN_TIMES
%token            TOKEN_SLASH
%token            TOKEN_GLOBAL
%token            TOKEN_EXTERN
%token            TOKEN_SECTION
%token            TOKEN_WORD
%token            TOKEN_SKIP
%token            TOKEN_ASCII
%token            TOKEN_EQU
%token            TOKEN_END
%token<ident>     TOKEN_STRING
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
%token <num_32>   TOKEN_LITERAL
%token <ident>    TOKEN_IDENT

%%

prog
  :
  | instr prog;

instr
  : I_HALT
  | I_INT
  | I_IRET
  | I_CALL TOKEN_LITERAL;

%%