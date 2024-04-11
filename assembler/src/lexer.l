%{
	#include "../include/parser.hpp"
    int yyerror(const char* msg);
    void yyerror_num(const int msg);
%}

%option outfile="src/lexer.cpp" header-file="include/lexer.hpp"

%option noyywrap

WS          [ \r\n\t]*
COMMENT     "#"[^\n\r]*
HEX         0[Xx][0-9A-Fa-f]+
DEC         [0-9]+
IDENT       [A-Za-z_][A-Za-z0-9_]*
STRING      \"[^\"]+\"

%%
{COMMENT}           { }
{HEX}               {
                      sscanf(yytext, "%x", &yylval.num_32);
			          return TOKEN_LITERAL;
                    }
{DEC}               {
                      sscanf(yytext, "%d", &yylval.num_32);
                      return TOKEN_LITERAL;
                    }
","                 { return TOKEN_COMMA; }             /* "\"" { return TOKEN_QUATATION; } */
":"                 { return TOKEN_COLON; }
"$"                 { return TOKEN_DOLLAR; }
"%"                 { return TOKEN_PERCENT; }
"["                 { return TOKEN_OBRACKET; }
"]"                 { return TOKEN_CBRACKET; }
"+"                 { return TOKEN_PLUS; }
"-"                 { return TOKEN_MINUS; }
"*"                 { return TOKEN_TIMES; }
"/"                 { return TOKEN_SLASH; }
".global"           { return TOKEN_GLOBAL; }
".extern"           { return TOKEN_EXTERN; }
".section"          { return TOKEN_SECTION; }
".word"             { return TOKEN_WORD; }
".skip"             { return TOKEN_SKIP; }
".ascii"            { return TOKEN_ASCII; }
".equ"              { return TOKEN_EQU; }
".end"              { return TOKEN_END; }
"."{IDENT}          {
                      yyerror(yytext);
                    }
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
"handler"           { return CSR_HANDLER; }
"cause"             { return CSR_CAUSE; }
"status"            { return CSR_STATUS; }
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
{STRING}            {
                      /* char* temp = yytext;  */
                      /*  int size = sizeof(temp)-2; */
                      /*  char* ident = new char[size]; */
                      /*  strncpy(ident, temp, size); */
                      /* yylval.ident = strdup(yytext+1); */
                      yylval.ident = strdup(yytext+1);;
                      return TOKEN_STRING;
                    }
{IDENT}             {
                      yylval.ident = strdup(yytext);
                      return TOKEN_IDENT;
                    }

{WS}                {  }
.                   {
                      yyerror(yytext);
                    }
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
