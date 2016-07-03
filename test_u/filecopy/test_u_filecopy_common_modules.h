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

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_file_sink.h"
#include "stream_file_source.h"
#include "stream_misc_runtimestatistic.h"

#include "test_u_filecopy_common.h"
#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

// declare module(s)
typedef Stream_Module_FileReader_T<ACE_SYNCH_MUTEX,
                                   ///////
                                   Stream_Filecopy_SessionMessage,
                                   Stream_Filecopy_Message,
                                   ///////
                                   Stream_Filecopy_ModuleHandlerConfiguration,
                                   ///////
                                   int,
                                   int,
                                   Stream_State,
                                   ///////
                                   Stream_Filecopy_SessionData,
                                   Stream_Filecopy_SessionData_t,
                                   ///////
                                   Stream_Statistic> Stream_Filecopy_Module_FileReader;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                               // task synch type
                              Common_TimePolicy_t,                        // time policy
                              Stream_ModuleConfiguration,                 // module configuration type
                              Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_Filecopy_Module_FileReader);         // writer type

typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Stream_Filecopy_SessionMessage,
                                             Stream_Filecopy_Message,
                                             Stream_CommandType_t,
                                             Stream_Statistic,
                                             Stream_Filecopy_SessionData,
                                             Stream_Filecopy_SessionData_t> Stream_Filecopy_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Stream_Filecopy_SessionMessage,
                                             Stream_Filecopy_Message,
                                             Stream_CommandType_t,
                                             Stream_Statistic,
                                             Stream_Filecopy_SessionData,
                                             Stream_Filecopy_SessionData_t> Stream_Filecopy_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                  // task synch type
                          Common_TimePolicy_t,                           // time policy type
                          Stream_ModuleConfiguration,                    // module configuration type
                          Stream_Filecopy_ModuleHandlerConfiguration,    // module handler configuration type
                          Stream_Filecopy_Module_Statistic_ReaderTask_t, // reader type
                          Stream_Filecopy_Module_Statistic_WriterTask_t, // writer type
                          Stream_Filecopy_Module_RuntimeStatistic);      // name

typedef Stream_Module_FileWriter_T<Stream_Filecopy_SessionMessage,
                                   Stream_Filecopy_Message,
                                   ///////
                                   Stream_Filecopy_ModuleHandlerConfiguration,
                                   ///////
                                   Stream_Filecopy_SessionData> Stream_Filecopy_Module_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                               // task synch type
                              Common_TimePolicy_t,                        // time policy
                              Stream_ModuleConfiguration,                 // module configuration type
                              Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_Filecopy_Module_FileWriter);         // writer type

#endif
