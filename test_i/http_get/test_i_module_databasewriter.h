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

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_timer_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_mysqlwriter.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

class Test_I_Stream_DataBaseWriter
 : public Stream_Module_MySQLWriter_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Test_I_ModuleHandlerConfiguration,
                                      ACE_Message_Block,
                                      Test_I_Stream_Message,
                                      Test_I_Stream_SessionMessage,
                                      Test_I_Stream_SessionData>
{
 public:
  Test_I_Stream_DataBaseWriter ();
  virtual ~Test_I_Stream_DataBaseWriter ();

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

 private:
  typedef Stream_Module_MySQLWriter_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Test_I_ModuleHandlerConfiguration,
                                      ACE_Message_Block,
                                      Test_I_Stream_Message,
                                      Test_I_Stream_SessionMessage,
                                      Test_I_Stream_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_DataBaseWriter (const Test_I_Stream_DataBaseWriter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_DataBaseWriter& operator= (const Test_I_Stream_DataBaseWriter&))

  //bool commit_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Stream_SessionData,         // session data type
                              Stream_SessionMessageType,         // session event type
                              Test_I_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_IStreamNotify_t,            // stream notification interface type
                              Test_I_Stream_DataBaseWriter);     // writer type

#endif
