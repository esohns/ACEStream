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

#ifndef TEST_I_MODULE_HTMLWRITER_H
#define TEST_I_MODULE_HTMLWRITER_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_timer_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_htmlwriter.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

class Test_I_Stream_Module_HTMLWriter
 : public Stream_Module_HTMLWriter_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_Stream_Message,
                                     Test_I_Stream_SessionMessage,
                                     Test_I_Stream_SessionData_t,
                                     struct Test_I_Stream_SessionData>
{
  typedef Stream_Module_HTMLWriter_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_Stream_Message,
                                     Test_I_Stream_SessionMessage,
                                     Test_I_Stream_SessionData_t,
                                     struct Test_I_Stream_SessionData> inherited;

 public:
  Test_I_Stream_Module_HTMLWriter ();
  virtual ~Test_I_Stream_Module_HTMLWriter ();

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Module_HTMLWriter (const Test_I_Stream_Module_HTMLWriter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Module_HTMLWriter& operator= (const Test_I_Stream_Module_HTMLWriter&))

  bool fileWritten_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Stream_SessionData,         // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_html_writer_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_Stream_Module_HTMLWriter);         // writer type

#endif
