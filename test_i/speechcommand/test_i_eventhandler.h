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

#ifndef TEST_I_EVENTHANDLER_H
#define TEST_I_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "stream_common.h"

// forward declarations
#if defined (GUI_SUPPORT)
struct Test_I_UI_CBData;
#endif // GUI_SUPPORT

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
class Test_I_EventHandler_T
 : public NotificationType
{
 public:
#if defined (GUI_SUPPORT)
   Test_I_EventHandler_T (struct Test_I_SpeechCommand_UI_CBData* // UI callback data
#if defined (GTK_USE)
                          );
#elif defined (QT_USE)
                          );
#elif defined (WXWIDGETS_USE)
                          ,InterfaceType*);                      // wxWidgets application handle
#else
                          );
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
#else
   Test_I_EventHandler_T ();
#endif // GUI_SUPPORT
  inline virtual ~Test_I_EventHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const typename SessionMessageType::DATA_T::DATA_T&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&,
                       bool = false);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const DataMessageType&);
  virtual void notify (Stream_SessionId_t,
                       const SessionMessageType&);

 private:
#if defined (GUI_SUPPORT)
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_T ())
#endif // GUI_SUPPORT
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_T (const Test_I_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_T& operator= (const Test_I_EventHandler_T&))

#if defined (GUI_SUPPORT)
  struct Test_I_SpeechCommand_UI_CBData*       CBData_;
#if defined (WXWIDGETS_USE)
  InterfaceType*                               interface_;
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
  typename SessionMessageType::DATA_T::DATA_T* sessionData_;
};

//////////////////////////////////////////

template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
class Test_I_InputHandler_T
 : public NotificationType
{
 public:
#if defined (GUI_SUPPORT)
  Test_I_InputHandler_T (struct Test_I_SpeechCommand_UI_CBData* // UI callback data
#if defined (GTK_USE)
                         );
#elif defined (QT_USE)
                         );
#elif defined (WXWIDGETS_USE)
                         ,InterfaceType*);                      // wxWidgets application handle
#else
                         );
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
#else
  Test_I_InputHandler_T ();
#endif // GUI_SUPPORT
  inline virtual ~Test_I_InputHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const typename SessionMessageType::DATA_T::DATA_T&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&,
                       bool = false);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const DataMessageType&);
  virtual void notify (Stream_SessionId_t,
                       const SessionMessageType&);

 private:
#if defined (GUI_SUPPORT)
  ACE_UNIMPLEMENTED_FUNC (Test_I_InputHandler_T ())
#endif // GUI_SUPPORT
  ACE_UNIMPLEMENTED_FUNC (Test_I_InputHandler_T (const Test_I_InputHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_InputHandler_T& operator= (const Test_I_InputHandler_T&))

#if defined (GUI_SUPPORT)
  struct Test_I_SpeechCommand_UI_CBData*       CBData_;
#if defined (WXWIDGETS_USE)
  InterfaceType*                               interface_;
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
  typename SessionMessageType::DATA_T::DATA_T* sessionData_;
};

// include template definition
#include "test_i_eventhandler.inl"

#endif
