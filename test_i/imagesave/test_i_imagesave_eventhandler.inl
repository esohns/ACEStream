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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "common_image_tools.h"

#include "stream_macros.h"

#include "test_i_imagesave_common.h"
#include "test_i_imagesave_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_imagesave_gtk_callbacks.h"
#endif // GTK_USE
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
                      SessionMessageType>::Test_I_EventHandler_T (struct Test_I_ImageSave_UI_CBData* CBData_in
#if defined (GTK_USE)
                                                                  )
#elif defined (QT_USE)
                                                                  )
#elif defined (WXWIDGETS_USE)
                                                                  ,InterfaceType* interface_in)
#else
                                                                  )
#endif
#else
                              SessionMessageType>::Test_I_EventHandler_T ()
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
#if defined (WXWIDGETS_USE)
 , interface_ (interface_in)
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
 , numberOfFrames_ (0)
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
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif
#endif // GUI_SUPPORT
  ACE_ASSERT (!sessionData_);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  UIStateType& state_r =
    const_cast<UIStateType&> (interface_->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  //numberOfFrames_ = 0;
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

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
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
  ACE_ASSERT (CBData_);
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
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#if defined (GTK_USE)
    CBData_->numberOfFrames = numberOfFrames_;
    event_source_id = g_idle_add (idle_update_frames_cb,
                                  CBData_);
    if (event_source_id == 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_frames_cb): \"%m\", continuing\n")));
    else
      state_r.eventSourceIds.insert (event_source_id);
  
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
#endif // GUI_SUPPORT

  numberOfFrames_ = 0;
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
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->configuration);
#if defined(GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#elif defined (WXWIDGETS_USE)
  ACE_ASSERT (interface_);
#endif
#endif // GUI_SUPPORT

  numberOfFrames_++;

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
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  Test_I_StreamConfiguration_t::ITERATOR_T stream_iterator =
    CBData_->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != CBData_->configuration->streamConfiguration.end ());
  if ((*stream_iterator).second.second->frameNumber == numberOfFrames_)
  { ACE_ASSERT (sessionData_);
    ACE_ASSERT (!sessionData_->formats.empty ());
    std::string filename_string = Common_File_Tools::getWorkingDirectory ();
    filename_string += ACE_DIRECTORY_SEPARATOR_STR_A;
    filename_string += ACE_TEXT_ALWAYS_CHAR ("output.bmp");
    uint8_t* buffers_a[1];
    buffers_a[0] = reinterpret_cast<uint8_t*> (message_in.rd_ptr ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Common_Image_Tools::saveBMP (sessionData_->formats.front ().video.resolution,
                                 AV_PIX_FMT_RGB24,
                                 buffers_a,
                                 filename_string);
#else
    ACE_UNUSED_ARG (buffers_a);
    ACE_ASSERT (false); // *TODO*
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    CBData_->progressData.statistic.bytes += message_in.total_length ();
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
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
                                                   const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
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
      ACE_UINT64 current_bytes_i = 0;

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
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
        } // end IF

        // *NOTE*: the byte counter is more current than what is received here
        //         (see above) --> do not update
        current_bytes_i = CBData_->progressData.statistic.bytes;
        CBData_->progressData.statistic = sessionData_->statistic;
        CBData_->progressData.statistic.bytes = current_bytes_i;

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
#else
  ACE_UNUSED_ARG (event_e);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
}
