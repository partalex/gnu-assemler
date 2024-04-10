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

%output "../src/parser.cpp"
%defines "../include/parser.hpp"
/* %define parse.trace */

%union {
	char*               ident;
}

%token            TOKEN_COMMA

%%

%%