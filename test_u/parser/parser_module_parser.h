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

#ifndef PARSER_MODULE_PARSER_H
#define PARSER_MODULE_PARSER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Task_T.h"

#include "common_parser_common.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_parser.h"

#include "parser_message.h"
#include "parser_session_message.h"
#include "parser_stream_common.h"

#include "bencoding_parser_driver.h"

class Parser_Module_Parser
 : public Stream_Module_Parser_T <ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Parser_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Parser_Message,
                                 Parser_SessionMessage,
                                 Bencoding_ParserDriver,
                                 struct Stream_UserData>
{
  typedef Stream_Module_Parser_T <ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct Parser_ModuleHandlerConfiguration,
                                  Stream_ControlMessage_t,
                                  Parser_Message,
                                  Parser_SessionMessage,
                                  Bencoding_ParserDriver,
                                  struct Stream_UserData> inherited;

 public:
  Parser_Module_Parser (ISTREAM_T*); // stream handle
  inline virtual ~Parser_Module_Parser () {}

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Parser_Module_Parser ())
  ACE_UNIMPLEMENTED_FUNC (Parser_Module_Parser (const Parser_Module_Parser&))
  ACE_UNIMPLEMENTED_FUNC (Parser_Module_Parser& operator= (const Parser_Module_Parser&))

  virtual void record (Bencoding_Dictionary_t*&); // data record
  virtual void record_2 (Bencoding_List_t*&); // data record
  virtual void record_3 (std::string*&); // data record
  virtual void record_4 (unsigned int); // data record
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Parser_SessionData,                // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Parser_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_parser_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Parser_Module_Parser);                    // writer type

#endif
