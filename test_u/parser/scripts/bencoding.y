/*%require                          "3.4.1"*/
%require                          "3.0"
/* %file-prefix                      "" */
/*%language                         "c++"*/
%language                         "C"
%locations
%no-lines
/*%skeleton                         "glr.c"*/
%skeleton                         "lalr1.cc"
%verbose
/* %yacc */

%defines                          "bencoding_parser.h"
%output                           "bencoding_parser.cpp"

/*%define "parser_class_name"       "Bencoding_Parser"*/
/* *NOTE*: this is the namespace AND the (f)lex prefix */
/*%name-prefix                      "y"*/
/*%pure-parser*/
/*%token-table*/
%define parse.error               verbose
%debug

%code top {
#include "stdafx.h"

//#include "ace/Synch.h"
#include "bencoding_parser.h"
}

/* %define location_type */
/* %define api.location.type         {} */
/* %define api.namespace             {yy} */
/* %define api.prefix                {yy} */
//%define api.pure                    true
/* *TODO*: implement a push parser */
/* %define api.push-pull             push */
/*%define api.token.constructor*/
%define api.token.prefix            {}
/*%define api.value.type            variant*/
/* %define api.value.union.name      YYSTYPE */
/* %define lr.default-reduction      most */
/* %define lr.default-reduction      accepting */
/* %define lr.keep-unreachable-state false */
/* %define lr.type                   lalr */

/*%define parse.assert              {true}*/
/*%define parse.error               verbose*/
/* %define parse.lac                 {full} */
/* %define parse.lac                 {none} */
/*%define parser_class_name         {BitTorrent_Bencoding_Parser}*/
/* *NOTE*: enabling debugging functionality implies inclusion of <iostream> (see
           below). This interferes with ACE (version 6.2.3), when compiled with
           support for traditional iostreams */
/*%define parse.trace               {true}*/

%code requires {
// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef BitTorrent_MetaInfo_Parser_H
//#define BitTorrent_MetaInfo_Parser_H

/*#include <cstdio>
#include <string>*/

//#include "common_iparser.h"

#include "common_parser_bencoding_common.h"

#include "bencoding_iparser.h"

/* enum yytokentype
{
  END = 0,
  HANDSHAKE = 258,
  MESSAGE_BITFIELD = 260,
  MESSAGE_CANCEL = 262,
  MESSAGE_HAVE = 259,
  MESSAGE_PIECE = 263,
  MESSAGE_PORT = 263,
  MESSAGE_REQUEST = 261,
}; */
//#define YYTOKENTYPE
/*#undef YYTOKENTYPE*/
/* enum yytokentype; */
//struct YYLTYPE;
//class Bencoding_Scanner;

/* #define YYSTYPE
typedef union YYSTYPE
{
  int          ival;
  std::string* sval;
} YYSTYPE; */
/*#undef YYSTYPE*/
//union YYSTYPE;

/*typedef void* yyscan_t;*/

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually; apparently, there is no easy way to add the export
//         symbol to the declaration
#define YYDEBUG 1
/*extern int BitTorrent_Export yydebug;*/
#define YYERROR_VERBOSE 1
}

// calling conventions / parameter passing
%parse-param              { Bencoding_IParser* iparser }
%parse-param              { Bencoding_IScanner_t* iscanner }
// *NOTE*: cannot use %initial-action, as it is scoped
%lex-param                { Bencoding_IParser* iparser }
%lex-param                { yyscan_t iscanner->getR ().lexState }

%initial-action
{
  // initialize the location
  @$.initialize (NULL);
}

// symbols
%union
{
  ACE_INT64               ival;
  std::string*            sval;
  Bencoding_Element*      eval;
  Bencoding_List_t*       lval;
  Bencoding_Dictionary_t* dval;
}
//%token <ACE_INT64>              INTEGER;
//%token <std::string>            STRING;
//%token <Bencoding_Element>      ELEMENT;
//%token <Bencoding_List_t>       LIST;
//%token <Bencoding_Dictionary_t> DICTIONARY;
//%token EOF 0;

%code {
// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
/*#if defined (YYDEBUG)
#include <iostream>
#endif*/
/*#include <regex>*/
#include <sstream>
#include <string>

// *WORKAROUND*
/*using namespace std;*/
// *IMPORTANT NOTE*: several ACE headers include ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
/*#define ACE_IOSFWD_H*/

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_macros.h"

#include "common_parser_bencoding_tools.h"

#include "bencoding_parser_driver.h"
#include "bencoding_scanner.h"

// *TODO*: this shouldn't be necessary
#define yylex Bencoding_lex
//#define yylex iscanner->lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)
}

// terminals
%token
// END_OF_FRAGMENT   "end_of_fragment"
 END 0             "end"
;
%token <ival> INTEGER    "integer"
%token <sval> STRING     "string"
%token <lval> LIST       "list"
%token <dval> DICTIONARY "dictionary"
// non-terminals
%type  <dval> dictionary_items
%type  <lval> list_items
%type  <eval> bencoding dictionary_item dictionary_value list_item

/*%precedence "integer" "string"
%precedence "list" "dictionary"*/

%code provides {
/*void BitTorrent_Export yysetdebug (int);
void BitTorrent_Export yyerror (YYLTYPE*, BitTorrent_MetaInfo_IParser*, yyscan_t, const char*);
int BitTorrent_Export yyparse (BitTorrent_MetaInfo_IParser*, yyscan_t);
void BitTorrent_Export yyprint (FILE*, yytokentype, YYSTYPE);*/

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // BitTorrent_MetaInfo_Parser_H
}

/*%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), $$->c_str ()); } <sval>
%printer    { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %d"), $$); } <ival>
%destructor { delete $$; $$ = NULL; } <sval>
%destructor { $$ = 0; } <ival>*/
/* %destructor               { ACE_DEBUG ((LM_DEBUG,
                                        ACE_TEXT ("discarding tagless symbol...\n"))); } <> */
/*%printer    { yyoutput << $$; } <*>;*/
%printer    { debug_stream () << $$; } <ival>
%printer    { debug_stream () << *$$; } <sval>
/*%printer    { yyoutput << BitTorrent_Tools::ListToString (*$$); } <eval>*/
%printer    { debug_stream () << Common_Parser_Bencoding_Tools::ListToString (*$$); } <lval>
%printer    { if ($$) debug_stream () << Common_Parser_Bencoding_Tools::DictionaryToString (*$$); } <dval>

%%
%start            bencoding;
bencoding:        "dictionary" {
                    iparser->pushDictionary ($1); }
                  dictionary_items {
                    Bencoding_Dictionary_t& dictionary_r = iparser->current ();
                    Bencoding_Dictionary_t* dictionary_p = &dictionary_r;
                    try {
                      iparser->record (dictionary_p);
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record(), continuing\n")));
                    }
                    YYACCEPT; }
                  | "list" {
                    iparser->pushList ($1); }
                  list_items {
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    Bencoding_List_t* list_p = &list_r;
                    try {
                      iparser->record_2 (list_p);
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record_2(), continuing\n")));
                    }
                    YYACCEPT; }
                  | "string" {
                    try {
                      iparser->record_3 ($1);
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record_3(), continuing\n")));
                    }
                    YYACCEPT; }
                  | "integer" {
                    try {
                      iparser->record_4 ($1);
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record_4(), continuing\n")));
                    }
                    YYACCEPT; }
                  | %empty             { }
list_items:       list_items list_item { }
/*                  |                    { }*/
                  | %empty             { }
list_item:        "string" {
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_STRING;
                    element_p->string = $1;
                    list_r.push_back (element_p); }
                  | "integer" {
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_INTEGER;
                    element_p->integer = $1;
                    list_r.push_back (element_p); }
                  | "list" {
                    iparser->pushList ($1); }
                  list_items {
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_LIST;
                    element_p->list = $3;
                    iparser->popList ();
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    list_r.push_back (element_p); }
                  | "dictionary"  {
                    iparser->pushDictionary ($1); }
                  dictionary_items {
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type =
                      Bencoding_Element::BENCODING_TYPE_DICTIONARY;
                    element_p->dictionary = $3;
                    iparser->popDictionary ();
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    list_r.push_back (element_p); }
                  | %empty             { }
dictionary_items: dictionary_items dictionary_item { }
/*                  |                                { }*/
                  | %empty { }
dictionary_item:  "string" {
                    iparser->pushKey ($1); }
                  dictionary_value { }
                  | %empty { }
dictionary_value: "string" {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_STRING;
                    element_p->string = $1;
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
/*                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("[dictionary: %@] key: \"%s\": \"%s\"\n"),
                                &dictionary_r,
                                ACE_TEXT (key_string_p->c_str ()),
                                ACE_TEXT ($1->c_str ()))); }*/
                  | "integer" {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_INTEGER;
                    element_p->integer = $1;
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
/*                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("[dictionary: %@] key: \"%s\": %d\n"),
                                &dictionary_r,
                                ACE_TEXT (key_string_p->c_str ()),
                                $1)); }*/
                  | "list" {
                    iparser->pushList ($1); }
                  list_items {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_LIST;
                    element_p->list = $3;
                    iparser->popList ();
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
/*                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("[dictionary: %@] key: \"%s\": \"%s\"\n"),
                                &dictionary_r,
                                ACE_TEXT (key_string_p->c_str ()),
                                ACE_TEXT (BitTorrent_Tools::ListToString (*$3).c_str ()))); }*/
                  | "dictionary"  {
                    iparser->pushDictionary ($1); }
                  dictionary_items {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type =
                      Bencoding_Element::BENCODING_TYPE_DICTIONARY;
                    element_p->dictionary = $3;
                    iparser->popDictionary ();
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
/*                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("[dictionary: %@] key: \"%s\": \"%s\"\n"),
                                &dictionary_r,
                                ACE_TEXT (key_string_p->c_str ()),
                                ACE_TEXT (BitTorrent_Tools::DictionaryToString (*$3).c_str ()))); }*/
%%

void
yy::parser::error (const location_type& location_in,
                   const std::string& message_in)
{
  COMMON_TRACE (ACE_TEXT ("yy::parser::error"));

  // sanity check(s)
  ACE_ASSERT (iparser);

  try {
    iparser->error (location_in, message_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Bencoding_IParser_t::error(), continuing\n")));
  }
}

void
yyerror (yy::location* location_in,
         Bencoding_IParser_t* iparser_in,
         yyscan_t context_in,
         const char* message_in)
{
  COMMON_TRACE (ACE_TEXT ("::yyerror"));

//  ACE_UNUSED_ARG (location_in);
  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (iparser_in);

  try {
    iparser_in->error (*location_in, message_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Bencoding_IParser::error(), continuing\n")));
  }
}

void
yyprint (FILE* file_in,
         yy::parser::token::yytokentype type_in,
         yy::parser::semantic_type value_in)
{
  COMMON_TRACE (ACE_TEXT ("::yyprint"));

  int result = -1;

  std::string format_string;
  switch (type_in)
  {
    case yy::parser::token::STRING:
    case yy::parser::token::INTEGER:
    case yy::parser::token::LIST:
    case yy::parser::token::DICTIONARY:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case yy::parser::token::END:
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
