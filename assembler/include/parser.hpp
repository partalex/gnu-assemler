/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_INCLUDE_PARSER_HPP_INCLUDED
# define YY_YY_INCLUDE_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_COMMA = 258,
    T_COLON = 259,
    T_DOLLAR = 260,
    T_PERCENT = 261,
    T_OBRACKET = 262,
    T_CBRACKET = 263,
    T_PLUS = 264,
    T_MINUS = 265,
    T_TIMES = 266,
    T_SLASH = 267,
    T_GLOBAL = 268,
    T_EXTERN = 269,
    T_SECTION = 270,
    T_WORD = 271,
    T_SKIP = 272,
    T_ASCII = 273,
    T_END = 274,
    T_STRING = 275,
    I_HALT = 276,
    I_INT = 277,
    I_IRET = 278,
    I_CALL = 279,
    I_RET = 280,
    I_JMP = 281,
    I_BEQ = 282,
    I_BNE = 283,
    I_BGT = 284,
    I_PUSH = 285,
    I_POP = 286,
    I_XCHG = 287,
    I_ADD = 288,
    I_SUB = 289,
    I_MUL = 290,
    I_DIV = 291,
    I_NOT = 292,
    I_AND = 293,
    I_OR = 294,
    I_XOR = 295,
    I_SHL = 296,
    I_SHR = 297,
    I_LD = 298,
    I_ST = 299,
    I_CSRRD = 300,
    I_CSRWR = 301,
    CSR_STATUS = 302,
    CSR_HANDLER = 303,
    CSR_CAUSE = 304,
    REG = 305,
    SP = 306,
    PC = 307,
    T_LITERAL = 308,
    T_IDENT = 309
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 21 "src/parser.y"

	char*               ident;
    unsigned char       num_u8;
    unsigned int        num_32;
    class SymbolList*   symbolList;
    class Operand*      operand;

#line 120 "./include/parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_INCLUDE_PARSER_HPP_INCLUDED  */
