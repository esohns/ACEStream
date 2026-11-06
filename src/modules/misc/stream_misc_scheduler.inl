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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "avrt.h"
#endif // ACE_WIN32 || ACE_WIN64

#include <chrono>
#include <thread>

#include "ace/Log_Msg.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_misc_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::Stream_Module_VideoScheduler_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , isFirstDispatchingThread_ (true)
 , queue_ (0,     // max # slots --> unlimited
           NULL)  // notification handle
 , queue_2_ (0,    // max # slots --> unlimited
             NULL) // notification handle
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , task_ (ACE_INVALID_HANDLE)
 , task_2_ (ACE_INVALID_HANDLE)
#endif // ACE_WIN32 || ACE_WIN64
 , isPCRInitialized_ (false)
 , initialPCR_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::Stream_Module_VideoScheduler_T"));

  // set group id for worker thread(s)
  // *TODO*: pass this in from outside
  inherited::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::handleControlMessage"));

  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      else if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u inbound data messages\n"),
                    inherited::mod_->name (),
                    result));

      result = queue_2_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      else if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u outbound data messages\n"),
                    inherited::mod_->name (),
                    result));

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::handleDataMessage"));

  passMessageDownstream_out = false;

  int result = queue_.enqueue_tail (message_inout, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::enqueue_tail(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF

  return;

error:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::configuration_);

  bool high_priority_b = false;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      else if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u inbound data messages\n"),
                    inherited::mod_->name (),
                    result));

      result = queue_2_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      else if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u outbound data messages\n"),
                    inherited::mod_->name (),
                    result));

      high_priority_b = true;
      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *NOTE*: this prevents a race condition in svc()
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        inherited::threadCount_ = 2;
        bool lock_activate_was_b = inherited::TASK_BASE_T::TASK_BASE_T::lockActivate_;
        inherited::lockActivate_ = false;
        if (unlikely (inherited::open (NULL) == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_Task_Base_T::open(), aborting\n"),
                      inherited::mod_->name ()));
          inherited::lockActivate_ = lock_activate_was_b;
          inherited::threadCount_ = 0;
          goto error;
        } // end IF
        inherited::lockActivate_ = lock_activate_was_b;
        inherited::threadCount_ = 0;
        ACE_ASSERT (!inherited::threadIds_.empty ());
      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    //case STREAM_SESSION_MESSAGE_RESIZE:
    //{
    //  // sanity check(s)
    //  ACE_ASSERT (inherited::configuration_);
    //  if (!inherited::configuration_->handleResize)
    //    break;

    //  resizeOccured_ = true;
    //  unsigned int result = queue_.flush (false); // flush all data messages
    //  if (unlikely (result == static_cast<unsigned int> (-1)))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", returning\n"),
    //                inherited::mod_->name ()));
    //    return;
    //  } // end IF
    //  else if (result > 0)
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("%s: resizing: flushed %u inbound data messages\n"),
    //                inherited::mod_->name (),
    //                result));

    //  result = queue_2_.flush (false); // flush all data messages
    //  if (unlikely (result == static_cast<unsigned int> (-1)))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", returning\n"),
    //                inherited::mod_->name ()));
    //    return;
    //  } // end IF
    //  else if (result > 0)
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("%s: resizing: flushed %u outbound data messages\n"),
    //                inherited::mod_->name (),
    //                result));

    //  break;
    //}
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      //if (inherited::configuration_->waitForDataOnEnd)
      //{
      //  queue_.waitForIdleState ();
      //  queue_2_.waitForIdleState ();
      //  // wait for the next (i.e. at least one-) dispatch cycle to complete
      //  int result;
      //  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
      //    result = condition_.wait (NULL);
      //    if (unlikely (result == -1))
      //      ACE_DEBUG ((LM_ERROR,
      //                  ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::wait(): \"%m\", continuing\n"),
      //                  inherited::mod_->name ()));
      //  } // end lock scope
      //} // end IF

      // *NOTE*: the inbound-processing thread will stop() the outbound-processing
      //         thread (see below)
      // *TODO*: waiting here is problematic, because this blocks the source
      //         module; OTOH failing to wait here will rip state out from under
      //         any downstream module(s)...
      stop (true,                                                                              // inbound- ?
            true,                                                                              // wait ?
            high_priority_b ? high_priority_b : !inherited::configuration_->waitForDataOnEnd); // high priority ?

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
bool
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    isFirstDispatchingThread_ = true;
    isPCRInitialized_ = false;
    initialPCR_ = 0;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::stop (bool stopInbound_in,
                                                    bool waitForCompletion_in,
                                                    bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::stop"));

  ACE_Message_Block* message_block_p = NULL;
  int result;

  if (unlikely (!stopInbound_in))
    goto outbound;

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

  result = (highPriority_in ? queue_.enqueue_head (message_block_p, NULL) :
                              queue_.enqueue_tail (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::%s(): \"%m\", continuing\n"),
                inherited::mod_->name (),
                (highPriority_in ? ACE_TEXT ("enqueue_head") : ACE_TEXT ("enqueue_tail"))));
    message_block_p->release (); message_block_p = NULL;
  } // end IF  
  message_block_p = NULL;

  goto continue_;

outbound:
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

  result = (highPriority_in ? queue_2_.enqueue_head (message_block_p, NULL) :
                              queue_2_.enqueue_tail (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::%s(): \"%m\", continuing\n"),
                inherited::mod_->name (),
                (highPriority_in ? ACE_TEXT ("enqueue_head") : ACE_TEXT ("enqueue_tail"))));
    message_block_p->release (); message_block_p = NULL;
  } // end IF  

continue_:
  if (waitForCompletion_in)
  {
    Common_ITask* itask_p = this;
    itask_p->wait (true); // wait for message queue(s) ?
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
int
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  ACE_Message_Block* message_block_p = NULL;
  DataMessageType* message_p = NULL;
  int result = -1;
  int error = -1;
  bool dispatch_outbound_queue_b = false;

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    if (isFirstDispatchingThread_)
    {
      isFirstDispatchingThread_ = false;
      dispatch_outbound_queue_b = true;
    } // end IF
  } // end lock scope
  if (dispatch_outbound_queue_b)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (unlikely (inherited::configuration_->delayConfiguration->isMultimediaTask))
    {
      DWORD task_index_i = 0;
      ACE_ASSERT (task_2_ == ACE_INVALID_HANDLE);
      task_2_ =
        AvSetMmThreadCharacteristics (TEXT (STREAM_LIB_WASAPI_RENDER_DEFAULT_TASKNAME),
                                      &task_index_i);
      if (unlikely (!task_2_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to AvSetMmThreadCharacteristics(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (STREAM_LIB_WASAPI_RENDER_DEFAULT_TASKNAME),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
        result = -1;
        goto done_2;
      } // end IF
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64

    result = svc_2 ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (unlikely (inherited::configuration_->delayConfiguration->isMultimediaTask))
    { ACE_ASSERT (task_2_ != ACE_INVALID_HANDLE);
      AvRevertMmThreadCharacteristics (task_2_);
      task_2_ = ACE_INVALID_HANDLE;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64

    goto done_2;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (inherited::configuration_->delayConfiguration->isMultimediaTask))
  {
    DWORD task_index_i = 0;
    ACE_ASSERT (task_ == ACE_INVALID_HANDLE);
    task_ =
      AvSetMmThreadCharacteristics (TEXT (STREAM_LIB_WASAPI_RENDER_DEFAULT_TASKNAME),
                                    &task_index_i);
    if (unlikely (!task_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to AvSetMmThreadCharacteristics(\"%s\"): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (STREAM_LIB_WASAPI_RENDER_DEFAULT_TASKNAME),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
      result = -1;
      goto done;
    } // end IF
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  do
  {
    result = queue_.dequeue_head (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      //if (error == ETIME)
      //  goto continue_;

      if (unlikely (error != ESHUTDOWN))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Message_Queue_T::dequeue_head(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
      result = 0; // OK, queue has been deactivate()d
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      message_block_p->release (); message_block_p = NULL;
      break; // done
    } // end IF

    message_p = static_cast<DataMessageType*> (message_block_p);
    dispatch (message_p);
    message_block_p = NULL;
  } while (true);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (inherited::configuration_->delayConfiguration->isMultimediaTask))
  { ACE_ASSERT (task_ != ACE_INVALID_HANDLE);
    AvRevertMmThreadCharacteristics (task_);
    task_ = ACE_INVALID_HANDLE;
  } // end IF
done:
#endif // ACE_WIN32 || ACE_WIN64

  stop (false,                                         // inbound ?
        false,                                         // wait ?
        !inherited::configuration_->waitForDataOnEnd); // high priority ?
  inherited::thr_mgr ()->join (inherited::threadIds_[0].id (), NULL);

done_2:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
int
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::svc_2 (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::svc_2"));

  ACE_Message_Block* message_block_p = NULL;
  int result, result_2;
  int error;

  do
  {
    result = queue_2_.dequeue_head (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      //if (error == ETIME)
      //  goto continue_;

      if (unlikely (error != ESHUTDOWN))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Message_Queue_T::dequeue_head(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        return -1;
      } // end IF
      result = 0; // OK, queue has been deactivate()d
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      message_block_p->release (); message_block_p = NULL;
      result = 0; // OK, received STOP message
      break;
    } // end IF

    result_2 = inherited::put_next (message_block_p, NULL);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      break;
    } // end IF
    message_block_p = NULL;
  } while (true);

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::dispatch (DataMessageType* message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::dispatch"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);
  ACE_ASSERT (inherited::configuration_->masterClock);
  ACE_ASSERT (message_in);

  int result;
  typename DataMessageType::DATA_T& message_data_container_r =
    const_cast<typename DataMessageType::DATA_T&> (message_in->getR ());
  typename DataMessageType::DATA_T::DATA_T& message_data_r =
    const_cast<typename DataMessageType::DATA_T::DATA_T&> (message_data_container_r.getR ());
  // *TODO*: remove type inference(s)
  updatePCR (message_data_r.timeStamps.PCR);
  double elapsed_video_seconds_d;
  double elapsed_audio_seconds_d =
    inherited::configuration_->masterClock->getTimeInSeconds ();
  double drift_d;

  switch (inherited::configuration_->delayConfiguration->mode)
  {
    case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_DTS:
    {
      // *TODO*: remove type inference(s)
      elapsed_video_seconds_d =
        getElapsedTimeInSeconds (message_data_r.timeStamps.DTS);
      break;
    }
    case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_PTS:
    {
      // *TODO*: remove type inference(s)
      elapsed_video_seconds_d =
        getElapsedTimeInSeconds (message_data_r.timeStamps.PTS);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown delay mode (was: %d), returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->delayConfiguration->mode));
      message_in->release ();
      return;
    }
  } // end SWITCH

  drift_d = elapsed_video_seconds_d - elapsed_audio_seconds_d;
  if (drift_d > STREAM_MISC_DEFAULT_SCHEDULER_MIN_SYNC_THRESHOLD_D)
  { // --> video is ahead of audio; sleep a bit to let audio catch up
    std::chrono::duration<double> sleep_duration (std::max (0.0, std::min (drift_d * 0.5, STREAM_MISC_DEFAULT_SCHEDULER_MAX_SLEEP_PER_FRAME_D)));
    std::this_thread::sleep_for (std::chrono::duration_cast<std::chrono::microseconds> (sleep_duration));
  } // end IF
  else if (drift_d < -STREAM_MISC_DEFAULT_SCHEDULER_MAX_DROP_THRESHOLD_D)
  { // --> audio is ahead of video; let video catch up
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("%s: dropping video frame to catch up with audio (drift: %.3f seconds)\n"),
    //            inherited::mod_->name (),
    //            drift_d));
    //message_in->release ();
    //return;
  } // end ELSE IF

  result = queue_2_.enqueue_tail (message_in,
                                  NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::enqueue_tail(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_in->release ();
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::updatePCR (ACE_UINT64 PCR_27MHz_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::updatePCR"));

  if (unlikely (!isPCRInitialized_))
  { isPCRInitialized_ = true;
    initialPCR_ = PCR_27MHz_in / 300; // convert to 90kHz
    return;
  } // end IF
  if (unlikely ((PCR_27MHz_in / 300) < initialPCR_))
    initialPCR_ = PCR_27MHz_in / 300; // PCR restarted / wrapped
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
double
Stream_Module_VideoScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::getElapsedTimeInSeconds (ACE_UINT64 timeStamp_90kHz_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_VideoScheduler_T::getElapsedTimeInSeconds"));

  // sanity check(s)
  if (unlikely (!isPCRInitialized_))
    return 0.0;
  if (unlikely (timeStamp_90kHz_in < initialPCR_))
    return 0.0;

  return (timeStamp_90kHz_in - initialPCR_) / 90000.0;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
Stream_Module_AudioScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::Stream_Module_AudioScheduler_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 //, frameSize_ (0)
 , isFirstDataMessage_ (true)
 , sampleRate_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_AudioScheduler_T::Stream_Module_AudioScheduler_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_AudioScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_AudioScheduler_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->masterClock);
  //ACE_ASSERT (frameSize_);
  ACE_ASSERT (sampleRate_);

  if (unlikely (isFirstDataMessage_))
  { isFirstDataMessage_ = false;
    inherited::configuration_->masterClock->initialize (Stream_MasterClock::CLOCK_MODE_STOPWATCH,
                                                        sampleRate_);
  } // end IF
  //int number_of_samples_i = message_inout->total_length () / frameSize_;
  //inherited::configuration_->masterClock->advance (number_of_samples_i);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_AudioScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_AudioScheduler_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->masterClock);
      ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      int sample_rate_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      struct tWAVEFORMATEX* waveformatex_p = NULL;
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      struct Stream_MediaFramework_V4L_MediaType media_type_2;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);
      sample_rate_i = waveformatex_p->nSamplesPerSec;
      //frameSize_ = waveformatex_p->nBlockAlign;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      sample_rate_i = media_type_s.rate;
      //frameSize_ = (snd_pcm_format_width (media_type_s.format) / 8) *
      //             media_type_s.channels;
#endif // ACE_WIN32 || ACE_WIN64
      sampleRate_ = sample_rate_i;
      //inherited::configuration_->masterClock->initialize (sample_rate_i);

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
bool
Stream_Module_AudioScheduler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType,
                               UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_AudioScheduler_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    //frameSize_ = 0;
    isFirstDataMessage_ = true;
    sampleRate_ = 0;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}
