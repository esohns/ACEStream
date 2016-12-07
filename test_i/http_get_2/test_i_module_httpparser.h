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

#ifndef TEST_I_MODULE_HTTPPARSER_H
#define TEST_I_MODULE_HTTPPARSER_H

#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>

#include "stream_common.h"
//#include "stream_streammodule_base.h"

#include "http_module_parser.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

class Test_I_Module_HTTPParser
 : public HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Test_I_ControlMessage_t,
                                Test_I_Stream_Message,
                                Test_I_Stream_SessionMessage,
                                struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                int,
                                enum Stream_SessionMessageType,
                                struct Test_I_HTTPGet_StreamState,
                                struct Test_I_HTTPGet_SessionData,
                                Test_I_HTTPGet_SessionData_t,
                                Test_I_RuntimeStatistic_t>
{
 public:
  Test_I_Module_HTTPParser ();
  virtual ~Test_I_Module_HTTPParser ();

  // override (part of) HTTP_IParser
  virtual void record (struct HTTP_Record*&); // target data record

 private:
  typedef HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Test_I_ControlMessage_t,
                                Test_I_Stream_Message,
                                Test_I_Stream_SessionMessage,
                                struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                int,
                                enum Stream_SessionMessageType,
                                struct Test_I_HTTPGet_StreamState,
                                struct Test_I_HTTPGet_SessionData,
                                Test_I_HTTPGet_SessionData_t,
                                Test_I_RuntimeStatistic_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_HTTPParser (const Test_I_Module_HTTPParser&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_HTTPParser& operator= (const Test_I_Module_HTTPParser&))
};

//// declare module
//DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Stream_SessionData,                 // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
//                              Stream_IStreamNotify_t,                           // stream notification interface type
//                              Test_I_Module_HTTPParser);                        // writer type

#endif
