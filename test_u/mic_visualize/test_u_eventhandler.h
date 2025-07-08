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

#ifndef TEST_U_EVENTHANDLER_H
#define TEST_U_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "test_u_message.h"
#include "test_u_session_message.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// forward declarations
struct Test_U_MicVisualize_DirectShow_UI_CBData;
class Test_U_MicVisualize_DirectShow_SessionData;

class Test_U_DirectShow_EventHandler
 : public Test_U_MicVisualize_DirectShow_ISessionNotify_t
{
 public:
  Test_U_DirectShow_EventHandler (struct Test_U_MicVisualize_DirectShow_UI_CBData*);
  inline virtual ~Test_U_DirectShow_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const Test_U_MicVisualize_DirectShow_SessionData&);
  inline virtual void notify (Stream_SessionId_t, const enum Stream_SessionMessageType&, bool = false) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_DirectShow_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_DirectShow_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_DirectShow_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_DirectShow_EventHandler (const Test_U_DirectShow_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_DirectShow_EventHandler& operator= (const Test_U_DirectShow_EventHandler&))

  struct Test_U_MicVisualize_DirectShow_UI_CBData* CBData_;
  Test_U_MicVisualize_DirectShow_SessionData*      sessionData_;
};

//////////////////////////////////////////

// forward declarations
struct Test_U_MicVisualize_MediaFoundation_UI_CBData;
class Test_U_MicVisualize_MediaFoundation_SessionData;

class Test_U_MediaFoundation_EventHandler
 : public Test_U_MicVisualize_MediaFoundation_ISessionNotify_t
{
 public:
  Test_U_MediaFoundation_EventHandler (struct Test_U_MicVisualize_MediaFoundation_UI_CBData*);
  inline virtual ~Test_U_MediaFoundation_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const Test_U_MicVisualize_MediaFoundation_SessionData&);
  inline virtual void notify (Stream_SessionId_t, const enum Stream_SessionMessageType&, bool = false) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_MediaFoundation_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_MediaFoundation_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_MediaFoundation_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_MediaFoundation_EventHandler (const Test_U_MediaFoundation_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_MediaFoundation_EventHandler& operator= (const Test_U_MediaFoundation_EventHandler&))

  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* CBData_;
  Test_U_MicVisualize_MediaFoundation_SessionData*      sessionData_;
};
#else
// forward declarations
struct Test_U_MicVisualize_UI_CBData;
class Test_U_MicVisualize_SessionData;

class Test_U_EventHandler
 : public Test_U_MicVisualize_ISessionNotify_t
{
 public:
  Test_U_EventHandler (struct Test_U_MicVisualize_UI_CBData*);
  inline virtual ~Test_U_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,
                      const Test_U_MicVisualize_SessionData&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&,
                       bool = false);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler (const Test_U_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler& operator= (const Test_U_EventHandler&))

  struct Test_U_MicVisualize_UI_CBData* CBData_;
  Test_U_MicVisualize_SessionData*      sessionData_;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
