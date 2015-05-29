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

#ifndef STREAM_COMMON_H
#define STREAM_COMMON_H

#include "ace/Module.h"
#include "ace/Notification_Strategy.h"
#include "ace/Stream.h"
#include "ace/Synch_Traits.h"
#include "ace/Task.h"
#include "ace/Time_Value.h"

#include "common_time_common.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_imodule.h"
#include "stream_session_data_base.h"
#include "stream_statistichandler.h"

struct Stream_Statistic_t
{
  // convenience
  inline Stream_Statistic_t ()
   : numDataMessages (0)
   , numDroppedMessages (0)
   , numBytes (0.0)
  {};

  inline Stream_Statistic_t operator+= (const Stream_Statistic_t& rhs_in)
  {
    numDataMessages += rhs_in.numDataMessages;
    numDroppedMessages += rhs_in.numDroppedMessages;
    numBytes += rhs_in.numBytes;

    return *this;
  };

  unsigned int numDataMessages;    // (protocol) messages
  unsigned int numDroppedMessages; // dropped messages
  double       numBytes;           // amount of processed data
};

struct Stream_State_t
{
  Stream_State_t ()
   : sessionID (0)
   , startOfSession (ACE_Time_Value::zero)
   , currentStatistics ()
   , lastCollectionTimestamp (ACE_Time_Value::zero)
   , userAborted (false)
  {};

  unsigned int       sessionID; // (== socket handle !)
  ACE_Time_Value     startOfSession;
  Stream_Statistic_t currentStatistics;
  ACE_Time_Value     lastCollectionTimestamp;
  bool               userAborted;
};

struct Stream_ModuleConfiguration_t
{
  Stream_State_t* streamState;
  void*           userData;
};

typedef ACE_Task<ACE_MT_SYNCH,
                 Common_TimePolicy_t> Stream_Task_t;
typedef ACE_Module<ACE_MT_SYNCH,
                   Common_TimePolicy_t> Stream_Module_t;
typedef Stream_IModule<ACE_MT_SYNCH,
                       Common_TimePolicy_t,
                       Stream_ModuleConfiguration_t> Stream_IModule_t;
typedef ACE_Stream_Iterator<ACE_MT_SYNCH,
                            Common_TimePolicy_t> Stream_Iterator_t;

struct Stream_Configuration_t
{
  Stream_Configuration_t ()
   : bufferSize (STREAM_BUFFER_SIZE)
   , deleteModule (false)
   , messageAllocator (NULL)
   , module (NULL)
   , moduleConfiguration (NULL)
   , notificationStrategy (NULL)
   , printFinalReport (false)
   , serializeOutput (false)
   , statisticReportingInterval (0)
   , useThreadPerConnection (false)
  {};

  unsigned int                  bufferSize;
  bool                          deleteModule;
  Stream_IAllocator*            messageAllocator;
  Stream_Module_t*              module;
  Stream_ModuleConfiguration_t* moduleConfiguration;
  ACE_Notification_Strategy*    notificationStrategy;
  bool                          printFinalReport;
  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be
  // dispatching the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
  bool                          serializeOutput;
  unsigned int                  statisticReportingInterval; // 0: don't report
  bool                          useThreadPerConnection;
};

typedef Stream_SessionDataBase_T<Stream_State_t> Stream_SessionData_t;

typedef Stream_StatisticHandler_Reactor_T<Stream_Statistic_t> Stream_StatisticHandler_Reactor_t;
typedef Stream_StatisticHandler_Proactor_T<Stream_Statistic_t> Stream_StatisticHandler_Proactor_t;

#endif
