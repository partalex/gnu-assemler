%{
    #include "parser.hpp"

    int yyerror(const char* msg);
%}

%option outfile="lexer.cpp" header-file="lexer.hpp"

%option noyywrap

WS          [ \r\n\t]*
COMMENT     "#"[^\n\r]*
HEX         0[Xx][0-9A-Fa-f]+
DEC         [0-9]+
IDENT       [A-Za-z_][A-Za-z0-9_]*
STRING      \"[^\"]+\"

%%

%%

int yyerror(const char* msg)
{
    fprintf(stderr, "lexer error: %s\n", msg);
    return 0;
}

