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
#include <list>
#include <map>
#include <string>

#include "ace/config-lite.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_istatemachine.h"

#include "common_time_common.h"

#include "stream_defines.h"
#include "stream_ilock.h"
#include "stream_inotify.h"
#include "stream_statemachine_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_common.h"
#else
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

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
template <typename ControlType,
          typename MessageType,
          typename AllocatorConfigurationType>
class Stream_ControlMessage_T;
template <ACE_SYNCH_DECL,
          typename TimePolicyType>
class Stream_IStream_T;
struct Stream_UserData;

#if defined (__llvm__)
enum Stream_HeadModuleConcurrency
#else
enum Stream_HeadModuleConcurrency : int
#endif
{
  STREAM_HEADMODULECONCURRENCY_INVALID = -1,
  ////////////////////////////////////////
  STREAM_HEADMODULECONCURRENCY_ACTIVE,      // <-- dedicated worker thread(s)
  STREAM_HEADMODULECONCURRENCY_CONCURRENT,  // <-- in-line (concurrent put())
  STREAM_HEADMODULECONCURRENCY_PASSIVE,     // <-- in-line (invokes svc() on start())
  ////////////////////////////////////////
  STREAM_HEADMODULECONCURRENCY_MAX,
};

#if defined (__llvm__)
enum Stream_MessageType
#else
enum Stream_MessageType : int
#endif
{
  STREAM_MESSAGE_INVALID       = -1,
  ////////////////////////////////////////
  // *NOTE*: see "ace/Message_Block.h" for details
  STREAM_MESSAGE_MASK          = ACE_Message_Block::MB_USER, // == 0x200
  STREAM_MESSAGE_CONTROL,
  STREAM_MESSAGE_SESSION,
  ////////////////////////////////////////
  // *** data ***
  STREAM_MESSAGE_DATA          = ACE_Message_Block::MB_DATA,  // data (raw)
  STREAM_MESSAGE_OBJECT        = ACE_Message_Block::MB_PROTO, // data (dynamic type)
  STREAM_MESSAGE_DATA_MASK     = 0x400,                       // data
  STREAM_MESSAGE_PROTOCOL_MASK = 0x800,                       // protocol
  ////////////////////////////////////////
  STREAM_MESSAGE_MAX           = (int)0xFFFFFFFF,
};

#if defined (__llvm__)
enum Stream_ControlType
#else
enum Stream_ControlType : int
#endif
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_CONTROL_USER_MASK   = 0x400, // user-defined message mask
  STREAM_CONTROL_END,                 // end session (i.e. user cancel)
  STREAM_CONTROL_ABORT,
  STREAM_CONTROL_CONNECT,
  STREAM_CONTROL_DISCONNECT  = ACE_Message_Block::MB_HANGUP,
  STREAM_CONTROL_LINK        = 0x404,
  STREAM_CONTROL_RESIZE,
  STREAM_CONTROL_UNLINK      = ACE_Message_Block::MB_BREAK,
  STREAM_CONTROL_FLUSH       = ACE_Message_Block::MB_FLUSH,
  STREAM_CONTROL_RESET       = ACE_Message_Block::MB_NORMAL,
  STREAM_CONTROL_STEP        = 0x406, // e.g. take screenshot, split target file, etc.
  STREAM_CONTROL_STEP_2,              // e.g. take screenshot, split target file, etc.
  ////////////////////////////////////////
  STREAM_CONTROL_USER_MASK_2 = 0x800, // user-defined message mask
  ////////////////////////////////////////
  STREAM_CONTROL_MAX,
  STREAM_CONTROL_INVALID
};
#if defined (__llvm__)
enum Stream_ControlMessageType
#else
enum Stream_ControlMessageType : int
#endif
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_CONTROL_MESSAGE_MASK      = 0x400,
  // *** control ***
  STREAM_CONTROL_MESSAGE_END,
  STREAM_CONTROL_MESSAGE_ABORT,
  STREAM_CONTROL_MESSAGE_CONNECT,
  STREAM_CONTROL_MESSAGE_DISCONNECT,
  STREAM_CONTROL_MESSAGE_LINK,
  STREAM_CONTROL_MESSAGE_RESIZE,
  STREAM_CONTROL_MESSAGE_UNLINK,
  STREAM_CONTROL_MESSAGE_FLUSH,
  STREAM_CONTROL_MESSAGE_RESET,
  STREAM_CONTROL_MESSAGE_STEP,
  STREAM_CONTROL_MESSAGE_STEP_2,
  ////////////////////////////////////////
  STREAM_CONTROL_MESSAGE_USER_MASK = 0x800, // user-defined message mask
  ////////////////////////////////////////
  STREAM_CONTROL_MESSAGE_MAX,
  STREAM_CONTROL_MESSAGE_INVALID
};

#if defined (__llvm__)
enum Stream_SessionMessageType
#else
enum Stream_SessionMessageType : int
#endif
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_SESSION_MESSAGE_MASK      = ACE_Message_Block::MB_USER, // == 0x200
  // *** notification ***
  STREAM_SESSION_MESSAGE_ABORT,
  STREAM_SESSION_MESSAGE_CONNECT,
  STREAM_SESSION_MESSAGE_DISCONNECT,
  STREAM_SESSION_MESSAGE_LINK,
  STREAM_SESSION_MESSAGE_RESIZE,
  STREAM_SESSION_MESSAGE_UNLINK,
  // *** control ***
  STREAM_SESSION_MESSAGE_BEGIN,
  STREAM_SESSION_MESSAGE_END,
  STREAM_SESSION_MESSAGE_STEP, // i.e. processing next source file
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
  Stream_Statistic ()
   : capturedFrames (0)
   , droppedFrames (0)
   , bytes (0.0F)
   , dataMessages (0)
   , bytesPerSecond (0.0F)
   , messagesPerSecond (0.0F)
   , timeStamp (ACE_Time_Value::zero)
  {}

  struct Stream_Statistic operator~ ()
  {
    capturedFrames = 0;
    droppedFrames = 0;
    bytes = 0.0F;
    dataMessages = 0;
    bytesPerSecond = 0.0F;
    messagesPerSecond = 0.0F;
    timeStamp = ACE_Time_Value::zero;

    return *this;
  }
  struct Stream_Statistic operator+= (const struct Stream_Statistic& rhs_in)
  {
    capturedFrames += rhs_in.capturedFrames;
    droppedFrames += rhs_in.droppedFrames;

    bytes += rhs_in.bytes;
    dataMessages += rhs_in.dataMessages;

    return *this;
  }

  unsigned int   capturedFrames; // captured/generated frames
  unsigned int   droppedFrames;  // dropped frames (i.e. driver congestion, buffer overflow, etc)

  float          bytes;          // amount of processed data
  unsigned int   dataMessages;   // (protocol) messages

  // (current) runtime performance
  float          bytesPerSecond;
  float          messagesPerSecond;

  ACE_Time_Value timeStamp;
};

// *NOTE*: 'unsigned long' allows efficient atomic increments on many platforms
//         (see: available ACE_Atomic_Op template specializations)
typedef unsigned long Stream_SessionId_t;

struct Net_ConnectionState;
typedef std::map<ACE_HANDLE, struct Net_ConnectionState*> Stream_ConnectionStates_t;
typedef Stream_ConnectionStates_t::iterator Stream_ConnectionStatesIterator_t;

struct Stream_SessionData
{
  Stream_SessionData ()
   : aborted (false)
   , connectionStates ()
   , lastCollectionTimeStamp (ACE_Time_Value::zero)
   , lock (NULL)
   , sessionId (0)
   , startOfSession (ACE_Time_Value::zero)
   , state (NULL)
   , statistic ()
   , userData (NULL)
  {}
  // *NOTE*: the idea is to 'copy' the data
  Stream_SessionData (const struct Stream_SessionData& data_in)
   : aborted (data_in.aborted)
   , connectionStates (data_in.connectionStates)
   , lastCollectionTimeStamp (data_in.lastCollectionTimeStamp)
   , lock (data_in.lock)
   , sessionId (data_in.sessionId)
   , startOfSession (data_in.startOfSession)
   , state (data_in.state)
   , statistic (data_in.statistic)
   , userData (data_in.userData)
  {}

  // *NOTE*: the idea is to 'merge' the data
  struct Stream_SessionData& operator+= (const struct Stream_SessionData& rhs_in)
  {
    aborted = (aborted ? aborted : rhs_in.aborted);
    connectionStates.insert (rhs_in.connectionStates.begin (),
                             rhs_in.connectionStates.end ());
    lastCollectionTimeStamp =
        ((lastCollectionTimeStamp >= rhs_in.lastCollectionTimeStamp) ? lastCollectionTimeStamp
                                                                     : rhs_in.lastCollectionTimeStamp);
    //lock = (lock ? lock : rhs_in.lock);
    // *IMPORTANT NOTE*: always retain the current session id, if any
    sessionId = (sessionId ? sessionId : rhs_in.sessionId);
    startOfSession =
        (startOfSession >= rhs_in.startOfSession ? startOfSession
                                                 : rhs_in.startOfSession);
    statistic =
        ((statistic.timeStamp >= rhs_in.statistic.timeStamp) ? statistic
                                                             : rhs_in.statistic);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  // *NOTE*: set when/iff:
  //         - modules notify initialization/processing errors
  //         - stream processing ends 'early' (i.e. user abort, connection
  //           reset, ...)
  bool                      aborted;
  Stream_ConnectionStates_t connectionStates;
  ACE_Time_Value            lastCollectionTimeStamp;
  ACE_SYNCH_MUTEX*          lock;
  Stream_SessionId_t        sessionId;
  ACE_Time_Value            startOfSession;
  struct Stream_State*      state;
  struct Stream_Statistic   statistic;

  struct Stream_UserData*   userData;
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

typedef std::list<Stream_Module_t*> Stream_ModuleList_t;
typedef Stream_ModuleList_t::const_iterator Stream_ModuleListIterator_t;
typedef Stream_ModuleList_t::reverse_iterator Stream_ModuleListReverseIterator_t;
typedef std::deque<std::string> Stream_Branches_t;
typedef Stream_Branches_t::const_iterator Stream_BranchesIterator_t;

typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_INotify_t;
typedef Common_IStateMachine_T<enum Stream_StateMachine_ControlState> Stream_IStateMachine_t;

struct Stream_State
{
  Stream_State ()
   : deleteModule (false)
   , module (NULL)
   , sessionData (NULL)
   , stateMachineLock (NULL)
   , userData (NULL)
  {}

  struct Stream_State& operator+= (const struct Stream_State& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data

    //deleteModule = (deleteModule ? deleteModule : rhs_in.deleteModule);
    //module = (module ? module : rhs_in.module);
    //sessionData = (sessionData ? sessionData : rhs_in.sessionData);
    //stateMachineLock =
      //(stateMachineLock ? stateMachineLock : rhs_in.stateMachineLock);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  bool                       deleteModule;
  Stream_Module_t*           module; // final-
  struct Stream_SessionData* sessionData;
  ACE_SYNCH_MUTEX*           stateMachineLock;

  struct Stream_UserData*    userData;
};

struct Stream_UserData
{
  Stream_UserData ()
   : userData (NULL)
  {}

  void* userData;
};

//////////////////////////////////////////

typedef int Stream_CommandType_t;

// *NOTE*: 'unsigned long' allows efficient atomic increments on many platforms
//         (see: available ACE_Atomic_Op template specializations)
typedef unsigned long Stream_MessageId_t;

struct Stream_AllocatorConfiguration;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Stream_ControlMessage_t;

typedef Stream_ILock_T<ACE_MT_SYNCH> Stream_ILock_t;
typedef Stream_IStream_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t> Stream_IStream_t;

#endif
