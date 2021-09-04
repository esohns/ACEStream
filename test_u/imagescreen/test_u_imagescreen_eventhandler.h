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

#ifndef TEST_U_IMAGESCREEN_EVENTHANDLER_H
#define TEST_U_IMAGESCREEN_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "stream_common.h"

// forward declarations
struct Stream_ImageScreen_UI_CBData;

template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
          typename SessionMessageType>
class Stream_ImageScreen_EventHandler_T
 : public NotificationType
{
 public:
  Stream_ImageScreen_EventHandler_T (struct Stream_ImageScreen_UI_CBData* // UI callback data
#if defined (GTK_USE)
                                    );
#elif defined (QT_USE)
                                    );
#elif defined (WXWIDGETS_USE)
                                     ,InterfaceType*);                    // wxWidgets application handle
#else
                                    );
#endif
  inline virtual ~Stream_ImageScreen_EventHandler_T () {}

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
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_EventHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_EventHandler_T (const Stream_ImageScreen_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_EventHandler_T& operator= (const Stream_ImageScreen_EventHandler_T&))

  struct Stream_ImageScreen_UI_CBData*             CBData_;
#if defined (WXWIDGETS_USE)
  InterfaceType*                               interface_;
#endif // WXWIDGETS_USE
  typename SessionMessageType::DATA_T::DATA_T* sessionData_;
};

// include template definition
#include "test_u_imagescreen_eventhandler.inl"

#endif
