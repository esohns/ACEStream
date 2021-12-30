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

#ifndef TEST_I_MODULES_H
#define TEST_I_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_defines.h"
#include "stream_misc_messagehandler.h"

#include "test_i_speechcommand_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_DirectShow_SessionMessage_t,
                                       Test_I_DirectShow_SessionData,
                                       struct Stream_UserData> Test_I_DirectShow_MessageHandler;
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_MediaFoundation_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_MediaFoundation_SessionMessage_t,
                                       Test_I_MediaFoundation_SessionData,
                                       struct Stream_UserData> Test_I_MediaFoundation_MessageHandler;
#else
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_ALSA_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_ALSA_SessionMessage_t,
                                       Test_I_ALSA_SessionData,
                                       struct Stream_UserData> Test_I_ALSA_MessageHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////
// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_DirectShow_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_DirectShow_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_DirectShow_MessageHandler);                         // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_MediaFoundation_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_MediaFoundation_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_MediaFoundation_MessageHandler);                    // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ALSA_SessionData,                                   // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ALSA_ModuleHandlerConfiguration,             // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_ALSA_MessageHandler);                               // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
