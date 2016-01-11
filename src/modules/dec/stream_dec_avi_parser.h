/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison GLR parsers in C
   
      Copyright (C) 2002-2006, 2009-2010 Free Software Foundation, Inc.
   
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

/* "%code requires" blocks.  */


#ifndef STREAM_DEC_AVI_PARSER_H
#define STREAM_DEC_AVI_PARSER_H

#include "ace/config-lite.h"

#include "dshow.h"

struct RIFF_chunk_header
{
  FOURCC       fourcc;
  unsigned int size;

  unsigned int offset;
};

class Stream_Decoder_AVIParserDriver;
//class RIFF_Scanner;
struct YYLTYPE;
union YYSTYPE;
enum yytokentype;

typedef void* yyscan_t;

#define YYERROR_VERBOSE 1
//#define YYPRINT 1
#define YYTOKEN_TABLE 1
extern void yyerror (YYLTYPE*, Stream_Decoder_AVIParserDriver*, yyscan_t, const char*);
extern int yyparse (Stream_Decoder_AVIParserDriver*, yyscan_t);
extern void yyprint (FILE*, yytokentype, YYSTYPE);




/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 0,
     RIFF = 258,
     LIST = 259,
     CHUNK = 260,
     DATA = 261
   };
#endif


#ifndef YYSTYPE
typedef union YYSTYPE
{


  struct RIFF_chunk_header chunk_header;
  unsigned int             size;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{

  int first_line;
  int first_column;
  int last_line;
  int last_column;

} YYLTYPE;
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

/* "%code provides" blocks.  */


#endif // STREAM_DEC_AVI_PARSER_H










