/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

#ifndef YY_AVI_STREAM_DEC_AVI_PARSER_H_INCLUDED
# define YY_AVI_STREAM_DEC_AVI_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef AVI_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define AVI_DEBUG 1
#  else
#   define AVI_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define AVI_DEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined AVI_DEBUG */
#if AVI_DEBUG
extern int avi_debug;
#endif
/* "%code requires" blocks.  */

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef STREAM_DEC_AVI_PARSER_H
//#define STREAM_DEC_AVI_PARSER_H

#include "stream_dec_riff_common.h"
//#include "stream_dec_exports.h"

// forward declarations
class Stream_Decoder_AVIParserDriver;
struct AVI_STYPE
{
  struct RIFF_chunk_meta chunk_meta;
  ACE_UINT32             size;
};
#define AVI_STYPE_IS_DECLARED
#define YYSTYPE AVI_STYPE
#define YYLTYPE AVI_LTYPE

typedef void* yyscan_t;

#define YYDEBUG 1
extern int avi_debug;
//#define YYERROR_VERBOSE
//#define YYPRINT 1
//#define YYTOKEN_TABLE 1

#define YYINITDEPTH 1000
#define YYMAXDEPTH 100000


/* Token kinds.  */
#ifndef AVI_TOKENTYPE
# define AVI_TOKENTYPE
  enum avi_tokentype
  {
    AVI_EMPTY = -2,
    END = 0,                       /* "end_of_buffer"  */
    AVI_error = 256,               /* error  */
    AVI_UNDEF = 257,               /* "invalid token"  */
    RIFF = 258,                    /* "riff"  */
    LIST = 259,                    /* "list"  */
    CHUNK = 260                    /* "chunk"  */
  };
  typedef enum avi_tokentype avi_token_kind_t;
#endif

/* Value type.  */
#if ! defined AVI_STYPE && ! defined AVI_STYPE_IS_DECLARED
typedef  struct YYSTYPE  AVI_STYPE;
# define AVI_STYPE_IS_TRIVIAL 1
# define AVI_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined AVI_LTYPE && ! defined AVI_LTYPE_IS_DECLARED
typedef struct AVI_LTYPE AVI_LTYPE;
struct AVI_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define AVI_LTYPE_IS_DECLARED 1
# define AVI_LTYPE_IS_TRIVIAL 1
#endif



int avi_parse (Stream_Decoder_AVIParserDriver* driver, yyscan_t yyscanner);
/* "%code provides" blocks.  */

extern void yy_debug (int);
extern void yyerror (AVI_LTYPE*, Stream_Decoder_AVIParserDriver*, yyscan_t, const char*);
extern int yyparse (Stream_Decoder_AVIParserDriver*, yyscan_t);
//extern void yyprint (FILE*, yytokentype, YYSTYPE);

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // STREAM_DEC_AVI_PARSER_H


#endif /* !YY_AVI_STREAM_DEC_AVI_PARSER_H_INCLUDED  */
