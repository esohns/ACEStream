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

#ifndef TEST_U_FILECOPY_STREAM_H
#define TEST_U_FILECOPY_STREAM_H

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Thread_Mutex.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_filecopy_common.h"
#include "test_u_filecopy_common_modules.h"
#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

// forward declarations
class Stream_IAllocator;

class Stream_Filecopy_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Stream_Filecopy_StreamConfiguration,
                        Stream_Statistic,
                        Stream_ModuleConfiguration,
                        Stream_Filecopy_ModuleHandlerConfiguration,
                        Stream_Filecopy_SessionData,   // session data
                        Stream_Filecopy_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Stream_Filecopy_Message,
                        Stream_Filecopy_SessionMessage>
{
 public:
  Stream_Filecopy_Stream ();
  virtual ~Stream_Filecopy_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&); // return value: module list

  // implement Common_IInitialize_T
  virtual bool initialize (const Stream_Filecopy_StreamConfiguration&, // configuration
                           bool = true,                                // setup pipeline ?
                           bool = true);                               // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Stream_Statistic&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Stream_Filecopy_StreamConfiguration,
                        Stream_Statistic,
                        Stream_ModuleConfiguration,
                        Stream_Filecopy_ModuleHandlerConfiguration,
                        Stream_Filecopy_SessionData,   // session data
                        Stream_Filecopy_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Stream_Filecopy_Message,
                        Stream_Filecopy_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Stream (const Stream_Filecopy_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Stream& operator= (const Stream_Filecopy_Stream&))

  // modules
  Stream_Filecopy_Module_FileReader_Module       fileReader_;
  Stream_Filecopy_Module_RuntimeStatistic_Module runtimeStatistic_;
  Stream_Filecopy_Module_FileWriter_Module       fileWriter_;

  static ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> currentSessionID;
};

#endif
