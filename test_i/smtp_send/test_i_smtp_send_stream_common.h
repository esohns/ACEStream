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

#ifndef TEST_I_SMTP_SEND_STREAM_COMMON_H
#define TEST_I_SMTP_SEND_STREAM_COMMON_H

#include "stream_common.h"
#include "stream_configuration.h"

#include "smtp_common.h"
#include "smtp_configuration.h"
#include "smtp_message.h"
#include "smtp_sessionmessage.h"
#include "smtp_stream.h"

//#include "test_i_smtp_send_common.h"

//struct Stream_SMTPSend_StreamConfiguration
// : SMTP_StreamConfiguration
//{
//  Stream_SMTPSend_StreamConfiguration ()
//   : SMTP_StreamConfiguration ()
//  {
//    printFinalReport = true;
//  }
//};

//typedef Stream_IStreamControl_T<enum Stream_ControlType,
//                                enum Stream_SessionMessageType,
//                                enum Stream_StateMachine_ControlState,
//                                struct SMTP_StreamState> Stream_SMTPSend_IStreamControl_t;

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct SMTP_StreamConfiguration,
                               struct Stream_SMTPSend_ModuleHandlerConfiguration> Stream_SMTPSend_StreamConfiguration_t;

typedef SMTP_Stream_T<struct SMTP_StreamState,
                      struct SMTP_StreamConfiguration,
                      SMTP_Statistic_t,
                      SMTP_StatisticHandler_t,
                      struct Stream_SMTPSend_ModuleHandlerConfiguration,
                      struct SMTP_Stream_SessionData,
                      SMTP_Stream_SessionData_t,
                      Stream_ControlMessage_t,
                      SMTP_Message_t,
                      SMTP_SessionMessage_t> Test_I_SMTPSend_Stream_t;

#endif
