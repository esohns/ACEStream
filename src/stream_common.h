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

#include <deque>

#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include "common_istatemachine.h"
#include "common_time_common.h"

#include "stream_defines.h"
#include "stream_ilock.h"
#include "stream_inotify.h"
#include "stream_statistichandler.h"
#include "stream_statemachine_control.h"

// forward declarations
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Message_Queue;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Task;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Stream;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Stream_Iterator;
class ACE_Notification_Strategy;
class Stream_IAllocator;

enum Stream_MessageType : int
{
  // *NOTE*: see "ace/Message_Block.h" for details
  STREAM_MESSAGE_MASK         = ACE_Message_Block::MB_USER, // == 0x200
  STREAM_MESSAGE_CONTROL,
  STREAM_MESSAGE_SESSION,
  ////////////////////////////////////////
  STREAM_MESSAGE_DATA_MASK    = 0x400,                      // data
  // *** data ***
  STREAM_MESSAGE_DATA,                                      // data (raw)
  STREAM_MESSAGE_OBJECT,                                    // data (dynamic type)
  STREAM_MESSAGE_PROTCOL_MASK = 0x800,                      // protocol
  ////////////////////////////////////////
  STREAM_MESSAGE_MAX,
  STREAM_MESSAGE_INVALID
};

enum Stream_ControlType : int
{
  STREAM_CONTROL_CONNECT    = 0,
  STREAM_CONTROL_DISCONNECT = ACE_Message_Block::MB_HANGUP,
  STREAM_CONTROL_FLUSH      = ACE_Message_Block::MB_FLUSH,
  STREAM_CONTROL_LINK,
  STREAM_CONTROL_STEP,
  STREAM_CONTROL_UNLINK     = ACE_Message_Block::MB_BREAK,
  ////////////////////////////////////////
  STREAM_CONTROL_USER_MASK  = 0x200, // user-defined message mask
  ////////////////////////////////////////
  STREAM_CONTROL_MAX,
  STREAM_CONTROL_INVALID
};
enum Stream_ControlMessageType : int
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_CONTROL_MESSAGE_MASK      = ACE_Message_Block::MB_USER, // == 0x200
  // *** control ***
  STREAM_CONTROL_MESSAGE_CONNECT,
  STREAM_CONTROL_MESSAGE_DISCONNECT,
  STREAM_CONTROL_MESSAGE_FLUSH,
  STREAM_CONTROL_MESSAGE_LINK,
  STREAM_CONTROL_MESSAGE_STEP,
  STREAM_CONTROL_MESSAGE_UNLINK,
  ////////////////////////////////////////
  STREAM_CONTROL_MESSAGE_USER_MASK = 0x400, // user-defined message mask
  ////////////////////////////////////////
  STREAM_CONTROL_MESSAGE_MAX,
  STREAM_CONTROL_MESSAGE_INVALID
};

enum Stream_SessionMessageType : int
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_SESSION_MESSAGE_MASK      = ACE_Message_Block::MB_USER, // == 0x200
  // *** notification ***
  STREAM_SESSION_MESSAGE_ABORT,
  STREAM_SESSION_MESSAGE_CONNECT,
  STREAM_SESSION_MESSAGE_DISCONNECT,
  STREAM_SESSION_MESSAGE_LINK,
  STREAM_SESSION_MESSAGE_UNLINK,
  // *** control ***
  STREAM_SESSION_MESSAGE_BEGIN,
  STREAM_SESSION_MESSAGE_END,
  STREAM_SESSION_MESSAGE_STEP,
  // *** data ***
  STREAM_SESSION_MESSAGE_STATISTIC,
  ////////////////////////////////////////
  STREAM_SESSION_MESSAGE_USER_MASK = 0x400, // user-defined message mask
  ////////////////////////////////////////
  STREAM_SESSION_MESSAGE_MAX,
  STREAM_SESSION_MESSAGE_INVALID
};

struct Stream_Statistic
{
  inline Stream_Statistic ()
   : capturedFrames (0)
   , droppedFrames (0)
   , bytes (0.0F)
   , dataMessages (0)
   , bytesPerSecond (0.0F)
   , messagesPerSecond (0.0F)
   , timeStamp (ACE_Time_Value::zero)
  {};

  inline Stream_Statistic operator+= (const Stream_Statistic& rhs_in)
  {
    capturedFrames += rhs_in.capturedFrames;
    droppedFrames += rhs_in.droppedFrames;

    bytes += rhs_in.bytes;
    dataMessages += rhs_in.dataMessages;

    timeStamp = rhs_in.timeStamp;

    return *this;
  };

  unsigned int   capturedFrames; // captured/generated frames
  unsigned int   droppedFrames;  // dropped frames (i.e. driver congestion, buffer overflow, etc)

  float          bytes;          // amount of processed data
  unsigned int   dataMessages;   // (protocol) messages

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

typedef unsigned int Stream_SessionId_t;

struct Stream_SessionData
{
  inline Stream_SessionData ()
   : aborted (false)
   , currentStatistic ()
   , lastCollectionTimeStamp (ACE_Time_Value::zero)
   , lock (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , sessionID (reinterpret_cast<Stream_SessionId_t> (ACE_INVALID_HANDLE))
#else
   , sessionID (static_cast<Stream_SessionId_t> (ACE_INVALID_HANDLE))
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
        ((sessionID == reinterpret_cast<Stream_SessionId_t> (ACE_INVALID_HANDLE)) ? rhs_in.sessionID
                                                                                  : sessionID);
#else
    sessionID =
        ((sessionID == static_cast<Stream_SessionId_t> (ACE_INVALID_HANDLE)) ? rhs_in.sessionID
                                                                             : sessionID);
#endif
    startOfSession =
        (startOfSession > rhs_in.startOfSession ? startOfSession
                                                : rhs_in.startOfSession);
    //userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  // *NOTE*: this will be set when/if modules notify initialization/processing
  //         errors and/or when the stream processing ends early (i.e. user
  //         abort, connection reset, etc...)
  bool               aborted;

  Stream_Statistic   currentStatistic;
  ACE_Time_Value     lastCollectionTimeStamp;
  ACE_SYNCH_MUTEX*   lock;

  Stream_SessionId_t sessionID; // (== socket handle !)
  ACE_Time_Value     startOfSession;

  Stream_UserData*   userData;
};

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
typedef std::deque<Stream_Module_t*> Stream_ModuleList_t;
typedef Stream_ModuleList_t::const_iterator Stream_ModuleListIterator_t;
typedef Stream_ModuleList_t::reverse_iterator Stream_ModuleListReverseIterator_t;

struct Stream_ModuleConfiguration;
struct Stream_ModuleHandlerConfiguration;
typedef Stream_INotify_T<Stream_SessionMessageType> Stream_INotify_t;
typedef Common_IStateMachine_T<enum Stream_StateMachine_ControlState> Stream_IStateMachine_t;

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
   : notify (NULL)
   , streamConfiguration (NULL)
  {};

  Stream_INotify_t*     notify;
  // *TODO*: remove this ASAP
  Stream_Configuration* streamConfiguration;
};

typedef Stream_ILock_T<ACE_MT_SYNCH> Stream_ILock_t;

struct Stream_ModuleHandlerConfiguration
{
  inline Stream_ModuleHandlerConfiguration ()
   : active (false) // head module
   , bufferSize (STREAM_MESSAGE_DATA_BUFFER_SIZE)
   // *WARNING*: when disabled, this 'locks down' the pipeline head module. It
   //            will then hold the 'stream lock' during message processing to
   //            support (down)stream synchronization. This really only makes
   //            sense in fully synchronous layouts, or 'concurrent' scenarios
   //            with non-reentrant modules
   //            --> disable only if you know what you are doing
   , concurrent (true)
   , crunchMessages (STREAM_MODULE_DEFAULT_CRUNCH_MESSAGES)
   , hasHeader (false)
   , ilock (NULL)
   , messageAllocator (NULL)
   , passive (true)                           // net module(s)
   , reportingInterval (0)
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , stateMachineLock (NULL)
   , streamConfiguration (NULL)
   , traceParsing (STREAM_DEFAULT_YACC_TRACE) // parser module(s)
   , traceScanning (STREAM_DEFAULT_LEX_TRACE) // parser module(s)
  {};

  bool                     active; // head module(s)
  unsigned int             bufferSize;
  bool                     concurrent; // head module(s)
  // *NOTE*: this option may be useful for (downstream) modules that only work
  //         on CONTIGUOUS buffers (i.e. cannot parse chained message blocks)
  bool                     crunchMessages;
  bool                     hasHeader;
  // *NOTE*: modules can use this to temporarily relinquish the stream lock
  //         while they wait on some condition, in order to avoid deadlocks
  //         --> used primarily in 'non-concurrent' (see above) scenarios
  Stream_ILock_t*          ilock;
  Stream_IAllocator*       messageAllocator;
  bool                     passive; // network/device/... module(s)

  unsigned int             reportingInterval; // (statistic) reporting interval (second(s)) [0: off]
  ACE_Time_Value           statisticCollectionInterval; // head module(s)

  ACE_SYNCH_MUTEX*         stateMachineLock; // head module(s)

  //Net_SocketConfiguration* socketConfiguration;
  // *TODO*: remove this ASAP
  Stream_Configuration*    streamConfiguration;

  // *NOTE*: this distinction applies mostly to (f)lex/yacc(bison)-based parsers
  bool                     traceParsing;  // parser module(s)
  bool                     traceScanning; // parser module(s)
};

typedef Stream_StatisticHandler_Reactor_T<Stream_Statistic> Stream_StatisticHandler_Reactor_t;
typedef Stream_StatisticHandler_Proactor_T<Stream_Statistic> Stream_StatisticHandler_Proactor_t;

#endif
