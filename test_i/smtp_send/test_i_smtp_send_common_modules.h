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

#ifndef TEST_I_SMTPSEND_COMMON_MODULES_H
#define TEST_I_SMTPSEND_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_defines.h"
#include "stream_misc_messagehandler.h"

#include "smtp_common.h"
#include "smtp_message.h"
#include "smtp_sessionmessage.h"

#include "test_i_smtp_send_common.h"

// declare module(s)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_SMTPSend_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       SMTP_Message_t,
                                       SMTP_SessionMessage_t,
                                       struct SMTP_Stream_SessionData,
                                       struct Stream_UserData> Stream_SMTPSend_MessageHandler;

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (struct SMTP_Stream_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_SMTPSend_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_SMTPSend_MessageHandler);                      // writer type

#endif
