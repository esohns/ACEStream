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

#include "ace/Log_Msg.h"
#include "ace/Message_Queue.h"
#include "ace/Task.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   SessionDataType>::Stream_Miscellaneous_Distributor_T (ISTREAM_T* stream_in)
#else
                                   SessionDataType>::Stream_Miscellaneous_Distributor_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , branches_ ()
 , heads_ ()
 , modules_ ()
 , queues_ ()
 , data_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::Stream_Miscellaneous_Distributor_T"));

  inherited::threadCount_ = 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::forward (ACE_Message_Block* messageBlock_in,
                                                              bool dispose_in,
                                                              bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::forward"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    for (THREAD_TO_QUEUE_CONST_ITERATOR_T iterator = queues_.begin ();
         iterator != queues_.end ();
         ++iterator)
    { ACE_ASSERT ((*iterator).second);
      ACE_ASSERT (!message_block_p);
      message_block_p = messageBlock_in->duplicate ();
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto continue_;
      } // end IF

      // iff this is a session message, update its' data
      switch (messageBlock_in->msg_type ())
      {
        case ACE_Message_Block::MB_STOP: // see: stop()
        case STREAM_MESSAGE_CONTROL:
        case STREAM_MESSAGE_DATA:
        case STREAM_MESSAGE_OBJECT:
          break;
        case STREAM_MESSAGE_SESSION:
        {
          // retrieve branch session data
          QUEUE_TO_MODULE_CONST_ITERATOR_T iterator_2 =
              modules_.find ((*iterator).second);
          ACE_ASSERT (iterator_2 != modules_.end ());
          ACE_ASSERT ((*iterator_2).second);
          HEAD_TO_SESSIONDATA_ITERATOR_T iterator_3 =
              data_.find ((*iterator_2).second);
          SessionMessageType* session_message_p =
            static_cast<SessionMessageType*> (messageBlock_in);
          // *NOTE*: if some upstream initialization failed and the task sends
          //         'abort', 'this' has not been initialized yet
          if (likely (iterator_3 != data_.end ()))
          { ACE_ASSERT ((*iterator_3).second);
            (*iterator_3).second->increase ();
            session_message_p->setP ((*iterator_3).second);
          } // end IF
          else
          {
            typename SessionMessageType::DATA_T& session_data_container_r =
              const_cast<typename SessionMessageType::DATA_T&> (session_message_p->getR ());
            session_data_container_r.increase ();
          } // end ELSE

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown message (type was: \"%s\"), returning\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
          goto continue_;
        }
      } // end SWITCH

      result =
        (highPriority_in ? (*iterator).second->enqueue_head (message_block_p, NULL)
                         : (*iterator).second->enqueue_tail (message_block_p, NULL));
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue_Base::%s(): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    (highPriority_in ? ACE_TEXT ("enqueue_head") : ACE_TEXT ("enqueue_tail"))));
        message_block_p->release (); message_block_p = NULL;
        goto continue_;
      } // end IF
      message_block_p = NULL;
    } // end FOR
  } // end lock scope

continue_:
  if (unlikely (dispose_in))
    messageBlock_in->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::handleControlMessage"));

  bool is_high_priority_b = false;
  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      QUEUE_TO_MODULE_CONST_ITERATOR_T iterator_2;
      BRANCH_TO_HEAD_CONST_ITERATOR_T iterator_3;
      Stream_IMessageQueue* iqueue_p = NULL;
      unsigned int flushed_messages_i = 0;
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        for (THREAD_TO_QUEUE_CONST_ITERATOR_T iterator = queues_.begin ();
             iterator != queues_.end ();
             ++iterator)
        { // retrieve branch name
          iterator_2 = modules_.find ((*iterator).second);
          ACE_ASSERT (iterator_2 != modules_.end ());
          iterator_3 =
            std::find_if (heads_.begin (), heads_.end (),
                          std::bind2nd (BRANCH_TO_HEAD_MAP_FIND_S (),
                                        (*iterator_2).second));
          ACE_ASSERT (iterator_3 != heads_.end ());

          // flush data messages
          ACE_ASSERT ((*iterator).second);
          iqueue_p = dynamic_cast<Stream_IMessageQueue*> ((*iterator).second);
          ACE_ASSERT (iqueue_p);
          flushed_messages_i = iqueue_p->flush (false); // flush session messages ?
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s/%s: aborting: flushed %u data messages\n"),
                      inherited::mod_->name (), ACE_TEXT ((*iterator_3).first.c_str ()),
                      flushed_messages_i));
        } // end FOR
      } // end lock scope
      is_high_priority_b = true;
      break;
    }
    default:
      break;
  } // end SWITCH

  forward (&message_in,
           false,               // dispose original ?
           is_high_priority_b); // high priority ?
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
      typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;

      // *NOTE*: clone (!) the session data for each processing branch
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        for (BRANCH_TO_HEAD_CONST_ITERATOR_T iterator = heads_.begin ();
             iterator != heads_.end ();
             ++iterator)
        {
          ACE_NEW_NORETURN (session_data_p,
                            typename SessionMessageType::DATA_T::DATA_T ());
          if (unlikely (!session_data_p))
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                        inherited::mod_->name ()));
            goto error;
          } // end IF
          *session_data_p = session_data_r;
          ACE_NEW_NORETURN (session_data_container_p,
                            typename SessionMessageType::DATA_T (session_data_p));
          if (unlikely (!session_data_container_p))
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                        inherited::mod_->name ()));
            delete session_data_p; session_data_p = NULL;
            goto error;
          } // end IF
          session_data_p = NULL;

          ACE_ASSERT ((*iterator).second);
          data_.insert (std::make_pair ((*iterator).second,
                                        session_data_container_p));
          session_data_container_p = NULL;
        } // end FOR
      } // end lock scope

      forward (message_inout,
               false,         // dispose original ?
               false);        // high priority ?

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      forward (message_inout,
               false,         // dispose original ?
               false);        // high priority ?

      stop (true,   // wait ?
            false); // high priority ?

      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        for (HEAD_TO_SESSIONDATA_ITERATOR_T iterator = data_.begin ();
             iterator != data_.end ();
             ++iterator)
        { ACE_ASSERT ((*iterator).second);
          (*iterator).second->decrease ();
        } // end FOR
        data_.clear ();
      } // end lock scope

      break;
    }
    default:
    {
      forward (message_inout,
               false,
               false);

      break;
    }
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
bool
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::initialize (const Stream_Branches_t& branches_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!branches_in.empty ());

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, false);
    branches_ = branches_in;
  } // end lock scope

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
bool
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::push (Stream_Module_t* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::push"));

  // sanity check(s)
  ACE_ASSERT (module_in);

  typename inherited::MESSAGE_QUEUE_T* queue_p = NULL;
  ACE_NEW_NORETURN (queue_p,
                    typename inherited::MESSAGE_QUEUE_T (STREAM_QUEUE_MAX_SLOTS, // max # slots
                                                         NULL));                 // notification handle
  if (unlikely (!queue_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // *NOTE*: this prevents a race condition in svc()
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, false);
    ACE_ASSERT (!branches_.empty ());
    inherited::threadCount_ = 1;
    bool lock_activate_was_b = inherited::TASK_BASE_T::TASK_BASE_T::lockActivate_;
    inherited::lockActivate_ = false;
    if (unlikely (!inherited::start (NULL)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_Task_Base_T::start(), aborting\n"),
                  inherited::mod_->name ()));
      inherited::lockActivate_ = lock_activate_was_b;
      inherited::threadCount_ = 0;
      return false;
    } // end IF
    inherited::lockActivate_ = lock_activate_was_b;
    inherited::threadCount_ = 0;
    ACE_ASSERT (!inherited::threadIds_.empty ());
    queues_.insert (std::make_pair (inherited::threadIds_.back ().id (),
                                    queue_p));
    modules_.insert (std::make_pair (queue_p, module_in));
    ACE_ASSERT (!branches_.empty ());
    std::pair<BRANCH_TO_HEAD_ITERATOR_T, bool> result_s =
        heads_.insert (std::make_pair (branches_.front (),
                                       module_in));
    ACE_ASSERT (result_s.second);
    branches_.pop_front ();
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: pushed branch \"%s\" head module \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT ((*(result_s.first)).first.c_str ()),
                module_in->name ()));
  } // end lock scope

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
bool
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::pop (Stream_Module_t* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::pop"));

  ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, false);

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_t*
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::head (const std::string& branchName_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::head"));

  BRANCH_TO_HEAD_CONST_ITERATOR_T iterator;
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, NULL);
    iterator = heads_.find (branchName_in);
    ACE_ASSERT (iterator != heads_.end ());
    return (*iterator).second;
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
std::string
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::branch (Stream_Module_t* headModule_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::branch"));

  // initialize return value(s)
  std::string return_value;

  // sanity check(s)
  ACE_ASSERT (headModule_in);

  BRANCH_TO_HEAD_CONST_ITERATOR_T iterator;
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, return_value);
    iterator =
        std::find_if (heads_.begin (), heads_.end (),
                      std::bind2nd (BRANCH_TO_HEAD_MAP_FIND_S (),
                                    headModule_in));
    if (likely (iterator != heads_.end ()))
      return_value = (*iterator).first;
  } // end lock scope

  return return_value;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
bool
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::has (const std::string& branchName_in,
                                                          unsigned int& index_out) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::has"));

  // initialize return value(s)
  index_out = 0;

  ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, false);

  Stream_BranchesIterator_t iterator =
      std::find (branches_.begin (), branches_.end (), branchName_in);
  if (iterator != branches_.end ())
  {
    index_out = std::distance (branches_.begin (), iterator);
    return true;
  } // end IF
  ACE_DEBUG ((LM_WARNING,
              ACE_TEXT ("%s: branch (was: %s) index not found (head module already push()ed ?), continuing\n"),
              inherited::mod_->name (),
              ACE_TEXT (branchName_in.c_str ())));

  // *NOTE*: this is a map search; the index may be wrong...
  return (heads_.find (branchName_in) != heads_.end ());
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_ModuleList_t
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::next () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::next"));

  // initialize return value(s)
  Stream_ModuleList_t return_value;

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, return_value);
    for (BRANCH_TO_HEAD_CONST_ITERATOR_T iterator = heads_.begin ();
         iterator != heads_.end ();
         ++iterator)
      return_value.push_back ((*iterator).second);
  } // end lock scope

  return return_value;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::onLink (ACE_Module_Base* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::onLink"));

  ACE_UNUSED_ARG (module_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::onUnlink (ACE_Module_Base* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::onUnlink"));

  ACE_UNUSED_ARG (module_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::stop (bool waitForCompletion_in,
                                                           bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::stop"));

  ACE_Message_Block* message_block_p = NULL;

  // enqueue a control message
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  forward (message_block_p,
           true, // dispose of message ?
           highPriority_in);
  message_block_p = NULL;

  if (waitForCompletion_in)
    wait (true); // wait for message queue(s) ?
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::idle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::idle"));

  typename inherited::MESSAGE_QUEUE_T* queue_p = NULL;

retry:
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    for (THREAD_TO_QUEUE_CONST_ITERATOR_T iterator = queues_.begin ();
         iterator != queues_.end ();
         ++iterator)
    { ACE_ASSERT ((*iterator).second);
      if (!(*iterator).second->is_empty ())
      { // *NOTE*: wait outside of the lock scope
        queue_p = (*iterator).second;
        goto wait;
      } // end IF
    } // end FOR
  } // end lock scope

  return;

wait:
  ACE_ASSERT (queue_p);
  queue_p->waitForIdleState ();
  goto retry;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::wait (bool waitForMessageQueues_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::wait"));

  int result = -1;
  ACE_Time_Value one_second (1, 0);
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);

  // step1: wait for the message queue to empty
  if (likely (waitForMessageQueues_in))
    this_p->idle ();

retry:
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    if (!queues_.empty ())
      goto wait;
  } // end lock scope

  return;

wait:
  result = ACE_OS::sleep (one_second);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_OS::sleep(%#T), returning\n"),
                inherited::mod_->name (),
                &one_second));
    return;
  } // end IF
  goto retry;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
int
Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Distributor_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64

  ACE_Message_Block* message_block_p = NULL;
  int result = 0;
  int result_2 = -1;
  typename inherited::MESSAGE_QUEUE_T* message_queue_p = NULL;
  MODULE_T* module_p = NULL;
  std::string branch_string;
  ACE_Task_Base* task_p = NULL;

  // sanity check(s)
  THREAD_TO_QUEUE_CONST_ITERATOR_T iterator;
  QUEUE_TO_MODULE_CONST_ITERATOR_T iterator_2;
  BRANCH_TO_HEAD_CONST_ITERATOR_T iterator_3;
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    iterator = queues_.find (ACE_OS::thr_self ());
    ACE_ASSERT (iterator != queues_.end ());
    message_queue_p = (*iterator).second;

    iterator_2 = modules_.find (message_queue_p);
    ACE_ASSERT (iterator_2 != modules_.end ());
    module_p = (*iterator_2).second;
    iterator_3 =
        std::find_if (heads_.begin (), heads_.end (),
                      std::bind2nd (BRANCH_TO_HEAD_MAP_FIND_S (),
                                    (*iterator_2).second));
    ACE_ASSERT (iterator_3 != heads_.end ());
    branch_string = (*iterator_3).first;
  } // end lock scope
  ACE_ASSERT (message_queue_p);
  ACE_ASSERT (module_p);
  task_p = module_p->writer ();
  ACE_ASSERT (task_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: worker thread (id: %t, group: %d, branch: \"%s\") starting\n"),
              inherited::mod_->name (),
              inherited::grp_id_,
              ACE_TEXT (branch_string.c_str ())));

  do
  {
    message_block_p = NULL;
    result_2 = message_queue_p->dequeue (message_block_p,
                                         NULL);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Message_Queue_Base::dequeue(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      result = -1;
      goto done;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        message_block_p->release (); message_block_p = NULL;
        goto done;
      }
      default:
      {
        result_2 = task_p->put (message_block_p, NULL);
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Task_Base::put(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          message_block_p->release (); message_block_p = NULL;
          result = -1;
          goto done;
        } // end IF

        break;
      }
    } // end SWITCH
  } while (true);

done:
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    iterator = queues_.find (ACE_OS::thr_self ());
    ACE_ASSERT (iterator != queues_.end ());
    ACE_ASSERT ((*iterator).second);
    iterator_2 = modules_.find ((*iterator).second);
    ACE_ASSERT (iterator_2 != modules_.end ());
    iterator_3 =
        std::find_if (heads_.begin (), heads_.end (),
                      std::bind2nd (BRANCH_TO_HEAD_MAP_FIND_S (),
                                    (*iterator_2).second));
    ACE_ASSERT (iterator_3 != heads_.end ());
    heads_.erase (iterator_3);
    modules_.erase (iterator_2);
    queues_.erase (iterator);
  } // end lock scope

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: worker thread (id: %t) leaving\n"),
              inherited::mod_->name ()));

  return result;
}
