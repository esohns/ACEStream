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

#ifndef STREAM_BASE_H
#define STREAM_BASE_H

#include <deque>

#include <ace/Global_Macros.h>
#include <ace/Stream.h>
#include <ace/Synch_Traits.h>

#include "common_idumpstate.h"
//#include "common_iget.h"
//#include "common_iinitialize.h"
#include "common_istatistic.h"

#include "stream_common.h"
#include "stream_head_task.h"
#include "stream_ilock.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_streammodule_base.h"
#include "stream_itask.h"

// forward declaration(s)
class ACE_Notification_Strategy;
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ControlType,
          typename NotificationType,
          typename StatusType,               // (state machine) status
          typename StateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Base_T
 : public ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType>
 , public Stream_IStreamControl_T<ControlType,
                                  NotificationType,
                                  StatusType,
                                  StateType>
 , public Stream_ILock_T<ACE_SYNCH_USE>
 , public Common_IStatistic_T<StatisticContainerType>
// , public Common_IGetSet_T<SessionDataType>
// , public Common_IInitialize_T<ConfigurationType>
 , public Common_IDumpState
{
 public:
  // convenient types
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef Stream_IModule_T</*Stream_SessionId_t,
                           SessionDataType,
                           Stream_SessionMessageType,*/
                           ACE_SYNCH_USE,
                           TimePolicyType,
                           ModuleConfigurationType,
                           HandlerConfigurationType> IMODULE_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef ACE_Stream_Iterator<ACE_SYNCH_USE,
                              TimePolicyType> ITERATOR_T;
  typedef Stream_IStreamControl_T<ControlType,
                                  NotificationType,
                                  StatusType,
                                  StateType> ISTREAM_CONTROL_T;
  typedef Stream_ILock_T<ACE_SYNCH_USE> ILOCK_T;
  typedef ConfigurationType CONFIGURATION_T;
  typedef StateType STATE_T;
  typedef SessionDataContainerType SESSION_DATA_CONTAINER_T;
  typedef SessionDataType SESSION_DATA_T;
  typedef ControlMessageType CONTROL_MESSAGE_T;
  typedef DataMessageType MESSAGE_T;
  typedef SessionMessageType SESSION_MESSAGE_T;
  typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                      SessionDataType,
                                      NotificationType,
                                      DataMessageType,
                                      SessionMessageType> IDATA_NOTIFY_T;

//  using STREAM_T::get;

  // *NOTE*: this will try to sanely close down the stream:
  // 1: tell all worker threads to exit gracefully
  // 2: close() all modules which have not been enqueued onto the stream
  //    (next() == NULL)
  // 3: close() the stream (closes all enqueued modules: wait for queue to flush
  //    and threads, if any, to join)
  virtual ~Stream_Base_T ();

  // implement Stream_IStreamControl_T
  // *NOTE*: delegate these calls to the head module (which also implements this
  //         API)
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual bool isRunning () const;

  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?
  virtual void flush (bool = true,   // flush inbound data ?
                      bool = false,  // flush session messages ?
                      bool = false); // flush upstream (if any) ?
  virtual void pause ();
  virtual void rewind ();
  virtual void wait (bool = true,   // wait for any worker thread(s) ?
                     bool = false,  // wait for upstream (if any) ?
                     bool = false); // wait for downstream (if any) ?
  //virtual void idle (bool = false) const; // wait for upstream (if any) ?

  virtual const MODULE_T* find (const std::string&) const; // module name
  virtual std::string name () const;

  virtual void upStream (STREAM_T*);
  virtual STREAM_T* upStream () const;

  virtual void control (ControlType,   // control type
                        bool = false); // forward upstream ?
  // *NOTE*: the default implementation forwards calls to the head module
  virtual void notify (NotificationType, // notification type
                       bool = false);    // forward upstream ?
  virtual StatusType status () const;
  virtual const StateType& state () const;

  // implement Stream_ILock_T
  // *WARNING*: handle with care
  virtual bool lock (bool = true); // block ?
  virtual int unlock (bool = false); // unblock ?
  virtual ACE_SYNCH_RECURSIVE_MUTEX& getLock ();
  virtual bool hasLock ();

  // implement Common_IDumpState
  virtual void dump_state () const;

  // implement Common_IGetSet_T
  virtual const SessionDataContainerType* get () const;
  virtual void set (const SessionDataContainerType*);

//  // implement Common_IInitialize_T
  // *IMPORTANT NOTE*: sets the stream state machine lock in the module handler
  //                   configuration
  virtual bool initialize (const ConfigurationType&, // configuration
                           bool = true,              // setup pipeline ?
                           bool = true);             // reset session data ?

  // override ACE_Stream method(s)
  virtual int get (ACE_Message_Block*&, // return value: message block handle
                   ACE_Time_Value*);    // timeout (NULL: block)
  // *NOTE*: the default ACE impementation of link() joins writer A to
  //         reader B and writer B to reader A. Prefer 'concat' method: writer
  //         A to writer B(, reader A to reader B; see explanation)
  // *TODO*: the 'outbound' pipe of readers is not currently implemented, as it
  //         creates problems in conjunction with libACENetwork.
  //         libACENetwork connections use the connection streams'
  //         ACE_Stream_Head reader queue/notification to buffer outbound data
  //         while the reactor/connection thread dispatches the corresponding
  //         events.
  //         libACEStream modules encapsulating a network connection may be
  //         tempted to link the data processing stream to the connections'
  //         stream. For 'inbound' (i.e. reader-side oriented) modules, the
  //         connection stream is prepended, for 'outbound' modules appended.
  //         However, doing so requires consideration of the fact that the
  //         connection will still look for data on the connections' (!)
  //         ACE_Stream_Head, which is 'hidden' by the default linking
  //         procedure
  //         --> avoid linking the outbound side of the stream for now
  // *WARNING*: this method is NOT (!) threadsafe in places
  //            --> handle with care !
  // *TODO*: note that for linked streams, the session data passed downstream
  //         is always the upstream instances'. Inconsistencies arise when
  //         modules cache and update session data passed at session start
  virtual int link (STREAM_T&);
  virtual int unlink (void);

  // *NOTE*: the ACE implementation close(s) the removed module. This is not the
  //         intended behavior when the module is being used by several streams
  //         at once
  // *NOTE*: this also removes any trailing modules after the given one
  bool remove (MODULE_T*,    // module handle
               bool = true); // reset() removed module for re-use ?
  // *NOTE*: make sure the original API is not hidden
  using STREAM_T::remove;

  bool isInitialized () const;

  void finished (bool = true); // finish upstream (if any) ?

 protected:
  // convenient types
  typedef ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType> HEAD_BASE_T;
  typedef Stream_HeadTask_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ModuleConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            Stream_SessionMessageType> HEAD_T;
  typedef ACE_Stream_Tail<ACE_SYNCH_USE,
                          TimePolicyType> TAIL_T;
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType> QUEUE_T;
  typedef Stream_IModuleHandler_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  HandlerConfigurationType> IMODULE_HANDLER_T;
  typedef Stream_StateMachine_IControl_T<Stream_StateMachine_ControlState> STATEMACHINE_ICONTROL_T;

  Stream_Base_T (const std::string&, // name
                 bool = false);      // support (upstream) linking ?

  bool finalize ();
  // *NOTE*: derived classes should call this prior to module reinitialization
  //         (i.e. in their own initialize()); this function
  //         - pop/close()s push()ed modules, remove default head/tail modules
  //         - reset reader/writer tasks for all modules
  //         - generate new default head/tail modules
  // *WARNING*: calling this while isRunning() == true blocks until the stream
  //            finishes (because close() of a module waits for its worker
  //            thread(s))
  bool reset ();
  bool setup (ACE_Notification_Strategy* = NULL); // head module (reader task)
                                                  // notification handle

  bool putSessionMessage (Stream_SessionMessageType); // session message type

  // *NOTE*: derived classes must call this in their dtor
  void shutdown ();

  ConfigurationType*        configuration_;
  bool                      delete_;
  // *NOTE*: derived classes set this IF their initialization succeeded;
  //         otherwise, the dtor will NOT stop all worker threads before
  //         close()ing the modules
  bool                      isInitialized_;
  Stream_ModuleList_t       modules_;
  SessionDataContainerType* sessionData_;
  ACE_SYNCH_MUTEX_T         sessionDataLock_;
  StateType                 state_;
  // *NOTE*: cannot currently reach ACE_Stream::linked_us_
  //         --> use this instead
  STREAM_T*                 upStream_;

 private:
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> inherited;

  // convenient types
  typedef Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> OWN_TYPE_T;
  typedef Stream_ITask_T<ControlMessageType,
                         DataMessageType,
                         SessionMessageType> ITASK_T;

//  // make friends between ourselves; instances need to access the session data
//  // lock during (un)link() calls
//  friend class Stream_Base_T<LockType,
//                             ACE_SYNCH_USE,
//                             TimePolicyType,
//                             ControlType,
//                             NotificationType,
//                             StatusType,
//                             StateType,
//                             ConfigurationType,
//                             StatisticContainerType,
//                             ModuleConfigurationType,
//                             HandlerConfigurationType,
//                             SessionDataType,
//                             SessionDataContainerType,
//                             ControlMessageType,
//                             DataMessageType,
//                             SessionMessageType>;

  ACE_UNIMPLEMENTED_FUNC (Stream_Base_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Base_T (const Stream_Base_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Base_T& operator= (const Stream_Base_T&))

  // helper methods
  // implement (part of) Common_IControl
  virtual void initialize (bool = true,  // setup pipeline ?
                           bool = true); // reset session data ?

  // wrap inherited::open/close() calls
  void deactivateModules ();
  void unlinkModules ();

  // *TODO*: replace with state_.module ASAP
  bool                      hasFinal_;
  ACE_SYNCH_RECURSIVE_MUTEX lock_;
  std::string               name_;
};

// include template definition
#include "stream_base.inl"

#endif
