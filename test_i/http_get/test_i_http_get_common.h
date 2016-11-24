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

#ifndef TEST_I_HTTP_GET_COMMON_H
#define TEST_I_HTTP_GET_COMMON_H

#include "stream_control_message.h"

#include "test_i_common.h"
//#include "test_i_message.h"
//#include "test_i_session_message.h"

typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Test_I_AllocatorConfiguration,
                                Test_I_Stream_Message,
                                Test_I_Stream_SessionMessage> Test_I_ControlMessage_t;

//typedef Stream_IModuleHandler_T<Test_I_Stream_ModuleHandlerConfiguration> Test_I_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Stream_Message,
                                          Test_I_Stream_SessionMessage> Test_I_MessageAllocator_t;

#endif
