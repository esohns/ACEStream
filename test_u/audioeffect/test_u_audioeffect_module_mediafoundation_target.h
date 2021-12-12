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

#ifndef TEST_U_AUDIOEFFECT_MODULE_MEDIAFOUNDATION_TARGET_H
#define TEST_U_AUDIOEFFECT_MODULE_MEDIAFOUNDATION_TARGET_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_lib_mediafoundation_target.h"

#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_message.h"
#include "test_u_audioeffect_session_message.h"

class Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget
 : public Stream_MediaFramework_MediaFoundation_Target_T<ACE_MT_SYNCH,
                                                         Common_TimePolicy_t,
                                                         struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration,
                                                         Stream_ControlMessage_t,
                                                         Test_U_AudioEffect_MediaFoundation_Message,
                                                         Test_U_AudioEffect_MediaFoundation_SessionMessage,
                                                         Test_U_AudioEffect_MediaFoundation_SessionData,
                                                         Test_U_AudioEffect_MediaFoundation_SessionData_t,
                                                         IMFMediaType*>
{
  typedef Stream_MediaFramework_MediaFoundation_Target_T<ACE_MT_SYNCH,
                                                         Common_TimePolicy_t,
                                                         struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration,
                                                         Stream_ControlMessage_t,
                                                         Test_U_AudioEffect_MediaFoundation_Message,
                                                         Test_U_AudioEffect_MediaFoundation_SessionMessage,
                                                         Test_U_AudioEffect_MediaFoundation_SessionData,
                                                         Test_U_AudioEffect_MediaFoundation_SessionData_t,
                                                         IMFMediaType*> inherited;

 public:
  Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget (ISTREAM_T*); // stream handle
  inline virtual ~Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget () {}

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget (const Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget& operator= (const Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget&))

  virtual bool updateMediaSession (IMFMediaType*); // (new) source media type handle
};

//////////////////////////////////////////

// declare module
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AudioEffect_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_mediafoundation_target_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget);            // writer type

#endif
