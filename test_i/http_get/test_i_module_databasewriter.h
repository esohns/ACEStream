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

#ifndef TEST_I_MODULE_DATABASEWRITER_H
#define TEST_I_MODULE_DATABASEWRITER_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_timer_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (MYSQL_SUPPORT)
#include "stream_module_mysqlwriter.h"
#else
#include "stream_task_base_synch.h"
#endif // MYSQL_SUPPORT

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#if defined (MYSQL_SUPPORT)
#else
extern const char libacestream_default_test_i_db_target_module_name_string[];
#endif // MYSQL_SUPPORT

class Test_I_Module_DataBaseWriter
#if defined (MYSQL_SUPPORT)
 : public Stream_Module_MySQLWriter_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Stream_Message,
                                      Test_I_Stream_SessionMessage,
                                      struct Test_I_Stream_SessionData>
#else
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Stream_Message,
                                 Test_I_Stream_SessionMessage,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
#endif // MYSQL_SUPPORT
{
#if defined (MYSQL_SUPPORT)
  typedef Stream_Module_MySQLWriter_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Stream_Message,
                                      Test_I_Stream_SessionMessage,
                                      struct Test_I_Stream_SessionData> inherited;
#else
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Stream_Message,
                                 Test_I_Stream_SessionMessage,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
#endif // MYSQL_SUPPORT

 public:
  Test_I_Module_DataBaseWriter (inherited::ISTREAM_T*); // stream handle
  inline virtual ~Test_I_Module_DataBaseWriter () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_DataBaseWriter ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_DataBaseWriter (const Test_I_Module_DataBaseWriter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_DataBaseWriter& operator= (const Test_I_Module_DataBaseWriter&))

  //bool commit_;
};

// declare module
#if defined (MYSQL_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Stream_SessionData,         // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_db_mysql_target_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_Module_DataBaseWriter);            // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Stream_SessionData,         // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_test_i_db_target_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_Module_DataBaseWriter);            // writer type
#endif // MYSQL_SUPPORT

#endif
