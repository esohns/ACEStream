%top{
//#undef avi_tokentype
#include "stream_dec_avi_parser.h"

#define YY_DECL                                           \
enum avi_tokentype                                        \
RIFF_Scanner_lex (AVI_STYPE* yylval_param,                \
                  AVI_LTYPE* yylloc_param,                \
                  Stream_Decoder_AVIParserDriver* driver, \
                  void* yyscanner)
// ... and declare it for the parser's sake
YY_DECL;

void RIFF_Scanner_set_column (int, yyscan_t);
void RIFF_Scanner_reset_hold_char (yyscan_t);

//#define FLEXINT_H
}

%{
#include <regex>

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "common_image_defines.h"

#include "stream_macros.h"

#include "stream_dec_avi_parser_driver.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
#define yyterminate() return avi_tokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option header-file="stream_dec_riff_scanner.h" outfile="stream_dec_riff_scanner.cpp"
%option batch 8bit nodefault never-interactive nostdinit yylineno yywrap
/* %option c++ yyclass="RIFF_Scanner" */
/* %option ansi-definitions ansi-prototypes bison-bridge */
/* *NOTE*: the 'line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
/* %option ansi-definitions ansi-prototypes */
%option bison-locations
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
%option reentrant pointer prefix="RIFF_Scanner_" nomain
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, header-file extra-type'
*/
%option extra-type="Stream_Decoder_AVIParserDriver*"
/* %option nounput */

FOURCC         [[:print:]]{4}
 /* little-endian */
CHUNK_SIZE     [\x00-\xFF]{4}
DATA           [\x00-\xFF]

%s             RIFF_LIST_header
%s             RIFF_LIST_chunk
%s             LIST_header
%s             LIST_chunk
%s             chunk_size
%s             chunk_data
%s             chunk_pad
%s             chunks

%{
/* handle locations */
/*#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
                       yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng; */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
                       yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%%
%{
  //yylloc->step ();
  yy_flex_debug = driver->getDebugScanner ();

  int missing_bytes = 0;
%}

<INITIAL>{
"RIFF"                     { ACE_ASSERT (yyleng == 4);
                             driver->fragmentOffset_ += 4;
                             yylval->chunk_meta.offset = driver->offset_;
                             driver->offset_ += 4;
                             yylval->chunk_meta.identifier =
                               FOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (RIFF_LIST_header); }
} // end <INITIAL>
<RIFF_LIST_header>{
{CHUNK_SIZE}               { ACE_ASSERT (yyleng == 4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.size =
                               ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? *reinterpret_cast<unsigned int*> (yytext)
                                                                      : ACE_SWAP_LONG (*reinterpret_cast<unsigned int*> (yytext)));
                             BEGIN (LIST_chunk); }
} // end <RIFF_LIST_header>
<RIFF_LIST_chunk>{
{FOURCC}                   { ACE_ASSERT (yyleng == 4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.riff_list_identifier =
                                 FOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (chunks);
                             return avi_tokentype::RIFF; }
} // end <RIFF_LIST_chunk>
<chunks>{
"RIFF"                     { ACE_ASSERT (yyleng == 4);
                             yyless (0);
                             BEGIN (INITIAL); }
"LIST"                     { ACE_ASSERT (yyleng == 4);
                             yylval->chunk_meta.offset = driver->offset_;
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.identifier =
                                 FOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (LIST_header); }
{FOURCC}                   { ACE_ASSERT (yyleng == 4);
                             yylval->chunk_meta.offset = driver->offset_;
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.identifier =
                                 FOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             yylval->chunk_meta.riff_list_identifier = 0;
                             BEGIN (chunk_size); }
} // end <chunks>
<LIST_header>{
{CHUNK_SIZE}               { ACE_ASSERT (yyleng == 4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.size =
                               ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? *reinterpret_cast<unsigned int*> (yytext)
                                                                      : ACE_SWAP_LONG (*reinterpret_cast<unsigned int*> (yytext)));
                             BEGIN (LIST_chunk); }
} // end <LIST_header>
<LIST_chunk>{
{FOURCC}                   { ACE_ASSERT (yyleng == 4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.riff_list_identifier =
                                 FOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (chunks);
                             return avi_tokentype::LIST; }
} // end <LIST_chunk>
<chunk_size>{
{CHUNK_SIZE}               { ACE_ASSERT (yyleng == 4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.size =
                               ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? *reinterpret_cast<unsigned int*> (yytext)
                                                                      : ACE_SWAP_LONG (*reinterpret_cast<unsigned int*> (yytext)));
                             missing_bytes = yylval->chunk_meta.size;
                             BEGIN (chunk_data); }
} // end <chunks>
<chunk_data>{
{DATA}{1}                  { ACE_ASSERT (missing_bytes);
                             --missing_bytes;
                             char c = 0;
                             for (int i = 0;
                                  i < missing_bytes;
                                  ++i)
                             {
                               c = yyinput (yyscanner);
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;
                             } // end FOR
/*                             if (missing_bytes)
                               break;*/
                             driver->fragmentOffset_ += yylval->chunk_meta.size;
                             driver->offset_ += yylval->chunk_meta.size;
                             if (unlikely (yylval->chunk_meta.size % 2))
                               BEGIN (chunk_pad);
                             else
                             {
                               BEGIN (chunks);
                               return avi_tokentype::CHUNK;
                             } }
} // end <chunk_data>
<chunk_pad>{
{DATA}{1}                  { ACE_ASSERT (yyleng == 1);
                             ++driver->fragmentOffset_;
                             ++driver->offset_;
                             BEGIN (chunks);
                             return avi_tokentype::CHUNK; }
} // end <chunk_pad>
<<EOF>>                    { yyterminate (); }
<*>{DATA}                  { /* *TODO*: use (?s:.) ? */
                             // debug info
                             std::string error_message = ACE_TEXT_ALWAYS_CHAR ("invalid character: \"");
                             error_message += yytext[0];
                             error_message += ACE_TEXT_ALWAYS_CHAR ("\", aborting\n");
                             driver->error (*yylloc, error_message);
                             yyterminate (); }
%%
void
RIFF_Scanner_reset_hold_char (yyscan_t yyscanner)
{
  STREAM_TRACE (ACE_TEXT ("::RIFF_Scanner_reset_hold_char"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  ACE_ASSERT (yyg);
  *yyg->yy_c_buf_p = yyg->yy_hold_char;
}

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
RIFF_Scanner_wrap (yyscan_t yyscanner)
{
  STREAM_TRACE (ACE_TEXT ("::RIFF_Scanner_wrap"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  Stream_Decoder_AVIParserDriver* driver = RIFF_Scanner_get_extra (yyscanner);

  // sanity check(s)
  ACE_ASSERT (driver);
  if (driver->finished_)
    return 1; // done
  ACE_ASSERT (driver->fragment_);

  // *NOTE*: there is more data
  // 1. gobble/save the rest
  // 2. switch buffers
  // 3. unput the rest
  // 4. continue scanning

  // step1
//  std::string the_rest;
//  the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
//  yyg->yy_c_buf_p += yyleng;
//  yyg->yy_hold_char = *yyg->yy_c_buf_p;
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("the rest: \"%s\"\n"),
//              ACE_TEXT (the_rest.c_str ())));

  // step2
  if (!driver->switchBuffer ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Decoder_AVI_ParserDriver::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  // step3
//  for (std::string::reverse_iterator iterator = the_rest.rbegin ();
//       iterator != the_rest.rend ();
//       ++iterator)
//    unput (*iterator);

  // step4
  // yymore ();
  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
