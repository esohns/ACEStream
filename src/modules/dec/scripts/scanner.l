%top{
#undef YYTOKENTYPE
#include "stream_dec_avi_parser.h"

#define YY_DECL                                           \
yytokentype                                               \
RIFF_Scanner_lex (YYSTYPE* yylval_param,                  \
                  YYLTYPE* yylloc_param,                  \
                  Stream_Decoder_AVIParserDriver* driver, \
                  yyscan_t yyscanner)
// ... and declare it for the parser's sake
YY_DECL;

void RIFF_Scanner_set_column (int, yyscan_t);

//#define FLEXINT_H
}

%{
#include <sstream>
#include <string>

#include <ace/ace_wchar.h>
#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/OS_Memory.h>

#include "stream_macros.h"

#include "stream_dec_avi_parser_driver.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
#define yyterminate() return yytokentype::END

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
%option ansi-definitions ansi-prototypes bison-locations
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
%}

<INITIAL>{
"RIFF"                     { ACE_ASSERT (yyleng == 4);
                             yylval->chunk_header.offset = driver->offset_;
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (RIFF_LIST_header);
                             yylval->chunk_header.fourcc = MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]); }
} // end <INITIAL>
<RIFF_LIST_header>{
{CHUNK_SIZE}               { ACE_ASSERT (yyleng == 4);
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (RIFF_LIST_chunk);
                             yylval->chunk_header.size =
                               ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? *reinterpret_cast<unsigned int*> (yytext)
                                                                      : ACE_SWAP_LONG (*reinterpret_cast<unsigned int*> (yytext))); }
} // end <RIFF_LIST_header>
<RIFF_LIST_chunk>{
{FOURCC}                   { ACE_ASSERT (yyleng == 4);
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (chunks);
                             yylval->chunk_header.fourcc =
                                 MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             return ((driver->offset_ == 12) ? yytokentype::RIFF : yytokentype::LIST); }
} // end <RIFF_type>
<chunks>{
"LIST"                     { ACE_ASSERT (yyleng == 4);
                             yylval->chunk_header.offset = driver->offset_;
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (RIFF_LIST_header);
                             yylval->chunk_header.fourcc =
                                 MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]); }
{FOURCC}                   { ACE_ASSERT (yyleng == 4);
                             yylval->chunk_header.offset = driver->offset_;
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (chunk_size);
                             yylval->chunk_header.fourcc =
                                 MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]); }
} // end <chunks>
<chunk_size>{
{CHUNK_SIZE}               { ACE_ASSERT (yyleng == 4);
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (chunk_data);
                             yylval->chunk_header.size =
                               ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? *reinterpret_cast<unsigned int*> (yytext)
                                                                      : ACE_SWAP_LONG (*reinterpret_cast<unsigned int*> (yytext)));
                             return yytokentype::META; }
} // end <chunks>
<chunk_data>{
{DATA}{1}                  { ACE_ASSERT (yylval->chunk_header.size);
                             /* undo the effects of YY_DO_BEFORE_ACTION */
                             *yy_cp = yyg->yy_hold_char;

//                             ACE_DEBUG ((LM_DEBUG,
//                                         ACE_TEXT ("found chunk @%u[%u]: %u byte(s)\n"),
//                                         driver->offset_,
//                                         (driver->fragment_->rd_ptr () - driver->fragment_->base ()),
//                                         yylval->chunk_header.size));

//                             // discard data ?
//                             bool discard_data =
//                                 (yylval->chunk_header.fourcc != MAKEFOURCC ('0', '0', 'd', 'b'));

                             // adjust write pointer ?
                             ACE_Message_Block* message_p, *message_2;
                             unsigned int remainder = driver->fragment_->length ();
                             if (yylval->chunk_header.size <= remainder)
                             { // current fragment contains the whole chunk
                               // --> insert buffer
                               message_p =
                                 driver->fragment_->duplicate ();
                               ACE_ASSERT (message_p);
                               message_2 =
                                 driver->fragment_->cont ();
                               if (message_2)
                                 message_p->cont (message_2);
                               driver->fragment_->cont (message_p);

                               driver->fragment_->wr_ptr (driver->fragment_->rd_ptr () + yylval->chunk_header.size);
                               remainder = yylval->chunk_header.size;
                             } // end IF
                             else
                             {
                               // skip over trailing chunk data fragment(s)
                               unsigned int skipped_bytes = remainder;
                               while (skipped_bytes <= yylval->chunk_header.size)
                               {
                                 if (!driver->switchBuffer ())
                                 {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("failed to Stream_Decoder_AVI_ParserDriver::switchBuffer(), aborting\n")));
                                   yyterminate ();
                                 } // end IF

                                 skipped_bytes += driver->fragment_->length ();
                               } // end WHILE
                               remainder = (skipped_bytes - yylval->chunk_header.size);

                               // this chunk ends in the current fragment
                               // --> insert buffer, adjust writer pointer,
                               //     switch buffers one more time
                               message_p =
                                 driver->fragment_->duplicate ();
                               ACE_ASSERT (message_p);
                               message_2 =
                                 driver->fragment_->cont ();
                               if (message_2)
                                 message_p->cont (message_2);
                               driver->fragment_->cont (message_p);
                               // computer offset to end of chunk
                               remainder =
                                 driver->fragment_->length () - remainder;
                               driver->fragment_->wr_ptr (driver->fragment_->rd_ptr () + remainder);
                             } // end ELSE

                             if (!driver->switchBuffer ())
                             {
                               ACE_DEBUG ((LM_ERROR,
                                           ACE_TEXT ("failed to Stream_Decoder_AVI_ParserDriver::switchBuffer(), aborting\n")));
                               yyterminate ();
                             } // end IF
                             driver->fragment_->rd_ptr (remainder); // offset

                             // gobble initial bytes
                             yyg->yy_c_buf_p += remainder;
                             yyg->yy_hold_char = *yyg->yy_c_buf_p;
//                           YY_FLUSH_BUFFER; // --> refill scan buffer
//                           char c;
//                           for (unsigned int i = 0;
//                                i < remainder;
//                                ++i)
//                             c = yyinput (yyscanner);
//                           ACE_UNUSED_ARG (c);

                             driver->offset_ += yylval->chunk_header.size;
                             yylval->size = yylval->chunk_header.size;
                             if (yylval->size % 2)
                               BEGIN (chunk_pad);
                             else
                             {
                               BEGIN (chunks);
                               return yytokentype::DATA;
                             } }
} // end <chunk_data>
<chunk_pad>{
{DATA}{1}                  { ACE_ASSERT (yyleng == 1);
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             BEGIN (chunks);
                             return yytokentype::DATA; }
} // end <chunk_pad>
<<EOF>>                    { yyterminate (); }
<*>{DATA}                  { /* *TODO*: use (?s:.) ? */
                             driver->fragment_->rd_ptr (yyleng);
                             driver->offset_ += yyleng;
                             // debug info
                             std::string error_message = ACE_TEXT_ALWAYS_CHAR ("invalid character: \"");
                             error_message += yytext[0];
                             error_message += ACE_TEXT_ALWAYS_CHAR ("\", aborting\n");
                             driver->error (*yylloc, error_message);
                             yyterminate (); }
%%
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
  std::string the_rest;
  the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
  yyg->yy_c_buf_p += yyleng;
  yyg->yy_hold_char = *yyg->yy_c_buf_p;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("the rest: \"%s\"\n"),
              ACE_TEXT (the_rest.c_str ())));

  // step2
  if (!driver->switchBuffer ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Decoder_AVI_ParserDriver::switchBuffer() [remaining data was: \"%s\"], aborting\n"),
                ACE_TEXT (the_rest.c_str ())));
    return 1;
  } // end IF

  // step3
  for (std::string::reverse_iterator iterator = the_rest.rbegin ();
       iterator != the_rest.rend ();
       ++iterator)
    unput (*iterator);

  // step4
  // yymore ();
  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
