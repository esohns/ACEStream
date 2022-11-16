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
#include "ace/Synch_Traits.h"

#include "stream_macros.h"

#include "test_i_camera_ml_common.h"
#include "test_i_camera_ml_defines.h"

template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_CameraML_EventHandler_T<NotificationType,
                               DataMessageType,
                               SessionMessageType>::Stream_CameraML_EventHandler_T ()
 : sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraML_EventHandler_T::Stream_CameraML_EventHandler_T"));

}

template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CameraML_EventHandler_T<NotificationType,
                               DataMessageType,
                               SessionMessageType>::start (Stream_SessionId_t sessionId_in,
                                                           const typename SessionMessageType::DATA_T::DATA_T& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraML_EventHandler_T::start"));

  // sanity check(s)
  ACE_ASSERT (!sessionData_);

  sessionData_ =
    &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_in);
}

template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CameraML_EventHandler_T<NotificationType,
                               DataMessageType,
                               SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                            const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraML_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CameraML_EventHandler_T<NotificationType,
                               DataMessageType,
                               SessionMessageType>::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraML_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  if (sessionData_)
    sessionData_ = NULL;
}

template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CameraML_EventHandler_T<NotificationType,
                               DataMessageType,
                               SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                            const DataMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraML_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
}

template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CameraML_EventHandler_T<NotificationType,
                               DataMessageType,
                               SessionMessageType>::notify (Stream_SessionId_t sessionId_in,
                                                            const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraML_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

//  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
//      float current_bytes = 0.0F;

      // sanity check(s)
      if (!sessionData_)
        goto continue_;

continue_:
      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
      return;
  } // end SWITCH
  ACE_UNUSED_ARG (event_e);
}
