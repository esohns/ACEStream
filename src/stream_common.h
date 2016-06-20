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

#include "ace/Message_Block.h"
#include "ace/Notification_Strategy.h"
#include "ace/Stream.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_istatemachine.h"
#include "common_time_common.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_imodule.h"
#include "stream_session_data.h"
#include "stream_statistichandler.h"
#include "stream_statemachine_control.h"

enum Stream_MessageType : int
{
  // *NOTE*: see "ace/Message_Block.h" for details
  STREAM_MESSAGE_MAP   = ACE_Message_Block::MB_USER, // session (== 0x200)
  // *** control ***
  STREAM_MESSAGE_SESSION,
  // *** control - END ***
  STREAM_MESSAGE_MAP_2 = 0x300,                      // data
  // *** data ***
  STREAM_MESSAGE_DATA,                               // protocol data
  STREAM_MESSAGE_OBJECT,                             // (OO) message object type (--> dynamic type)
  // *** data - END ***
  STREAM_MESSAGE_MAP_3 = 0x400,                      // protocol
  // *** protocol ***
  // *** protocol - END ***
  ///////////////////////////////////////
  STREAM_MESSAGE_MAX,
  STREAM_MESSAGE_INVALID
};

enum Stream_SessionMessageType
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_SESSION_MESSAGE_MAP = ACE_Message_Block::MB_USER + 1,
  // *** control ***
  STREAM_SESSION_BEGIN,
  STREAM_SESSION_STEP,
  STREAM_SESSION_END,
  // *** control - END ***
  // *** data ***
  STREAM_SESSION_STATISTIC,
  // *** data - END ***
  ///////////////////////////////////////
  STREAM_SESSION_MAX,
  STREAM_SESSION_INVALID
};

enum Stream_ControlType
{
  STREAM_CONTROL_STEP = 0,
  ///////////////////////////////////////
  STREAM_CONTROL_MAX,
  STREAM_CONTROL_INVALID
};

struct Stream_Statistic
{
  inline Stream_Statistic ()
   : bytes (0.0F)
   , dataMessages (0)
   , droppedMessages (0)
   , bytesPerSecond (0.0F)
   , messagesPerSecond (0.0F)
   , timeStamp (ACE_Time_Value::zero)
  {};

  inline Stream_Statistic operator+= (const Stream_Statistic& rhs_in)
  {
    bytes += rhs_in.bytes;
    dataMessages += rhs_in.dataMessages;
    droppedMessages += rhs_in.droppedMessages;

    timeStamp = rhs_in.timeStamp;

    return *this;
  };

  float          bytes;           // amount of processed data
  unsigned int   dataMessages;    // (protocol) messages
  unsigned int   droppedMessages; // dropped messages

  // (current) runtime performance
  float          bytesPerSecond;
  float          messagesPerSecond;

  ACE_Time_Value timeStamp;
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
   , lastCollectionTimeStamp (ACE_Time_Value::zero)
   , lock (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , sessionID (reinterpret_cast<unsigned int> (ACE_INVALID_HANDLE))
#else
   , sessionID (static_cast<unsigned int> (ACE_INVALID_HANDLE))
#endif
   , startOfSession (ACE_Time_Value::zero)
   , userData (NULL)
  {};

  inline Stream_SessionData& operator+= (const Stream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    aborted = (aborted ? aborted : rhs_in.aborted);
    currentStatistic =
        ((currentStatistic.timeStamp > rhs_in.currentStatistic.timeStamp) ? currentStatistic
                                                                          : rhs_in.currentStatistic);
    lastCollectionTimeStamp =
        ((lastCollectionTimeStamp > rhs_in.lastCollectionTimeStamp) ? lastCollectionTimeStamp
                                                                    : rhs_in.lastCollectionTimeStamp);
    //lock = (lock ? lock : rhs_in.lock);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    sessionID =
        ((sessionID == reinterpret_cast<unsigned int> (ACE_INVALID_HANDLE)) ? rhs_in.sessionID
                                                                            : sessionID);
#else
    sessionID =
        ((sessionID == static_cast<unsigned int> (ACE_INVALID_HANDLE)) ? rhs_in.sessionID
                                                                       : sessionID);
#endif
    startOfSession =
        (startOfSession > rhs_in.startOfSession ? startOfSession
                                                : rhs_in.startOfSession);
    //userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  // *NOTE*: modules can set this to signal (internal) processing errors.
  //         The (stream / process) control logic may (or may not) then react to
  //         abort processing early
  bool             aborted;

  Stream_Statistic currentStatistic;
  ACE_Time_Value   lastCollectionTimeStamp;
  ACE_SYNCH_MUTEX* lock;

  unsigned int     sessionID; // (== socket handle !)
  ACE_Time_Value   startOfSession;

  Stream_UserData* userData;
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

struct Stream_State
{
  inline Stream_State ()
   : currentSessionData (NULL)
   , deleteModule (false)
   , module (NULL)
   , stateMachineLock (NULL, // name
                       NULL) // attributes
   , userData (NULL)
  {};

  Stream_SessionData* currentSessionData;
  bool                deleteModule;
  Stream_Module_t*    module;
  ACE_SYNCH_MUTEX     stateMachineLock;
  Stream_UserData*    userData;
};

struct Stream_AllocatorConfiguration
{
  inline Stream_AllocatorConfiguration ()
   : buffer (0)
  {};

  // *NOTE*: adds bytes to each malloc(), override as needed
  //         (e.g. flex requires additional 2 YY_END_OF_BUFFER_CHARs)
  unsigned int buffer;
};

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
   , sessionID (reinterpret_cast<unsigned int> (ACE_INVALID_HANDLE))
#else
   , sessionID (static_cast<unsigned int> (ACE_INVALID_HANDLE))
#endif
   , statisticReportingInterval (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
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
  unsigned int                       sessionID;
  ACE_Time_Value                     statisticReportingInterval; // [ACE_Time_Value::zero: off]
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
   , crunchMessages (STREAM_MODULE_DEFAULT_CRUNCH_MESSAGES)
   , hasHeader (false)
   , passive (true)
   , stateMachineLock (NULL)
   , streamConfiguration (NULL)
   , traceParsing (STREAM_DEFAULT_YACC_TRACE)
   , traceScanning (STREAM_DEFAULT_LEX_TRACE)
  {};

  bool                  active; // *NOTE*: head module(s)
  // *NOTE*: this option may be useful for (downstream) modules that only work
  //         on CONTIGUOUS buffers (i.e. cannot parse chained message blocks)
  bool                  crunchMessages;
  bool                  hasHeader;
  bool                  passive; // *NOTE*: head module(s)

  ACE_SYNCH_MUTEX*      stateMachineLock;

  Stream_Configuration* streamConfiguration;
  bool                  traceParsing;  // debug yacc (bison) ?
  bool                  traceScanning; // debug (f)lex ?
};

typedef Stream_SessionData_T<Stream_SessionData> Stream_SessionData_t;

typedef Stream_StatisticHandler_Reactor_T<Stream_Statistic> Stream_StatisticHandler_Reactor_t;
typedef Stream_StatisticHandler_Proactor_T<Stream_Statistic> Stream_StatisticHandler_Proactor_t;

#endif
