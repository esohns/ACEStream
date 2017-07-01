%top{
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#define YY_DECL                                              \
int                                                          \
Stream_Decoder_H264_NAL_Bisector_lex (yyscan_t yyscanner)
// ... and declare it for the parser's sake
YY_DECL;

class Common_IScanner;

void Stream_Decoder_H264_NAL_Bisector_set_column (int, yyscan_t);

//#define FLEXINT_H
}

%{
#include <regex>

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "common_iscanner.h"

#include "stream_macros.h"
%}

%option header-file="stream_dec_h264_nal_bisector.h" outfile="stream_dec_h264_nal_bisector.cpp"
%option batch 8bit nodefault never-interactive nostdinit yylineno yywrap
/* %option c++ yyclass="Stream_Decoder_H264_NAL_Bisector" */
/* *NOTE*: the 'line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
%option ansi-definitions ansi-prototypes
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
%option reentrant pointer prefix="Stream_Decoder_H264_NAL_Bisector_" nomain nounistd
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, header-file extra-type'
*/
%option extra-type="Common_IScanner*"
%option noinput nounput noyymore

CHAR                   [\x00-\xFF]
START_CODE             \x00\x00\x01
NAL_UNIT_DIVIDER       \x00{START_CODE}

%s                     end_of_frame

%% /* end of definitions */

%{
unsigned int offset = 0;
%}

<INITIAL>{
{NAL_UNIT_DIVIDER}     {
                         /* undo the effects of YY_DO_BEFORE_ACTION */
                         *yy_cp = yyg->yy_hold_char;
                         return offset + 1; }
} // end <INITIAL>
<<EOF>>                { return -1; }
<*>{CHAR}              { /* *TODO*: use (?s:.) ? */
                         ++offset; }

%% /* end of rules */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
Stream_Decoder_H264_NAL_Bisector_wrap (yyscan_t yyscanner)
{
  STREAM_TRACE (ACE_TEXT ("::Stream_Decoder_H264_NAL_Bisector_wrap"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  Common_IScanner* iscanner_p =
    Stream_Decoder_H264_NAL_Bisector_get_extra (yyscanner);

  // sanity check(s)
  ACE_ASSERT (iscanner_p);
  if (!iscanner_p->isBlocking ())
    return 1; // not enough data, cannot proceed

  // *NOTE*: there is more data
  // 1. gobble/save the rest
  // 2. switch buffers
  // 3. unput the rest
  // 4. continue scanning

  // step1
  //std::string the_rest;
  //the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
  //yyg->yy_c_buf_p += yyleng;
  //yyg->yy_hold_char = *yyg->yy_c_buf_p;
  //if (yy_flex_debug)
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("the rest: \"%s\" (%d byte(s))\n"),
  //              ACE_TEXT (the_rest.c_str ()),
  //              the_rest.size ()));

  // step2
  if (!iscanner_p->switchBuffer ())
  {
    // *NOTE*: most probable reason: received session end message
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Common_IScanner::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  // step3
  //for (std::string::reverse_iterator iterator = the_rest.rbegin ();
  //     iterator != the_rest.rend ();
  //     ++iterator)
  //  unput (*iterator);

  // step4
  //yyg->yy_did_buffer_switch_on_eof = 1;
  // yymore ();

  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
