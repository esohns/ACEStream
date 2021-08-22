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

#ifndef TEST_I_MODULE_MSOFFICE_SPREADSHEETWRITER_H
#define TEST_I_MODULE_MSOFFICE_SPREADSHEETWRITER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_http_get_common.h"

extern const char libacestream_default_doc_msoffice_writer_module_name_string[];

class Test_I_MSOffice_SpreadsheetWriter
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Stream_Message,
                                 Test_I_Stream_SessionMessage,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Stream_Message,
                                 Test_I_Stream_SessionMessage,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Test_I_MSOffice_SpreadsheetWriter (ISTREAM_T*); // stream handle
  virtual ~Test_I_MSOffice_SpreadsheetWriter ();

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_MSOffice_SpreadsheetWriter ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_MSOffice_SpreadsheetWriter (const Test_I_MSOffice_SpreadsheetWriter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_MSOffice_SpreadsheetWriter& operator= (const Test_I_MSOffice_SpreadsheetWriter&))

  IDispatch* application_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_HTTPGet_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_doc_msoffice_writer_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_MSOffice_SpreadsheetWriter);               // writer type

#endif
