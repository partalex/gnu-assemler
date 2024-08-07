%{
	#include "../include/parser.h"
	#include "../../common/include/symbol_list.h"
    int yyerror(const char* msg);
    void yyerror_num(const int msg);
%}

%option outfile="src/lexer.cpp" header-file="include/lexer.h"

%option noyywrap

WS          [ \r\n\t]*
COMMENT     "#"[^\n\r]*
HEX         0[Xx][0-9A-Fa-f]+
DEC         [0-9]+
IDENT       [A-Za-z_][A-Za-z0-9_]*
STRING      \"[^\"]+\"
OFFSET      [\+\-]({HEX}|{DEC})

%%
{COMMENT}           { }
{HEX}               {
                      sscanf(yytext, "%x", &yylval.num_32);
			          return T_LITERAL;
                    }
{DEC}               {
                        sscanf(yytext, "%d", &yylval.num_32);
                        return T_LITERAL;
                    }

","                 { return T_COMMA; }             /* "\"" { return T_QUATATION; } */
":"                 { return T_COLON; }
"$"                 { return T_DOLLAR; }
"%"                 { return T_PERCENT; }
"["                 { return T_OBRACKET; }
"]"                 { return T_CBRACKET; }
"+"                 { return T_PLUS; }
"-"                 { return T_MINUS; }
"*"                 { return T_TIMES; }
"/"                 { return T_SLASH; }

".global"           { return T_GLOBAL; }
".extern"           { return T_EXTERN; }
".section"          { return T_SECTION; }
".word"             { return T_WORD; }
".equ"              { return T_EQU; }
".skip"             { return T_SKIP; }
".ascii"            { return T_ASCII; }
".end"              { return T_END; }
"."{IDENT}          { yyerror(yytext); }

"halt"              { return I_HALT; }
"int"               { return I_INT; }
"iret"              { return I_IRET; } /* ? */
"call"              { return I_CALL; }
"ret"               { return I_RET; } /* ? */
"jmp"               { return I_JMP; }
"beq"               { return I_BEQ; }
"bne"               { return I_BNE; }
"bgt"               { return I_BGT; }
"push"              { return I_PUSH; }
"pop"               { return I_POP; }
"xchg"              { return I_XCHG; }
"add"               { return I_ADD; }
"sub"               { return I_SUB; }
"mul"               { return I_MUL; }
"div"               { return I_DIV; }
"not"               { return I_NOT; }
"and"               { return I_AND; }
"or"                { return I_OR; }
"xor"               { return I_XOR; }
"shl"               { return I_SHL; }
"shr"               { return I_SHR; }
"ld"                { return I_LD; }
"st"                { return I_ST; }

"csrrd"             { return I_CSRRD; }
"csrwr"             { return I_CSRWR; }

"r0"                { yylval.num_u8 = 0; return REG; }
"r1"                { yylval.num_u8 = 1; return REG; }
"r2"                { yylval.num_u8 = 2; return REG; }
"r3"                { yylval.num_u8 = 3; return REG; }
"r4"                { yylval.num_u8 = 4; return REG; }
"r5"                { yylval.num_u8 = 5; return REG; }
"r6"                { yylval.num_u8 = 6; return REG; }
"r7"                { yylval.num_u8 = 7; return REG; }
"r8"                { yylval.num_u8 = 8; return REG; }
"r9"                { yylval.num_u8 = 9; return REG; }
"r10"               { yylval.num_u8 = 10; return REG; }
"r11"               { yylval.num_u8 = 11; return REG; }
"r12"               { yylval.num_u8 = 12; return REG; }
"r13"               { yylval.num_u8 = 13; return REG; }
"sp"                { yylval.num_u8 = 14; return SP; }
"pc"                { yylval.num_u8 = 15; return PC; } /* T - li treba da se vrati PC */

"%handler"           { return HANDLER_CSR; }
"%cause"             { return CAUSE_CSR; }
"%status"            { return STATUS_CSR; }

{STRING}            {
                      char* temp = strdup(yytext+1);
                      temp[strlen(temp)-1] = '\0';
                      yylval.ident = temp;
                      return T_STRING;
                    }
{IDENT}             {
                      yylval.ident = strdup(yytext);
                      return T_IDENT;
                    }

{WS}                {  }
.                   { yyerror(yytext); }

%%

int yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}

void yyerror_num(const int msg)
{
  fprintf(stderr, "lexer error: %d\n", msg);
}
