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

//#include "ace/Message_Queue.h"
//#include "ace/Module.h"
#include "ace/Notification_Strategy.h"
#include "ace/Stream.h"
#include "ace/Synch_Traits.h"
//#include "ace/Task.h"
#include "ace/Time_Value.h"

#include "common_istatemachine.h"
#include "common_time_common.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_imodule.h"
#include "stream_session_data_base.h"
#include "stream_statistichandler.h"
#include "stream_statemachine_control.h"

struct Stream_Statistic
{
  inline Stream_Statistic ()
   : bytes (0.0F)
   , dataMessages (0)
   , droppedMessages (0)
   , bytesPerSecond (0.0F)
   , messagesPerSecond (0.0F)
   , timestamp (ACE_Time_Value::zero)
  {};

  inline Stream_Statistic operator+= (const Stream_Statistic& rhs_in)
  {
    bytes += rhs_in.bytes;
    dataMessages += rhs_in.dataMessages;
    droppedMessages += rhs_in.droppedMessages;

    return *this;
  };

  float          bytes;           // amount of processed data
  unsigned int   dataMessages;    // (protocol) messages
  unsigned int   droppedMessages; // dropped messages

  // (current) runtime performance
  float          bytesPerSecond;
  float          messagesPerSecond;

  ACE_Time_Value timestamp;
};

struct Stream_UserData
{
  inline Stream_UserData ()
   : userData (NULL)
  {};

  void* userData;
};

struct Stream_SessionData
{
  inline Stream_SessionData ()
   : aborted (false)
   , currentStatistic ()
   , lastCollectionTimestamp (ACE_Time_Value::zero)
   , lock (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , sessionID (reinterpret_cast<size_t> (ACE_INVALID_HANDLE))
#else
   , sessionID (static_cast<size_t> (ACE_INVALID_HANDLE))
#endif
   , startOfSession (ACE_Time_Value::zero)
   , userData (NULL)
  {};
  inline Stream_SessionData& operator= (Stream_SessionData& rhs_in)
  {
    aborted = (aborted ? aborted : rhs_in.aborted);
    currentStatistic =
        (currentStatistic.timestamp == ACE_Time_Value::zero ? rhs_in.currentStatistic
                                                            : currentStatistic);
    lastCollectionTimestamp =
        (lastCollectionTimestamp == ACE_Time_Value::zero ? rhs_in.lastCollectionTimestamp
                                                         : lastCollectionTimestamp);
    lock = rhs_in.lock;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    sessionID =
        ((sessionID == reinterpret_cast<size_t> (ACE_INVALID_HANDLE)) ? rhs_in.sessionID
                                                                      : sessionID);
#else
    sessionID =
        ((sessionID == static_cast<size_t> (ACE_INVALID_HANDLE)) ? rhs_in.sessionID
                                                                 : sessionID);
#endif
    startOfSession =
        (startOfSession == ACE_Time_Value::zero ? rhs_in.startOfSession
                                                : startOfSession);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  // *NOTE*: modules can set this to signal (internal) processing errors.
  //         The (stream / process) control logic may (or may not) then react to
  //         abort processing early
  bool             aborted;

  Stream_Statistic currentStatistic;
  ACE_Time_Value   lastCollectionTimestamp;
  ACE_SYNCH_MUTEX* lock;

  size_t           sessionID; // (== socket handle !)
  ACE_Time_Value   startOfSession;

  Stream_UserData* userData;
};

struct Stream_State
{
  inline Stream_State ()
   : currentSessionData (NULL)
   , userData (NULL)
  {};

  Stream_SessionData* currentSessionData;
  Stream_UserData*    userData;
};

// forward declarations
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Message_Queue;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Task;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Stream_Iterator;

typedef ACE_Message_Queue<ACE_MT_SYNCH,
                          Common_TimePolicy_t> Stream_Queue_t;
typedef ACE_Task<ACE_MT_SYNCH,
                 Common_TimePolicy_t> Stream_Task_t;
typedef ACE_Module<ACE_MT_SYNCH,
                   Common_TimePolicy_t> Stream_Module_t;
typedef ACE_Stream<ACE_MT_SYNCH,
                   Common_TimePolicy_t> Stream_Base_t;
typedef ACE_Stream_Iterator<ACE_MT_SYNCH,
                            Common_TimePolicy_t> Stream_Iterator_t;

struct Stream_ModuleConfiguration;
struct Stream_ModuleHandlerConfiguration;
typedef Stream_IModule_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t,
                         Stream_ModuleConfiguration,
                         Stream_ModuleHandlerConfiguration> Stream_IModule_t;
typedef Common_IStateMachine_T<Stream_StateMachine_ControlState> Stream_IStateMachine_t;

struct Stream_Configuration
{
  inline Stream_Configuration ()
   : bufferSize (STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , cloneModule (false) // *NOTE*: cloneModule ==> deleteModule
   , deleteModule (false)
   , messageAllocator (NULL)
   , module (NULL)
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
   , notificationStrategy (NULL)
   , printFinalReport (false)
   , serializeOutput (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , sessionID (reinterpret_cast<size_t> (ACE_INVALID_HANDLE))
#else
   , sessionID (static_cast<size_t> (ACE_INVALID_HANDLE))
#endif
   , statisticReportingInterval (0)
   , useThreadPerConnection (false)
  {};

  unsigned int                       bufferSize;
  bool                               cloneModule;
  bool                               deleteModule;
  Stream_IAllocator*                 messageAllocator;
  Stream_Module_t*                   module;
  Stream_ModuleConfiguration*        moduleConfiguration;
  Stream_ModuleHandlerConfiguration* moduleHandlerConfiguration;
  ACE_Notification_Strategy*         notificationStrategy;
  bool                               printFinalReport;
  // *IMPORTANT NOTE*: in a multi-threaded environment, threads MAY be
  //                   dispatching the reactor notification queue concurrently
  //                   (most notably, ACE_TP_Reactor)
  //                   --> enforce proper serialization
  bool                               serializeOutput;
  size_t                             sessionID;
  unsigned int                       statisticReportingInterval; // 0: don't report
  bool                               useThreadPerConnection;
};

struct Stream_ModuleConfiguration
{
  inline Stream_ModuleConfiguration ()
   : streamConfiguration (NULL)
  {};

//  // *TODO*: consider moving this somewhere else
//  Stream_State* streamState;
  Stream_Configuration* streamConfiguration;
};

struct Stream_ModuleHandlerConfiguration
{
  inline Stream_ModuleHandlerConfiguration ()
   : active (false)
   , streamConfiguration (NULL)
  {};

  bool                  active; // *NOTE*: applies to head modules only
  Stream_Configuration* streamConfiguration;
};

typedef Stream_SessionDataBase_T<Stream_SessionData> Stream_SessionData_t;

typedef Stream_StatisticHandler_Reactor_T<Stream_Statistic> Stream_StatisticHandler_Reactor_t;
typedef Stream_StatisticHandler_Proactor_T<Stream_Statistic> Stream_StatisticHandler_Proactor_t;

#endif
