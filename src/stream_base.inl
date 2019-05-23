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

#include <deque>

#include "ace/Log_Msg.h"

#include "stream_data_base.h"
#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_imessagequeue.h"
#include "stream_macros.h"
#include "stream_tools.h"

#include "stream_stat_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::Stream_Base_T ()
 : inherited (NULL, // default argument to module open()
              NULL, // --> allocate head module
              NULL) // --> allocate tail module
 , configuration_ (NULL)
 , finishOnDisconnect_ (false)
 , isInitialized_ (false)
 , layout_ ()
 , lock_ ()
 , messageQueue_ (STREAM_QUEUE_MAX_SLOTS,
                  NULL)
 , sessionData_ (NULL)
 , sessionDataLock_ ()
 , state_ ()
 , upstream_ (NULL)
 /////////////////////////////////////////
 , delete_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::Stream_Base_T"));

  // *NOTE*: pass a handle to the message queue member to the head module reader
  //         task; this will become the outbound queue

  int result = -1;
  HEAD_T* writer_p = NULL, *reader_p = NULL;
  TAIL_T* writer_2 = NULL, *reader_2 = NULL;
  MODULE_T* module_p = NULL, *module_2 = NULL;
  ACE_NEW_NORETURN (writer_p,
                    HEAD_T (NULL));
  ACE_NEW_NORETURN (reader_p,
                    HEAD_T (&messageQueue_));
  if (unlikely (!writer_p || !reader_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    delete writer_p; writer_p = NULL;
    delete reader_p; reader_p = NULL;
    return;
  } // end IF
  ACE_NEW_NORETURN (module_p,
                    MODULE_T (ACE_TEXT (STREAM_MODULE_HEAD_NAME),
                              writer_p, reader_p,
                              NULL,
                              ACE_Module_Base::M_DELETE_NONE));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    delete writer_p; writer_p = NULL;
    delete reader_p; reader_p = NULL;
    return;
  } // end IF

  ACE_NEW_NORETURN (writer_2,
                    TAIL_T (NULL));
  ACE_NEW_NORETURN (reader_2,
                    TAIL_T (NULL));
  if (unlikely (!writer_2 || !reader_2))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    delete module_p; module_p = NULL;
    return;
  } // end IF
  ACE_NEW_NORETURN (module_2,
                    MODULE_T (ACE_TEXT (STREAM_MODULE_TAIL_NAME),
                              writer_2, reader_2,
                              NULL,
                              ACE_Module_Base::M_DELETE_NONE));
  if (unlikely (!module_2))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    delete writer_2; writer_2 = NULL;
    delete reader_2; reader_2 = NULL;
    return;
  } // end IF

  result = inherited::close ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::close(): \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    delete module_p; module_p = NULL;
    delete module_2; module_2 = NULL;
    return;
  } // end IF
  result = inherited::open (NULL,      // argument passed to module open()
                            module_p,  // head module handle
                            module_2); // tail module handle
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::open(): \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    delete module_p; module_p = NULL;
    delete module_2; module_2 = NULL;
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::~Stream_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::~Stream_Base_T"));

  //{ ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
  //  if (unlikely (state_.module && !state_.moduleIsClone))
  //    if (unlikely (!remove (state_.module,
  //                           false,   // lock ?
  //                           false))) // reset ? (see above)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
  //                  ACE_TEXT (StreamName),
  //                  state_.module->name ()));
  //} // end lock scope

  if (unlikely (sessionData_))
    sessionData_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::reset"));

  bool result = false;

  // handle final module (if any)
  // *NOTE*: this module may be shared by multiple stream instances, so it
  //         must not be close()d or reset here
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
    if (state_.module)
    {
      MODULE_T* module_p =
          layout_.find (ACE_TEXT_ALWAYS_CHAR (state_.module->name ()));
      if (likely (module_p))
      {
        if (unlikely (!remove (module_p,
                               false,    // lock ?
                               false)))  // reset ? (see above)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                      ACE_TEXT (StreamName),
                      state_.module->name ()));
      } // end IF
    } // end IF
  } // end lock scope

  // - pop/close all modules
  // *NOTE*: will implicitly (blocking !) wait for any active worker threads
  // - reset reader/writers tasks of all modules
  result = finalize ();
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::finalize(), aborting\n"),
                ACE_TEXT (StreamName)));

  // - re-initialize head/tail modules
  initialize (true,  // set up pipeline ?
              true); // reset session data ?

  return result;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::setup (ACE_Notification_Strategy* notificationStrategy_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::setup"));

  // step1: setup layout
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
    if (!layout_.setup (*this))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Layout::setup(): \"%m\", aborting\n"),
                  ACE_TEXT (StreamName)));
      return false;
    } // end IF
  } // end lock scope
#if defined (_DEBUG)
//  layout_.dump_state ();
//  std::cerr << ACE_TEXT_ALWAYS_CHAR ("----------------------\n");
  dump_state ();
#endif // _DEBUG

  // step2: set notification strategy ?
  if (notificationStrategy_in)
  {
    MODULE_T* module_p = inherited::head ();
    if (unlikely (!module_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module found, aborting\n"),
                  ACE_TEXT (StreamName)));
      return false;
    } // end IF
    TASK_T* task_p = module_p->reader ();
    if (unlikely (!task_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module reader task found, aborting\n"),
                  ACE_TEXT (StreamName)));
      return false;
    } // end IF
    task_p->msg_queue_->notification_strategy (notificationStrategy_in);
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (bool setupPipeline_in,
                                               bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  // step1: allocate session data ?
  if (resetSessionData_in)
  {
    // sanity check(s)
    if (unlikely (sessionData_))
    {
      sessionData_->decrease (); sessionData_ = NULL;
    } // end IF

    SessionDataType* session_data_p = NULL;
    ACE_NEW_NORETURN (session_data_p,
                      SessionDataType ());
    if (unlikely (!session_data_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                  ACE_TEXT (StreamName)));
      return;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: allocated %u byte(s) of session data: %@ (lock: %@)\n"),
    //            ACE_TEXT (StreamName),
    //            sizeof (SessionDataType),
    //            session_data_p,
    //            &sessionDataLock_));
    // *TODO*: remove type inferences
    session_data_p->lock = &sessionDataLock_;

    { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
      state_.sessionData = session_data_p;
    } // end lock scope

    // *IMPORTANT NOTE*: fire-and-forget API (session_data_p)
    ACE_NEW_NORETURN (sessionData_,
                      SessionDataContainerType (session_data_p));
    if (unlikely (!sessionData_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                  ACE_TEXT (StreamName)));
      { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
        state_.sessionData = NULL;
      } // end lock scope
      delete session_data_p; session_data_p = NULL;
      return;
    } // end IF
  } // end IF

  // step2: load/initialize modules
  IMODULE_T* imodule_p = NULL;
  TASK_T* task_p = NULL;
  IMODULE_HANDLER_T* imodule_handler_p = NULL;
  typename CONFIGURATION_T::ITERATOR_T iterator_2;
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    if (unlikely (!load (&layout_,
                         delete_)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_IStreamLayout_T::load(), returning\n"),
                  ACE_TEXT (StreamName)));
      return;
    } // end IF

    // *NOTE*: iff this is set, the module has already been clone()d as
    //         appropriate (see: initialize():3066)
    if (state_.module)
    {
      if (!layout_.append (state_.module,
                           configuration_->configuration_.moduleBranch))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Layout_T::append(\"%s\",\"%s\"), returning\n"),
                    ACE_TEXT (StreamName),
                    state_.module->name (),
                    ACE_TEXT (configuration_->configuration_.moduleBranch.c_str ())));
        return;
      } // end IF
//#if defined (_DEBUG)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: appended \"%s\" to \"%s\" branch\n"),
//                  ACE_TEXT (StreamName),
//                  configuration_->configuration_.module->name (),
//                  (configuration_->configuration_.moduleBranch.empty () ? ACE_TEXT ("main") : ACE_TEXT (configuration_->configuration_.moduleBranch.c_str ()))));
//#endif // _DEBUG
    } // end IF
    for (LAYOUT_ITERATOR_T iterator = layout_.begin ();
         iterator != layout_.end ();
         ++iterator)
    {
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (unlikely (!imodule_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, returning\n"),
                    ACE_TEXT (StreamName), (*iterator)->name ()));
        return;
      } // end IF
      imodule_p->reset ();

      // *TODO*: remove type inference
      iterator_2 = configuration_->find ((*iterator)->name ());
      if (iterator_2 == configuration_->end ())
        iterator_2 = configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s: applying dedicated configuration\n"),
                    ACE_TEXT (StreamName), (*iterator)->name ()));
#endif // _DEBUG
      ACE_ASSERT (iterator_2 != configuration_->end ());
      if (unlikely (!imodule_p->initialize ((*iterator_2).second.first)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Common_IInitialize_T::initialize(), returning\n"),
                    ACE_TEXT (StreamName), (*iterator)->name ()));
        return;
      } // end IF
//#if defined (_DEBUG)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s/%s: initialized\n"),
//                  ACE_TEXT (StreamName), (*iterator)->name ()));
//#endif // _DEBUG

      task_p = (*iterator)->writer ();
      ACE_ASSERT (task_p);
      imodule_handler_p = dynamic_cast<IMODULE_HANDLER_T*> (task_p);
      if (!imodule_handler_p)
      { // *TODO*: determine the 'active' side of the module by some
        //         member/function
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("%s: dynamic_cast<Stream_IModuleHandler_T> failed, continuing\n"),
        //            (*iterator)->name ()));
        task_p = (*iterator)->reader ();
        ACE_ASSERT (task_p);
        imodule_handler_p = dynamic_cast<IMODULE_HANDLER_T*> (task_p);
        if (unlikely (!imodule_handler_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModuleHandler_T> failed, continuing\n"),
                      ACE_TEXT (StreamName), (*iterator)->name ()));
          continue;
        } // end IF
      } // end IF
      if (unlikely (!imodule_handler_p->initialize ((*iterator_2).second.second,
                                                    configuration_->configuration_.messageAllocator)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Stream_IModuleHandler_T::initialize(), continuing\n"),
                    ACE_TEXT (StreamName), (*iterator)->name ()));
    } // end FOR
  } // end lock scope

  // step3: setup pipeline ?
  if (setupPipeline_in)
    if (unlikely (!setup (NULL)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::setup(), returning\n"),
                  ACE_TEXT (StreamName)));
      return;
    } // end IF

  isInitialized_ = true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::finalize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finalize"));

  int result = -1;

  // delegate this to base class close(ACE_Module_Base::M_DELETE_NONE)
  try {
    // *NOTE*: unwinds the stream, pop()ing all push()ed modules
    //         --> pop()ing a module will close() it
    //         --> close()ing a module will module_closed() and flush() its
    //             tasks
    //         --> flush()ing a task will close() its queue
    //         --> close()ing a queue will deactivate() and flush() it
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Stream::close(M_DELETE_NONE), continuing\n"),
                ACE_TEXT (StreamName)));
    result = -1;
  }
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::close(M_DELETE_NONE): \"%m\", aborting\n"),
                ACE_TEXT (StreamName)));

  IMODULE_T* imodule_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
    // *NOTE*: ACE_Stream::close() resets the task handles of all modules
    //         --> reset them manually
    for (LAYOUT_ITERATOR_T iterator = layout_.begin ();
         iterator != layout_.end ();
         iterator++)
    {
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (unlikely (!imodule_p))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, continuing"),
                    ACE_TEXT (StreamName),
                    (*iterator)->name ()));
        continue;
      } // end IF
      try {
        imodule_p->reset ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_IModule_T::reset(), continuing\n"),
                    ACE_TEXT (StreamName),
                    (*iterator)->name ()));
      }
    } // end FOR

    if (delete_)
    {
      for (LAYOUT_ITERATOR_T iterator = layout_.begin ();
           iterator != layout_.end ();
           ++iterator)
        delete *iterator;
    } // end IF
    layout_.clear ();
  } // end lock scope

  return (result == 0);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::start"));

  int result = -1;

  // sanity check(s)
  if (unlikely (!isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: not initialized, returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF
  if (unlikely (isRunning ()))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: already running, returning\n"),
                ACE_TEXT (StreamName)));
    return; // nothing to do
  } // end IF
  ACE_ASSERT (configuration_);
  ACE_ASSERT (sessionData_);

  // initialize session data
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (sessionData_->getR ());
  // *TODO*: remove type inferences
  ACE_ASSERT (session_data_r.lock);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
    session_data_r.sessionId =
      (configuration_->configuration_.sessionId ? configuration_->configuration_.sessionId
                                                : ++inherited2::currentSessionId);
    session_data_r.startOfSession = COMMON_TIME_NOW;
    session_data_r.state = &state_;
  } // end lock scope

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found: \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF
  ISTREAM_CONTROL_T* istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->start ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::start(), returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::stop (bool wait_in,
                                         bool recurseupstream_in,
                                         bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::stop"));

  int result = -1;
  MODULE_T* module_p = NULL;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // stop upstream ?
  if (upstream_ &&
      recurseupstream_in)
  {
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (upstream_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      return;
    } // end IF
    try {
      istreamcontrol_p->stop (wait_in,
                               lockedAccess_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl::stop(), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      return;
    }
  } // end IF

  if (unlikely (!isRunning ()))
    goto wait;

  // delegate to the head module, skip over ACE_Stream_Head
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found: \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF
  // *WARNING*: cannot flush(), as this deactivates() the queue as well, which
  //            causes mayhem for any (blocked) worker(s)
  // *TODO*: consider optimizing this
  //module->writer ()->flush ();
  istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->stop (wait_in,
                             recurseupstream_in,
                             lockedAccess_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::stop(), returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  }

wait:
  if (wait_in)
    wait (true,   // wait for any worker thread(s) ?
          false,  // wait for upstream (if any) ?
          false); // wait for downstream (if any) ?
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isRunning"));

  int result = -1;

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = const_cast<OWN_TYPE_T*> (this)->top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", aborting\n")));
    return false;
  } // end IF

  ISTREAM_CONTROL_T* istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    // *NOTE*: perhaps not all modules have been enqueued yet ?
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl_T> failed, aborting\n"),
//                module_p->name ()));
    return false;
  } // end IF

  try {
    return istreamcontrol_p->isRunning ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::isRunning(), aborting\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
  }

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::control (ControlType control_in,
                                            bool recurseupstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::control"));

  int result = -1;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // forward upstream ?
  if (upstream_ &&
      recurseupstream_in)
  {
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (upstream_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upstream_));
      return;
    } // end IF
    try {
      istreamcontrol_p->control (control_in,
                                 recurseupstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::control(%d), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  control_in));
      return;
    }

    return;
  } // end IF

  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF
  ACE_ASSERT (module_p);

  switch (control_in)
  {
    case STREAM_CONTROL_END:
    case STREAM_CONTROL_ABORT:
    case STREAM_CONTROL_CONNECT:
    case STREAM_CONTROL_DISCONNECT:
    case STREAM_CONTROL_LINK:
    case STREAM_CONTROL_RESIZE:
    case STREAM_CONTROL_UNLINK:
    case STREAM_CONTROL_FLUSH:
    case STREAM_CONTROL_RESET:
    case STREAM_CONTROL_STEP:
    case STREAM_CONTROL_STEP_2:
    {
      istreamcontrol_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
      if (unlikely (!istreamcontrol_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                    ACE_TEXT (StreamName),
                    module_p->name ()));
        return;
      } // end IF
      try {
        istreamcontrol_p->control (control_in,
                                   false); // forward upstream ?
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::control(%d), returning\n"),
                    ACE_TEXT (StreamName),
                    module_p->name (),
                    control_in));
        return;
      }

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown control (was: %d), returning\n"),
                  ACE_TEXT (StreamName),
                  control_in));
      return;
    }
  } // end IF
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::notify (NotificationType notification_in,
                                           bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::notify"));

  int result = -1;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // forward upstream ?
  if (upstream_ &&
      recurseUpstream_in)
  {
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (upstream_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upstream_));
      return;
    } // end IF
    try {
      istreamcontrol_p->notify (notification_in,
                                recurseUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::notify(%d), continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  notification_in));
    }

    return;
  } // end IF

  // send control ?
  // *TODO*: this is strategy and needs to be traited ASAP
//  bool send_control = false;
  ControlType control_type = STREAM_CONTROL_INVALID;
  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      control_type = STREAM_CONTROL_FLUSH;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      control_type = STREAM_CONTROL_DISCONNECT;
      break;
    }
    default:
      break;
  } // end SWITCH
  ACE_UNUSED_ARG (control_type);

  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to ACE_Stream::top(), continuing\n"),
                ACE_TEXT (StreamName)));
    goto continue_;
  } // end IF
  ACE_ASSERT (module_p);
  istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->notify (notification_in,
                              false);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::notify(%d), returning\n"),
                ACE_TEXT (StreamName),
                module_p->name (),
                notification_in));
    return;
  }

continue_:
  // finished ?
  // *TODO*: this is strategy and needs to be traited ASAP
  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      if (finishOnDisconnect_)
        finished (false); // recurse upstream ?
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
unsigned int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::flush (bool flushInbound_in,
                                          bool flushSessionMessages_in,
                                          bool flushUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::flush"));

  unsigned int result = 0;

  // forward upstream ?
  if (unlikely (upstream_ &&
                flushUpstream_in))
  {
    ISTREAM_CONTROL_T* istreamcontrol_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (upstream_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upstream_));
      return 0;
    } // end IF
    try {
      return istreamcontrol_p->flush (flushInbound_in,
                                       flushSessionMessages_in,
                                       flushUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::flush(), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      return 0;
    }
  } // end IF

  Stream_ModuleList_t modules_a = layout_.list (false);
  TASK_T* task_p = NULL;
  Stream_IMessageQueue* iqueue_p = NULL;
  int result_2 = -1;

  // *TODO*: implement a dedicated control message to push this functionality
  //         into the task object
  if (unlikely (!flushInbound_in))
    goto continue_;

  // writer (inbound) side
  for (Stream_ModuleListIterator_t iterator = modules_a.begin ();
       iterator != modules_a.end ();
       ++iterator)
  {
    task_p = const_cast<MODULE_T*> (*iterator)->writer ();
    if (unlikely (!task_p))
      continue; // close()d already ?

    ACE_ASSERT (task_p->msg_queue_);
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
    if (unlikely (!iqueue_p))
    {
      // *NOTE*: most probable cause: module is (upstream) head
      // *TODO*: all messages are flushed here, this must not happen
      result_2 = task_p->msg_queue_->flush ();
    } // end IF
    else
      result_2 = static_cast<int> (iqueue_p->flush (flushSessionMessages_in));
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s writer: failed to Stream_IMessageQueue::flushData(): \"%m\", continuing\n"),
                  ACE_TEXT (StreamName), (*iterator)->name ()));
    } // end IF
    else if (likely (result_2))
    {
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s writer: flushed %d message(s)\n"),
                  ACE_TEXT (StreamName), (*iterator)->name (),
                  result_2));
#endif // _DEBUG
      result += result_2;
    } // end ELSE IF
  } // end FOR

continue_:
  // reader (outbound) side
  modules_a.push_front (const_cast<MODULE_T*> (inherited::head ())); // prepend head
  for (Stream_ModuleListReverseIterator_t iterator = modules_a.rbegin ();
       iterator != modules_a.rend ();
       ++iterator)
  {
    task_p = (*iterator)->reader ();
    if (unlikely (!task_p))
      continue; // close()d already ?

    ACE_ASSERT (task_p->msg_queue_);
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
    if (unlikely (!iqueue_p))
    {
      // *NOTE*: most probable cause: stream head, or module does not have a
      //         reader task
      result_2 = task_p->msg_queue_->flush ();
    } // end IF
    else
      result_2 = static_cast<int> (iqueue_p->flush (flushSessionMessages_in));
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s reader: failed to Stream_IMessageQueue::flushData(): \"%m\", continuing\n"),
                  ACE_TEXT (StreamName), (*iterator)->name ()));
    } // end IF
    else if (likely (result))
    {
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s reader: flushed %d message(s)\n"),
                  ACE_TEXT (StreamName), (*iterator)->name (),
                  result_2));
#endif // _DEBUG
      result += result_2;
    } // end ELSE IF
  } // end FOR

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::pause"));

  int result = -1;

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
    return;

  ISTREAM_CONTROL_T* istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  } // end IF

  try {
    istreamcontrol_p->pause ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl::pause(), returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::rewind"));

  int result = -1;

  // sanity check
  // *TODO*
  if (unlikely (isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: currently running, returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
    return;

  ISTREAM_CONTROL_T* istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->rewind ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::rewind(), returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
StatusType
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::status () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::status"));

  StatusType result = static_cast<StatusType> (-1);
  int result_2 = -1;

  // sanity check(s)
//  // *NOTE*: top() uses inherited::stream_head_. If the stream has not been
//  //         open()ed yet, or failed to initialize(), this will crash
//  if (unlikely (!const_cast<OWN_TYPE_T*> (this)->head ()))
//    return result;

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result_2 = const_cast<OWN_TYPE_T*> (this)->top (module_p);
  if (unlikely ((result_2 == -1) ||
                !module_p))
    return result;

  ISTREAM_CONTROL_T* istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, aborting\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return result;
  } // end IF

  try {
    result = istreamcontrol_p->status ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::status(), aborting\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return result;
  }

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::idle () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::idle"));

  // *NOTE*: if this stream has been linked (e.g. connection is part of another
  //         stream), make sure to wait for the whole pipeline
  if (upstream_)
  {
    Stream_IStreamControlBase* istreamcontrol_p =
      dynamic_cast<Stream_IStreamControlBase*> (upstream_);
    if (unlikely (!istreamcontrol_p))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to dynamic_cast<Stream_IStreamControlBase>(0x%@), returning\n"),
                  upstream_));
      return;
    } // end IF
    try {
      istreamcontrol_p->idle ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IStreamControlBase::idle(), continuing\n")));
    }
  } // end IF

  // step1: wait for inbound processing (i.e. the 'writer' side) pipeline to
  //        flush
  TASK_T* task_p = NULL;
  Stream_IMessageQueue* iqueue_p = NULL;
  Stream_ModuleList_t modules_a;
  ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> reverse_lock (lock_);
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    modules_a = layout_.list ();
    for (Stream_ModuleListIterator_t iterator = modules_a.begin ();
         iterator != modules_a.end ();
         ++iterator)
    {
      task_p = (*iterator)->writer ();
      if (unlikely (!task_p))
        continue; // close()d already ?
      if (likely (!task_p->msg_queue_))
        continue; // synchronous task --> nothing to do

      iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
      if (iqueue_p)
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
        iqueue_p->waitForIdleState ();
      } // end IF
      else
      {
        int result = -1;
        ACE_Time_Value one_second (1, 0);
        do
        {
          if (likely (task_p->msg_queue_->is_empty ()))
            break; // nothing to do
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s/%s writer: waiting to process ~ %d byte(s) in %u message(s)...\n"),
                      ACE_TEXT (StreamName),
                      (*iterator)->name (),
                      task_p->msg_queue_->message_bytes (),
                      task_p->msg_queue_->message_count ()));

          { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
            result = ACE_OS::sleep (one_second);
          } // end lock scope
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s/%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                        ACE_TEXT (StreamName),
                        (*iterator)->name (),
                        &one_second));
        } while (true);
      } // end ELSE
    } // end FOR
  } // end lock scope

  // step2: wait for outbound processing (i.e. the 'reader' side) pipeline to
  //        flush
  // *TODO*: this is incomplete
  messageQueue_.waitForIdleState ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::wait (bool waitForThreads_in,
                                         bool waitForupstream_in,
                                         bool waitForDownStream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::wait"));

  int result = -1;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);

  // forward upstream ?
  if (upstream_ &&
      waitForupstream_in)
  {
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (upstream_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upstream_));
      return;
    } // end IF
    try {
      istreamcontrol_p->wait (waitForThreads_in,
                              waitForupstream_in,
                              waitForDownStream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::wait(), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      return;
    }
  } // end IF

  // *NOTE*: the procedure here is this:
  //         step1: wait for (message source) processing to finish
  //         step2: wait for any upstreamed messages to 'flush' (message sink)

  // step1a: get head module (skip over ACE_Stream_Head)
  ITERATOR_T iterator (*this);
  if (unlikely (iterator.done ()))
    return;
  result = iterator.advance ();
  if (unlikely (result == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found, returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF
  const MODULE_T* module_p = NULL;
  result = iterator.next (module_p);
  if (unlikely (!result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found, returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF
  // sanity check: head == tail ? possible reasons:
  // - no modules have been push()ed (yet)
  // - stream hasn't been intialized (at all)
  // --> nothing to do
  if (unlikely (module_p == this_p->inherited::tail ()))
    return;

  // ... and wait for the state switch (xxx --> FINISHED) (/ any head module
  // thread(s))
  // *TODO*: in 'concurrent' scenarios(, or if the head module is a queue),
  //         messages are enqueued by 'third parties' (e.g. the kernel, or
  //         external threads); transitioning to the FINISHED state must
  //         prevent more data from 'leaking' in beyond this point
  istreamcontrol_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (const_cast<MODULE_T*> (module_p)->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (StreamName), module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->wait (false,                 // wait for threads ?
                            waitForupstream_in,
                            waitForDownStream_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl::wait(), returning\n"),
                ACE_TEXT (StreamName), module_p->name ()));
    return;
  }
  // --> no new messages will be dispatched from the head module

  // step1b: wait for inbound processing (i.e. the 'writer' side) to complete
  //         --> grab the lock to freeze the layout
  ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);

  Stream_ModuleList_t modules_a = layout_.list (false);
  Stream_ModuleList_t main_modules_a = layout_.list (true);
  modules_a.push_front (this_p->inherited::head ());
  TASK_T* task_p = NULL;
  ACE_Time_Value one_second (1, 0);
  size_t message_count = 0;
  ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> reverse_lock (lock_);
  for (Stream_ModuleListIterator_t iterator_2 = modules_a.begin ();
       iterator_2 != modules_a.end ();
       ++iterator_2)
  { ACE_ASSERT (*iterator_2);
    // skip stream tail (i.e. last module)
    if (!ACE_OS::strcmp ((*iterator_2)->name (),
                         ACE_TEXT (STREAM_MODULE_TAIL_NAME)) ||
        !ACE_OS::strcmp ((*iterator_2)->name (),
                         ACE_TEXT ("ACE_Stream_Tail")))
      break;

    task_p = const_cast<MODULE_T*> (*iterator_2)->writer ();
    if (unlikely (!task_p))
      continue; // close()d already ?
    ACE_ASSERT (task_p->msg_queue_);
    do
    {
      //result = task_p->msg_queue_->wait ();
      message_count = task_p->msg_queue_->message_count ();
      if (likely (!message_count))
        break;
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s writer: waiting to process %d byte(s) in %u message(s)...\n"),
                  ACE_TEXT (StreamName), (*iterator_2)->name (),
                  task_p->msg_queue_->message_bytes (), message_count));
#endif // _DEBUG

      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
        result = ACE_OS::sleep (one_second);
      } // end lock scope
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    ACE_TEXT (StreamName), (*iterator_2)->name (),
                    &one_second));
    } while (true);

    if (waitForThreads_in)
    {
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
        result = task_p->wait ();
      } // end lock scope
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if (error != ENXIO) // *NOTE*: see also: common_task_base.inl:350
#endif
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s writer: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                      ACE_TEXT (StreamName), (*iterator_2)->name ()));
      } // end IF
    } // end IF

    if (!waitForDownStream_in &&
        (*iterator_2 == main_modules_a.back ()))
      break;
  } // end FOR

  // step2: wait for outbound/upstream processing (i.e. 'reader') stream to
  //        complete (/ any worker(s) to idle)
  for (Stream_ModuleListReverseIterator_t iterator_2 = modules_a.rbegin ();
       iterator_2 != modules_a.rend ();
       ++iterator_2)
  {
    task_p = (*iterator_2)->reader ();
    if (unlikely (!task_p))
      continue; // close()d already ?
    ACE_ASSERT (task_p->msg_queue_);
    do
    {
      //result = task_p->msg_queue_->wait ();
      message_count = task_p->msg_queue_->message_count ();
      if (!message_count)
        break;
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s reader: waiting to process %d byte(s) in %u message(s)...\n"),
                  ACE_TEXT (StreamName), (*iterator_2)->name (),
                  task_p->msg_queue_->message_bytes (), message_count));
#endif // _DEBUG

      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
        result = ACE_OS::sleep (one_second);
      } // end lock scope
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    ACE_TEXT (StreamName), (*iterator_2)->name (),
                    &one_second));
    } while (true);

    if (waitForThreads_in)
    {
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
        result = task_p->wait ();
      } // end lock scope
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s reader: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                    ACE_TEXT (StreamName), (*iterator_2)->name ()));
    } // end IF
  } // end FOR
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
// *NOTE*: fore some reason, this does not compile with gcc
//const typename ISTREAM_T::MODULE_T*
const typename Stream_IStream_T<ACE_SYNCH_USE, TimePolicyType>::MODULE_T*
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::find (const std::string& name_in,
                                         bool sanitizeModuleNames_in,
                                         bool recurseUpstream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::find"));

  if (upstream_ &&
      recurseUpstream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
    if (istream_p)
      return istream_p->find (name_in,
                              sanitizeModuleNames_in,
                              recurseUpstream_in);
  } // end IF

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, NULL);
    return layout_.find (name_in,
                         sanitizeModuleNames_in);
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::link (typename ISTREAM_T::STREAM_T* upstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // sanity check(s)
  ACE_ASSERT (upstream_in);

  ISTREAM_T* istream_p = NULL;
  if (upstream_)
  {
    istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
    if (unlikely (!istream_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStream_T>(0x%@) upstream, aborting\n"),
                  ACE_TEXT (StreamName),
                  upstream_));
      return false;
    } // end IF
    try {
      return istream_p->link (upstream_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStream_T::link(), aborting\n"),
                  ACE_TEXT (istream_p->name ().c_str ())));
      return false;
    }
  } // end IF

  int result = link (*upstream_in);
  if (unlikely (result == -1))
  {
    istream_p = dynamic_cast<ISTREAM_T*> (upstream_in);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::link(%s): \"%m\", aborting\n"),
                ACE_TEXT (StreamName),
                (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
  } // end IF

  return (result == 0);
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::_unlink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::_unlink"));

  // sanity check(s)
  if (!upstream_)
  { // *TODO*: consider all scenarios where this might happen
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: no upstream; cannot unlink, returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF

  int result = unlink ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::unlink(), returning\n"),
                ACE_TEXT (StreamName)));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
typename Stream_IStream_T<ACE_SYNCH_USE, TimePolicyType>::STREAM_T*
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::downstream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::downstream"));

  int result = -1;
  typename ISTREAM_T::MODULE_T* module_p = NULL;
  STATE_MACHINE_CONTROL_T* state_machine_control_p = NULL;
  bool is_first = true;
  IGET_T* iget_p = NULL;
  ISTREAM_T* istream_p = NULL;

  result = const_cast<OWN_TYPE_T*> (this)->inherited::top (module_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                ACE_TEXT (StreamName)));
    return NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (module_p);

  // step1: locate the second (!) downstream head module
  do
  {
    state_machine_control_p = NULL;
    if (!module_p->next ())
      break; // --> reached tail end
    state_machine_control_p =
      dynamic_cast<STATE_MACHINE_CONTROL_T*> (module_p->writer ());
    if (state_machine_control_p)
    {
      if (!is_first)
        break;
      is_first = false;
    } // end IF
    module_p = module_p->next ();
  } while (true);
  if (!state_machine_control_p)
    return NULL; // 'this' is the most downstream (sub-)stream

  // sanity check(s)
  ACE_ASSERT (module_p);
  ACE_ASSERT (module_p->writer ());

  iget_p = dynamic_cast<IGET_T*> (module_p->writer ());
  if (unlikely (!iget_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IGetP_T<Stream_IStream_T>>(0x%@), aborting\n"),
                ACE_TEXT (StreamName),
                module_p->name (),
                ACE_TEXT (module_p->writer ())));
    return NULL;
  } // end IF
  istream_p = const_cast<ISTREAM_T*> (iget_p->getP ());
  ACE_ASSERT (istream_p);

  return dynamic_cast<STREAM_T*> (istream_p);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
typename Stream_IStream_T<ACE_SYNCH_USE, TimePolicyType>::STREAM_T*
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::upstream (bool recurse_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::upstream"));

  // sanity check(s)
  if (!recurse_in)
    return upstream_;

  // break recursion
  if (!upstream_)
    return const_cast<OWN_TYPE_T*> (this);

  ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
  if (!istream_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStream_T>(0x%@), aborting\n"),
                ACE_TEXT (StreamName),
                upstream_));
    return const_cast<OWN_TYPE_T*> (this); // *TODO*: potential false positive
  } // end IF

  return istream_p->upstream (true);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::lock (bool block_in,
                                         bool recurseupstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::lock"));

  int result = -1;

  if (upstream_ &&
      recurseupstream_in)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (upstream_);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upstream_));
      return false;
    } // end IF
    try {
      return ilock_p->lock (block_in,
                            recurseupstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
    }
    return false;
  } // end IF

  // *IMPORTANT NOTE*: currently,
  //                   ACE_Recursive_Thread_Mutex::get_nesting_level() is not
  //                   supported on some UNIX platforms (returns -1, see:
  //                   Recursive_Thread_Mutex.cpp:96)
  //                   --> use result of the locking operation instead
//  int previous_nesting_level = lock_.get_nesting_level ();
  //ACE_recursive_thread_mutex_t& mutex_r = lock_.lock ();

  result = (block_in ? lock_.acquire () : lock_.tryacquire ());
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (!block_in &&
        (error == EBUSY))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[%T][%t]: lock %sheld by %d --> false\n"),
//                  (block_in ? ACE_TEXT ("(block) ") : ACE_TEXT ("")),
//                  mutex_r.OwningThread));
#endif
    } // end IF
    return false;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("[%T][%t]: lock %s%d --> unlock ? %s\n"),
  //            (block_in ? ACE_TEXT ("(block) ") : ACE_TEXT ("")),
  //            lock_.get_nesting_level (),
  //            ((lock_.get_nesting_level () != nesting_level) ? ACE_TEXT ("true") : ACE_TEXT ("false"))));

  return true;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlock (bool unlock_in,
                                           bool recurseupstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlock"));

  int result = -1;
  int result_2 = -1;
  int previous_nesting_level = -1;

  if (upstream_ &&
      recurseupstream_in)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (upstream_);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upstream_));
      return -1;
    } // end IF
    try {
      return ilock_p->unlock (unlock_in,
                              recurseupstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
    }
    return -1;
  } // end IF

  // *NOTE*: on Win32 platforms, the ACE implementation does not currently
  //         support ACE_Recursive_Thread_Mutex::get_thread_id(), although the
  //         data type 'struct _RTL_CRITICAL_SECTION' contains the necessary
  //         information ('OwningThread')
  //         --> work around these inconsistencies
  // *TODO*: submit a bug report/patch
  ACE_recursive_thread_mutex_t& mutex_r = lock_.lock ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!ACE_OS::thr_equal (reinterpret_cast<ACE_thread_t> (mutex_r.OwningThread),
                          ACE_OS::thr_self ()))
#elif !defined (ACE_HAS_RECURSIVE_MUTEXES)
  // IMPORTANT NOTE*: currently, ACE_Recursive_Thread_Mutex::get_thread_id() is
  //                  not supported on non-Win32 platforms (returns
  //                  ACE_OS::NULL_thread), see: Recursive_Thread_Mutex.cpp:70)
  ACE_thread_t thread_id = lock_.get_thread_id ();
  if (!ACE_OS::thr_equal (thread_id, ACE_OS::NULL_thread) &&
      !ACE_OS::thr_equal (thread_id, ACE_OS::thr_self ()))
#else
  goto continue_;
#endif
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("[%T][%t]: unlock held by %d --> -1\n"),
    //            mutex_r.OwningThread));
#endif
    return -1;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
continue_:
#endif
  // *NOTE*: currently, ACE_Recursive_Thread_Mutex::get_nesting_level() is not
  //         supported on some UNIX (Linux in particular) and Win64 platforms
  //         (returns -1, see: Recursive_Thread_Mutex.cpp:96)
  //         --> work around these inconsistencies
  // *TODO*: submit a bug report/patch
  previous_nesting_level = lock_.get_nesting_level ();
  if (!previous_nesting_level)
    return -1; // nothing to do
  result =
      ((previous_nesting_level == -1) ? -1 // <-- *TODO*: this is broken
                                      : (unlock_in ? 0
                                                   : (previous_nesting_level - 1)));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: use ACE_Recursive_Thread_Mutex::get_nesting_level()
  result = (mutex_r.RecursionCount > 0 ? mutex_r.RecursionCount - 1 : 0);
  do
  {
    result_2 = lock_.release ();
    if (!unlock_in)
      break;
//  } while (lock_.get_nesting_level () > 0);
  } while (mutex_r.RecursionCount > 0);
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                ACE_TEXT (StreamName)));
#else
  // *PORTABILITY*: use ACE_Recursive_Thread_Mutex::get_nesting_level()
  result = (mutex_r.__data.__count > 0 ? mutex_r.__data.__count - 1 : 0);
  do
  {
    result_2 = lock_.release ();
    if (result_2 == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EPERM) // not locked ?
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", returning\n"),
                    ACE_TEXT (StreamName)));
        break;
      } // end IF
    } // end IF
    if (!unlock_in)
      break;
//  } while (lock_.get_nesting_level () > 0);
  } while (mutex_r.__data.__count > 0);
#endif
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("[%T][%t]: unlock %s%d --> %d\n"),
  //            (unlock_in ? ACE_TEXT ("(full) ") : ACE_TEXT ("")),
  //            lock_.get_nesting_level (),
  //            result));

  return result;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
ACE_SYNCH_RECURSIVE_MUTEX&
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::getLock (bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::getLock"));

  if (upstream_ &&
      recurseUpstream_in)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (upstream_);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upstream_));
      ACE_SYNCH_RECURSIVE_MUTEX dummy;
      return dummy;
    } // end IF
    try {
      return ilock_p->getLock (recurseUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::getLock(), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
    }
  } // end IF

  return lock_;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::hasLock (bool recurseupstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::hasLock"));

  int result = -1;

  if (upstream_)
  {
    typename ISTREAM_T::MODULE_T* module_p = NULL;
    TASK_T* task_p = NULL;
    ILOCK_T* ilock_p = NULL;

    result = upstream_->top (module_p);
    if (unlikely ((result == -1) ||
                  !module_p))
    {
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("%s: no head module found: \"%m\", aborting\n"),
      //            ACE_TEXT (upstream_->name ().c_str ())));
      return false; // *WARNING*: false negative
    } // end IF
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);

    ilock_p = dynamic_cast<ILOCK_T*> (task_p);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name (),
                  task_p));
      return false; // *WARNING*: false negative
    } // end IF
    return ilock_p->hasLock (recurseupstream_in);
  } // end IF

  // *IMPORTANT NOTE*: currently,
  //                   ACE_Recursive_Thread_Mutex::get_nesting_level() is not
  //                   supported on some UNIX platforms (returns -1, see:
  //                   Recursive_Thread_Mutex.cpp:96)
  result = lock_.get_nesting_level ();

  // *TODO*: on Windows platforms, the current ACE implementation does not
  //         support ACE_Recursive_Thread_Mutex::get_thread_id(), although the
  //         data type 'struct _RTL_CRITICAL_SECTION' contains the necessary
  //         information ('OwningThread')
  //         --> submit a bug report
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_recursive_thread_mutex_t& mutex_r = lock_.lock ();
  return (ACE_OS::thr_equal (reinterpret_cast<ACE_thread_t> (mutex_r.OwningThread),
                             ACE_OS::thr_self ()) &&
          (lock_.get_nesting_level () > 0));
#else
  return ((result == -1) ? ACE_OS::thr_equal (lock_.get_thread_id (), ACE_OS::thr_self ())
                         : (ACE_OS::thr_equal (lock_.get_thread_id (), ACE_OS::thr_self ()) && (result > 0)));
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::dump_state"));

  if (upstream_)
  {
    Common_IDumpState* idump_state_p =
        dynamic_cast<Common_IDumpState*> (upstream_);
    if (idump_state_p)
    {
      try {
        idump_state_p->dump_state ();
      } catch (...) {
        ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Common_IDumpState(), continuing\n"),
                    (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      }
    } // end IF
    return;
  } // end IF

  std::string stream_layout_string;

  const MODULE_T* module_p = NULL;
  std::vector<Stream_IDistributorModule*> distributors_a;
  Stream_IDistributorModule* idistributor_p = NULL;
  for (ITERATOR_T iterator (*this);
       iterator.next (module_p);
       iterator.advance ())
  {
    if (!ACE_OS::strcmp (module_p->name (),
                         ACE_TEXT (STREAM_MODULE_HEAD_NAME)) ||
        !ACE_OS::strcmp (module_p->name (), ACE_TEXT ("ACE_Stream_Head")))
    {
      stream_layout_string.append (ACE_TEXT ("| "));
      continue;
    } // end IF
    if (!ACE_OS::strcmp (module_p->name (),
                         ACE_TEXT (STREAM_MODULE_TAIL_NAME)) ||
        !ACE_OS::strcmp (module_p->name (), ACE_TEXT ("ACE_Stream_Tail")))
    {
      stream_layout_string.append (ACE_TEXT (" |"));
      continue;
    } // end IF

    stream_layout_string.append (Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR (module_p->name ())));
    ACE_ASSERT (const_cast<MODULE_T*> (module_p)->next ());
    if (ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                        ACE_TEXT (STREAM_MODULE_TAIL_NAME)) &&
        ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                        ACE_TEXT ("ACE_Stream_Tail")))
      stream_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

    idistributor_p =
        dynamic_cast<Stream_IDistributorModule*> (const_cast<MODULE_T*> (module_p)->writer ());
    if (idistributor_p)
      distributors_a.push_back (idistributor_p);

    module_p = NULL;
  } // end FOR

  Stream_ModuleList_t heads_a;
  unsigned int indentation_i = 1;
  for (std::vector<Stream_IDistributorModule*>::const_iterator iterator = distributors_a.begin ();
       iterator != distributors_a.end ();
       ++iterator, ++indentation_i)
  {
    heads_a = (*iterator)->next ();
    stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("\n"));
    stream_layout_string.append (indentation_i, '\t');
    for (Stream_ModuleListIterator_t iterator_2 = heads_a.begin ();
         iterator_2 != heads_a.end ();
         ++iterator_2)
    {
next:
      std::vector<Stream_IDistributorModule*> distributors_2;
      module_p = *iterator_2;
      do {
        ACE_ASSERT (module_p);
        stream_layout_string.append (Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR (module_p->name ())));
        if (ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                            ACE_TEXT (STREAM_MODULE_TAIL_NAME)) &&
            ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                            ACE_TEXT ("ACE_Stream_Tail")))
          stream_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

        if (!ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                             ACE_TEXT (STREAM_MODULE_TAIL_NAME)) ||
            !ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                             ACE_TEXT ("ACE_Stream_Tail")))
        {
          stream_layout_string.append (ACE_TEXT (" |"));
          Stream_ModuleListIterator_t iterator_3 = iterator_2;
          if (++iterator_3 != heads_a.end ())
          {
            stream_layout_string.append (ACE_TEXT ("\n"));
            stream_layout_string.append (indentation_i, '\t');
          } // end IF
          break;
        } // end IF

        idistributor_p =
            dynamic_cast<Stream_IDistributorModule*> ((*iterator_2)->writer ());
        if (idistributor_p)
          distributors_2.push_back (idistributor_p);

        module_p = const_cast<MODULE_T*> (module_p)->next ();
      } while (true);

      Stream_ModuleList_t heads_2;
      for (std::vector<Stream_IDistributorModule*>::const_iterator iterator_3 = distributors_2.begin ();
           iterator_3 != distributors_2.end ();
           ++iterator_3)
      {
        heads_2 = (*iterator_3)->next ();
        stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("\n"));
        stream_layout_string.insert (std::string::npos, indentation_i, '\t');
        for (Stream_ModuleListIterator_t iterator_2 = heads_2.begin ();
             iterator_2 != heads_2.end ();
             ++iterator_2)
          goto next;
      } // end FOR
      --indentation_i;
    } // end FOR
  } // end FOR
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s: \"%s\"\n"),
              ACE_TEXT (StreamName),
              ACE_TEXT (stream_layout_string.c_str ())));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::setPR (SessionDataContainerType*& sessionData_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::setPR"));

  // sanity check(s)
  ACE_ASSERT (sessionData_inout);

  // clean up
  if (sessionData_)
    sessionData_->decrease ();

  sessionData_ = sessionData_inout;
  sessionData_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  if (isInitialized_)
  {
    // handle final module (if any)
    // *NOTE*: this module may be shared by multiple stream instances, so it
    //         must not be close()d or reset here
    { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
      if (state_.module && !state_.moduleIsClone)
        if (unlikely (!remove (state_.module,
                               false,   // lock ?
                               false))) // reset ? (see above)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                      ACE_TEXT (StreamName),
                      state_.module->name ()));
      state_.module = NULL;
    } // end lock scope

    if (unlikely (!finalize ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::finalize(), aborting\n"),
                  ACE_TEXT (StreamName)));
      return false;
    }// end IF

    if (configuration_->configuration_.resetSessionData &&
        sessionData_)
    {
      sessionData_->decrease (); sessionData_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<CONFIGURATION_T&> (configuration_in);
  ACE_ASSERT (configuration_->isInitialized_);

  // *TODO*: remove type inferences
  if (configuration_->configuration_.module)
  {
    { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
      ACE_ASSERT (!state_.module);
      if (configuration_->configuration_.cloneModule)
      {
        IMODULE_T* imodule_p =
            dynamic_cast<IMODULE_T*> (configuration_->configuration_.module);
        if (unlikely (!imodule_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                      ACE_TEXT (StreamName), configuration_->configuration_.module->name ()));
          return false;
        } // end IF

        try {
          state_.module = imodule_p->clone ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: caught exception in Stream_IModule_T::clone(), aborting\n"),
                      ACE_TEXT (StreamName), configuration_->configuration_.module->name ()));
        }
        if (unlikely (!state_.module))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to Stream_IModule_T::clone(), aborting\n"),
                      ACE_TEXT (StreamName), configuration_->configuration_.module->name ()));
          return false;
        } // end IF
        state_.moduleIsClone = true;
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("%s/%s: cloned final module (handle: 0x%@), clone handle is: 0x%@)\n"),
        //            ACE_TEXT (StreamName),
        //            configuration_->configuration_.module->name (),
        //            configuration_->configuration_.module,
        //            state_.module));
      } // end IF
      else
      {
        state_.module = configuration_->configuration_.module;
        state_.moduleIsClone = false;
      } // end ELSE
      ACE_ASSERT (state_.module);
    } // end lock scope
  } // end IF

  // sanity check(s)
  // *TODO*: remove type inferences
  for (typename CONFIGURATION_T::ITERATOR_T iterator = configuration_->begin ();
       iterator != configuration_->end ();
       iterator++)
  {
    (*iterator).second.first.notify = this;
    (*iterator).second.first.stream = this;
    //(*iterator).second.second->stateMachineLock = state_.stateMachineLock;
  } // end FOR

  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
    state_.userData = configuration_->configuration_.userData;
  } // end lock scope

  // *TODO*: initialize the module handler configuration here as well
  initialize (configuration_->configuration_.setupPipeline,
              configuration_->configuration_.resetSessionData);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::collect"));

  int result = -1;
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (find (ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING),
                                        true,
                                        false));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (StreamName),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return false;
  } // end IF
  STATISTIC_REPORT_MODULE_WRITER_T* statistic_impl_p =
    dynamic_cast<STATISTIC_REPORT_MODULE_WRITER_T*> (module_p->writer ());
  if (unlikely (!statistic_impl_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T> failed, aborting\n"),
                ACE_TEXT (StreamName)));
    return false;
  } // end IF

  // update session data as well ?
  bool release_lock = false;
  bool result_2 = false;
  if (unlikely (!state_.sessionData))
    goto continue_;

  if (likely (state_.sessionData->lock))
  {
    result = state_.sessionData->lock->acquire ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (StreamName)));
      return false;
    } // end IF
    release_lock = true;
  } // end IF

  state_.sessionData->statistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistic module
continue_:
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (StreamName)));
  }
  if (unlikely (!result_2))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (StreamName)));
  else
    state_.sessionData->statistic = data_out;

  if (likely (release_lock))
  { ACE_ASSERT (state_.sessionData->lock);
    result = state_.sessionData->lock->release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (StreamName)));
  } // end IF

  return result_2;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::update ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::update"));

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (find (ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING),
                                        true,
                                        false));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, returning\n"),
                ACE_TEXT (StreamName),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  STATISTIC_REPORT_MODULE_WRITER_T* statistic_impl_p =
    dynamic_cast<STATISTIC_REPORT_MODULE_WRITER_T*> (module_p->writer ());
  if (unlikely (!statistic_impl_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T> failed, returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF

  // delegate to the statistic module
  try {
    statistic_impl_p->update ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::update(), continuing\n"),
                ACE_TEXT (StreamName)));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::report"));

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (find (ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING),
                                        true,
                                        false));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, returning\n"),
                ACE_TEXT (StreamName),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  STATISTIC_REPORT_MODULE_WRITER_T* statistic_impl_p =
    dynamic_cast<STATISTIC_REPORT_MODULE_WRITER_T*> (module_p->writer ());
  if (unlikely (!statistic_impl_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T> failed, returning\n"),
                ACE_TEXT (StreamName)));
    return;
  } // end IF

  // delegate to the statistic module
  try {
    statistic_impl_p->report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::report(), continuing\n"),
                ACE_TEXT (StreamName)));
  }

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %d] RUNTIME STATISTIC ***\n--> stream statistic @ %#D<--\n (data) messages: %u\n dropped messages: %u\n bytes total: %.0f\n*** RUNTIME STATISTIC ***\\END\n"),
              (state_.sessionData ? static_cast<int> (state_.sessionData->sessionId) : -1),
              &(state_.sessionData->lastCollectionTimeStamp),
              state_.sessionData->statistic.dataMessages,
              state_.sessionData->statistic.droppedFrames,
              state_.sessionData->statistic.bytes));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::link (STREAM_T& upstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // *WARNING*: cannot reach the base class lock from here --> not thread-safe !
  // *TODO*: submit change request to the ACE maintainers

  // sanity check(s)
  if (unlikely (upstream_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: already linked, aborting\n"),
                ACE_TEXT (StreamName)));
    return -1;
  } // end IF

  int result = -1;
  ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (&upstream_in);
  bool reset_upstream = false;
  bool unlink_modules = false;
  StateType* state_p = NULL;
  ISESSION_DATA_T* iget_p = NULL;
  SessionDataContainerType* session_data_container_p = NULL;
  SessionDataType* session_data_p = NULL;
  SessionDataType* session_data_2 = NULL;

  // *IMPORTANT NOTE*: in fully synchronous, or 'concurrent' scenarios, with
  //                   non-reentrant modules, the caller needs to hold the
  //                   stream lock(s) to securely perform this operation.
  //                   These issues need to be considered:
  //                   - data processing may fail when the pipeline layout is
  //                     suddenly changed (see above)
  //                   - downstream threads synchronizing with upstream, for
  //                     whatever reason, will 'desynchronize' (and likely
  //                     deadlock soon after) when the stream lock interface is
  //                     switched (e.g. due to an asynchronous unlink())
  //                   - threads accessing session data must block until it
  //                     has been merged between both streams
  //  // *TODO*: this needs more work

  // *NOTE*: set upstream early (required for link notification of aggregated
  //         module(s)
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, -1);
    // *NOTE*: ACE_Stream::linked_us_ is currently private
    //         --> retain another handle
    // *TODO*: modify ACE to make this a protected member
    upstream_ = &upstream_in;
  } // end lock scope
  reset_upstream = true;

  // locate the module just above the upstreams' tail and this' 'top' module
  // (i.e. the module just below the head)
  MODULE_T* upstream_tail_module_p = upstream_in.tail ();
  MODULE_T* trailing_module_p = upstream_in.head ();
  MODULE_T* heading_module_p = NULL;

  // sanity check(s)
  ACE_ASSERT (upstream_tail_module_p);
  ACE_ASSERT (trailing_module_p);

  MODULE_T* module_p = NULL;
  do
  {
    module_p = trailing_module_p->next ();
    if (unlikely (!module_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to ACE_Module::next(), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  trailing_module_p->name ()));
      goto error;
    } // end IF
    if (!ACE_OS::strcmp (module_p->name (),
                         ACE_TEXT ("ACE_Stream_Tail")) ||
        !ACE_OS::strcmp (module_p->name (),
                         ACE_TEXT (STREAM_MODULE_TAIL_NAME)))
      break;
    trailing_module_p = module_p;
  } while (true);
  ACE_ASSERT (trailing_module_p);

  result = inherited::top (heading_module_p);
  if (unlikely ((result == -1) ||
                !heading_module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                ACE_TEXT (StreamName)));
    goto error;
  } // end IF

  // combine these two modules
  heading_module_p->reader ()->next (trailing_module_p->reader ());
  trailing_module_p->next (heading_module_p);
  trailing_module_p->writer ()->next (heading_module_p->writer ());
  unlink_modules = true;

  ////////////////////////////////////////

//  int nesting_level = unlock (true);
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, -1);
    // (try to) update module configurations
    if (!istream_p)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: upstream (was: 0x%@) does not implement Stream_IStream_T, cannot update module configurations, continuing\n"),
                  ACE_TEXT (StreamName),
                  &upstream_in));
      goto continue_;
    } // end IF
//    // *TODO*: remove type inference
//    for (typename CONFIGURATION_T::ITERATOR_T iterator = configuration_->begin ();
//         iterator != configuration_->end ();
//         iterator++)
//      (*iterator).second.first.stream = istream_p;

continue_:
    // (try to) merge upstream state data
    ISTREAM_CONTROL_T* istreamcontrol_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (&upstream_in);
    if (!istreamcontrol_p)
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: upstream (was: 0x%@) does not implement Stream_IStreamControl_T, cannot update state, continuing\n"),
//                  ACE_TEXT (StreamName),
//                  upstream_in));
      goto continue_2;
    } // end IF

    if (istream_p)
      istream_p->lock (true,   // block ?
                       false); // forward upstream (if any) ?

    state_p = &const_cast<StateType&> (istreamcontrol_p->state ());
    // *NOTE*: the idea here is to 'merge' the two datasets
    state_ += *state_p;
    *state_p += state_;

    if (istream_p)
      istream_p->unlock (false,  // unlock ?
                         false); // forward upstream (if any) ?

continue_2:
    // (try to) merge upstream session data
    iget_p = dynamic_cast<ISESSION_DATA_T*> (&upstream_in);
    if (!iget_p)
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: upstream (was: 0x%@) does not implement Common_IGetR_T<SessionDataContainerType>, cannot update session data, continuing\n"),
//                  ACE_TEXT (StreamName),
//                  upstream_in));
      goto continue_3;
    } // end IF
    session_data_container_p =
        &const_cast<SessionDataContainerType&> (iget_p->getR ());
    // *TODO*: race condition here
    //         --> make Stream_Base_T::get() return a reference directly
    session_data_container_p->increase ();
    session_data_p =
        &const_cast<SessionDataType&> (session_data_container_p->getR ());

    if (likely (sessionData_))
    {
      session_data_2 =
          &const_cast<SessionDataType&> (sessionData_->getR ());
      ACE_ASSERT (session_data_2->lock);
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_2->lock, -1);
        ACE_ASSERT (session_data_p->lock);
        ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard_2, *session_data_p->lock, -1);
        // *IMPORTANT NOTE*: the idea here is to 'merge' the two datasets
        *session_data_p += *session_data_2;

        // switch session data
        sessionData_->decrease ();
        sessionData_ = session_data_container_p;
      } // end lock scope
    } // end IF
    else
      sessionData_ = session_data_container_p;

continue_3:
    ;
  } // end lock scope
//  // relock ?
//  if (nesting_level >= 0)
//    COMMON_ILOCK_ACQUIRE_N (this, nesting_level + 1);

  // notify pipeline modules
  control (STREAM_CONTROL_LINK,
           true); // forward upstream ?

  return 0;

error:
  if (unlink_modules)
  {
    heading_module_p->reader ()->next (inherited::head ()->reader ());
    trailing_module_p->next (upstream_tail_module_p);
    trailing_module_p->writer ()->next (upstream_tail_module_p->writer ());
  } // end IF
  if (reset_upstream)
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, -1);
    upstream_ = NULL;
  } // end IF

  return -1;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlink (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlink"));

  // *WARNING*: cannot reach the base class lock from here --> not thread-safe !
  // *TODO*: submit change request to the ACE maintainers

  // sanity check(s)
  if (unlikely (!upstream_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: no upstream; cannot unlink, aborting\n"),
                ACE_TEXT (StreamName)));
    return -1;
  } // end IF

  int result = -1;
  // locate the module just above the upstreams' tail and this' 'top' module
  // (i.e. the module just below the head)
  MODULE_T* upstream_tail_module_p = upstream_->tail ();
  MODULE_T* trailing_module_p = upstream_->head ();
  MODULE_T* heading_module_p = NULL;

  result = inherited::top (heading_module_p);
  if (unlikely ((result == -1) ||
                !heading_module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                ACE_TEXT (StreamName)));
    return -1;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (trailing_module_p);

  MODULE_T* module_p = trailing_module_p->next ();
  do
  {
    if (unlikely (!module_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to ACE_Module::next(), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  trailing_module_p->name ()));
      return -1;
    } // end IF
    ACE_ASSERT (module_p->next ());
    if (!ACE_OS::strcmp (module_p->next ()->name (),
                         heading_module_p->name ()) ||
        // *IMPORTANT NOTE*: aggregated modules return tail as next()
        (!ACE_OS::strcmp (module_p->next ()->name (),
                          ACE_TEXT ("ACE_Stream_Tail")) ||
         !ACE_OS::strcmp (module_p->next ()->name (),
                          ACE_TEXT (STREAM_MODULE_TAIL_NAME))))
    {
      trailing_module_p = module_p;
      break;
    } // end IF
    module_p = module_p->next ();
  } while (true);
  ACE_ASSERT (trailing_module_p);

  // separate these two modules
  heading_module_p->reader ()->next (inherited::head ()->reader ());
  trailing_module_p->next (upstream_tail_module_p);
  trailing_module_p->writer ()->next (upstream_tail_module_p->writer ());

  ////////////////////////////////////////

//  int nesting_level = unlock (true);
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, -1);
    upstream_ = NULL;

//    // update configuration
//    // *TODO*: remove type inference
//    for (typename CONFIGURATION_T::ITERATOR_T iterator = configuration_->begin ();
//         iterator != configuration_->end ();
//         iterator++)
//      (*iterator).second.first.stream = this;
  } // end lock scope
//  // relock ?
//  if (nesting_level >= 0)
//    COMMON_ILOCK_ACQUIRE_N (this, nesting_level + 1);

  // notify any module(s)
  control (STREAM_CONTROL_UNLINK,
           false);

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::remove (MODULE_T* module_in,
                                           bool lock_in,
                                           bool reset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::remove"));

  // sanity check(s)
  ACE_ASSERT (module_in);

  int result = -1;
  IMODULE_T* imodule_p = NULL;
  Stream_ModuleList_t prev_a, next_a;
  MODULE_T* next_p = NULL, *tail_p = inherited::tail ();

  if (likely (lock_in))
  {
    result = lock_.acquire ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (StreamName)));
      return false;
    } // end IF
  } // end IF

  prev_a = layout_.prev (ACE_TEXT_ALWAYS_CHAR (module_in->name ()));
  next_a = layout_.next (ACE_TEXT_ALWAYS_CHAR (module_in->name ()));
  // sanity check(s)
  ACE_ASSERT (next_a.size () <= 1); // *TODO*: handle distributors
  ACE_ASSERT (tail_p);

  for (Stream_ModuleListIterator_t iterator = prev_a.begin ();
       iterator != prev_a.end ();
       ++iterator)
  { ACE_ASSERT (*iterator);
    next_p = (next_a.empty () ? tail_p : *next_a.begin ());
    ACE_ASSERT (next_p);
    (*iterator)->link (next_p);
  } // end FOR

  layout_.remove (ACE_TEXT_ALWAYS_CHAR (module_in->name ()));

  if (likely (lock_in))
  {
    result = lock_.release ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", aborting\n"),
                  ACE_TEXT (StreamName)));
      return false;
    } // end IF
  } // end IF

  if (reset_in)
  {
    result = module_in->close (ACE_Module_Base::M_DELETE_NONE);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to ACE_Module::close(M_DELETE_NONE): \"%m\", aborting\n"),
                  ACE_TEXT (StreamName), module_in->name ()));
      return false;
    } // end IF

    // *NOTE*: ACE_Module::close() resets the modules' task handles
    //         --> call reset() so it can be reused
    imodule_p = dynamic_cast<IMODULE_T*> (module_in);
    if (imodule_p)
      imodule_p->reset ();
  } // end IF
//#if defined (_DEBUG)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: removed module \"%s\"\n"),
//              ACE_TEXT (StreamName),
//              module_in->name ()));
//#endif // _DEBUG

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::finished (bool recurseupstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finished"));

  int result = -1;

  MODULE_T* module_p = NULL;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // foward upstream ?
  if (upstream_ &&
      recurseupstream_in)
  {
    // delegate to the head module
    result = upstream_->top (module_p);
    if (unlikely ((result == -1) ||
                  !module_p))
      goto _continue;

    istreamcontrol_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
    if (!istreamcontrol_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: dynamic_cast<Stream_ISTREAM_CONTROL_T> failed, continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name ()));
      goto _continue;
    } // end IF

    try {
      istreamcontrol_p->finished ();
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upstream_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: caught exception in Stream_ISTREAM_CONTROL_T::finished(), continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name ()));
      goto _continue;
    }
  } // end IF

_continue:
  // delegate to the head module
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return;
  } // end IF

  istreamcontrol_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_ISTREAM_CONTROL_T> failed, returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  } // end IF

  try {
    istreamcontrol_p->finished ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_ISTREAM_CONTROL_T::finished(), returning\n"),
                ACE_TEXT (StreamName),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::shutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::shutdown"));

  MODULE_T* module_p = NULL;

  // step0: if not properly initialized, this needs to deactivate any hitherto
  // enqueued ACTIVE modules, or the stream will wait forever during closure
  // --> possible scenarios:
  // - (re-)init() failed halfway through (i.e. MAYBE some modules push()ed
  //   correctly)
  if (!isInitialized_)
  {
    // sanity check: successfully pushed() ANY modules ?
    module_p = inherited::head ();
    if (module_p)
    {
      module_p = module_p->next ();
      if (module_p && (module_p != inherited::tail ()))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: not initialized - deactivating module(s)...\n"),
                    ACE_TEXT (StreamName)));
        deactivateModules ();
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: not initialized - deactivating module(s)...DONE\n"),
                    ACE_TEXT (StreamName)));
      } // end IF
    } // end IF
  } // end IF
  else
  {
    // handle final module (if any)
    // *NOTE*: this module may be shared by multiple stream instances, so it
    //         must not be close()d or reset here
    { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
      if (state_.module && !state_.moduleIsClone)
        if (unlikely (!remove (state_.module,
                               false,   // lock ?
                               false))) // reset ? (see above)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                      ACE_TEXT (StreamName),
                      state_.module->name ()));
      state_.module = NULL;
    } // end lock scope
  } // end ELSE

  // step2: shutdown stream
  // check the ACE documentation on ACE_Stream to understand why this is needed
  // *TODO*: ONLY do this if stream_head != 0 !!! (warning: obsolete ?)
  // *NOTE*: will NOT destroy all modules in the current stream, as this leads
  //         to exceptions in debug builds under MS Windows (can't delete object
  //         in a different DLL than where it was created...)
  //         --> do this manually !
  //         this invokes close() on each module (and waits for any worker
  //         thread(s) to return)
  if (unlikely (!finalize ()))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::finalize(): \"%m\", continuing\n"),
                ACE_TEXT (StreamName)));

  // *NOTE*: every ACTIVE module will join with its worker thread in close()
  //         --> ALL stream-related threads should have returned by now
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::deactivateModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::deactivateModules"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (sessionData_);

  const SessionDataType& session_data_r = sessionData_->getR ();

  // *NOTE*: the message instance assumes responsibility for the session data
  //         --> increment the reference counter
  sessionData_->increase ();

  // allocate SESSION_END session message
  SessionMessageType* message_p = NULL;
  // *TODO*: remove type inference
  if (configuration_->configuration_.messageAllocator)
  {
    try { // *NOTE*: 0 --> session message
      message_p =
        static_cast<SessionMessageType*> (configuration_->configuration_.messageAllocator->malloc (0));
    } catch (...) {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(0), returning\n"),
                  ACE_TEXT (StreamName)));

      // clean up
      sessionData_->decrease ();

      return;
    }
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      SessionMessageType (session_data_r.sessionId,
                                          STREAM_SESSION_MESSAGE_END,
                                          sessionData_,
                                          state_.userData));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate SessionMessageType: \"%m\", returning\n"),
                ACE_TEXT (StreamName)));

    // clean up
    sessionData_->decrease ();

    return;
  } // end IF
  // *TODO*: remove type inferences
  if (configuration_->configuration_.messageAllocator)
    message_p->initialize (session_data_r.sessionId,
                           STREAM_SESSION_MESSAGE_END,
                           sessionData_,
                           state_.userData);

  // forward message
  int result = inherited::put (message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::put(): \"%m\", returning\n"),
                ACE_TEXT (StreamName)));

    // clean up
    message_p->release ();

    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              AllocatorConfigurationType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlinkModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlinkModules"));

  MODULE_T* head_p = inherited::head ();
  for (MODULE_T* module_p = head_p;
       module_p;
       module_p = module_p->next ())
    module_p->next (NULL);
  head_p->next (inherited::tail ());
}
