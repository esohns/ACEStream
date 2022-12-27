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

#include "parser_module_parser.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Parser_Module_Parser::Parser_Module_Parser (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_Module_Parser::Parser_Module_Parser"));

}

ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
Parser_Module_Parser:: clone ()
{
  STREAM_TRACE (ACE_TEXT ("Parser_Module_Parser::clone"));

  // initialize return value(s)
  Stream_Task_t* task_p = NULL;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_NORETURN (module_p,
                    Parser_Module_Parser_Module (const_cast<ISTREAM_T*> (inherited::getP ()),
                                                 ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
  if (!module_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory(%u): %m, aborting\n"),
                inherited::mod_->name (),
                sizeof (Parser_Module_Parser_Module)));
  else
  {
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);

    Parser_Module_Parser* eventHandler_impl =
        dynamic_cast<Parser_Module_Parser*> (task_p);
    if (!eventHandler_impl)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Parser_Module_Parser> failed, aborting\n"),
                  inherited::mod_->name ()));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
    eventHandler_impl->initialize (*inherited::STREAM_TASK_BASE_T::configuration_,
                                   inherited::allocator_);
  } // end ELSE

  return task_p;
}

void
Parser_Module_Parser::record (Bencoding_Dictionary_t*& record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Parser_Module_Parser::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT("%s\n"),
  //            ACE_TEXT (Common_Parser_Bencoding_Tools::DictionaryToString (*record_inout).c_str ())));

  // make sure the whole fragment chain references the same data record
  ACE_ASSERT (inherited::headFragment_);
  Parser_Message* message_p = static_cast<Parser_Message*> (inherited::headFragment_);
  ACE_ASSERT (message_p->isInitialized ());
  Parser_MessageData_t& data_container_r =
    const_cast<Parser_MessageData_t&> (message_p->getR ());
  struct Parser_MessageData& data_r =
    const_cast<struct Parser_MessageData&> (data_container_r.getR ());
  data_r.dictionary = record_inout;
  Parser_MessageData_t* data_container_2 = NULL;
  Parser_Message* message_2 = static_cast<Parser_Message*> (inherited::headFragment_->cont ());
  while (message_2)
  {
    data_container_r.increase ();
    data_container_2 = &data_container_r;
    message_2->initialize (data_container_2,
                           message_p->sessionId (),
                           NULL);
    message_2 = static_cast<Parser_Message*> (message_2->cont ());
  } // end WHILE

  int result = put_next (inherited::headFragment_, NULL);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT("%s: failed to ACE_Module::put_next(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

  inherited::headFragment_ = NULL;
  inherited::fragment_ = NULL;
  inherited::PARSER_BASE_T::finished_ = true;
}

void
Parser_Module_Parser::record_2 (Bencoding_List_t*& record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Parser_Module_Parser::record_2"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("%s\n"),
             ACE_TEXT (Common_Parser_Bencoding_Tools::ListToString (*record_inout).c_str ())));

  Common_Parser_Bencoding_Tools::free (record_inout);
  ACE_ASSERT (!record_inout);

  inherited::PARSER_BASE_T::finished_ = true;
}

void
Parser_Module_Parser::record_3 (std::string*& record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Parser_Module_Parser::record_3"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("\"%s\"\n"),
             ACE_TEXT (record_inout->c_str ())));

  delete record_inout; record_inout = NULL;

  inherited::PARSER_BASE_T::finished_ = true;
}

void
Parser_Module_Parser::record_4 (unsigned int record_inout)
{
  COMMON_TRACE (ACE_TEXT ("Parser_Module_Parser::record_4"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT("%d\n"),
             record_inout));

  inherited::PARSER_BASE_T::finished_ = true;
}
