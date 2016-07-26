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

#ifndef TEST_I_TARGET_STREAM_H
#define TEST_I_TARGET_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_statemachine_control.h"

#include "test_i_common.h"
#include "test_i_common_modules.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;

class Test_I_Target_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Test_I_Stream_State,
                        Test_I_Stream_Configuration,
                        Test_I_RuntimeStatistic_t,
                        Stream_ModuleConfiguration,
                        Test_I_Stream_ModuleHandlerConfiguration,
                        Test_I_Stream_SessionData,   // session data
                        Test_I_Stream_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage>
{
 public:
  Test_I_Target_Stream (const std::string&); // name
  virtual ~Test_I_Target_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&); // return value: module list

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_Stream_Configuration&, // configuration
                           bool = true,                        // setup pipeline ?
                           bool = true);                       // reset session data ?

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Test_I_Stream_State,
                        Test_I_Stream_Configuration,
                        Test_I_RuntimeStatistic_t,
                        Stream_ModuleConfiguration,
                        Test_I_Stream_ModuleHandlerConfiguration,
                        Test_I_Stream_SessionData,   // session data
                        Test_I_Stream_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream (const Test_I_Target_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream& operator= (const Test_I_Target_Stream&))

  // modules
  Test_I_Module_Net_IO_Module                  netIO_;
  Test_I_Target_Module_RuntimeStatistic_Module runtimeStatistic_;
  Test_I_Module_FileWriter_Module              fileWriter_;
};

#endif
