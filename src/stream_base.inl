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

#include <functional>

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::Stream_Base_T ()
 : inherited (NULL, // default argument to module open()
              NULL, // --> allocate head module
              NULL) // --> allocate tail module
 , configuration_ (NULL)
 , id_ ()
 , isInitialized_ (false)
 , layout_ ()
 , messageQueue_ (STREAM_QUEUE_MAX_SLOTS,
                  NULL)
 , name_ (StreamName)
 , state_ ()
 , statistic_ ()
 /////////////////////////////////////////
 , delete_ (false)
 , subscribers_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::Stream_Base_T"));

  { //ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    state_.statistic = &statistic_;
  } // end lock scope

  // generate "unique" id
  std::ostringstream converter;
  // *TODO*: incorporate timestamp ?
  std::hash<void*> ptr_hash;
  converter << ptr_hash (this);
  id_ = converter.str ();

  // create session data instance (managed by session manager)
  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  session_manager_p->create (id_);

  if (unlikely (!initializeHeadTail ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initializeHeadTail(), returning\n"),
                ACE_TEXT (name_.c_str ())));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::~Stream_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::~Stream_Base_T"));

  // destroy session data instance (iff managed by session manager)
  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  session_manager_p->destroy (id_);

  // deregister messageQueue_; it falls off the stack before
  // inherited::stream_head_
  if (likely (inherited::stream_head_))
  {
    TASK_T* task_p = inherited::stream_head_->reader ();
    ACE_ASSERT (task_p);
    task_p->msg_queue (NULL);
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::reset"));

  bool result = false;

  // handle final module (if any)
  // *NOTE*: this module may be shared by multiple stream instances, so it
  //         must not be close()d or reset here
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
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
                      ACE_TEXT (name_.c_str ()),
                      state_.module->name ()));
      } // end IF
    } // end IF
  } // end lock scope

  // - pop/close all modules
  // *NOTE*: will implicitly (blocking !) wait for any active worker threads
  // - reset reader/writers tasks of all modules
  result = finalize (true); // re-initialize head/tail modules
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::finalize(), aborting\n"),
                ACE_TEXT (name_.c_str ())));

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::setup (ACE_Notification_Strategy* notificationStrategy_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::setup"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->configuration_);

  IMODULE_T* imodule_p = NULL;
  TASK_T* task_p = NULL;
  IMODULE_HANDLER_T* imodule_handler_p = NULL;
  typename CONFIGURATION_T::ITERATOR_T iterator_2;
  Common_ISetP_T<ACE_Stream<ACE_SYNCH_USE, TimePolicyType> >* isetp_p = NULL;

  // step1: initialize modules
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
    for (LAYOUT_ITERATOR_T iterator = layout_.begin ();
         iterator != layout_.end ();
         ++iterator)
    {
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (likely (imodule_p))
        imodule_p->reset ();
      else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, continuing\n"),
                    ACE_TEXT (name_.c_str ()), (*iterator)->name ()));

      // *NOTE*: some modules (i.e. aggregating modules) need to know which
      //         stream they are being pushed to
      isetp_p =
        dynamic_cast<Common_ISetP_T<ACE_Stream<ACE_SYNCH_USE, TimePolicyType> >*> (*iterator);
      if (unlikely (isetp_p))
        isetp_p->setP (this);

      // *TODO*: remove type inference
      iterator_2 = configuration_->find ((*iterator)->name ());
      if (iterator_2 == configuration_->end ())
        iterator_2 = configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s: applying dedicated configuration\n"),
                    ACE_TEXT (name_.c_str ()), (*iterator)->name ()));
      ACE_ASSERT (iterator_2 != configuration_->end ());
      if (unlikely (imodule_p &&
                    !imodule_p->initialize (*(*iterator_2).second.first)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Common_IInitialize_T::initialize(), aborting\n"),
                    ACE_TEXT (name_.c_str ()), (*iterator)->name ()));
        return false;
      } // end IF

      task_p = (*iterator)->writer ();
      ACE_ASSERT (task_p);
      imodule_handler_p = dynamic_cast<IMODULE_HANDLER_T*> (task_p);
      if (!imodule_handler_p)
      { // *TODO*: determine the 'active' side of the module by some
        //         member/function
        task_p = (*iterator)->reader ();
        ACE_ASSERT (task_p);
        imodule_handler_p = dynamic_cast<IMODULE_HANDLER_T*> (task_p);
        if (unlikely (!imodule_handler_p))
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModuleHandler_T> failed, continuing\n"),
                      ACE_TEXT (name_.c_str ()), (*iterator)->name ()));
          continue;
        } // end IF
      } // end IF
      if (unlikely (!imodule_handler_p->initialize (*(*iterator_2).second.second,
                                                    configuration_->configuration_->messageAllocator)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Stream_IModuleHandler_T::initialize(), aborting\n"),
                    ACE_TEXT (name_.c_str ()), (*iterator)->name ()));
        return false;
      } // end IF
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s/%s: initialized\n"),
//                  ACE_TEXT (name_.c_str ()), (*iterator)->name ()));
    } // end FOR
  } // end lock scope

  // step2: setup layout
  // *WARNING*: invokes this->close(), which acquires inherited::lock_ as well
  //            --> do not lock here
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
    if (!layout_.setup (*this))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Layout::setup(): \"%m\", aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    } // end IF
//  } // end lock scope

#if defined (_DEBUG)
  //layout_.dump_state ();
  //std::cout << ACE_TEXT_ALWAYS_CHAR ("\n----------------------\n");
  dump_state ();
#endif // _DEBUG

  // step3: set notification strategy ?
  if (notificationStrategy_in)
  {
    MODULE_T* module_p = inherited::head ();
    if (unlikely (!module_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module found, aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    } // end IF
    TASK_T* task_p = module_p->reader ();
    if (unlikely (!task_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module reader task found, aborting\n"),
                  ACE_TEXT (name_.c_str ())));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::subscribe (IEVENT_T* interfaceHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::subscribe"));

  // sanity check(s)
  ACE_ASSERT (interfaceHandle_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    subscribers_.push_back (interfaceHandle_in);
    subscribers_.sort ();
    subscribers_.unique (SUBSCRIBERS_IS_EQUAL_P ());
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unsubscribe (IEVENT_T* interfaceHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unsubscribe"));

  // sanity check(s)
  ACE_ASSERT (interfaceHandle_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
    for (;
         iterator != subscribers_.end ();
         iterator++)
      if ((*iterator) == interfaceHandle_in)
        break;

    if (iterator != subscribers_.end ())
      subscribers_.erase (iterator);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid argument (was: %@), continuing\n"),
                  ACE_TEXT (name_.c_str ()),
                  interfaceHandle_in));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (bool setupPipeline_in,
                                               bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->configuration_);

  // step1: reset session data ?
  if (resetSessionData_in)
  {
    SessionManagerType* session_manager_p =
      SessionManagerType::SINGLETON_T::instance ();
    ACE_ASSERT (session_manager_p);
    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (id_));
    session_data_r.clear ();
  } // end IF

  // step2: load modules
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    if (unlikely (!this->load (&layout_,
                               delete_)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_IStreamLayout_T::load(), returning\n"),
                  ACE_TEXT (name_.c_str ())));
      goto error;
    } // end IF

    // *NOTE*: iff this is set, the module has already been clone()d as
    //         appropriate (see: initialize():3066)
    if (state_.module)
    {
      if (unlikely (!layout_.append (state_.module,
                                     configuration_->configuration_->moduleBranch)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Layout_T::append(\"%s\",\"%s\"), returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    state_.module->name (),
                    ACE_TEXT (configuration_->configuration_->moduleBranch.c_str ())));
        goto error;
      } // end IF
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: appended \"%s\" to \"%s\" branch\n"),
//                  ACE_TEXT (name_.c_str ()),
//                  configuration_->configuration->module->name (),
//                  (configuration_->configuration->moduleBranch.empty () ? ACE_TEXT ("main") : ACE_TEXT (configuration_->configuration->moduleBranch.c_str ()))));
    } // end IF

    if (unlikely (layout_.empty ()))
      goto continue_;

    // *NOTE*: push()ing a module will open() it
    //         --> (re)set the argument that is passed along
    LAYOUT_ITERATOR_T iterator = layout_.begin ();
    (*iterator)->arg (NULL);
    // *NOTE*: the head module writer task needs access to the stream state
    ISET_T* iset_p = dynamic_cast<ISET_T*> ((*iterator)->writer ());
    if (unlikely (!iset_p))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: head module \"%s\" does not inherit Common_ISetP_T<StateType> --> check implementation, continuing\n"),
                  ACE_TEXT (name_.c_str ()),
                  (*iterator)->name ()));
      goto continue_;
    } // end IF
    iset_p->setP (&state_);
  } // end lock scope

continue_:
  // step3: setup pipeline ?
  if (setupPipeline_in)
    if (unlikely (!setup (configuration_->configuration_->notificationStrategy)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::setup(), returning\n"),
                  ACE_TEXT (name_.c_str ())));
      goto error;
    } // end IF

  isInitialized_ = true;

  return;

error:
  isInitialized_ = false;
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::finalize (bool initializeHeadTailModules_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finalize"));

  int result = -1;

  unsigned int flushed_messages_i = messageQueue_.flush (true); // flush session messages ?
  ACE_UNUSED_ARG (flushed_messages_i);

  // *NOTE*: unwinds the stream, pop()ing all push()ed modules
  //         --> pop()ing a module will close() it
  //         --> close()ing a module will module_closed() and flush()/wait() (and
  //             reset its' tasks)
  //         --> flush()ing a task will close() its queue
  //         --> close()ing a queue will deactivate() and flush() it
  // *IMPORTANT NOTE*: passing anything but M_DELETE_NONE will 'delete' the
  //                   modules (see: Stream.cpp:219); do this below instead
  result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::close(M_DELETE_NONE): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return false;
  } // end IF

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
    // *NOTE*: ACE_Stream::close() resets the task handles of all modules
    //         --> reset them manually so they can be deleted below
    IMODULE_T* imodule_p = NULL;
    for (LAYOUT_ITERATOR_T iterator = layout_.begin ();
         iterator != layout_.end ();
         iterator++)
    {
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (unlikely (!imodule_p))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator)->name ()));
        continue;
      } // end IF
      try {
        imodule_p->reset ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_IModule_T::reset(), continuing\n"),
                    ACE_TEXT (name_.c_str ()),
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

  if (initializeHeadTailModules_in)
    if (unlikely (!initializeHeadTail ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::initializeHeadTail(), aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initializeHeadTail ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initializeHeadTail"));

  int result = -1;
  HEAD_WRITER_T* head_writer_p = NULL;
  HEAD_READER_T* head_reader_p = NULL;
  TAIL_WRITER_T* tail_writer_p = NULL;
  TAIL_READER_T* tail_reader_p = NULL;
  MODULE_T* module_p = NULL, *module_2 = NULL;
  ACE_NEW_NORETURN (head_writer_p,
                    HEAD_WRITER_T (this));
  // *NOTE*: pass a handle to the message queue member to the head module reader
  //         task; this will become the 'outbound' queue
  ACE_NEW_NORETURN (head_reader_p,
                    HEAD_READER_T (this,
                                   &messageQueue_,
                                   false, // enqueue incoming messages ? : release()
                                   id_));
  if (unlikely (!head_writer_p || !head_reader_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    delete head_writer_p; head_writer_p = NULL;
    delete head_reader_p; head_reader_p = NULL;
    return false;
  } // end IF
  ACE_NEW_NORETURN (module_p,
                    MODULE_T (ACE_TEXT (STREAM_MODULE_HEAD_NAME),
                              head_writer_p,
                              head_reader_p,
                              NULL,
                              ACE_Module_Base::M_DELETE));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    delete head_writer_p; head_writer_p = NULL;
    delete head_reader_p; head_reader_p = NULL;
    return false;
  } // end IF

  ACE_NEW_NORETURN (tail_writer_p,
                    TAIL_WRITER_T ());
  ACE_NEW_NORETURN (tail_reader_p,
                    TAIL_READER_T ());
  if (unlikely (!tail_writer_p || !tail_reader_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    delete tail_writer_p; tail_writer_p = NULL;
    delete tail_reader_p; tail_reader_p = NULL;
    delete module_p; module_p = NULL;
    return false;
  } // end IF
  ACE_NEW_NORETURN (module_2,
                    MODULE_T (ACE_TEXT (STREAM_MODULE_TAIL_NAME),
                              tail_writer_p,
                              tail_reader_p,
                              NULL,
                              ACE_Module_Base::M_DELETE));
  if (unlikely (!module_2))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    delete tail_writer_p; tail_writer_p = NULL;
    delete tail_reader_p; tail_reader_p = NULL;
    delete module_p; module_p = NULL;
    return false;
  } // end IF

  result = inherited::close (STREAM_T::M_DELETE);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::close(M_DELETE): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    delete module_p; module_p = NULL;
    delete module_2; module_2 = NULL;
    return false;
  } // end IF
  result = inherited::open (NULL,      // argument passed to module open()
                            module_p,  // head module handle
                            module_2); // tail module handle
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::open(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    delete module_p; module_p = NULL;
    delete module_2; module_2 = NULL;
    return false;
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  if (unlikely (isRunning ()))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: already running, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return; // nothing to do
  } // end IF
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->configuration_);

  // initialize session data
  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (id_));
  // *TODO*: remove type inferences
  ACE_ASSERT (session_data_r.lock);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
    session_data_r.sessionId =
      (configuration_->configuration_->sessionId ? configuration_->configuration_->sessionId
                                                 : ++inherited2::currentSessionId);
    session_data_r.startOfSession = ACE_OS::gettimeofday ();
  } // end lock scope

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  { // already closed ?
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no head module found, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  ISTREAM_CONTROL_T* istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->start ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::start(), returning\n"),
                ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::stop (bool wait_in,
                                         bool recurseUpstream_in,
                                         bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::stop"));

  int result = -1;
  MODULE_T* module_p = NULL;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // stop upstream ?
  if (inherited::linked_us_ &&
      recurseUpstream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (inherited::linked_us_);
    if (unlikely (!istreamcontrol_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl_T> failed, continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("N/A"))));
      goto continue_;
    } // end IF
    try {
      istreamcontrol_p->stop (wait_in,
                              recurseUpstream_in,
                              highPriority_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl::stop(), continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("N/A"))));
    }
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: stopped upstream: %s, continuing\n"),
                ACE_TEXT (name_.c_str ()),
                (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("N/A"))));
  } // end IF

continue_:
  // delegate to the head module, skip over ACE_Stream_Head
  result = inherited::top (module_p);
  if (unlikely (result == -1 || !module_p))
  { // already close()d/never open()ed ?
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("%s: no head module found: \"%m\", returning\n"),
    //            ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  istreamcontrol_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->stop (wait_in,
                            recurseUpstream_in,
                            highPriority_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::stop(), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  }

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_SessionId_t
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::sessionId () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::sessionId"));

  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  return session_manager_p->getR (id_).sessionId;
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isRunning"));

  int result = -1;
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);

  // sanity check(s)
  if (unlikely (!this_p->head ()))
    return false;
  // delegate to the head module (if any)
  MODULE_T* module_p = NULL;
  result = this_p->top (module_p);
  if (unlikely ((result == -1) || !module_p))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", aborting\n")));
    return false;
  } // end IF

  HEAD_TASK_T* task_p = static_cast<HEAD_TASK_T*> (module_p->writer ());
  ACE_ASSERT (task_p);

  try {
    return task_p->isRunning ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::isRunning(), aborting\n"),
                ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::control (ControlType control_in,
                                            bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::control"));

  int result = -1;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // forward upstream ?
  if (inherited::linked_us_ && recurseUpstream_in)
  {
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (inherited::linked_us_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  inherited::linked_us_));
      return;
    } // end IF
    try {
      istreamcontrol_p->control (control_in,
                                 recurseUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
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
                ACE_TEXT (name_.c_str ())));
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
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
        return;
      } // end IF
      try {
        istreamcontrol_p->control (control_in,
                                   false); // N/A
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::control(%d), returning\n"),
                    ACE_TEXT (name_.c_str ()),
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
                  ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::notify (NotificationType notification_in,
                                           bool recurseUpstream_in,
                                           bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::notify"));

  int result = -1;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // forward upstream ?
  if (inherited::linked_us_ &&
      recurseUpstream_in)
  {
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (inherited::linked_us_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  inherited::linked_us_));
      return;
    } // end IF
    try {
      istreamcontrol_p->notify (notification_in,
                                recurseUpstream_in,
                                expedite_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::notify(%d), continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  notification_in));
    }

    return;
  } // end IF

  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if (unlikely ((result == -1) ||
                !module_p))
  { // connection failed ?
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("%s: failed to ACE_Stream::top(), continuing\n"),
//                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (module_p);

  istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (unlikely (!istreamcontrol_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->notify (notification_in,
                              false,           // recurse upstream ?
                              expedite_in);    // expedite ?
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::notify(%d), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name (),
                notification_in));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::onEvent (const std::string& streamId_in,
                                            NotificationType notification_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::onEvent"));

  ACE_UNUSED_ARG (streamId_in);

  MODULE_T* module_p = NULL;
  int result = inherited::top (module_p);
  if (unlikely ((result == -1) || !module_p))
  { // connection failed ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(), returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (module_p);
  ISTATE_MACHINE_T* istatemachine_p =
    dynamic_cast<ISTATE_MACHINE_T*> (module_p->writer ());
  if (unlikely (!istatemachine_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Common_IStateMachine_2> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    session_manager_p->getR (id_);

  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      enum Stream_StateMachine_ControlState state_e =
        istatemachine_p->current ();

      // *NOTE*: there are two scenarios in this case:
      //         - session initialization failed and is being notified here
      //           --> stop session: set aborted flag and send SESSION_ABORT
      //         - session abort is complete
      //           --> end session normally
      if ((state_e == STREAM_STATE_SESSION_STARTING) &&
          !session_data_r.aborted)
      { // == first case; handled by head module writer task
        notify (notification_in,
                false); // recurse upstream ?
        break;
      } // end IF

      // == second case
      // *NOTE*: if there was upstream activity (e.g. network i/o) involved, the
      //         state may already be 'stopping'
      if (state_e >= STREAM_STATE_SESSION_STOPPING)
        goto session_end;

      try
      {
        istatemachine_p->change (STREAM_STATE_SESSION_STOPPING);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Common_IStateMachine_2::change(), returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
        return;
      }

      goto session_end;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: connect notified, returning\n"),
      //            ACE_TEXT (name_.c_str ())));
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: disconnect notified, returning\n"),
      //            ACE_TEXT (name_.c_str ())));
      break;
    }
    // *NOTE*: there are two scenarios in this case:
    //         - stream link/unlink is being notified here
    //           --> send SESSION_LINK/UNLINK
    //         - session link/unlink is complete
    //           --> do NOT recurse
    case STREAM_SESSION_MESSAGE_LINK:
    {
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
        state_.linked_ds_ = true;
      } // end lock scope
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: link notified, returning\n"),
      //            ACE_TEXT (name_.c_str ())));
      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
        state_.linked_ds_ = false;
      } // end lock scope
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: unlink notified, returning\n"),
      //            ACE_TEXT (name_.c_str ())));
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      try {
        istatemachine_p->change (STREAM_STATE_RUNNING);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Common_IStateMachine_2::change(), returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
        return;
      }

      try {
        onSessionBegin (session_data_r.sessionId);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_ISessionCB::onSessionBegin(), continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
      }

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
session_end:
      // sanity check(s)
      enum Stream_StateMachine_ControlState state_e =
        istatemachine_p->current ();
      if (state_e > STREAM_STATE_SESSION_STOPPING)
        break; // catch spurious abort(s); there should only be one (!) session-end notification

      try {
        istatemachine_p->change (STREAM_STATE_STOPPED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Common_IStateMachine_2::change(), returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
        return;
      }

      try {
        onSessionEnd (session_data_r.sessionId);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_ISessionCB::onSessionEnd(), continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
      }

      break;
    }
    default:
      break;
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
         iterator != subscribers_.end ();
         )
    {
      try {
        (*iterator++)->onEvent (id_,
                                notification_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_IEvent_T::onEvent(%d), continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    notification_in));
      }
    } // end FOR
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::flush (bool flushInbound_in,
                                          bool flushSessionMessages_in,
                                          bool flushUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::flush"));

  unsigned int result = 0;

  // forward upstream ?
  if (unlikely (inherited::linked_us_ &&
                flushUpstream_in))
  {
    ISTREAM_CONTROL_T* istreamcontrol_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (inherited::linked_us_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  inherited::linked_us_));
      return 0;
    } // end IF
    try {
      return istreamcontrol_p->flush (flushInbound_in,
                                      flushSessionMessages_in,
                                      flushUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
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
  { ACE_ASSERT (*iterator);
    task_p = const_cast<MODULE_T*> (*iterator)->writer ();
    if (unlikely (!task_p))
      continue; // close()d already ?
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
    if (unlikely (!iqueue_p))
    {
      // *NOTE*: most probable cause: module is (upstream) head
      // *TODO*: all messages are flushed here, this must not happen
      if (likely (task_p->msg_queue_))
        result_2 = task_p->msg_queue_->flush ();
    } // end IF
    else
      result_2 = static_cast<int> (iqueue_p->flush (flushSessionMessages_in));
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s writer: failed to Stream_IMessageQueue::flushData(): \"%m\", continuing\n"),
                  ACE_TEXT (name_.c_str ()), (*iterator)->name ()));
    } // end IF
    else if (likely (result_2))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s writer: flushed %d message(s)\n"),
                  ACE_TEXT (name_.c_str ()), (*iterator)->name (),
                  result_2));
      result += result_2;
    } // end ELSE IF
  } // end FOR

continue_:
  // reader (outbound) side
  modules_a.push_front (const_cast<MODULE_T*> (inherited::head ())); // prepend head
  for (Stream_ModuleListReverseIterator_t iterator = modules_a.rbegin ();
       iterator != modules_a.rend ();
       ++iterator)
  { ACE_ASSERT (*iterator);
    task_p = (*iterator)->reader ();
    if (unlikely (!task_p))
      continue; // close()d already ?
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
    if (unlikely (!iqueue_p))
    {
      // *NOTE*: most probable cause: stream head/tail, or module does not have
      //         a reader task
      // *TODO*: all messages are flushed here, this must not happen
      if (likely (task_p->msg_queue_))
        result_2 = task_p->msg_queue_->flush ();
    } // end IF
    else
      result_2 = static_cast<int> (iqueue_p->flush (flushSessionMessages_in));
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s reader: failed to Stream_IMessageQueue::flush(): \"%m\", continuing\n"),
                  ACE_TEXT (name_.c_str ()), (*iterator)->name ()));
    } // end IF
    else if (likely (result))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s reader: flushed %d message(s)\n"),
                  ACE_TEXT (name_.c_str ()), (*iterator)->name (),
                  result_2));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    istreamcontrol_p->pause ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl::pause(), returning\n"),
                ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
                ACE_TEXT (name_.c_str ())));
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
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->rewind ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::rewind(), returning\n"),
                ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return result;
  } // end IF

  try {
    result = istreamcontrol_p->status ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::status(), aborting\n"),
                ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::idle (bool waitForever_in,
                                         bool recurseUpstream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::idle"));

  // *NOTE*: if this stream has been linked (e.g. connection is part of another
  //         stream), make sure to wait for the whole pipeline
  if (unlikely (recurseUpstream_in &&
                inherited::linked_us_))
  {
    Stream_IStreamControlBase* istreamcontrol_p =
      dynamic_cast<Stream_IStreamControlBase*> (inherited::linked_us_);
    if (unlikely (!istreamcontrol_p))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to dynamic_cast<Stream_IStreamControlBase>(0x%@), returning\n"),
                  inherited::linked_us_));
      return;
    } // end IF
    try {
      istreamcontrol_p->idle (waitForever_in,
                              recurseUpstream_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IStreamControlBase::idle(), continuing\n")));
    }
  } // end IF

  // step1a: wait for inbound processing (i.e. 'writer' side) pipeline to flush
  TASK_T* task_p = NULL;
  Stream_IMessageQueue* iqueue_p = NULL;
  Stream_ModuleList_t modules_a;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T> reverse_lock (inherited::lock_);
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
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
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
        iqueue_p->waitForIdleState (waitForever_in);
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
                      ACE_TEXT ("%s/%s writer: waiting to process %B byte(s) in %B message(s)...\n"),
                      ACE_TEXT (name_.c_str ()),
                      (*iterator)->name (),
                      task_p->msg_queue_->message_bytes (),
                      task_p->msg_queue_->message_count ()));

          { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
            result = ACE_OS::sleep (one_second);
          } // end lock scope
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s/%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                        ACE_TEXT (name_.c_str ()),
                        (*iterator)->name (),
                        &one_second));
        } while (true);
      } // end ELSE
    } // end FOR

    // step1b: wait for outbound processing (i.e. 'reader' side) pipeline to flush
    for (Stream_ModuleListReverseIterator_t iterator = modules_a.rbegin ();
         iterator != modules_a.rend ();
         ++iterator)
    {
      task_p = (*iterator)->reader ();
      if (unlikely (!task_p))
        continue; // close()d already ?
      if (likely (!task_p->msg_queue_))
        continue; // synchronous task --> nothing to do

      iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
      if (iqueue_p)
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
        iqueue_p->waitForIdleState (waitForever_in);
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
                      ACE_TEXT ("%s/%s reader: waiting to process %B byte(s) in %B message(s)...\n"),
                      ACE_TEXT (name_.c_str ()),
                      (*iterator)->name (),
                      task_p->msg_queue_->message_bytes (),
                      task_p->msg_queue_->message_count ()));

          { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
            result = ACE_OS::sleep (one_second);
          } // end lock scope
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s/%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                        ACE_TEXT (name_.c_str ()),
                        (*iterator)->name (),
                        &one_second));
        } while (true);
      } // end ELSE
    } // end FOR
  } // end lock scope

  // step2: wait for outbound processing (i.e. 'reader' side) pipeline to flush
  // *NOTE*: never block here; a connection may be closing, and some event
  //         dispatchers (e.g. select reactor) may be holding their own lock
  //         here and therefore will hang forever !
  messageQueue_.waitForIdleState (false); // block ?
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::wait (bool waitForThreads_in,
                                         bool waitForUpstream_in,
                                         bool waitForDownStream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::wait"));

  int result = -1;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);

  // forward upstream ?
  if (inherited::linked_us_ &&
      waitForUpstream_in)
  {
    istreamcontrol_p = dynamic_cast<ISTREAM_CONTROL_T*> (inherited::linked_us_);
    if (unlikely (!istreamcontrol_p))
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  inherited::linked_us_));
      return;
    } // end IF
    try {
      istreamcontrol_p->wait (waitForThreads_in,
                              waitForUpstream_in,
                              waitForDownStream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
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
  { // already closed ?
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no head module found, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  const MODULE_T* module_p = NULL;
  result = iterator.next (module_p);
  if (unlikely (!result))
  { // already closed ?
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no head module found, returning\n"),
                ACE_TEXT (name_.c_str ())));
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
                ACE_TEXT (name_.c_str ()), module_p->name ()));
    return;
  } // end IF
  try {
    istreamcontrol_p->wait (false, // wait for threads ?
                            waitForUpstream_in,
                            waitForDownStream_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl::wait(), returning\n"),
                ACE_TEXT (name_.c_str ()), module_p->name ()));
    return;
  }
  // --> no new messages will be dispatched from the head module

  // step1b: wait for inbound processing (i.e. the 'writer' side) to complete
  //         --> grab the lock to freeze the layout
//  ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);

  Stream_ModuleList_t modules_a = layout_.list (false);
  Stream_ModuleList_t main_modules_a = layout_.list (true);
  modules_a.push_front (this_p->inherited::head ());
  TASK_T* task_p = NULL;
  ACE_Time_Value one_second (1, 0);
  size_t message_count = 0;
  //ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T> reverse_lock (inherited::lock_);
  unsigned int head_reader_retries_i = 0;
  // bool queue_has_data_b;
  // MESSAGE_QUEUE_T* queue_p = NULL;

  for (Stream_ModuleListIterator_t iterator_2 = modules_a.begin ();
       iterator_2 != modules_a.end ();
       ++iterator_2)
  { ACE_ASSERT (*iterator_2);
    task_p = const_cast<MODULE_T*> (*iterator_2)->writer ();
    if (unlikely (!task_p))
      continue; // close()d already ?
    if (task_p->msg_queue_)
    {
      // queue_has_data_b = true;
      do
      {
        // queue_has_data_b = task_p->msg_queue_->message_bytes () > 0;
        message_count = task_p->msg_queue_->message_count ();
        if (!message_count/* ||
            !queue_has_data_b*/) // *TODO*: remove this clause: should wait for session/control messages as well
          break;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s writer: waiting to process %B byte(s) in %B message(s)...\n"),
                    ACE_TEXT (name_.c_str ()), (*iterator_2)->name (),
                    task_p->msg_queue_->message_bytes (), message_count));

        { //ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
          result = ACE_OS::sleep (one_second);
        } // end lock scope
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                      ACE_TEXT (name_.c_str ()), (*iterator_2)->name (),
                      &one_second));
      } while (true);
    } // end IF

    if (waitForThreads_in)
    {
      { //ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
        result = task_p->wait ();
      } // end lock scope
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if (error != ENXIO) // *NOTE*: see also: common_task_base.inl:350
#endif // ACE_WIN32 || ACE_WIN64
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s writer: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                      ACE_TEXT (name_.c_str ()), (*iterator_2)->name ()));
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
    if (task_p->msg_queue_)
    {
      // queue_has_data_b = true;
      do
      {
        // queue_has_data_b = task_p->msg_queue_->message_bytes () > 0;
        message_count = task_p->msg_queue_->message_count ();
        if (!message_count/* ||
            !queue_has_data_b*/) // *TODO*: remove this clause: should wait for session/control messages as well
          break;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s reader: waiting to process %B byte(s) in %B message(s)...\n"),
                    ACE_TEXT (name_.c_str ()), (*iterator_2)->name (),
                    task_p->msg_queue_->message_bytes (), message_count));

        { //ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
          result = ACE_OS::sleep (one_second);
        } // end lock scope
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                      ACE_TEXT (name_.c_str ()), (*iterator_2)->name (),
                      &one_second));

        // *NOTE*: there is a slight chance that outbound data is being dispatched
        //         while waiting. Note that data dispatch currently happens
        //         synchronously, so unless no new outbound data is being
        //         generated 'asynchronously', this should be fine
        //         --> bail out after x retries
        if (unlikely ((*iterator_2) == modules_a.front ()))
        {
          if (head_reader_retries_i == STREAM_DEFAULT_STOP_WAIT_HEAD_READER_RETRIES)
          {
            ACE_DEBUG ((LM_WARNING,
                        ACE_TEXT ("%s/%s reader: retried waiting %d time(s), giving up\n"),
                        ACE_TEXT (name_.c_str ()), (*iterator_2)->name (),
                        STREAM_DEFAULT_STOP_WAIT_HEAD_READER_RETRIES));
            break;
          } // end IF
          ++head_reader_retries_i;
        } // end IF
      } while (true);
    } // end IF

    if (waitForThreads_in)
    {
      { //ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard2, reverse_lock);
        result = task_p->wait ();
      } // end lock scope
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s reader: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                    ACE_TEXT (name_.c_str ()), (*iterator_2)->name ()));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::find (const std::string& name_in,
                                         bool sanitizeModuleNames_in,
                                         bool recurseUpstream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::find"));

  if (inherited::linked_us_ &&
      recurseUpstream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
    if (istream_p)
      return istream_p->find (name_in,
                              sanitizeModuleNames_in,
                              recurseUpstream_in);
  } // end IF

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, NULL);
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::link (typename ISTREAM_T::STREAM_T* upstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // sanity check(s)
  ACE_ASSERT (upstream_in);

  ISTREAM_T* istream_p = NULL;
  if (inherited::linked_us_)
  {
    istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
    if (unlikely (!istream_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStream_T>(0x%@) upstream, aborting\n"),
                  ACE_TEXT (name_.c_str ()),
                  inherited::linked_us_));
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
                ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::_unlink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::_unlink"));

  // sanity check(s)
  if (!inherited::linked_us_)
  { // *TODO*: consider all scenarios where this might happen
    // *NOTE*: upstream finished and has already _unlink()ed downstream
    //         (connection closed abruptly by peer)
    //         (see: stream_headmoduletask_base.inl:2699), so second _unlink()
    //         in stream_net_target.inl:667 fails
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no upstream; cannot unlink, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  int result = unlink ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::unlink(), returning\n"),
                ACE_TEXT (name_.c_str ())));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::downstream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::downstream"));

  int result = -1;
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);
  typename ISTREAM_T::MODULE_T* module_p = NULL, *module_2 = NULL;
  STATE_MACHINE_CONTROL_T* state_machine_control_p = NULL;
  IGET_T* iget_p = NULL;
  ISTREAM_T* istream_p = NULL;

  result = this_p->inherited::top (module_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return NULL;
  } // end IF
  // sanity check(s)
  ACE_ASSERT (module_p);

  // step1: locate the second (!) downstream head module
  module_2 = module_p->next (); // skip over first head module
  // sanity check(s)
  ACE_ASSERT (module_2);
  do
  { ACE_ASSERT (module_2->writer ());
    state_machine_control_p =
      dynamic_cast<STATE_MACHINE_CONTROL_T*> (module_2->writer ());
    if (state_machine_control_p)
      break;
    module_2 = module_2->next ();
  } while (module_2);
  if (!state_machine_control_p)
    return NULL; // 'this' is the most downstream (sub-)stream

  // sanity check(s)
  ACE_ASSERT (module_2);
  ACE_ASSERT (module_2->writer ());

  iget_p = dynamic_cast<IGET_T*> (module_2->writer ());
  if (unlikely (!iget_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IGetP_T<Stream_IStream_T>>(0x%@), aborting\n"),
                ACE_TEXT (name_.c_str ()),
                module_2->name (),
                ACE_TEXT (module_2->writer ())));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::upstream (bool recurse_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::upstream"));

  // sanity check(s)
  if (!recurse_in)
    return inherited::linked_us_;

  // break recursion
  if (!inherited::linked_us_)
    return const_cast<OWN_TYPE_T*> (this);

  ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
  if (!istream_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStream_T>(0x%@), aborting\n"),
                ACE_TEXT (name_.c_str ()),
                inherited::linked_us_));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::lock (bool block_in,
                                         bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::lock"));

  int result = -1;

  if (inherited::linked_us_ && recurseUpstream_in)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (inherited::linked_us_);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  inherited::linked_us_));
      return false;
    } // end IF
    try {
      return ilock_p->lock (block_in,
                            recurseUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
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

  result =
    (block_in ? inherited::lock_.acquire () : inherited::lock_.tryacquire ());
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
  //            inherited::lock_.get_nesting_level (),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlock (bool unlock_in,
                                           bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlock"));

  if (inherited::linked_us_ && recurseUpstream_in)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (inherited::linked_us_);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  inherited::linked_us_));
      return -1;
    } // end IF
    try {
      return ilock_p->unlock (unlock_in,
                              recurseUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
    }
    return -1;
  } // end IF

  return inherited::lock_.release ();
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
ACE_SYNCH_MUTEX_T&
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::getLock (bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::getLock"));

  if (inherited::linked_us_ &&
      recurseUpstream_in)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (inherited::linked_us_);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  inherited::linked_us_));
      static ACE_SYNCH_MUTEX_T dummy;
      return dummy;
    } // end IF
    try {
      return ilock_p->getLock (recurseUpstream_in);
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::getLock(), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
    }
  } // end IF

  return inherited::lock_;
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::hasLock (bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::hasLock"));

  int result = -1;

  if (inherited::linked_us_)
  {
    typename ISTREAM_T::MODULE_T* module_p = NULL;
    TASK_T* task_p = NULL;
    ILOCK_T* ilock_p = NULL;

    result = inherited::linked_us_->top (module_p);
    if (unlikely ((result == -1) ||
                  !module_p))
    {
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("%s: no head module found: \"%m\", aborting\n"),
      //            ACE_TEXT (inherited::linked_us_->name ().c_str ())));
      return false; // *WARNING*: false negative
    } // end IF
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);

    ilock_p = dynamic_cast<ILOCK_T*> (task_p);
    if (!ilock_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name (),
                  task_p));
      return false; // *WARNING*: false negative
    } // end IF
    return ilock_p->hasLock (recurseUpstream_in);
  } // end IF

  ACE_thread_mutex_t& mutex_r = inherited::lock_.lock ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_hthread_t thread_h = NULL;
  ACE_OS::thr_self (thread_h);
  ACE_ASSERT (thread_h);
  return static_cast<bool> (ACE_OS::thr_cmp (mutex_r.OwningThread, thread_h));
#else
  return (static_cast<bool> (ACE_OS::thr_equal (mutex_r.__data.__owner, ACE_OS::thr_self ())));
#endif // ACE_WIN32 || ACE_WIN64
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::dump_state"));

  if (inherited::linked_us_)
  {
    Common_IDumpState* idump_state_p =
        dynamic_cast<Common_IDumpState*> (inherited::linked_us_);
    if (idump_state_p)
    {
      try {
        idump_state_p->dump_state ();
      } catch (...) {
        ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Common_IDumpState(), continuing\n"),
                    (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      }
    } // end IF
    return;
  } // end IF

  std::string stream_layout_string;
  STREAM_TASK_BASE_T* task_p = NULL;
  const MODULE_T* module_p = NULL, *module_2 = NULL;
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

    // mark asynchronous tasks with an asterisk
    task_p =
      dynamic_cast<STREAM_TASK_BASE_T*> (const_cast<MODULE_T*> (module_p)->writer ());
    if (task_p && task_p->get ())
      stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("*"));

    stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR (module_p->name ()));

    module_2 = const_cast<MODULE_T*> (module_p)->next ();
    if (module_2 && // *NOTE*: module_p might be an aggregator
        (ACE_OS::strcmp (module_2->name (),
                         ACE_TEXT (STREAM_MODULE_TAIL_NAME)) &&
         ACE_OS::strcmp (module_2->name (),
                         ACE_TEXT ("ACE_Stream_Tail"))))
      stream_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> ");
    if (!module_2) // *NOTE*: module_p is an aggregator
      stream_layout_string.append (ACE_TEXT (" |"));

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
       ++iterator)
  {
    heads_a = (*iterator)->next ();
    stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("\n"));
    stream_layout_string.append (indentation_i, '\t');
    for (Stream_ModuleListIterator_t iterator_2 = heads_a.begin ();
         iterator_2 != heads_a.end ();
         ++iterator_2)
    {
      stream_layout_string.append (ACE_TEXT ("| "));

      std::vector<Stream_IDistributorModule*> distributors_2;
      module_p = *iterator_2;
      do {
        ACE_ASSERT (module_p);

        // mark asynchronous tasks with an asterisk
        task_p =
          dynamic_cast<STREAM_TASK_BASE_T*> (const_cast<MODULE_T*> (module_p)->writer ());
        if (task_p && task_p->get ())
          stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("*"));

        stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR (module_p->name ()));

        if (ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                            ACE_TEXT (STREAM_MODULE_TAIL_NAME)) &&
            ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                            ACE_TEXT ("ACE_Stream_Tail")))
          stream_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

        idistributor_p =
          dynamic_cast<Stream_IDistributorModule*> (const_cast<MODULE_T*> (module_p)->writer ());
        if (idistributor_p)
          distributors_2.push_back (idistributor_p);

        if (!ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                             ACE_TEXT (STREAM_MODULE_TAIL_NAME)) ||
            !ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                             ACE_TEXT ("ACE_Stream_Tail")))
        {
          stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR (" |"));
          //Stream_ModuleListIterator_t iterator_3 = iterator_2;
          //if (++iterator_3 != heads_a.end ())
          //{
          //  stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("\n"));
          //  stream_layout_string.append (indentation_i, '\t');
          //} // end IF
          break;
        } // end IF

        module_p = const_cast<MODULE_T*> (module_p)->next ();
      } while (true);

      Stream_ModuleList_t heads_2;
      for (std::vector<Stream_IDistributorModule*>::const_iterator iterator_3 = distributors_2.begin ();
           iterator_3 != distributors_2.end ();
           ++iterator_3)
      {
        heads_2 = (*iterator_3)->next ();
        for (Stream_ModuleListIterator_t iterator_4 = heads_2.begin ();
             iterator_4 != heads_2.end ();
             ++iterator_4)
        {
          stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("\n"));
          stream_layout_string.append (indentation_i + 1, '\t');
          stream_layout_string += dump_state (*iterator_4,
                                              indentation_i + 1);
        } // end FOR
      } // end FOR

      Stream_ModuleListIterator_t iterator_3 = iterator_2;
      if (++iterator_3 != heads_a.end ())
      {
        stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR ("\n"));
        stream_layout_string.append (indentation_i, '\t');
      } // end IF
    } // end FOR
  } // end FOR
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s: \"%s\"\n"),
              ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
std::string
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::dump_state (MODULE_T* module_in,
                                               int indentation_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::dump_state"));

  // sanity check(s)
  ACE_ASSERT (module_in);

  std::string result = ACE_TEXT_ALWAYS_CHAR ("| ");

  std::vector<Stream_IDistributorModule*> distributors_a;
  Stream_IDistributorModule* idistributor_p = NULL;
  MODULE_T* module_p = module_in;
  STREAM_TASK_BASE_T* task_p = NULL;

  do
  { ACE_ASSERT (module_p);

    // mark asynchronous tasks with an asterisk
    task_p =
      dynamic_cast<STREAM_TASK_BASE_T*> (const_cast<MODULE_T*> (module_p)->writer ());
    if (task_p && task_p->get ())
      result.append (ACE_TEXT_ALWAYS_CHAR ("*"));

    result.append (ACE_TEXT_ALWAYS_CHAR (module_p->name ()));

    if (ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                        ACE_TEXT (STREAM_MODULE_TAIL_NAME)) &&
        ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                        ACE_TEXT ("ACE_Stream_Tail")))
      result += ACE_TEXT_ALWAYS_CHAR (" --> ");

    idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (const_cast<MODULE_T*> (module_p)->writer ());
    if (idistributor_p)
      distributors_a.push_back (idistributor_p);

    if (!ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                          ACE_TEXT (STREAM_MODULE_TAIL_NAME)) ||
        !ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                          ACE_TEXT ("ACE_Stream_Tail")))
    {
      result.append (ACE_TEXT_ALWAYS_CHAR (" |"));
      break;
    } // end IF

    module_p = const_cast<MODULE_T*> (module_p)->next ();
  } while (true);

  Stream_ModuleList_t heads_a;
  for (std::vector<Stream_IDistributorModule*>::const_iterator iterator = distributors_a.begin ();
       iterator != distributors_a.end ();
       ++iterator)
  {
    heads_a = (*iterator)->next ();
    for (Stream_ModuleListIterator_t iterator_2 = heads_a.begin ();
          iterator_2 != heads_a.end ();
          ++iterator_2)
    {
      result.append (ACE_TEXT_ALWAYS_CHAR ("\n"));
      result.append (indentation_in, '\t');
      result += dump_state (*iterator_2,
                            indentation_in + 1);
    } // end FOR
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
const typename SessionMessageType::DATA_T::DATA_T&
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::getR_2 () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::getR_2"));

  if (unlikely (inherited::linked_us_))
  {
    ISESSION_DATA_T* iget_p =
      dynamic_cast<ISESSION_DATA_T*> (inherited::linked_us_);
    if (!iget_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: upstream (was: %@) does not implement Common_IGetR_T<SESSION_DATA_T>, cannot retrieve session data, aborting\n"),
                  ACE_TEXT (name_.c_str ()),
                  inherited::linked_us_));
      static typename SessionMessageType::DATA_T::DATA_T dummy;
      return dummy;
    } // end IF

    return iget_p->getR_2 ();
  } // end IF

  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  return session_manager_p->getR (id_);
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
      state_.linked_ds_ = false;
    
      if (state_.module && !state_.moduleIsClone)
        if (unlikely (!remove (state_.module,
                               false,   // lock ?
                               false))) // reset ? (see above)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                      ACE_TEXT (name_.c_str ()),
                      state_.module->name ()));
      state_.module = NULL;
    } // end lock scope

    if (unlikely (!finalize (true))) // re-initialize head/tail modules
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::finalize(), aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    }// end IF

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<CONFIGURATION_T&> (configuration_in);
  ACE_ASSERT (configuration_->isInitialized_);
  ACE_ASSERT (configuration_->configuration_);

  // *TODO*: remove type inferences
  if (configuration_->configuration_->module)
  {
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
      ACE_ASSERT (!state_.module);
      if (configuration_->configuration_->cloneModule)
      {
        IMODULE_T* imodule_p =
            dynamic_cast<IMODULE_T*> (configuration_->configuration_->module);
        if (unlikely (!imodule_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                      ACE_TEXT (name_.c_str ()), configuration_->configuration_->module->name ()));
          return false;
        } // end IF

        try {
          state_.module = imodule_p->clone ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: caught exception in Stream_IModule_T::clone(), aborting\n"),
                      ACE_TEXT (name_.c_str ()), configuration_->configuration_->module->name ()));
        }
        if (unlikely (!state_.module))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to Stream_IModule_T::clone(), aborting\n"),
                      ACE_TEXT (name_.c_str ()), configuration_->configuration_->module->name ()));
          return false;
        } // end IF
        state_.moduleIsClone = true;
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("%s/%s: cloned final module (handle: 0x%@), clone handle is: 0x%@)\n"),
        //            ACE_TEXT (name_.c_str ()),
        //            configuration_->configuration->module->name (),
        //            configuration_->configuration->module,
        //            state_.module));
      } // end IF
      else
      {
        state_.module = configuration_->configuration_->module;
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
    (*iterator).second.first->notify = this;
    (*iterator).second.first->stream = this;
    //(*iterator).second.second->stateMachineLock = state_.stateMachineLock;
  } // end FOR

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
    state_.userData = configuration_->configuration_->userData;
  } // end lock scope

  // *TODO*: initialize the module handler configuration here as well
  initialize (configuration_->configuration_->setupPipeline,
              configuration_->configuration_->resetSessionData);
  if (unlikely (!isInitialized_))
  {
    if (state_.module && state_.moduleIsClone)
      delete state_.module;
    state_.module = NULL;
  } // end IF

  return isInitialized_;
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::collect"));

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (find (ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING),
                                        true,
                                        false));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (name_.c_str ()),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return false;
  } // end IF
  ISTATISTIC_T* statistic_impl_p =
    dynamic_cast<ISTATISTIC_T*> (module_p->writer ());
  if (unlikely (!statistic_impl_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Common_IStatistic_T> failed, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return false;
  } // end IF

  bool result_2 = false;
  // delegate to the statistic module
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (name_.c_str ())));
  }
  if (unlikely (!result_2))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (name_.c_str ())));
  else
  {
    // update session data as well
    SessionManagerType* session_manager_p =
      SessionManagerType::SINGLETON_T::instance ();
    ACE_ASSERT (session_manager_p);
    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (id_));
    ACE_ASSERT (session_data_r.lock);
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock, false);
      session_data_r.statistic = data_out;
      session_data_r.statistic.timeStamp = ACE_OS::gettimeofday ();
    } // end lock scope
  } // end ELSE

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::update (const ACE_Time_Value& interval_in)
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
                ACE_TEXT (name_.c_str ()),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  ISTATISTIC_T* statistic_impl_p =
    dynamic_cast<ISTATISTIC_T*> (module_p->writer ());
  if (unlikely (!statistic_impl_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Common_IStatistic_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  // delegate to the statistic module
  try {
    statistic_impl_p->update (interval_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::update(), continuing\n"),
                ACE_TEXT (name_.c_str ())));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
                ACE_TEXT (name_.c_str ()),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return;
  } // end IF
  ISTATISTIC_T* statistic_impl_p =
    dynamic_cast<ISTATISTIC_T*> (module_p->writer ());
  if (unlikely (!statistic_impl_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Common_IStatistic_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  // delegate to the statistic module
  try {
    statistic_impl_p->report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::report(), continuing\n"),
                ACE_TEXT (name_.c_str ())));
  }

  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    session_manager_p->getR (id_);
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %u] RUNTIME STATISTIC ***\n--> stream statistic @ %#D<--\n (data) messages: %u\n dropped messages: %u\n bytes total: %Q\n*** RUNTIME STATISTIC ***\\END\n"),
              session_data_r.sessionId,
              &session_data_r.lastCollectionTimeStamp,
              session_data_r.statistic.dataMessages,
              session_data_r.statistic.droppedFrames,
              session_data_r.statistic.bytes));
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::close (int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::close"));

  if (likely (flags_in == STREAM_T::M_DELETE))
    return inherited::close (flags_in);

  // sanity check(s)
  ACE_ASSERT (inherited::stream_head_);
  ACE_ASSERT (inherited::stream_tail_);

  int result = 0;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, -1);
    // Remove and cleanup all the intermediate modules
    while (inherited::stream_head_->next () != inherited::stream_tail_)
      if (this->pop (delete_ ? STREAM_T::M_DELETE : 0) == -1)
        result = -1;
  } // end lock scope

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::replace (const ACE_TCHAR* name_in,
                                            MODULE_T* module_in,
                                            int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::replace"));

  // step1: update the layout to reflect this change
  if (!layout_.replace (ACE_TEXT_ALWAYS_CHAR (name_in),
                        module_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Layout::replace(\"%s\"), aborting\n"),
                ACE_TEXT (name_.c_str ()),
                name_in));
    return -1;
  } // end IF

  // step2: update the stream itself
  int result = inherited::replace (name_in,
                                   module_in,
                                   flags_in);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::replace(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ()),
                name_in));
    return -1;
  } // end IF

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
ACE_Module<ACE_SYNCH_USE, TimePolicyType>*
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              StreamName,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::tail ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::tail"));

  Stream_ModuleList_t modules_a = layout_.list (true);
  if (likely (!modules_a.empty ()))
    return modules_a.back ();

  return inherited::tail ();
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::link (STREAM_T& upstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // *WARNING*: cannot reach the base class lock from here --> not thread-safe !
  // *TODO*: submit change request to the ACE maintainers

  // sanity check(s)
  if (unlikely (inherited::linked_us_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: already linked, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF

  int result = -1;
  ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (&upstream_in);
  bool reset_upstream = false;
  bool unlink_modules = false;
  StateType* state_p = NULL;
  ISESSION_DATA_T* iget_p = NULL;
  SESSION_DATA_T* session_data_p = NULL; // upstream-
  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  SESSION_DATA_T* session_data_2 = NULL; // this-

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
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, -1);
    // *NOTE*: ACE_Stream::linked_us_ is currently private
    //         --> retain another handle
    // *TODO*: modify ACE to make this a protected member
    inherited::linked_us_ = &upstream_in;
  } // end lock scope
  reset_upstream = true;

  // locate the module just above the upstreams' tail and this' 'top' module
  // (i.e. the module just below the head)
  MODULE_T* upstream_tail_module_p = upstream_in.STREAM_T::tail ();
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
                ACE_TEXT (name_.c_str ())));
    goto error;
  } // end IF

  // combine these two modules
  heading_module_p->reader ()->next (trailing_module_p->reader ());
  trailing_module_p->next (heading_module_p);
  trailing_module_p->writer ()->next (heading_module_p->writer ());
  unlink_modules = true;

  ////////////////////////////////////////

//  int nesting_level = unlock (true);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, -1);
    // (try to) update module configurations
    if (!istream_p)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: upstream (was: 0x%@) does not implement Stream_IStream_T, cannot update module configurations, continuing\n"),
                  ACE_TEXT (name_.c_str ()),
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
//                  ACE_TEXT (name_.c_str ()),
//                  inherited::linked_us_in));
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
//                  ACE_TEXT ("%s: upstream (was: 0x%@) does not implement Common_IGetR_T<SESSION_DATA_T>, cannot update session data, continuing\n"),
//                  ACE_TEXT (name_.c_str ()),
//                  inherited::linked_us_in));
      goto continue_3;
    } // end IF
    session_data_p = &const_cast<SESSION_DATA_T&> (iget_p->getR_2 ());
    ACE_ASSERT (session_manager_p);
    session_data_2 = &const_cast<SESSION_DATA_T&> (session_manager_p->getR (id_));
    ACE_ASSERT (session_data_2->lock);
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_2->lock, -1);
      ACE_ASSERT (session_data_p->lock);
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard_2, *session_data_p->lock, -1);
      // *IMPORTANT NOTE*: the idea here is to 'merge' the two datasets
      *session_data_p += *session_data_2;
    } // end lock scope

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
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, -1);
    inherited::linked_us_ = NULL;
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlink (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlink"));

  // *WARNING*: cannot reach the base class lock from here --> not thread-safe !
  // *TODO*: submit change request to the ACE maintainers

  // sanity check(s)
  if (unlikely (!inherited::linked_us_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no upstream; cannot unlink, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF

  int result = -1;
  // locate the module just above the upstreams' tail and this' 'top' module
  // (i.e. the module just below the head)
  MODULE_T* upstream_tail_module_p = inherited::linked_us_->STREAM_T::tail ();
  MODULE_T* trailing_module_p = inherited::linked_us_->head ();
  MODULE_T* heading_module_p = NULL;

  result = inherited::top (heading_module_p);
  if (unlikely ((result == -1) ||
                !heading_module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (trailing_module_p);

  MODULE_T* module_p = trailing_module_p->next ();
  do
  {
    // *IMPORTANT NOTE*: aggregated modules return NULL as next()
    if (!module_p->next () ||
        !ACE_OS::strcmp (module_p->next ()->name (),
                         heading_module_p->name ()) ||
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
  if (likely (trailing_module_p->next ()))
  {
    trailing_module_p->next (upstream_tail_module_p);
    trailing_module_p->writer ()->next (upstream_tail_module_p->writer ());
  } // end IF

  ////////////////////////////////////////

//  int nesting_level = unlock (true);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, -1);
    inherited::linked_us_ = NULL;

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
    result = inherited::lock_.acquire ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (name_.c_str ())));
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
    result = inherited::lock_.release ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    } // end IF
  } // end IF

  // *IMPORTANT NOTE*: the module might be on several streams and thus removed
  //                   several times --> reset its' next pointer the first time
  //                   around, otherwise it would point into void space
  // *WARNING*: do not invoke the overload here
  module_in->MODULE_T::next (NULL);

  if (likely (reset_in))
  {
    result = module_in->close (ACE_Module_Base::M_FLAGS_NOT_SET); // use the modules' set flags
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to ACE_Module::close(M_DELETE_NONE): \"%m\", aborting\n"),
                  ACE_TEXT (name_.c_str ()), module_in->name ()));
      return false;
    } // end IF

    // *NOTE*: ACE_Module::close() resets the modules' task handles
    //         --> call reset() so it can be reused
    imodule_p = dynamic_cast<IMODULE_T*> (module_in);
    if (likely (imodule_p))
      imodule_p->reset ();
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: removed module \"%s\"\n"),
//              ACE_TEXT (name_.c_str ()),
//              module_in->name ()));

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::finished (bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finished"));

  int result = -1;

  MODULE_T* module_p = NULL;
  ISTREAM_CONTROL_T* istreamcontrol_p = NULL;

  // foward upstream ?
  if (inherited::linked_us_ && recurseUpstream_in)
  {
    // delegate to the head module
    result = inherited::linked_us_->top (module_p);
    if (unlikely ((result == -1) ||
                  !module_p))
      goto _continue;

    istreamcontrol_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
    if (!istreamcontrol_p)
    {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: dynamic_cast<Stream_ISTREAM_CONTROL_T> failed, continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name ()));
      goto _continue;
    } // end IF

    try {
      istreamcontrol_p->finished ();
    } catch (...) {
      ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (inherited::linked_us_);
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
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    istreamcontrol_p->finished ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_ISTREAM_CONTROL_T::finished(), returning\n"),
                ACE_TEXT (name_.c_str ()),
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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
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
    module_p = inherited::stream_head_;
    if (module_p)
    {
      module_p = module_p->next ();
      if (module_p && (module_p != inherited::stream_tail_))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: not initialized - deactivating module(s)...\n"),
                    ACE_TEXT (name_.c_str ())));
        deactivateModules ();
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: not initialized - deactivating module(s)...DONE\n"),
                    ACE_TEXT (name_.c_str ())));
      } // end IF
    } // end IF
  } // end IF
  else
  {
    // handle final module (if any)
    // *NOTE*: this module may be shared by multiple stream instances, so it
    //         must not be close()d or reset here
    { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
      if (state_.module && !state_.moduleIsClone)
        if (unlikely (!remove (state_.module,
                               false,   // lock ?
                               false))) // reset ? (see above)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                      ACE_TEXT (name_.c_str ()),
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
  if (unlikely (!finalize (false))) // do not re-initialize head/tail modules
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::finalize(): \"%m\", continuing\n"),
                ACE_TEXT (name_.c_str ())));

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
          typename HandlerConfigurationType,
          typename SessionManagerType,
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
              HandlerConfigurationType,
              SessionManagerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::deactivateModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::deactivateModules"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->configuration_);
  SessionManagerType* session_manager_p =
    SessionManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (id_));
  typename SessionMessageType::DATA_T* session_data_container_p = NULL;
  typename SessionMessageType::DATA_T::DATA_T* session_data_p = &session_data_r;
  ACE_NEW_NORETURN (session_data_container_p,
                    typename SessionMessageType::DATA_T (session_data_p,
                                                         false)); // *NOTE*: do NOT delete the session data when the container is destroyed
  if (unlikely (!session_data_container_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  // allocate SESSION_END session message
  SessionMessageType* message_p = NULL;
  // *TODO*: remove type inference
  if (configuration_->configuration_->messageAllocator)
  {
    try { // *NOTE*: 0 --> session message
      message_p =
        static_cast<SessionMessageType*> (configuration_->configuration_->messageAllocator->malloc (0));
    } catch (...) {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(0), returning\n"),
                  ACE_TEXT (name_.c_str ())));

      // clean up
      session_data_container_p->decrease ();

      return;
    }
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      SessionMessageType (session_data_r.sessionId,
                                          STREAM_SESSION_MESSAGE_END,
                                          session_data_container_p, // *NOTE*: fire-and-forget session_data_container_p
                                          state_.userData,
                                          false)); // expedited ?
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate SessionMessageType: \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));

    // clean up
    session_data_container_p->decrease ();

    return;
  } // end IF
  // *TODO*: remove type inferences
  if (configuration_->configuration_->messageAllocator)
    message_p->initialize (session_data_r.sessionId,
                           STREAM_SESSION_MESSAGE_END,
                           session_data_container_p, // *NOTE*: fire-and-forget session_data_container_p
                           state_.userData,
                           false); // expedited ?

  // forward message
  int result = inherited::put (message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::put(): \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));

    // clean up
    message_p->release ();

    return;
  } // end IF
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          const char* StreamName,
//          typename ControlType,
//          typename NotificationType,
//          typename StatusType,
//          typename StateType,
//          typename ConfigurationType,
//          typename StatisticContainerType,
//          typename HandlerConfigurationType,
//          typename SessionManagerType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType>
//void
//Stream_Base_T<ACE_SYNCH_USE,
//              TimePolicyType,
//              StreamName,
//              ControlType,
//              NotificationType,
//              StatusType,
//              StateType,
//              ConfigurationType,
//              StatisticContainerType,
//              HandlerConfigurationType,
//              SessionManagerType,
//              ControlMessageType,
//              DataMessageType,
//              SessionMessageType>::unlinkModules ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlinkModules"));
//
//  MODULE_T* head_p = inherited::head ();
//  MODULE_T* next_p = NULL;
//  for (MODULE_T* module_p = head_p;
//       module_p;
//       )
//  {
//    next_p = module_p->next ();
//    module_p->next (NULL);
//    module_p = next_p;
//  } // end FOR
//  head_p->next (inherited::tail ());
//}
