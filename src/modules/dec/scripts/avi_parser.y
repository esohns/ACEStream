%defines                          "stream_dec_avi_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "stream_dec_avi_parser.cpp"
%require                          "2.4.1"
%skeleton                         "glr.c"
/* %skeleton                         "lalr1.cc" */
%token-table
%verbose
/* %yacc */

%code top {
#include "stdafx.h"
}

/* %define location_type */
/* %define api.location.type         {} */
/* %define namespace                    {yy} */
/* %define api.namespace                "yy" */
%name-prefix                         "yy"
/* %define api.prefix                "yy" */
%pure-parser
/* %define api.pure                  true */
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/* %define api.token.constructor */
/* %define api.token.prefix          {} */
%token-table
/* %define api.value.type            variant */
%define api.value.type               { struct YYSTYPE }
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/* %define parse.assert              {true} */
%error-verbose
/* %define parse.error               verbose */
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/* %define parser_class_name         {Stream_Decoder_AVIParser} */
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
%debug
/* %define parse.trace               {true} */

%code requires {
// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef STREAM_DEC_AVI_PARSER_H
//#define STREAM_DEC_AVI_PARSER_H

#include "stream_dec_common.h"
//#include "stream_dec_exports.h"

// forward declarations

class Stream_Decoder_AVIParserDriver;
//class RIFF_Scanner;
#undef YYTOKENTYPE
//enum yytokentype;
//struct YYLTYPE;
//#undef YYSTYPE
struct YYSTYPE
{
  RIFF_chunk_meta chunk_meta;
  unsigned int    size;
};

typedef void* yyscan_t;

#define YYDEBUG 1
extern int yydebug;
//#define YYERROR_VERBOSE
//#define YYPRINT 1
//#define YYTOKEN_TABLE 1
}

// calling conventions / parameter passing
%parse-param              { Stream_Decoder_AVIParserDriver* driver }
%parse-param              { yyscan_t yyscanner }
//%lex-param                { YYSTYPE* yylval }
//%lex-param                { YYLTYPE* yylloc }
%lex-param                { Stream_Decoder_AVIParserDriver* driver }
%lex-param                { yyscan_t yyscanner }
//%param                    { Stream_Decoder_AVIParserDriver* driver }
//%param                    { yyscan_t yyscanner }

%initial-action
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  //@$.begin.filename = @$.end.filename = &driver->file;
  ACE_OS::memset (&@$, 0, sizeof (YYLTYPE));

  // initialize the token value container
  $$.chunk_meta.identifier = 0;
  $$.chunk_meta.size = 0;
  $$.chunk_meta.riff_list_identifier = 0;
  $$.chunk_meta.offset = 0;
};

// symbols
//%union
//{
//  struct RIFF_chunk_meta chunk_meta;
////  unsigned int           size;
//};
/* %token <unsigned int>          INTEGER;
%token <struct RIFF_chunk_meta> CHUNK_META; */

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif
#include <regex>

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

#include "stream_dec_avi_parser_driver.h"
#include "stream_dec_defines.h"
#include "stream_dec_riff_scanner.h"

// *TODO*: this shouldn't be necessary
#define yylex RIFF_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

/* *NOTE*: 'SIZE' is a typedef on Win32 */
%token <size>      RIFF   "riff"
%token <size>      _SIZE  "size"
%token <size>      FOURCC "fourcc"
%token <size>      LIST   "list"
%token <size>      DATA   "data"
%token <size>      END 0  "end_of_buffer"

%type <chunk_meta> riff_list_meta chunk
%type <size>       buffer chunks

//%precedence DATA
//%precedence SIZE
//%precedence FOURCC
//%left FOURCC

%code provides {
extern void yy_debug (int);
extern void yyerror (YYLTYPE*, Stream_Decoder_AVIParserDriver*, yyscan_t, const char*);
extern int yyparse (Stream_Decoder_AVIParserDriver*, yyscan_t);
//extern void yyprint (FILE*, yytokentype, YYSTYPE);

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // STREAM_DEC_AVI_PARSER_H
}

/* %printer                  { yyoutput << $$; } <*>; */
/* %printer                  { yyoutput << $$; } <chunk_header>
%printer                  { debug_stream () << $$; }  <size> */
%printer                  { const char* char_p =
                              reinterpret_cast<const char*> (&$$.identifier);
                            ACE_OS::fprintf (yyoutput,
                                             ACE_TEXT_ALWAYS_CHAR ("@%u: fourcc: \"%c%c%c%c\", size: %u, offset: %u\n"),
                                             $$.offset,
                                             char_p[3],char_p[2],char_p[1],char_p[0],
                                             $$.size,
                                             $$.offset);
                          } <chunk_meta>
%printer                  { ACE_OS::fprintf (yyoutput,
                                             ACE_TEXT_ALWAYS_CHAR ("size: %u"),
                                             $$);
                          } <size>
%destructor               { $$.identifier = 0;
                            $$.size = 0;
                            $$.riff_list_identifier = 0;
                            $$.offset = 0; } <chunk_meta>
%destructor               { $$ = 0; } <size>
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */

%%
%start          buffer;
buffer:         riff_list_meta chunks    { $$ = 4 + 4 + 4 + $2;
                                           driver->chunks_.insert ($1);
                                           const char* char_p =
                                             reinterpret_cast<const char*> (&$1.riff_list_identifier);
                                           ACE_DEBUG ((LM_DEBUG,
                                                       ACE_TEXT ("found RIFF chunk: \"%c%c%c%c\": %u byte(s)\n"),
                                                       char_p[3], char_p[2], char_p[1], char_p[0],
                                                       $1.size)); };
                | "end_of_buffer"        /* default */
riff_list_meta: "riff" "size" "fourcc"   { $$ = $$; };
                | "list" "size" "fourcc" { $$ = $$; };
chunks:         riff_list_meta chunks    { $$ = 4 + 4 + 4 + $2;
                                           driver->chunks_.insert ($1);
                                           const char* char_p =
                                             reinterpret_cast<const char*> (&$1.riff_list_identifier);
                                           ACE_DEBUG ((LM_DEBUG,
                                                       ACE_TEXT ("found LIST chunk: \"%c%c%c%c\": %u byte(s)\n"),
                                                       char_p[3], char_p[2], char_p[1], char_p[0],
                                                       $1.size)); };
                | chunk chunks           { $$ = 4 + 4 + $1.size + $2; };
                | "end_of_buffer"        /* default */
chunk:          "fourcc" "size" "data"   { //$$ = $$;
                                           driver->chunks_.insert ($$);
                                           const char* char_p =
                                             reinterpret_cast<const char*> (&$$.identifier);
                                           ACE_DEBUG ((LM_DEBUG,
                                                       ACE_TEXT ("found chunk: \"%c%c%c%c\": %u byte(s)\n"),
                                                       char_p[3], char_p[2], char_p[1], char_p[0],
                                                       $$.size));

                                           if ($$.identifier == MAKEFOURCC ('s', 't', 'r', 'f'))
                                           {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                             ACE_ASSERT (false);
#else
                                             ACE_ASSERT (driver->frameSize_);
                                             char_p =
                                               (driver->extractFrames_ ? driver->fragment_->base () + (driver->fragmentOffset_ - $$.size)
                                                                       : driver->fragment_->rd_ptr () - $$.size);
                                             // *NOTE*: hard-coded offset into struct tagBITMAPINFOHEADER
                                             *driver->frameSize_ =
                                               *reinterpret_cast<const unsigned int*> (char_p + 4 + 4 + 4 + 2 + 2 + 4);
                                             ACE_DEBUG ((LM_DEBUG,
                                                         ACE_TEXT ("frame size is: %u byte(s)\n"),
                                                         *driver->frameSize_));
#endif
                                           } // end IF

                                           if (driver->parseHeaderOnly_)
                                           {
                                             char_p =
                                               reinterpret_cast<const char*> (&$$.identifier);
                                             // *NOTE*: in memory, the fourcc is stored back-to-front
                                             static std::string regex_string =
                                               ACE_TEXT_ALWAYS_CHAR ("^([[:alpha:]]{2})([[:digit:]]{2})$");
                                             std::regex regex (regex_string);
                                             std::cmatch match_results;
                                             if (std::regex_match (char_p,
                                                                   match_results,
                                                                   regex,
                                                                   std::regex_constants::match_default))
                                             {
                                               ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                               ACE_ASSERT (match_results[1].matched);
                                               ACE_ASSERT (match_results[2].matched);

                                               driver->finished_ = true;
                                               YYACCEPT;
                                             } // end IF
                                           } };
/*              | %empty                               empty */
//              |                                        /* empty */
%%

/* void
yy::AVI_Parser::error (const location_type& location_in,
                       const std::string& message_in)
{
  STREAM_TRACE (ACE_TEXT ("AVI_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::AVI_Parser::set (yyscan_t context_in)
{
  STREAM_TRACE (ACE_TEXT ("AVI_Parser::set"));

  yyscanner = context_in;
} */

void
yy_debug (int debug_in)
{
  STREAM_TRACE (ACE_TEXT ("::yy_debug"));

  yydebug = debug_in;
}

void
yyerror (YYLTYPE* location_in,
         Stream_Decoder_AVIParserDriver* driver_in,
         yyscan_t context_in,
         const char* message_in)
{
  STREAM_TRACE (ACE_TEXT ("::yyerror"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (location_in);
  ACE_ASSERT (driver_in);

  driver_in->error (*location_in, std::string (message_in));
}

void
yyprint (FILE* file_in,
         yytokentype type_in,
         YYSTYPE value_in)
{
  STREAM_TRACE (ACE_TEXT ("::yyprint"));

  int result = -1;

  std::string format_string;
  switch (type_in)
  {
    case FOURCC:
    case SIZE:
    case DATA:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case END:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %d");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH

  result = ACE_OS::fprintf (file_in,
                            ACE_TEXT (format_string.c_str ()),
                            value_in);
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
}
