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
#include "stream_statistic.h"

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
          typename StatusType,
          typename StateType>
class Stream_IStreamControlBase_T;
template <ACE_SYNCH_DECL,
          typename TimePolicyType>
class Stream_IStream_T;
struct Stream_UserData;

#if defined (__llvm__)
enum Stream_HeadModuleConcurrency
#else
enum Stream_HeadModuleConcurrency : int
#endif // __llvm__
{
  STREAM_HEADMODULECONCURRENCY_INVALID = -1,
  ////////////////////////////////////////
  STREAM_HEADMODULECONCURRENCY_ACTIVE,      // <-- dedicated worker thread(s)
  STREAM_HEADMODULECONCURRENCY_CONCURRENT,  // <-- in-line (concurrent put())
  STREAM_HEADMODULECONCURRENCY_PASSIVE,     // <-- in-line (invokes svc() on start())
  ////////////////////////////////////////
  STREAM_HEADMODULECONCURRENCY_MAX,
};

enum Stream_MessageDefragmentMode
{
  STREAM_DEFRAGMENT_CLONE = 0, // *NOTE*: includes defragmentation mode
  STREAM_DEFRAGMENT_CONDENSE,
  STREAM_DEFRAGMENT_DEFRAGMENT,
  ////////////////////////////////////////
  STREAM_DEFRAGMENT_INVALID,
  STREAM_DEFRAGMENT_MAX,
};

#if defined (__llvm__)
enum Stream_MessageType
#else
enum Stream_MessageType : int
#endif // __llvm__
{
  STREAM_MESSAGE_INVALID       = -1,
  ////////////////////////////////////////
  // *NOTE*: see "ace/Message_Block.h" for details
  STREAM_MESSAGE_MASK          = ACE_Message_Block::MB_USER,  // == 0x200
  STREAM_MESSAGE_CONTROL,
  STREAM_MESSAGE_SESSION       = STREAM_MESSAGE_SESSION_TYPE,
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
#endif // __llvm__
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_CONTROL_USER_MASK   = 0x400, // user-defined message mask
  STREAM_CONTROL_END,                 // [internal] linked downstream head module(s) only
  STREAM_CONTROL_ABORT,
  STREAM_CONTROL_CONNECT,
  STREAM_CONTROL_DISCONNECT  = ACE_Message_Block::MB_HANGUP,
  STREAM_CONTROL_LINK        = 0x404,                       // --> translated to session message (see also: Stream_HeadModuleTaskBase_T::control)
  STREAM_CONTROL_RESIZE,                                    // --> translated to session message (see also: Stream_HeadModuleTaskBase_T::control)
  STREAM_CONTROL_UNLINK      = ACE_Message_Block::MB_BREAK, // --> translated to session message (see also: Stream_HeadModuleTaskBase_T::control)
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
#endif // __llvm__
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
#endif // __llvm__
{
  // *NOTE*: see "ace/Message_Block.h" and "stream_message_base.h" for details
  STREAM_SESSION_MESSAGE_MASK      = ACE_Message_Block::MB_USER, // == 0x200
  // *** notification ***
  STREAM_SESSION_MESSAGE_ABORT,
  STREAM_SESSION_MESSAGE_CONNECT,
  STREAM_SESSION_MESSAGE_DISCONNECT,
  STREAM_SESSION_MESSAGE_LINK,
  STREAM_SESSION_MESSAGE_RESIZE, // *TODO*: disambiguate between updated session data (i.e. input data) and  updated configuration data
  STREAM_SESSION_MESSAGE_UNLINK,
  // *** control ***
  STREAM_SESSION_MESSAGE_BEGIN,
  STREAM_SESSION_MESSAGE_END,
  STREAM_SESSION_MESSAGE_STEP, // e.g. next source file, data complete, ...
  STREAM_SESSION_MESSAGE_STEP_DATA, // progress has been made; e.g. more data has arrived, ...
  // *** data ***
  STREAM_SESSION_MESSAGE_STATISTIC,
  ////////////////////////////////////////
  STREAM_SESSION_MESSAGE_USER_MASK = 0x400, // user-defined message mask
  ////////////////////////////////////////
  STREAM_SESSION_MESSAGE_MAX,
  STREAM_SESSION_MESSAGE_INVALID
};

// *NOTE*: 'unsigned long' allows efficient atomic increments on many platforms
//         (see: available ACE_Atomic_Op template specializations)
typedef unsigned long Stream_SessionId_t;

struct Stream_SessionData
{
  Stream_SessionData ()
   : aborted (false)
   , bytes (0)
   , lastCollectionTimeStamp (ACE_Time_Value::zero)
   , lock (NULL)
   , managed (false)
   , sessionId (0)
   , startOfSession (ACE_Time_Value::zero)
   , statistic ()
   , userData (NULL)
  {}
  // *NOTE*: the idea is to 'copy' the data
  Stream_SessionData (const struct Stream_SessionData& data_in)
   : aborted (data_in.aborted)
   , bytes (data_in.bytes)
   , lastCollectionTimeStamp (data_in.lastCollectionTimeStamp)
   , lock (data_in.lock)
   , managed (data_in.managed)
   , sessionId (data_in.sessionId)
   , startOfSession (data_in.startOfSession)
   , statistic (data_in.statistic)
   , userData (data_in.userData)
  {}
  inline virtual ~Stream_SessionData () {}

  // *NOTE*: the idea is to 'merge' the data
  struct Stream_SessionData& operator+= (const struct Stream_SessionData& rhs_in)
  {
    aborted = (aborted ? aborted : rhs_in.aborted);
    bytes += rhs_in.bytes;
    lastCollectionTimeStamp =
        ((lastCollectionTimeStamp >= rhs_in.lastCollectionTimeStamp) ? lastCollectionTimeStamp
                                                                     : rhs_in.lastCollectionTimeStamp);
    lock = lock ? lock : rhs_in.lock; // try to retain own lock (helps with consistency)
    sessionId = std::max (sessionId, rhs_in.sessionId);
    startOfSession =
        (startOfSession >= rhs_in.startOfSession ? startOfSession
                                                 : rhs_in.startOfSession);

    statistic += rhs_in.statistic;

    userData = userData ? userData : rhs_in.userData; // retain own user data (if any)

    return *this;
  }

  inline virtual void clear () { statistic.reset (); }

  // *NOTE*: set when/iff:
  //         - modules notify initialization/processing errors
  //         - stream processing ends 'early' (i.e. user abort, connection
  //           reset, ...)
  bool                    aborted;
  size_t                  bytes; // progress, i.e. STREAM_SESSION_MESSAGE_STEP_DATA
  ACE_Time_Value          lastCollectionTimeStamp;
  ACE_SYNCH_MUTEX*        lock;
  bool                    managed; // lifetime of 'this' managed by session manager ?
  Stream_SessionId_t      sessionId;
  ACE_Time_Value          startOfSession;
  struct Stream_Statistic statistic; // *TODO*: type should be a template parameter

  struct Stream_UserData* userData;
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

typedef Stream_IStream_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t> Stream_IStream_t;
typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_INotify_t;
typedef Common_IStateMachine_T<enum Stream_StateMachine_ControlState> Stream_IStateMachine_t;

struct Stream_State
{
  Stream_State ()
   : linked_ds_ (false)
   , module (NULL)
   , moduleIsClone (false)
   , stateMachineLock (NULL)
   , statistic (NULL)
   , userData (NULL)
  {}

  struct Stream_State& operator+= (const struct Stream_State& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data

    //deleteModule = (deleteModule ? deleteModule : rhs_in.deleteModule);
    //module = (module ? module : rhs_in.module);
    //stateMachineLock =
      //(stateMachineLock ? stateMachineLock : rhs_in.stateMachineLock);

    statistic = (statistic ? statistic : rhs_in.statistic);

    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  bool                       linked_ds_; // to downstream
  Stream_Module_t*           module; // final-
  bool                       moduleIsClone; // final-
  ACE_SYNCH_MUTEX*           stateMachineLock;
  struct Stream_Statistic*   statistic;

  struct Stream_UserData*    userData;
};

typedef Stream_IStreamControlBase_T<enum Stream_ControlType,
                                    enum Stream_StateMachine_ControlState,
                                    struct Stream_State> Stream_IStreamControlBase_t;

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

typedef Stream_ILock_T<ACE_MT_SYNCH> Stream_ILock_t;
typedef Stream_IStream_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t> Stream_IStream_t;

#endif
