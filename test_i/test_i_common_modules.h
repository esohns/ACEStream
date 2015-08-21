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

#ifndef TEST_I_COMMON_MODULES_H
#define TEST_I_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_filereader.h"
#include "stream_module_filewriter.h"
#include "stream_module_runtimestatistic.h"
#include "stream_module_tcpsource.h"
#include "stream_module_tcptarget.h"

#include "test_i_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

// declare module(s)
typedef Stream_Module_FileReader_T<Stream_SessionMessage,
                                   Stream_Message,
                                   //////
                                   Test_I_Stream_ModuleHandlerConfiguration,
                                   //////
                                   Test_I_Stream_State,
                                   //////
                                   Test_I_Stream_SessionData,
                                   Test_I_Stream_SessionData_t,
                                   //////
                                   Test_I_RuntimeStatistic_t> Test_I_Stream_Module_FileReader;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                             // task synch type
                              Common_TimePolicy_t,                      // time policy
                              Stream_ModuleConfiguration,               // module configuration type
                              Test_I_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Stream_Module_FileReader);         // writer type

typedef Stream_Module_TCPSource_T<Stream_SessionMessage,
                                  Stream_Message,
                                  //////
                                  Test_I_Stream_ModuleHandlerConfiguration,
                                  //////
                                  Test_I_Stream_State,
                                  //////
                                  Test_I_Stream_SessionData,
                                  Test_I_Stream_SessionData_t,
                                  //////
                                  Test_I_RuntimeStatistic_t,
                                  ///////
                                  Test_I_Stream_InetSourceConnectionManager_t,
                                  Test_I_Stream_TCPConnector_t> Test_I_Stream_Module_TCPSource;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                             // task synch type
                              Common_TimePolicy_t,                      // time policy
                              Stream_ModuleConfiguration,               // module configuration type
                              Test_I_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Stream_Module_TCPSource);          // writer type

typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Stream_SessionMessage,
                                             Stream_Message,
                                             Stream_CommandType_t,
                                             Test_I_RuntimeStatistic_t> Test_I_Stream_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Stream_SessionMessage,
                                             Stream_Message,
                                             Stream_CommandType_t,
                                             Test_I_RuntimeStatistic_t> Test_I_Stream_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                // task synch type
                          Common_TimePolicy_t,                         // time policy type
                          Stream_ModuleConfiguration,                  // module configuration type
                          Test_I_Stream_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_Stream_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Stream_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Stream_Module_RuntimeStatistic);      // name

typedef Stream_Module_FileWriter_T<Stream_SessionMessage,
                                   Stream_Message,
                                   //////
                                   Test_I_Stream_ModuleHandlerConfiguration,
                                   //////
                                   Test_I_Stream_SessionData> Test_I_Stream_Module_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                             // task synch type
                              Common_TimePolicy_t,                      // time policy
                              Stream_ModuleConfiguration,               // module configuration type
                              Test_I_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Stream_Module_FileWriter);         // writer type

typedef Stream_Module_TCPTarget_T<Stream_SessionMessage,
                                  Stream_Message,
                                  //////
                                  Test_I_Stream_ModuleHandlerConfiguration,
                                  //////
                                  Test_I_Stream_SessionData,
                                  //////
                                  Test_I_Stream_InetSourceConnectionManager_t,
                                  Test_I_Stream_TCPConnector_t> Test_I_Stream_Module_TCPTarget;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                             // task synch type
                              Common_TimePolicy_t,                      // time policy
                              Stream_ModuleConfiguration,               // module configuration type
                              Test_I_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Stream_Module_TCPTarget);          // writer type

#endif
