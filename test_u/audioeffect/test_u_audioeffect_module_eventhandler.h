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

#ifndef TEST_U_AUDIOEFFECT_MODULE_EVENTHANDLER_H
#define TEST_U_AUDIOEFFECT_MODULE_EVENTHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_message.h"
#include "test_u_audioeffect_session_message.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_AudioEffect_DirectShow_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration,
                                         Test_U_AudioEffect_DirectShow_ControlMessage_t,
                                         Test_U_AudioEffect_DirectShow_Message,
                                         Test_U_AudioEffect_DirectShow_SessionMessage,
                                         Stream_SessionId_t,
                                         Test_U_AudioEffect_DirectShow_SessionData>
{
 public:
  Test_U_AudioEffect_DirectShow_Module_EventHandler ();
  virtual ~Test_U_AudioEffect_DirectShow_Module_EventHandler ();

  // implement Common_IClone_T
  virtual Stream_Module_t* clone ();

 private:
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration,
                                         Test_U_AudioEffect_DirectShow_ControlMessage_t,
                                         Test_U_AudioEffect_DirectShow_Message,
                                         Test_U_AudioEffect_DirectShow_SessionMessage,
                                         Stream_SessionId_t,
                                         Test_U_AudioEffect_DirectShow_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_Module_EventHandler (const Test_U_AudioEffect_DirectShow_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_Module_EventHandler& operator= (const Test_U_AudioEffect_DirectShow_Module_EventHandler&))
};

//////////////////////////////////////////

class Test_U_AudioEffect_MediaFoundation_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration,
                                         Test_U_AudioEffect_MediaFoundation_ControlMessage_t,
                                         Test_U_AudioEffect_MediaFoundation_Message,
                                         Test_U_AudioEffect_MediaFoundation_SessionMessage,
                                         Stream_SessionId_t,
                                         Test_U_AudioEffect_MediaFoundation_SessionData>
{
 public:
  Test_U_AudioEffect_MediaFoundation_Module_EventHandler ();
  virtual ~Test_U_AudioEffect_MediaFoundation_Module_EventHandler ();

  // implement Common_IClone_T
  virtual Stream_Module_t* clone ();

 private:
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration,
                                         Test_U_AudioEffect_MediaFoundation_ControlMessage_t,
                                         Test_U_AudioEffect_MediaFoundation_Message,
                                         Test_U_AudioEffect_MediaFoundation_SessionMessage,
                                         Stream_SessionId_t,
                                         Test_U_AudioEffect_MediaFoundation_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_Module_EventHandler (const Test_U_AudioEffect_MediaFoundation_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_Module_EventHandler& operator= (const Test_U_AudioEffect_MediaFoundation_Module_EventHandler&))
};

//////////////////////////////////////////

// declare module
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AudioEffect_DirectShow_SessionData,                // session data type
                              Stream_SessionMessageType,                                // session event type
                              Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              Test_U_AudioEffect_IStreamNotify_t,                       // stream notification interface type
                              Test_U_AudioEffect_DirectShow_Module_EventHandler);       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AudioEffect_MediaFoundation_SessionData,                // session data type
                              Stream_SessionMessageType,                                     // session event type
                              Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              Test_U_AudioEffect_IStreamNotify_t,                            // stream notification interface type
                              Test_U_AudioEffect_MediaFoundation_Module_EventHandler);       // writer type

#else
class Test_U_AudioEffect_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Test_U_AudioEffect_ModuleHandlerConfiguration,
                                         Test_U_AudioEffect_ControlMessage_t,
                                         Test_U_AudioEffect_Message,
                                         Test_U_AudioEffect_SessionMessage,
                                         Stream_SessionId_t,
                                         Test_U_AudioEffect_SessionData>
{
 public:
  Test_U_AudioEffect_Module_EventHandler ();
  virtual ~Test_U_AudioEffect_Module_EventHandler ();

  // implement Common_IClone_T
  virtual Stream_Module_t* clone ();

 private:
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Test_U_AudioEffect_ModuleHandlerConfiguration,
                                         Test_U_AudioEffect_ControlMessage_t,
                                         Test_U_AudioEffect_Message,
                                         Test_U_AudioEffect_SessionMessage,
                                         Stream_SessionId_t,
                                         Test_U_AudioEffect_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_Module_EventHandler (const Test_U_AudioEffect_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_Module_EventHandler& operator= (const Test_U_AudioEffect_Module_EventHandler&))
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AudioEffect_SessionData,                // session data type
                              Stream_SessionMessageType,                     // session event type
                              Test_U_AudioEffect_ModuleHandlerConfiguration, // module handler configuration type
                              Test_U_AudioEffect_IStreamNotify_t,            // stream notification interface type
                              Test_U_AudioEffect_Module_EventHandler);       // writer type

#endif

#endif
