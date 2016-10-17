/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2013 Free Software Foundation, Inc.

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

#ifndef YY_YY_STREAM_DEC_AVI_PARSER_H_INCLUDED
# define YY_YY_STREAM_DEC_AVI_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
//extern int yydebug;
#endif
/* "%code requires" blocks.  */


// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef STREAM_DEC_AVI_PARSER_H
//#define STREAM_DEC_AVI_PARSER_H

#include "stream_dec_common.h"
#include "stream_dec_exports.h"

// forward declarations

class Stream_Decoder_AVIParserDriver;
//class RIFF_Scanner;
#undef YYTOKENTYPE
//enum yytokentype;
//struct YYLTYPE;
#undef YYSTYPE
//union YYSTYPE;

typedef void* yyscan_t;

#define YYDEBUG 1
extern int Stream_Dec_Export yydebug;
//#define YYERROR_VERBOSE
//#define YYPRINT 1
//#define YYTOKEN_TABLE 1



/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    RIFF = 258,
    LIST = 259,
    META = 260,
    DATA = 261
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{


  struct RIFF_chunk_header chunk_header;
  unsigned int             size;


};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (Stream_Decoder_AVIParserDriver* driver, yyscan_t yyscanner);
/* "%code provides" blocks.  */


extern void yy_debug (int);
extern void yyerror (YYLTYPE*, Stream_Decoder_AVIParserDriver*, yyscan_t, const char*);
extern int yyparse (Stream_Decoder_AVIParserDriver*, yyscan_t);
//extern void yyprint (FILE*, yytokentype, YYSTYPE);

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // STREAM_DEC_AVI_PARSER_H



#endif /* !YY_YY_STREAM_DEC_AVI_PARSER_H_INCLUDED  */
