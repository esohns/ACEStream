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

#ifndef TEST_I_STREAM_HTTPGET_H
#define TEST_I_STREAM_HTTPGET_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_source_http_get.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

class Test_I_Stream_HTTPGet
 : public Stream_Module_Net_Source_HTTP_Get_T<Test_I_Stream_ModuleHandlerConfiguration,
 
                                              Test_I_Stream_SessionMessage,
                                              Test_I_Stream_Message>
{
 public:
  Test_I_Stream_HTTPGet ();
  virtual ~Test_I_Stream_HTTPGet ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (Test_I_Stream_Message*&, // data message handle
                                  bool&);                  // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

 private:
  typedef Stream_Module_Net_Source_HTTP_Get_T<Test_I_Stream_ModuleHandlerConfiguration,

                                              Test_I_Stream_SessionMessage,
                                              Test_I_Stream_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_HTTPGet (const Test_I_Stream_HTTPGet&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_HTTPGet& operator= (const Test_I_Stream_HTTPGet&))

  Test_I_StockItemsIterator_t iterator_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                             // task synch type
                              Common_TimePolicy_t,                      // time policy
                              Stream_ModuleConfiguration,               // module configuration type
                              Test_I_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Stream_HTTPGet);                   // writer type

#endif