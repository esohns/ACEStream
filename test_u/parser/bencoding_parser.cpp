// A Bison parser, made by GNU Bison 3.4.1.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2019 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// Undocumented macros, especially those whose name start with YY_,
// are private implementation details.  Do not rely on them.

// //                    "%code top" blocks.

#include "stdafx.h"

//#include "ace/Synch.h"
#include "bencoding_parser.h"





#include "bencoding_parser.h"


// Unqualified %code blocks.

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



#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (Bencoding_IParser* iparser_yyarg, Bencoding_IScanner_t* iscanner_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      iparser (iparser_yyarg),
      iscanner (iscanner_yyarg)
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/

  // basic_symbol.
#if 201103L <= YY_CPLUSPLUS
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (basic_symbol&& that)
    : Base (std::move (that))
    , value (std::move (that.value))
    , location (std::move (that.location))
  {}
#endif

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (semantic_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}

  template <typename Base>
  bool
  parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return Base::type_get () == empty_symbol;
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_type.
  parser::by_type::by_type ()
    : type (empty_symbol)
  {}

#if 201103L <= YY_CPLUSPLUS
  parser::by_type::by_type (by_type&& that)
    : type (that.type)
  {
    that.clear ();
  }
#endif

  parser::by_type::by_type (const by_type& that)
    : type (that.type)
  {}

  parser::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  void
  parser::by_type::clear ()
  {
    type = empty_symbol;
  }

  void
  parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
  }

  int
  parser::by_type::type_get () const YY_NOEXCEPT
  {
    return type;
  }


  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_number_type
  parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    YYUSE (yysym.type_get ());
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
#if defined __GNUC__ && ! defined __clang__ && ! defined __ICC && __GNUC__ * 100 + __GNUC_MINOR__ <= 408
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
#endif
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    switch (yytype)
    {
      case 5: // "integer"
        { debug_stream () << (yysym.value.ival); }
        break;

      case 6: // "string"
        { debug_stream () << *(yysym.value.sval); }
        break;

      case 7: // "list"
        { debug_stream () << Common_Parser_Bencoding_Tools::ListToString (*(yysym.value.lval)); }
        break;

      case 8: // "dictionary"
        { if ((yysym.value.dval)) debug_stream () << Common_Parser_Bencoding_Tools::DictionaryToString (*(yysym.value.dval)); }
        break;

      case 13: // list_items
        { debug_stream () << Common_Parser_Bencoding_Tools::ListToString (*(yysym.value.lval)); }
        break;

      case 17: // dictionary_items
        { if ((yysym.value.dval)) debug_stream () << Common_Parser_Bencoding_Tools::DictionaryToString (*(yysym.value.dval)); }
        break;

      default:
        break;
    }
    yyo << ')';
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
{
  // initialize the location
  yyla.location.initialize (NULL);
}



    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << '\n';

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location, iparser, iscanner->getR ().lexState));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2:
    {
                    iparser->pushDictionary ((yystack_[0].value.dval)); }
    break;

  case 3:
    {
                    Bencoding_Dictionary_t& dictionary_r = iparser->current ();
                    Bencoding_Dictionary_t* dictionary_p = &dictionary_r;
                    try {
                      iparser->record (dictionary_p);
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record(), continuing\n")));
                    }
                    YYACCEPT; }
    break;

  case 4:
    {
                    iparser->pushList ((yystack_[0].value.lval)); }
    break;

  case 5:
    {
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    Bencoding_List_t* list_p = &list_r;
                    try {
                      iparser->record_2 (list_p);
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record_2(), continuing\n")));
                    }
                    YYACCEPT; }
    break;

  case 6:
    {
                    try {
                      iparser->record_3 ((yystack_[0].value.sval));
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record_3(), continuing\n")));
                    }
                    YYACCEPT; }
    break;

  case 7:
    {
                    try {
                      iparser->record_4 ((yystack_[0].value.ival));
                    } catch (...) {
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("caught exception in Bencoding_IParser::record_4(), continuing\n")));
                    }
                    YYACCEPT; }
    break;

  case 8:
    { }
    break;

  case 9:
    { }
    break;

  case 10:
    { }
    break;

  case 11:
    {
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_STRING;
                    element_p->string = (yystack_[0].value.sval);
                    list_r.push_back (element_p); }
    break;

  case 12:
    {
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_INTEGER;
                    element_p->integer = (yystack_[0].value.ival);
                    list_r.push_back (element_p); }
    break;

  case 13:
    {
                    iparser->pushList ((yystack_[0].value.lval)); }
    break;

  case 14:
    {
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_LIST;
                    element_p->list = (yystack_[1].value.lval);
                    iparser->popList ();
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    list_r.push_back (element_p); }
    break;

  case 15:
    {
                    iparser->pushDictionary ((yystack_[0].value.dval)); }
    break;

  case 16:
    {
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type =
                      Bencoding_Element::BENCODING_TYPE_DICTIONARY;
                    element_p->dictionary = (yystack_[1].value.dval);
                    iparser->popDictionary ();
                    Bencoding_List_t& list_r = iparser->current_2 ();
                    list_r.push_back (element_p); }
    break;

  case 17:
    { }
    break;

  case 18:
    { }
    break;

  case 19:
    { }
    break;

  case 20:
    {
                    iparser->pushKey ((yystack_[0].value.sval)); }
    break;

  case 21:
    { }
    break;

  case 22:
    { }
    break;

  case 23:
    {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_STRING;
                    element_p->string = (yystack_[0].value.sval);
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
    break;

  case 24:
    {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_INTEGER;
                    element_p->integer = (yystack_[0].value.ival);
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
    break;

  case 25:
    {
                    iparser->pushList ((yystack_[0].value.lval)); }
    break;

  case 26:
    {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type = Bencoding_Element::BENCODING_TYPE_LIST;
                    element_p->list = (yystack_[1].value.lval);
                    iparser->popList ();
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
    break;

  case 27:
    {
                    iparser->pushDictionary ((yystack_[0].value.dval)); }
    break;

  case 28:
    {
                    std::string* key_string_p = &iparser->getKey ();
                    Bencoding_Element* element_p = NULL;
                    ACE_NEW_NORETURN (element_p,
                                      Bencoding_Element ());
                    ACE_ASSERT (element_p);
                    element_p->type =
                      Bencoding_Element::BENCODING_TYPE_DICTIONARY;
                    element_p->dictionary = (yystack_[1].value.dval);
                    iparser->popDictionary ();
                    Bencoding_Dictionary_t& dictionary_r =
                      iparser->current ();
/*                    dictionary_r.insert (std::make_pair (key_string_p,
                                                         element_p)); }*/
                    dictionary_r.push_back (std::make_pair (key_string_p,
                                                            element_p)); }
    break;



            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -20;

  const signed char parser::yytable_ninf_ = -1;

  const signed char
  parser::yypact_[] =
  {
       2,   -20,   -20,   -20,   -20,    11,   -20,   -20,   -20,    -2,
      14,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
     -20,    20,    10,    25,   -20,   -20,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,    16,    26,   -20,   -20
  };

  const unsigned char
  parser::yydefact_[] =
  {
       8,     7,     6,     4,     2,     0,    10,    19,     1,     0,
       3,     5,    12,    11,    13,    15,     9,    20,    18,    10,
      19,     0,     0,     0,    24,    23,    25,    27,    21,    14,
      16,    10,    19,     0,     0,    26,    28
  };

  const signed char
  parser::yypgoto_[] =
  {
     -20,   -20,   -20,   -20,   -19,   -20,   -20,   -20,   -18,   -20,
     -20,   -20,   -20,   -20
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     5,     7,     6,     9,    16,    19,    20,    10,    18,
      21,    28,    31,    32
  };

  const unsigned char
  parser::yytable_[] =
  {
      22,    11,    23,    12,    13,    14,    15,     1,     2,     3,
       4,     8,    33,    29,    34,    12,    13,    14,    15,    35,
      17,    12,    13,    14,    15,    24,    25,    26,    27,    30,
      36,    17,    17
  };

  const unsigned char
  parser::yycheck_[] =
  {
      19,     3,    20,     5,     6,     7,     8,     5,     6,     7,
       8,     0,    31,     3,    32,     5,     6,     7,     8,     3,
       6,     5,     6,     7,     8,     5,     6,     7,     8,     4,
       4,     6,     6
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     5,     6,     7,     8,    10,    12,    11,     0,    13,
      17,     3,     5,     6,     7,     8,    14,     6,    18,    15,
      16,    19,    13,    17,     5,     6,     7,     8,    20,     3,
       4,    21,    22,    13,    17,     3,     4
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,     9,    11,    10,    12,    10,    10,    10,    10,    13,
      13,    14,    14,    15,    14,    16,    14,    14,    17,    17,
      19,    18,    18,    20,    20,    21,    20,    22,    20
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     0,     3,     0,     4,     1,     1,     0,     2,
       0,     1,     1,     0,     4,     0,     4,     0,     2,     0,
       0,     3,     0,     1,     1,     0,     4,     0,     4
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end\"", "error", "$undefined", "\"list_end\"", "\"dictionary_end\"",
  "\"integer\"", "\"string\"", "\"list\"", "\"dictionary\"", "$accept",
  "bencoding", "$@1", "$@2", "list_items", "list_item", "$@3", "$@4",
  "dictionary_items", "dictionary_item", "$@5", "dictionary_value", "$@6",
  "$@7", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short
  parser::yyrline_[] =
  {
       0,   217,   217,   217,   229,   229,   241,   249,   257,   258,
     260,   261,   270,   279,   279,   291,   291,   304,   305,   307,
     308,   308,   311,   312,   331,   350,   350,   372,   372
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << '\n';
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const token_number_type
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8
    };
    const unsigned user_token_number_max_ = 263;
    const token_number_type undef_token_ = 2;

    if (static_cast<int> (t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }

} // yy



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
