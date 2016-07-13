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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataContainerType>
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataContainerType>::Stream_Module_MessageHandler_T ()
 : inherited ()
 , delete_ (false)
 , lock_ (NULL)
 , subscribers_ (NULL)
 , sessionData_ (NULL)
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
          typename SessionDataContainerType>
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataContainerType>::~Stream_Module_MessageHandler_T ()
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
          typename SessionDataContainerType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataContainerType>::initialize (SUBSCRIBERS_T* subscribers_in,
                                                                      ACE_SYNCH_RECURSIVE_MUTEX* lock_in)
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
                      ACE_SYNCH_RECURSIVE_MUTEX (NULL, NULL));
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

  sessionData_ = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataContainerType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (lock_ && subscribers_);

//   try {
//     message_inout->getData ()->dump_state ();
//   } catch (...) {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
//   }

  // refer the data back to any subscriber(s)

  // synch access
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (*lock_);

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
        (*iterator++)->notify ((sessionData_ ? sessionData_->sessionID : 0),
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
          typename SessionDataContainerType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (lock_ && subscribers_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);

      // forward the session data to any subscriber(s)
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      sessionData_ =
          &const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.get ());

      // synch access
      {
        ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (*lock_);

        // *WARNING* if users unsubscribe() within the callback Bad Things (TM)
        // would happen, as the current iterator would be invalidated
        // --> use a slightly modified for-loop (advance first and THEN invoke
        //     the callback (*NOTE*: works for MOST containers...)
        // *NOTE*: this works because the lock is recursive
        for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_->begin ();
             iterator != subscribers_->end ();
             )
        {
          try {
            // *TODO*: remove type inference
            (*iterator++)->start (sessionData_->sessionID,
                                  *sessionData_);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Common_INotify_T::start(), continuing\n")));
          }
        } // end FOR
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      ACE_ASSERT (sessionData_);

      // refer the data back to any subscriber(s)

      // synch access
      {
        ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (*lock_);

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
            (*(iterator++))->end (sessionData_->sessionID);
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in Common_INotify_T::end(), continuing\n")));
          }
        } // end FOR
      } // end lock scope

      sessionData_ = NULL;

      break;
    }
    default:
    {
      // refer the data back to any subscriber(s)

      // synch access
      {
        ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (*lock_);

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
            (*(iterator++))->notify ((sessionData_ ? sessionData_->sessionID
                                                   : 0),
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

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionIdType,
//          typename SessionDataContainerType>
//bool
//Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
//                               TimePolicyType,
//                               ConfigurationType,
//                               ControlMessageType,
//                               DataMessageType,
//                               SessionMessageType,
//                               ModuleHandlerConfigurationType,
//                               SessionIdType,
//                               SessionDataContainerType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::initialize"));
//
//  configuration_ =
//      &const_cast<ModuleHandlerConfigurationType&> (configuration_in);
//
//  return true;
//}
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionIdType,
//          typename SessionDataContainerType>
//const ModuleHandlerConfigurationType&
//Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
//                               TimePolicyType,
//                               ConfigurationType,
//                               ControlMessageType,
//                               DataMessageType,
//                               SessionMessageType,
//                               ModuleHandlerConfigurationType,
//                               SessionIdType,
//                               SessionDataContainerType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::get"));
//
//  // sanity check(s)
//  ACE_ASSERT (configuration_);
//
//  return *configuration_;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataContainerType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataContainerType>::subscribe (INOTIFY_T* interfaceHandle_in)
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
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (*lock_);

  subscribers_->push_back (interfaceHandle_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataContainerType>
void
Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionIdType,
                               SessionDataContainerType>::unsubscribe (INOTIFY_T* interfaceHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MessageHandler_T::unsubscribe"));

  // sanity check(s)
  ACE_ASSERT (lock_ && subscribers_);
  ACE_ASSERT (interfaceHandle_in);

  // synch access to subscribers
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (*lock_);

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
