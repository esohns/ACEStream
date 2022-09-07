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

#include <string>

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Stream.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_istatistic.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_head_task.h"
#include "stream_headmoduletask_base.h"
#include "stream_ilink.h"
#include "stream_ilock.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_itask.h"
#include "stream_layout.h"
#include "stream_messagequeue.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_misc_distributor.h"

// forward declaration(s)
class ACE_Notification_Strategy;
class Stream_IAllocator;

// static variables
static const char default_stream_name_string_[] =
  ACE_TEXT_ALWAYS_CHAR ("Stream");

class Stream_Base
{
 public:
  inline virtual ~Stream_Base () {}

 protected:
  inline Stream_Base () {}

  // atomic id generator
  typedef ACE_Atomic_Op<ACE_SYNCH_MUTEX,
                        Stream_SessionId_t> SESSION_ID_GENERATOR_T;
  static SESSION_ID_GENERATOR_T currentSessionId;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Base (const Stream_Base&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Base& operator= (const Stream_Base&))
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName, // *TODO*: use a variadic character array
          ////////////////////////////////
          typename ControlType,
          typename NotificationType,         // session-
          typename StatusType,               // (state machine) status
          typename StateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // module-
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
 , public Stream_Base
 , public Stream_IStreamLayout_T<ACE_SYNCH_USE,
                                 TimePolicyType>
 , public Stream_IStreamControl_T<ControlType,
                                  NotificationType,
                                  StatusType,
                                  StateType>
 , public Stream_ILinkCB
 , public Stream_IEvent_T<NotificationType>
 , public Stream_ISessionCB
 , public Common_IInitialize_T<Stream_Configuration_T<//StreamName,
                                                      ConfigurationType,
                                                      HandlerConfigurationType> >
 , public Common_IStatistic_T<StatisticContainerType>
 , public Common_IGetR_2_T<SessionDataContainerType>
 , public Common_ISetPR_T<SessionDataContainerType>
 , public Common_IGetR_3_T<Stream_MessageQueue_T<ACE_SYNCH_USE,
                                                 TimePolicyType,
                                                 SessionMessageType> >
{
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> inherited;
  typedef Stream_Base inherited2;

 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef Common_TaskBase_T <ACE_SYNCH_USE,
                             TimePolicyType,
                             ACE_Message_Block,
                             ACE_Message_Queue<ACE_SYNCH_USE,
                                               TimePolicyType>,
                             TASK_T> COMMON_TASK_BASE_T;
  typedef Stream_IModule_T<SessionDataType,
                           NotificationType,
                           ACE_SYNCH_USE,
                           TimePolicyType,
                           struct Stream_ModuleConfiguration,
                           HandlerConfigurationType> IMODULE_T;
  typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_SYNCH_USE,
                                                        TimePolicyType,
                                                        HandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        SessionDataType> DISTRIBUTOR_READER_TASK_T;
  typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_SYNCH_USE,
                                                        TimePolicyType,
                                                        HandlerConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        SessionDataType> DISTRIBUTOR_WRITER_TASK_T;
  typedef Stream_StreamModule_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionDataType,
                                NotificationType,
                                struct Stream_ModuleConfiguration,
                                HandlerConfigurationType,
                                libacestream_default_misc_distributor_module_name_string,
                                Stream_INotify_T<NotificationType>,
                                DISTRIBUTOR_READER_TASK_T,
                                DISTRIBUTOR_WRITER_TASK_T> DISTRIBUTOR_MODULE_T;
  typedef Stream_TailWriterTask_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  NotificationType> TAIL_WRITER_T;
  typedef Stream_Layout_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          DISTRIBUTOR_MODULE_T,
                          TAIL_WRITER_T> LAYOUT_T;
  typedef typename LAYOUT_T::ITERATOR_T LAYOUT_ITERATOR_T;
  typedef ACE_Stream_Iterator<ACE_SYNCH_USE,
                              TimePolicyType> ITERATOR_T;
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef Stream_IStreamControl_T<ControlType,
                                  NotificationType,
                                  StatusType,
                                  StateType> ISTREAM_CONTROL_T;
  typedef Stream_Configuration_T<//StreamName,
                                 ConfigurationType,
                                 HandlerConfigurationType> CONFIGURATION_T;
  typedef Common_IInitialize_T<CONFIGURATION_T> IINITIALIZE_T;
  typedef Stream_ILock_T<ACE_SYNCH_USE> ILOCK_T;
  typedef StateType STATE_T;
  typedef SessionDataContainerType SESSION_DATA_CONTAINER_T;
  typedef SessionDataType SESSION_DATA_T;
  typedef Stream_MessageQueue_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionMessageType> MESSAGE_QUEUE_T;
  typedef ControlMessageType CONTROL_MESSAGE_T;
  typedef DataMessageType MESSAGE_T;
  typedef SessionMessageType SESSION_MESSAGE_T;
  typedef Stream_ISessionDataNotify_T<SessionDataType,
                                      NotificationType,
                                      DataMessageType,
                                      SessionMessageType> IDATA_NOTIFY_T;

  // *NOTE*: this will try to (sanely) close down the stream:
  // 1: tell all worker threads to exit gracefully
  // 2: close() all modules which have not been enqueued onto the stream
  //    (next() == NULL)
  // 3: close() the stream (closes all enqueued modules: wait for queue to flush
  //    and threads, if any, to join)
  virtual ~Stream_Base_T ();

  // implement Stream_IStreamControl_T
  // *NOTE*: delegate most of these calls to the head module (which also
  //         implements this API)
  virtual void start ();
  virtual void stop (bool = true,   // wait for completion ?
                     bool = true,   // recurse upstream (if any) ?
                     bool = false); // high priority ?
  inline virtual Stream_SessionId_t id () const { const StateType& state_r = state (); return (state_r.sessionData ? state_r.sessionData->sessionId : -1); }
  virtual bool isRunning () const;
  virtual void finished (bool = true); // recurse upstream (if any) ?
  virtual unsigned int flush (bool = true,   // flush inbound data ?
                              bool = false,  // flush session messages ?
                              bool = false); // flush upstream (if any) ?
  virtual void idle (bool = true) const; // recurse upstream (if any) ?
  virtual void wait (bool = true,         // wait for any worker thread(s) ?
                     bool = false,        // wait for upstream (if any) ?
                     bool = false) const; // wait for downstream (if any) ?
  virtual void pause ();
  virtual void rewind ();
  //virtual void idle (bool = false) const; // wait for upstream (if any) ?
  virtual void control (ControlType,   // control type
                        bool = false); // recurse upstream (if any) ?
  // *NOTE*: the default implementation forwards calls to the head module
  virtual void notify (NotificationType, // notification type
                       bool = false,     // recurse upstream (if any) ?
                       bool = false);    // expedite ?
  virtual StatusType status () const;
  inline virtual const StateType& state () const { return state_; }

  // implement Stream_IStreamLayout_T
  // *WARNING*: handle with care
  virtual bool lock (bool = true,  // block ?
                     bool = true); // recurse upstream (if any) ?
  virtual int unlock (bool = false, // unblock ?
                      bool = true); // recurse upstream (if any) ?
  virtual ACE_SYNCH_MUTEX_T& getLock (bool = true); // recurse upstream (if any) ?
  virtual bool hasLock (bool = true); // recurse upstream (if any) ?

  inline virtual const typename ISTREAM_T::STREAM_T& getR () const { return *this; }; // return value: type
  inline virtual void setP (typename ISTREAM_T::STREAM_T* upstream_in) { ACE_ASSERT (!inherited::linked_us_); inherited::linked_us_ = upstream_in; }
  // *WARNING*: this API is not thread-safe
  //            --> grab the lock() first and/or really know what you are doing
  virtual const typename ISTREAM_T::MODULE_T* find (const std::string&,  // module name
                                                    bool = false,        // sanitize module names ?
                                                    bool = false) const; // recurse upstream (if any) ?
  inline virtual std::string name () const { return name_; }
  inline virtual void name (const std::string& name_in) { name_ = name_in; }
  virtual bool link (typename ISTREAM_T::STREAM_T*); // upstream
  // *NOTE*: unlinks inherited::linked_us_, i.e. any link()ed up(!)stream
  virtual void _unlink ();
  // *WARNING*: these APIs are not thread-safe
  //            --> grab the lock() first and/or really know what you are doing
  virtual typename ISTREAM_T::STREAM_T* downstream () const;
  virtual typename ISTREAM_T::STREAM_T* upstream (bool = false) const; // recurse (if any) ?

  //virtual bool load (Stream_ILayout*, bool&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }

  // implement Stream_ILinkCB
  inline virtual void onLink () {}
  inline virtual void onUnlink () {}

  // implement Stream_IEvent_T
  virtual void onEvent (NotificationType);

  // implement Stream_ISessionCB
  inline virtual void onSessionBegin (Stream_SessionId_t) {}
  inline virtual void onSessionEnd (Stream_SessionId_t) {}

  // implement Common_IDumpState
  virtual void dump_state () const;

  // implement Common_IGet/Set_T
  inline virtual const SessionDataContainerType& getR_2 () const { ACE_ASSERT (sessionData_); return *sessionData_; }
  // *IMPORTANT NOTE*: this is a 'fire-and-forget' API
  virtual void setPR (SessionDataContainerType*&);
  inline virtual const Stream_MessageQueue_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             SessionMessageType>& getR_3 () const { return messageQueue_; }

  // implement Common_IInitialize_T
  virtual bool initialize (const CONFIGURATION_T&);

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to the statistic report module (if any)
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void update (const ACE_Time_Value&);
  virtual void report () const;

  // override ACE_Stream method(s)
  // *NOTE*: behaves like inherited::close(), but does not delete the head/tail
  //         modules unless the argument is M_DELETE
  virtual int close (int flags = STREAM_T::M_DELETE);
  // *NOTE*: returns: the last module (if any), inherited::tail() otherwise
  virtual ACE_Module<ACE_SYNCH_USE, TimePolicyType>* tail ();
  inline virtual int get (ACE_Message_Block*& messageBlock_inout, ACE_Time_Value* timeout_in) { return (inherited::linked_us_ ? inherited::linked_us_->get (messageBlock_inout, timeout_in) : inherited::get (messageBlock_inout, timeout_in)); }
  // *NOTE*: need to update the layout as well...
  virtual int replace (const ACE_TCHAR*,           // module name
                       MODULE_T*,                  // replacement
                       int = MODULE_T::M_DELETE);  // delete the replaced module ?

  // *NOTE*: the ACE implementation close(s) the removed module. This is not the
  //         intended behavior when the module is being used by several streams
  //         at once
  bool remove (MODULE_T*,    // module handle
               bool = true,  // lock ?
               bool = true); // close()/reset() removed module(s) for re-use ?

  inline bool isInitialized () const { return isInitialized_; }

 protected:
  // convenient types
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType> QUEUE_T;
  typedef Stream_HeadReaderTask_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  NotificationType> HEAD_READER_T;
  typedef Stream_HeadWriterTask_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  NotificationType> HEAD_WRITER_T;
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> TAIL_READER_T;
  typedef Common_IGetR_T<SessionDataContainerType> ISESSION_DATA_T;
  typedef Stream_IModuleHandler_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  HandlerConfigurationType> IMODULE_HANDLER_T;
  typedef Common_IStatistic_T<StatisticContainerType> ISTATISTIC_T;
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      HandlerConfigurationType,
                                      ControlType,
                                      NotificationType,
                                      StateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> HEAD_TASK_T;

  Stream_Base_T ();

  // *WARNING*: calling this while isRunning() == true blocks until the stream
  //            finishes (because close() of a module waits for its worker
  //            thread(s))
  bool reset ();
  // *NOTE*: derived classes must call this in their dtor
  void shutdown ();
  bool setup (ACE_Notification_Strategy* = NULL); // head module reader task' queue notification handle

  // *NOTE*: make sure the original API is not hidden
  using inherited::remove;

  CONFIGURATION_T*          configuration_;
  // *NOTE*: derived classes set this iff (!) their initialization succeeded;
  //         otherwise the dtor will NOT join any worker threads before
  //         close()ing the modules
  bool                      isInitialized_;
  LAYOUT_T                  layout_;
  MESSAGE_QUEUE_T           messageQueue_; // ('outbound'-) queue
  std::string               name_;
  SessionDataContainerType* sessionData_;
  ACE_SYNCH_MUTEX_T         sessionDataLock_;
  StateType                 state_;

 private:
  // convenient types
  typedef Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        StreamName,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> OWN_TYPE_T;
  typedef Stream_ITask_T<ControlMessageType,
                         DataMessageType,
                         SessionMessageType> ITASK_T;
  typedef Common_IStateMachine_2<enum Stream_StateMachine_ControlState> ISTATE_MACHINE_T;
  typedef Stream_StateMachine_Control_T<ACE_SYNCH_USE> STATE_MACHINE_CONTROL_T;
  typedef Common_IGetP_T<ISTREAM_T> IGET_T;
  typedef Common_ISetP_T<StateType> ISET_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Base_T (const Stream_Base_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Base_T& operator= (const Stream_Base_T&))

  // implement (part of) Common_IControl
  virtual void initialize (bool = true,  // set up pipeline ?
                           bool = true); // reset session data ?

  // override ACE_Stream method(s)
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
  // *WARNING*: this method is NOT (!) thread-safe in places
  //            --> handle with care
  // *NOTE*: that for linked streams, the session data passed downstream is
  //         always the most 'upstream' instances'. Inconsistencies arise when
  //         modules cache and fail to update session data passed at session
  //         start
  virtual int link (STREAM_T&);
  virtual int unlink (void);

  // helper methods
  bool finalize ();
  // calling ACE_Stream::close() deletes the head and tail modules
  // invoke this to re-create them (invokes ACE_Stream::open(NULL, head, tail))
  bool initializeHeadTail ();

  void deactivateModules ();
  void unlinkModules ();

  bool                      delete_; // delete modules ?
};

// include template definition
#include "stream_base.inl"

#endif
