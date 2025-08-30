%defines                          "stream_dec_avi_parser.h"
/* %file-prefix                      "" */
/* %language                         "c++" */
%language                         "C"
%locations
%no-lines
%output                           "stream_dec_avi_parser.cpp"
%require                          "3.8.1"
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
/* %name-prefix                         "avi_" */
%define api.prefix                   {avi_}
/* %pure-parser */
 %define api.pure                    true
/* %define api.push-pull                push */
/* %define api.token.constructor */
/* %define api.token.prefix          {} */
/* %define api.value.type            variant */
%define api.value.type               { struct YYSTYPE }
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/* %define parse.assert              {true} */
/* %error-verbose */
%define parse.error                  verbose
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

#define YYINITDEPTH 25000
#define YYMAXDEPTH 200000
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

#include "common_image_defines.h"

#include "stream_macros.h"

#include "stream_dec_avi_parser_driver.h"
#include "stream_dec_defines.h"
#include "stream_dec_riff_scanner.h"

// *TODO*: this shouldn't be necessary
#define yylex RIFF_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

%token <chunk_meta> RIFF  "riff"
%token <chunk_meta> LIST  "list"
%token <chunk_meta> CHUNK "chunk"
%token <size> END 0 "end_of_buffer"

%type <size>       riff_header chunks
//%type <chunk_meta> riff_list

//%precedence "chunk" "riff" "list"
/*%precedence "riff"
%precedence "list"*/
//%left "chunk" "riff" "list"

%code provides {
extern void yy_debug (int);
extern void yyerror (AVI_LTYPE*, Stream_Decoder_AVIParserDriver*, yyscan_t, const char*);
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
                                             ACE_TEXT_ALWAYS_CHAR ("@%lu: fourcc: \"%c%c%c%c\", size: %u, offset: %lu\n"),
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
%start          riff_header;
riff_header:    "riff"                   { driver->chunks_.push_back ($1); }
                  "list"                 { driver->chunks_.push_back ($3); }
                  chunks                 { $$ = (4 + 4) + (4 + 4 + 4) + $5; }
chunks:         "chunk"                  { const char* char_p = NULL;

                                           if ($1.identifier == FOURCC ('s', 't', 'r', 'h'))
                                           {
                                             char_p =
                                               driver->fragment_->base () + (driver->fragmentOffset_ - $1.size);
                                             driver->isVids_ =
                                                FOURCC ('s', 'd', 'i', 'v') == *reinterpret_cast<const ACE_UINT32*> (char_p);
                                           } // end IF

                                           if ($1.identifier == FOURCC ('s', 't', 'r', 'f'))
                                           {
                                             if (driver->isVids_ &&
                                                 !driver->frameSize_) // get first video stream only
                                             {
                                               char_p =
                                                 driver->fragment_->base () + (driver->fragmentOffset_ - $1.size);
                                               // *NOTE*: hard-coded offset into struct tagBITMAPINFOHEADER
                                               driver->frameSize_ =
                                                 *reinterpret_cast<const ACE_UINT32*> (char_p + 4 + 4 + 4 + 2 + 2 + 4);
                                               ACE_DEBUG ((LM_DEBUG,
                                                           ACE_TEXT ("video frame size is: %u byte(s)\n"),
                                                           driver->frameSize_));
                                             } // end IF
                                           } // end IF

                                           char buffer_a[5];
                                           ACE_OS::memset (buffer_a, 0, sizeof (char[5]));
                                           ACE_OS::memcpy (buffer_a, reinterpret_cast<const char*> (&$1.identifier), 4);
                                           char_p = buffer_a;
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

                                             driver->inFrames_ = true;
                                             if (driver->parseHeaderOnly_)
                                             {
                                               driver->header (driver->chunks_);
                                               driver->finished_ = true;
                                               YYACCEPT;
                                             } // end IF

                                             driver->chunks_.push_back ($1);

                                             if (!driver->frame ($1))
                                               YYABORT;
                                           } // end IF
                                           else
                                           {
                                             driver->chunks_.push_back ($1);

                                             if (driver->inFrames_)
                                               driver->betweenFrameChunk ($1);
                                           }

                                           ACE_UINT64 file_size_i = driver->fileSize ();
                                           if (file_size_i &&
                                               driver->offset_ >= file_size_i)
                                           {
                                             driver->finished_ = true;
                                             YYACCEPT;
                                           } // end IF
                                         }
                  chunks                 { $$ = 4 + 4 + $1.size + $3; }
                | "list"                 {
                                           driver->chunks_.push_back ($1);
                                           if (driver->inFrames_)
                                             driver->betweenFrameChunk ($1);
                                         }
                  chunks                 { $$ = 4 + 4 + 4 + $3; }
                | %empty                 { $$ = 0; }
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
yyerror (AVI_LTYPE* location_in,
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
         avi_tokentype type_in,
         YYSTYPE value_in)
{
  STREAM_TRACE (ACE_TEXT ("::yyprint"));

  int result = -1;

  std::string format_string;
  switch (type_in)
  {
    case RIFF:
    case LIST:
    case CHUNK:
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
