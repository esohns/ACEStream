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

#include "test_u_camsave_common.h"

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_ui_common.h"

#include "stream_macros.h"

#include "test_u_camsave_defines.h"
#if defined (GTK_SUPPORT)
#include "test_u_camsave_gtk_callbacks.h"
#endif // GTK_SUPPORT

template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
          typename SessionMessageType>
Stream_CamSave_EventHandler_T<NotificationType,
                              DataMessageType,
                              UIStateType,
#if defined (WXWIDGETS_USE)
                              InterfaceType,
#endif // WXWIDGETS_USE
                              SessionMessageType>::Stream_CamSave_EventHandler_T (struct Stream_CamSave_UI_CBData* CBData_in
#if defined (WXWIDGETS_USE)
                                                                                  ,InterfaceType* interface_in)
#else
                                                                                 )
#endif // WXWIDGETS_USE
 : CBData_ (CBData_in)
#if defined (WXWIDGETS_USE)
 , interface_ (interface_in)
#endif // WXWIDGETS_USE
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler_T::Stream_CamSave_EventHandler_T"));

}

template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
          typename SessionMessageType>
void
Stream_CamSave_EventHandler_T<NotificationType,
                              DataMessageType,
                              UIStateType,
#if defined (WXWIDGETS_USE)
                              InterfaceType,
#endif // WXWIDGETS_USE
                              SessionMessageType>::start (Stream_SessionId_t sessionId_in,
                                                          const typename SessionMessageType::DATA_T::DATA_T& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler_T::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_SUPPORT)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif
  ACE_ASSERT (!sessionData_);

#if defined (GTK_USE)
  UIStateType& state_r = const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r = const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE || WXWIDGETS_USE

  sessionData_ =
    &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_in);

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
}

template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
          typename SessionMessageType>
void
Stream_CamSave_EventHandler_T<NotificationType,
                              DataMessageType,
                              UIStateType,
#if defined (WXWIDGETS_USE)
                              InterfaceType,
#endif // WXWIDGETS_USE
                              SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                           const enum Stream_SessionMessageType& sessionEvent_in,
                                                           bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
          typename SessionMessageType>
void
Stream_CamSave_EventHandler_T<NotificationType,
                              DataMessageType,
                              UIStateType,
#if defined (WXWIDGETS_USE)
                              InterfaceType,
#endif // WXWIDGETS_USE
                              SessionMessageType>::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif // GTK_USE || WXWIDGETS_USE

#if defined (GTK_USE)
  UIStateType& state_r = const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r = const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE || WXWIDGETS_USE

#if defined (GTK_USE)
  guint event_source_id = 0;
#endif // GTK_USE
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
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE

  if (sessionData_)
    sessionData_ = NULL;
}

template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
          typename SessionMessageType>
void
Stream_CamSave_EventHandler_T<NotificationType,
                              DataMessageType,
                              UIStateType,
#if defined (WXWIDGETS_USE)
                              InterfaceType,
#endif // WXWIDGETS_USE
                              SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                           const DataMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif // GTK_USE || WXWIDGETS_USE

#if defined (GTK_USE)
  UIStateType& state_r = const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r = const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE || WXWIDGETS_USE

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    ++CBData_->progressData.statistic.totalFrames;
    CBData_->progressData.statistic.bytes += message_in.total_length ();
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE

#if defined (GTK_USE)
//  guint event_source_id = g_idle_add (idle_update_video_display_cb,
//                                      CBData_);
//  if (event_source_id == 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to g_idle_add(idle_update_video_display_cb): \"%m\", returning\n")));
//    return;
//  } // end IF
//  CBData_->UIState.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
}

template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType,
#endif // WXWIDGETS_USE
          typename SessionMessageType>
void
Stream_CamSave_EventHandler_T<NotificationType,
                              DataMessageType,
                              UIStateType,
#if defined (WXWIDGETS_USE)
                              InterfaceType,
#endif // WXWIDGETS_USE
                              SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                           const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif // GTK_USE || WXWIDGETS_USE

#if defined (GTK_USE)
  UIStateType& state_r = const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r = const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE || WXWIDGETS_USE

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      ACE_UINT64 current_bytes = 0;

      // sanity check(s)
      if (!sessionData_)
        goto continue_;

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE || WXWIDGETS_USE
        if (sessionData_->lock)
        {
          result = sessionData_->lock->acquire ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
        } // end IF

        // *NOTE*: the byte counter is more current than what is received here
        //         (see above) --> do not update
        current_bytes = CBData_->progressData.statistic.bytes;
        CBData_->progressData.statistic = sessionData_->statistic;
        CBData_->progressData.statistic.bytes = current_bytes;

        if (sessionData_->lock)
        {
          result = sessionData_->lock->release ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
        } // end IF
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE

continue_:
      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
      return;
  } // end SWITCH

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE || WXWIDGETS_USE
}
