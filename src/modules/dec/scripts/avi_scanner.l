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
#include <regex>

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "stream_macros.h"

#include "ace/Synch.h"
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
                             if (driver->extractFrames_)
                               driver->fragment_->rd_ptr (4);
                             yylval->chunk_meta.offset = 0;
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.identifier =
                               MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (RIFF_LIST_header);
                             return yytokentype::RIFF; }
} // end <INITIAL>
<RIFF_LIST_header>{
{CHUNK_SIZE}               { ACE_ASSERT (yyleng == 4);
                             if (driver->extractFrames_)
                               driver->fragment_->rd_ptr (4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.size =
                               ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? *reinterpret_cast<unsigned int*> (yytext)
                                                                      : ACE_SWAP_LONG (*reinterpret_cast<unsigned int*> (yytext)));
                             BEGIN (RIFF_LIST_chunk);
                             return yytokentype::_SIZE; }
} // end <RIFF_LIST_header>
<RIFF_LIST_chunk>{
{FOURCC}                   { ACE_ASSERT (yyleng == 4);
                             if (driver->extractFrames_)
                               driver->fragment_->rd_ptr (4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.riff_list_identifier =
                                 MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (chunks);
                             return yytokentype::_FOURCC; }
} // end <RIFF_type>
<chunks>{
"LIST"                     { ACE_ASSERT (yyleng == 4);
                             if (driver->extractFrames_)
                               driver->fragment_->rd_ptr (4);
                             yylval->chunk_meta.offset = driver->offset_;
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.identifier =
                                 MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (RIFF_LIST_header);
                             return yytokentype::LIST; }
{FOURCC}                   { ACE_ASSERT (yyleng == 4);
                             if (driver->extractFrames_)
                               driver->fragment_->rd_ptr (4);
                             yylval->chunk_meta.offset = driver->offset_;
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.identifier =
                                 MAKEFOURCC (yytext[0], yytext[1], yytext[2], yytext[3]);
                             BEGIN (chunk_size);
                             return yytokentype::_FOURCC; }
} // end <chunks>
<chunk_size>{
{CHUNK_SIZE}               { ACE_ASSERT (yyleng == 4);
                             if (driver->extractFrames_)
                               driver->fragment_->rd_ptr (4);
                             driver->fragmentOffset_ += 4;
                             driver->offset_ += 4;
                             yylval->chunk_meta.size =
                               ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? *reinterpret_cast<unsigned int*> (yytext)
                                                                      : ACE_SWAP_LONG (*reinterpret_cast<unsigned int*> (yytext)));
                             BEGIN (chunk_data);
                             return yytokentype::_SIZE; }
} // end <chunks>
<chunk_data>{
{DATA}{1}                  { ACE_ASSERT (yylval->chunk_meta.size);
                             /* undo the effects of YY_DO_BEFORE_ACTION */
                             *yy_cp = yyg->yy_hold_char;

//                             ACE_DEBUG ((LM_DEBUG,
//                                         ACE_TEXT ("found chunk @%u[%u]: %u byte(s)\n"),
//                                         driver->offset_,
//                                         driver->fragmentOffset_,
//                                         yylval->chunk_meta.size));

                             bool is_frame_chunk = false;
                             const char* char_p =
                               reinterpret_cast<const char*> (&yylval->chunk_meta.identifier);
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

                               is_frame_chunk = true;
                             } // end IF

                             // skip over chunk data
                             ACE_Message_Block* message_p, *message_2;
                             unsigned int remainder =
                                 (driver->extractFrames_ ? ((driver->fragment_->wr_ptr () - driver->fragment_->base ()) - driver->fragmentOffset_)
                                                         : (driver->fragment_->length () - driver->fragmentOffset_));
                             unsigned int skipped_bytes = 0;
                             if (yylval->chunk_meta.size <= remainder)
                             { // current fragment contains the whole chunk
                               // --> insert buffer
                               message_p = driver->fragment_->duplicate ();
                               ACE_ASSERT (message_p);
                               message_2 = driver->fragment_->cont ();
                               if (message_2)
                                 message_p->cont (message_2);
                               driver->fragment_->cont (message_p);

                               driver->fragment_->wr_ptr (driver->fragment_->base () + driver->fragmentOffset_);
                               remainder = yylval->chunk_meta.size;
                               // set bytes to skip by the scanner (see below)
                               skipped_bytes =
                                   driver->fragmentOffset_ + yylval->chunk_meta.size;
                             } // end IF
                             else
                             {
                               // skip over trailing chunk data fragment(s)
                               while (skipped_bytes <= yylval->chunk_meta.size)
                               {
                                 if (!driver->switchBuffer ())
                                 {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("failed to Stream_Decoder_AVI_ParserDriver::switchBuffer(), aborting\n")));
                                   yyterminate ();
                                 } // end IF

                                 skipped_bytes += driver->fragment_->length ();
                               } // end WHILE
                               remainder =
                                 (skipped_bytes - yylval->chunk_meta.size);

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
                               // compute offset to end of chunk
                               remainder =
                                 driver->fragment_->length () - remainder;
                               driver->fragment_->wr_ptr (driver->fragment_->rd_ptr () + remainder);
                               // set bytes to skip by the scanner (see below)
                               skipped_bytes = remainder;
                             } // end ELSE
                             if (!driver->switchBuffer ())
                             {
                               ACE_DEBUG ((LM_ERROR,
                                           ACE_TEXT ("failed to Stream_Decoder_AVI_ParserDriver::switchBuffer(), aborting\n")));
                               yyterminate ();
                             } // end IF
                             driver->fragment_->rd_ptr (driver->fragmentOffset_);
                             if (driver->extractFrames_ &&
                                 !is_frame_chunk)
                               driver->fragment_->rd_ptr (remainder);

                             // gobble initial bytes
//                             yyg->yy_c_buf_p += skipped_bytes;
//                             yyg->yy_hold_char = *yyg->yy_c_buf_p;
//                           YY_FLUSH_BUFFER; // --> refill scan buffer
                             char c = 0;
                             for (unsigned int i = 0; i < skipped_bytes; ++i)
                               c = yyinput (yyscanner);
                             ACE_UNUSED_ARG (c);

                             driver->fragmentOffset_ = remainder;
                             driver->offset_ += yylval->chunk_meta.size;
//                             yylval->size = yylval->chunk_meta.size;

                             if (yylval->chunk_meta.size % 2)
                               BEGIN (chunk_pad);
                             else
                             {
                               BEGIN (chunks);
                               return yytokentype::DATA;
                             } }
} // end <chunk_data>
<chunk_pad>{
{DATA}{1}                  { ACE_ASSERT (yyleng == 1);
                             if (driver->extractFrames_)
                               driver->fragment_->rd_ptr (1);
                             ++driver->fragmentOffset_;
                             ++driver->offset_;
                             ++yylval->size;
                             BEGIN (chunks);
                             return yytokentype::DATA; }
} // end <chunk_pad>
<<EOF>>                    { yyterminate (); }
<*>{DATA}                  { /* *TODO*: use (?s:.) ? */
                             driver->fragment_->rd_ptr (yyleng);
                             driver->fragmentOffset_ += yyleng;
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
