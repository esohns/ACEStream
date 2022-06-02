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

#ifndef TEST_I_POP_RECEIVE_STREAM_COMMON_H
#define TEST_I_POP_RECEIVE_STREAM_COMMON_H

#include "stream_common.h"
#include "stream_configuration.h"

#include "pop_common.h"
#include "pop_configuration.h"
#include "pop_message.h"
#include "pop_sessionmessage.h"

//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct POP_StreamConfiguration,
                               struct Stream_POPReceive_ModuleHandlerConfiguration> Stream_POPReceive_StreamConfiguration_t;

//typedef POP_Stream_T<struct POP_StreamState,
//                      struct POP_StreamConfiguration,
//                      POP_Statistic_t,
//                      POP_StatisticHandler_t,
//                      struct Stream_POPReceive_ModuleHandlerConfiguration,
//                      struct POP_Stream_SessionData,
//                      POP_Stream_SessionData_t,
//                      Stream_ControlMessage_t,
//                      POP_Message_t,
//                      POP_SessionMessage_t> Test_I_POPReceive_Stream_t;

#endif
