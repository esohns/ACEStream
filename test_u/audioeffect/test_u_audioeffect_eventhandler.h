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

#ifndef TEST_U_AUDIOEFFECT_EVENTHANDLER_H
#define TEST_U_AUDIOEFFECT_EVENTHANDLER_H

#include <ace/Global_Macros.h>

#include "stream_common.h"

#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_message.h"
#include "test_u_audioeffect_session_message.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_AudioEffect_DirectShow_EventHandler
 : public Test_U_AudioEffect_DirectShow_ISessionNotify_t
{
 public:
  Test_U_AudioEffect_DirectShow_EventHandler (Test_U_AudioEffect_DirectShow_GTK_CBData*); // GTK state
  virtual ~Test_U_AudioEffect_DirectShow_EventHandler ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const Test_U_AudioEffect_DirectShow_SessionData&);
  inline virtual void notify (Stream_SessionId_t,
                              const Stream_SessionMessageType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_AudioEffect_DirectShow_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_AudioEffect_DirectShow_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_EventHandler (const Test_U_AudioEffect_DirectShow_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_EventHandler& operator= (const Test_U_AudioEffect_DirectShow_EventHandler&))

  Test_U_AudioEffect_DirectShow_GTK_CBData*  CBData_;
  Test_U_AudioEffect_DirectShow_SessionData* sessionData_;
};

//////////////////////////////////////////

class Test_U_AudioEffect_MediaFoundation_EventHandler
 : public Test_U_AudioEffect_MediaFoundation_ISessionNotify_t
{
 public:
  Test_U_AudioEffect_MediaFoundation_EventHandler (Test_U_AudioEffect_MediaFoundation_GTK_CBData*); // GTK state
  virtual ~Test_U_AudioEffect_MediaFoundation_EventHandler ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const Test_U_AudioEffect_MediaFoundation_SessionData&);
  inline virtual void notify (Stream_SessionId_t, const Stream_SessionMessageType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_AudioEffect_MediaFoundation_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_AudioEffect_MediaFoundation_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_EventHandler (const Test_U_AudioEffect_MediaFoundation_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_EventHandler& operator= (const Test_U_AudioEffect_MediaFoundation_EventHandler&))

  Test_U_AudioEffect_MediaFoundation_GTK_CBData*  CBData_;
  Test_U_AudioEffect_MediaFoundation_SessionData* sessionData_;
};
#else
class Test_U_AudioEffect_EventHandler
 : public Test_U_AudioEffect_ISessionNotify_t
{
 public:
  Test_U_AudioEffect_EventHandler (Test_U_AudioEffect_GTK_CBData*); // GTK state
  virtual ~Test_U_AudioEffect_EventHandler ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const Test_U_AudioEffect_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_AudioEffect_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_AudioEffect_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_EventHandler (const Test_U_AudioEffect_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_EventHandler& operator= (const Test_U_AudioEffect_EventHandler&))

  Test_U_AudioEffect_GTK_CBData*  CBData_;
  Test_U_AudioEffect_SessionData* sessionData_;
};
#endif

#endif
