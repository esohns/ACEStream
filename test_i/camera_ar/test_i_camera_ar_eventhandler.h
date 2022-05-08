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

#ifndef TEST_I_CAMERA_AR_EVENTHANDLER_H
#define TEST_I_CAMERA_AR_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "stream_common.h"

template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_CameraAR_EventHandler_T
 : public NotificationType
{
 public:
  Stream_CameraAR_EventHandler_T ();
  inline virtual ~Stream_CameraAR_EventHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const typename SessionMessageType::DATA_T::DATA_T&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const DataMessageType&);
  virtual void notify (Stream_SessionId_t,
                       const SessionMessageType&);

 private:
//  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_EventHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_EventHandler_T (const Stream_CameraAR_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_EventHandler_T& operator= (const Stream_CameraAR_EventHandler_T&))

  typename SessionMessageType::DATA_T::DATA_T* sessionData_;
};

// include template definition
#include "test_i_camera_ar_eventhandler.inl"

#endif
