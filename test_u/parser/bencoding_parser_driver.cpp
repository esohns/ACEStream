/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "stdafx.h"

#include "bencoding_parser_driver.h"

#include <sstream>

#include "ace/Log_Msg.h"

#include "common_macros.h"

#include "common_parser_bencoding_tools.h"

#include "bencoding_scanner.h"

Bencoding_ParserDriver::Bencoding_ParserDriver ()
 : inherited ()
 , dictionaries_ ()
 , key_ (NULL)
 , lists_ ()
{
  COMMON_TRACE (ACE_TEXT ("Bencoding_ParserDriver::Bencoding_ParserDriver_T"));

//  inherited::parser_.set (this);
}

void
Bencoding_ParserDriver::record (Bencoding_Dictionary_t*& record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Bencoding_ParserDriver_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("%s\n"),
             ACE_TEXT (Common_Parser_Bencoding_Tools::DictionaryToString (*record_inout).c_str ())));

  Common_Parser_Bencoding_Tools::free (record_inout);
  ACE_ASSERT (!record_inout);

  inherited::fragment_ = NULL;
  inherited::finished_ = true;
}

void
Bencoding_ParserDriver::record_2 (Bencoding_List_t*& record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Bencoding_ParserDriver_T::record_2"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("%s\n"),
             ACE_TEXT (Common_Parser_Bencoding_Tools::ListToString (*record_inout).c_str ())));

  Common_Parser_Bencoding_Tools::free (record_inout);
  ACE_ASSERT (!record_inout);

  inherited::finished_ = true;
}

void
Bencoding_ParserDriver::record_3 (std::string*& record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Bencoding_ParserDriver_T::record_3"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("\"%s\"\n"),
             ACE_TEXT (record_inout->c_str ())));

  delete record_inout; record_inout = NULL;

  inherited::finished_ = true;
}

void
Bencoding_ParserDriver::record_4 (unsigned int record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Bencoding_ParserDriver_T::record_4"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("%d\n"),
             record_inout));

  inherited::finished_ = true;
}

yy_buffer_state*
Bencoding_ParserDriver::create (yyscan_t state_in,
                                char* buffer_in,
                                size_t size_in)
{
  COMMON_TRACE (ACE_TEXT ("Bencoding_ParserDriver_T::create"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (state_in);

  struct yy_buffer_state* result_p = NULL;

  if (inherited::configuration_->useYYScanBuffer)
    result_p =
      Bencoding__scan_buffer (buffer_in,
                              size_in + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                              state_in);
  else
    result_p = Bencoding__scan_bytes (buffer_in,
                                      static_cast<int> (size_in) + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                                      state_in);
  if (!result_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yy_scan_buffer/bytes(0x%@, %d), aborting\n"),
                buffer_in,
                size_in));
    return NULL;
  } // end IF

  //// *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
  //Bencoding__switch_to_buffer (result_p,
  //                             state_in);

  return result_p;
}
