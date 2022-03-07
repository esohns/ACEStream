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

#include <iostream>

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "stream_macros.h"

#include "test_i_speechcommand_common.h"
#include "test_i_speechcommand_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
Test_I_EventHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
                      SessionMessageType>::Test_I_EventHandler_T (struct Test_I_SpeechCommand_UI_CBData* CBData_in
#if defined (GTK_USE)
                                                                  )
#elif defined (QT_USE)
                                                                  )
#elif defined (WXWIDGETS_USE)
                                                                  ,InterfaceType* interface_in)
#else
                                                                  )
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
#else
                      SessionMessageType>::Test_I_EventHandler_T ()
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
#if defined (WXWIDGETS_USE)
 , interface_ (interface_in)
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_T::Test_I_EventHandler_T"));

}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_EventHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::start (Stream_SessionId_t sessionId_in,
                                                  const typename SessionMessageType::DATA_T::DATA_T& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_T::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif
#endif // GUI_SUPPORT
//  ACE_ASSERT (!sessionData_);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ =
    &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_in);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_EventHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                   const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_EventHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  guint event_source_id = 0;
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
  if (likely (CBData_))
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#if defined (GTK_USE)
    event_source_id = g_idle_add (idle_session_end_cb,
                                  CBData_);
    if (event_source_id == 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
    else
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
    state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
  } // end IF/lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

  if (sessionData_)
    sessionData_ = NULL;
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_EventHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                  const DataMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif
#endif // GUI_SUPPORT

  typename DataMessageType::DATA_T& data_r =
    const_cast<typename DataMessageType::DATA_T&> (message_in.getR ());
  const DataMessageType* message_p = &message_in;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  do
  {
#if defined (GUI_SUPPORT)
    if (likely (CBData_))
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      CBData_->progressData.statistic.bytes += message_in.total_length ();
      state_r.eventStack.push (COMMON_UI_EVENT_DATA);

      CBData_->progressData.words.insert (CBData_->progressData.words.end (),
                                          data_r.words.begin (), data_r.words.end ());
      goto continue_;
    } // end IF/lock scope
#else
      goto continue_;
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
    for (Stream_Decoder_DeepSpeech_ResultConstIterator_t iterator = data_r.words.begin ();
         iterator != data_r.words.end ();
         ++iterator)
      std::cout << *iterator << ACE_TEXT_ALWAYS_CHAR (" ");
    if (!data_r.words.empty ())
      std::cout.flush ();

continue_:
    message_p = static_cast<DataMessageType*> (message_p->cont ());
    if (message_p)
      data_r =
        const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
  } while (message_p);
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_EventHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                  const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  int result = -1;
#if defined (GUI_SUPPORT)
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
#endif // GUI_SUPPORT
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      float current_bytes = 0.0F;

      // sanity check(s)
      if (!sessionData_)
        goto continue_;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE || WXWIDGETS_USE
        if (sessionData_->lock)
        {
          result = sessionData_->lock->acquire ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
        } // end IF

        // *NOTE*: the byte counter is more current than what is received here
        //         (see above) --> do not update
#if defined (GUI_SUPPORT)
        if (likely (CBData_))
        {
          current_bytes = CBData_->progressData.statistic.bytes;
          CBData_->progressData.statistic = sessionData_->statistic;
          CBData_->progressData.statistic.bytes = current_bytes;
        } // end IF
#endif // GUI_SUPPORT

        if (sessionData_->lock)
        {
          result = sessionData_->lock->release ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
        } // end IF
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

continue_:
#if defined (GUI_SUPPORT)
      event_e = COMMON_UI_EVENT_STATISTIC;
#endif // GUI_SUPPORT
      break;
    }
    default:
      return;
  } // end SWITCH

#if defined (GUI_SUPPORT)
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}

//////////////////////////////////////////

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
Test_I_InputHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
                      SessionMessageType>::Test_I_InputHandler_T (struct Test_I_SpeechCommand_UI_CBData* CBData_in
#if defined (GTK_USE)
                                                                  )
#elif defined (QT_USE)
                                                                  )
#elif defined (WXWIDGETS_USE)
                                                                  ,InterfaceType* interface_in)
#else
                                                                  )
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
#else
                      SessionMessageType>::Test_I_InputHandler_T ()
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
#if defined (WXWIDGETS_USE)
 , interface_ (interface_in)
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::Test_I_InputHandler_T"));

}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_InputHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::start (Stream_SessionId_t sessionId_in,
                                                  const typename SessionMessageType::DATA_T::DATA_T& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::start"));

  ACE_UNUSED_ARG (sessionId_in);

  sessionData_ =
    &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_in);
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_InputHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                   const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_InputHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  sessionData_ = NULL;
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_InputHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                   const DataMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  enum Test_I_SpeechCommand_InputCommand command_e =
    TEST_I_INPUT_COMMAND_INVALID;
  DataMessageType* message_p = const_cast<DataMessageType*> (&message_in);
  int result = -1;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _KEY_EVENT_RECORD* key_event_record_p = NULL;
#else
  char* data_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  do
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    key_event_record_p =
      reinterpret_cast<struct _KEY_EVENT_RECORD*> (message_p->rd_ptr ());
    switch (key_event_record_p->wVirtualKeyCode)
    {
      case VK_UP:
      {
        command_e = TEST_I_INPUT_COMMAND_GAIN_INCREASE;
        break;
      }
      case VK_DOWN:
      {
        command_e = TEST_I_INPUT_COMMAND_GAIN_DECREASE;
        break;
      }
      case VK_Q:
      {
        command_e = TEST_I_INPUT_COMMAND_SHUTDOWN;
        break;
      }
      default:
        break;
    } // end SWITCH
#else
    data_p = message_p->rd_ptr ();
    do
    {
      switch (*data_p)
      {
        case 72: // up
        {
          command_e = TEST_I_INPUT_COMMAND_GAIN_INCREASE;
          break;
        }
        case 80: // down
        {
          command_e = TEST_I_INPUT_COMMAND_GAIN_DECREASE;
          break;
        }
        case 113: // 'q'
        {
          command_e = TEST_I_INPUT_COMMAND_SHUTDOWN;
          break;
        }
        default:
          break;
      } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64

    // process input command
    switch (command_e)
    {
      case TEST_I_INPUT_COMMAND_SHUTDOWN:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("shutting down...\n")));
        // *NOTE*: let the signal handler do the work
        result = ACE_OS::raise (SIGINT);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to raise(SIGINT): \"%m\", continuing\n")));
        break;
      }
      case TEST_I_INPUT_COMMAND_GAIN_DECREASE:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("decreasing gain\n")));
        break;
      }
      case TEST_I_INPUT_COMMAND_GAIN_INCREASE:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("increasing gain\n")));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown command (was: %d), continuing\n"),
                    command_e));
        break;
      }
    } // end SWITCH

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      ++data_p;
    } while (*data_p);
#endif // ACE_WIN32 || ACE_WIN64

    message_p = static_cast<DataMessageType*> (message_p->cont ());
  } while (message_p);
}

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
void
Test_I_InputHandler_T<NotificationType,
                      DataMessageType,
#if defined (GUI_SUPPORT)
                      UIStateType,
#if defined (WXWIDGETS_USE)
                      InterfaceType,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
                      SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                   const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionMessage_in);
}
