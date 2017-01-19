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

#ifndef TEST_U_FILECOPY_COMMON_MODULES_H
#define TEST_U_FILECOPY_COMMON_MODULES_H

#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_file_sink.h"
#include "stream_file_source.h"
#include "stream_misc_statistic_report.h"

#include "test_u_filecopy_common.h"
#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

// declare module(s)
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    ACE_Message_Block,
                                    Stream_Filecopy_Message,
                                    Stream_Filecopy_SessionMessage,
                                    Stream_Filecopy_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Stream_State,
                                    struct Stream_Filecopy_SessionData,
                                    Stream_Filecopy_SessionData_t,
                                    struct Stream_Statistic,
                                    struct Stream_UserData> Stream_Filecopy_FileReader;
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_Filecopy_SessionData,                // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_Filecopy_IStreamNotify_t,                   // stream notification interface type
                              Stream_Filecopy_FileReader);                       // writer type

typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Stream_Filecopy_ModuleHandlerConfiguration,
                                                   ACE_Message_Block,
                                                   Stream_Filecopy_Message,
                                                   Stream_Filecopy_SessionMessage,
                                                   Stream_CommandType_t,
                                                   struct Stream_Statistic,
                                                   struct Stream_Filecopy_SessionData,
                                                   Stream_Filecopy_SessionData_t> Stream_Filecopy_Module_Statistic_ReaderTask_t;
typedef Stream_Module_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Stream_Filecopy_ModuleHandlerConfiguration,
                                                   ACE_Message_Block,
                                                   Stream_Filecopy_Message,
                                                   Stream_Filecopy_SessionMessage,
                                                   Stream_CommandType_t,
                                                   struct Stream_Statistic,
                                                   struct Stream_Filecopy_SessionData,
                                                   Stream_Filecopy_SessionData_t> Stream_Filecopy_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Stream_Filecopy_SessionData,                // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                          Stream_Filecopy_IStreamNotify_t,                   // stream notification interface type
                          Stream_Filecopy_Module_Statistic_ReaderTask_t,     // reader type
                          Stream_Filecopy_Module_Statistic_WriterTask_t,     // writer type
                          Stream_Filecopy_StatisticReport);                  // name

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_Filecopy_ModuleHandlerConfiguration,
                                   ACE_Message_Block,
                                   Stream_Filecopy_Message,
                                   Stream_Filecopy_SessionMessage,
                                   struct Stream_Filecopy_SessionData> Stream_Filecopy_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_Filecopy_SessionData,                // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_Filecopy_IStreamNotify_t,                   // stream notification interface type
                              Stream_Filecopy_FileWriter);                       // writer type

#endif
