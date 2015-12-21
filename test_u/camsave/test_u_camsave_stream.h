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

#ifndef TEST_U_CAMSAVE_STREAM_H
#define TEST_U_CAMSAVE_STREAM_H

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Thread_Mutex.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_common_modules.h"
#include "test_u_camsave_message.h"
#include "test_u_camsave_session_message.h"

// forward declarations
class Stream_IAllocator;

class Stream_CamSave_Stream
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        /////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        /////////////////
                        Stream_CamSave_StreamConfiguration,
                        /////////////////
                        Stream_Statistic,
                        /////////////////
                        Stream_ModuleConfiguration,
                        Stream_CamSave_ModuleHandlerConfiguration,
                        /////////////////
                        Stream_CamSave_SessionData,   // session data
                        Stream_CamSave_SessionData_t, // session data container (reference counted)
                        Stream_CamSave_SessionMessage,
                        Stream_CamSave_Message>
{
 public:
  Stream_CamSave_Stream ();
  virtual ~Stream_CamSave_Stream ();

  // implement Common_IInitialize_T
  virtual bool initialize (const Stream_CamSave_StreamConfiguration&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Stream_Statistic&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                        /////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        /////////////////
                        Stream_CamSave_StreamConfiguration,
                        /////////////////
                        Stream_Statistic,
                        /////////////////
                        Stream_ModuleConfiguration,
                        Stream_CamSave_ModuleHandlerConfiguration,
                        /////////////////
                        Stream_CamSave_SessionData,   // session data
                        Stream_CamSave_SessionData_t, // session data container (reference counted)
                        Stream_CamSave_SessionMessage,
                        Stream_CamSave_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_Stream (const Stream_CamSave_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_Stream& operator= (const Stream_CamSave_Stream&))

  // modules
  Stream_CamSave_Module_CamSource_Module        camSource_;
  Stream_CamSave_Module_RuntimeStatistic_Module runtimeStatistic_;
  Stream_CamSave_Module_FileWriter_Module       fileWriter_;

  static ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> currentSessionID;
};

#endif
