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

#include <ace/Guard_T.h>
#include <ace/Log_Msg.h>
#include <ace/Module.h>
#include <ace/OS_Memory.h>

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::Stream_Module_MessageHandler_T ()
 : inherited ()
 , delete_ (false)
 , lock_ (NULL)
 , subscribers_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::Stream_Module_MessageHandler_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::~Stream_Module_MessageHandler_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::~Stream_Module_MessageHandler_T"));

  // clean up
  if (delete_)
  {
    delete lock_;
    delete subscribers_;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::initialize (SUBSCRIBERS_T* subscribers_in,
                                                             typename ACE_SYNCH_USE::RECURSIVE_MUTEX* lock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::initialize"));

  // sanity check(s)
  ACE_ASSERT ((subscribers_in && lock_in) ||
              (!subscribers_in && !lock_in));

  // clean up ?
  if (delete_)
  {
    delete_ = false;
    delete lock_;
    lock_ = NULL;
    delete subscribers_;
    subscribers_ = NULL;
  } // end IF

  delete_ = (!lock_in && !subscribers_in);
  if (lock_in)
    lock_ = lock_in;
  else
  {
    ACE_NEW_NORETURN (lock_,
                      typename ACE_SYNCH_USE::RECURSIVE_MUTEX (NULL, NULL));
    if (!lock_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

      // clean up
      delete_ = false;

      return;
    } // end IF
  } // end IF
  if (subscribers_in)
    subscribers_ = subscribers_in;
  else
  {
    ACE_NEW_NORETURN (subscribers_,
                      SUBSCRIBERS_T ());
    if (!subscribers_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

      // clean up
      delete_ = false;
      delete lock_;

      return;
    } // end IF
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (!inherited::sessionData_)
    return;
  ACE_ASSERT (lock_ && subscribers_);

//   try {
//     message_inout->getData ()->dump_state ();
//   } catch (...) {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
//   }

  // refer the data back to any subscriber(s)
  const SessionDataType& session_data_r =
      inherited::sessionData_->get ();

  // synch access
  {
    ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, *lock_);

    // *WARNING* if users unsubscribe() within the callback Bad Things (TM)
    // would happen, as the current iter would be invalidated
    // --> use a slightly modified for-loop (advance first and THEN invoke the
    // callback (*NOTE*: works for MOST containers...)
    // *NOTE*: this works due to the ACE_RECURSIVE_Thread_Mutex used as a lock...
    for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_->begin ();
         iterator != subscribers_->end ();
         )
    {
      try {
        // *TODO*: remove type inference
        (*iterator++)->notify (session_data_r.sessionID,
                               *message_inout);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_INotify_T::notify (), continuing\n")));
      }
    } // end FOR
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (lock_ && subscribers_);

  const SessionDataType* session_data_p = NULL;
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (inherited::sessionData_);
      session_data_p = &inherited::sessionData_->get ();

      // synch access
      {
        ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, *lock_);

        // *NOTE*: this works because the lock is recursive
        // *WARNING* if callees unsubscribe() within the callback bad things
        //           happen, as the current iterator is invalidated
        //           --> use a slightly modified for-loop (advance first before
        //               invoking the callback (works for MOST containers...)
        for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_->begin ();
             iterator != subscribers_->end ();
             )
        {
          try {
            // *TODO*: remove type inference
            (*iterator++)->start (session_data_p->sessionID,
                                  *session_data_p);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Common_INotify_T::start(), continuing\n")));
            goto error;
          }
        } // end FOR
      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      ACE_ASSERT (inherited::sessionData_);
      session_data_p = &inherited::sessionData_->get ();

      // synch access
      {
        ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, *lock_);

        // *NOTE*: this works because the lock is recursive
        // *WARNING* if callees unsubscribe() within the callback bad things
        //           happen, as the current iterator is invalidated
        //           --> use a slightly modified for-loop (advance first before
        //               invoking the callback (works for MOST containers...)
        for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_->begin ();
             iterator != subscribers_->end ();
             )
        {
          try {
            // *TODO*: remove type inference
            (*(iterator++))->end (session_data_p->sessionID);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Common_INotify_T::end(), continuing\n")));
          }
        } // end FOR
      } // end lock scope

      break;
    }
    default:
    {
      // *TODO*: this does not work in 'concurrent' scenarios
      if (inherited::sessionData_)
        session_data_p = &inherited::sessionData_->get ();

      // synch access
      {
        ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, *lock_);

        // *WARNING* callees unsubscribe()ing within the callback invalidate the
        //           iterator
        //           --> use a slightly modified for-loop (advance before
        //               invoking the callback; works for MOST containers)
        // *NOTE*: this works because the lock is recursive
        for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_->begin ();
             iterator != subscribers_->end ();
             )
        {
          try {
            // *TODO*: remove type inference
            (*(iterator++))->notify ((session_data_p ? session_data_p->sessionID
                                                     : static_cast<SessionIdType> (-1)),
                                     *message_inout);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Common_INotify_T::notify(), continuing\n")));
          }
        } // end FOR
      } // end lock scope

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
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::subscribe (INOTIFY_T* interfaceHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::subscribe"));

  // sanity check(s)
  ACE_ASSERT (lock_ && subscribers_);
  ACE_ASSERT (interfaceHandle_in);
  //if (!interfaceHandle_in)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("invalid argument (was: %@), returning\n"),
  //              interfaceHandle_in));
  //  return;
  //} // end IF

  // synch access to subscribers
  ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, *lock_);

  subscribers_->push_back (interfaceHandle_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::unsubscribe (INOTIFY_T* interfaceHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::unsubscribe"));

  // sanity check(s)
  ACE_ASSERT (lock_ && subscribers_);
  ACE_ASSERT (interfaceHandle_in);

  // synch access to subscribers
  ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, *lock_);

  SUBSCRIBERS_ITERATOR_T iterator = subscribers_->begin ();
  for (;
       iterator != subscribers_->end ();
       iterator++)
    if ((*iterator) == interfaceHandle_in)
      break;

  if (iterator != subscribers_->end ())
    subscribers_->erase (iterator);
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: %@), continuing\n"),
                interfaceHandle_in));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
bool
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataType>::postClone (ACE_Module<ACE_SYNCH_USE,
                                                            TimePolicyType>* clone_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::postClone"));

  // sanity check(s)
  ACE_ASSERT (clone_in);

  OWN_TYPE_T* message_handler_impl_p =
    dynamic_cast<OWN_TYPE_T*> (clone_in->writer ());
  if (!message_handler_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_MessageHandler_T> failed, aborting\n")));
    return false;
  } // end IF

  message_handler_impl_p->initialize (subscribers_,
                                      lock_);

  return true;
}
