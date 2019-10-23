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

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_dec_mp3_decoder.h"

#include "stream_stat_statistic_report.h"

//#include "stream_dec_wav_encoder.h"
#include "stream_dev_target_wavout.h"

//#include "stream_file_sink.h"

#include "test_i_common.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

// declare module(s)
typedef Stream_Decoder_MP3Decoder_T<ACE_MT_SYNCH,
                                    Test_I_ControlMessage_t,
                                    Test_I_Stream_Message,
                                    Test_I_Stream_SessionMessage,
                                    struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_I_MP3Player_StreamState,
                                    struct Test_I_MP3Player_SessionData,
                                    Test_I_MP3Player_SessionData_t,
                                    struct Stream_Statistic,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                    struct _AMMediaType> Test_I_MP3Decoder;
#else
                                    struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_MP3Decoder;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,      // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_mp3_decoder_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_MP3Decoder);                        // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Test_I_MP3Player_SessionData,
                                                      Test_I_MP3Player_SessionData_t> Test_I_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Test_I_MP3Player_SessionData,
                                                      Test_I_MP3Player_SessionData_t> Test_I_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_MP3Player_SessionData,      // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                         // stream notification interface type
                          Test_I_Statistic_ReaderTask_t,            // reader type
                          Test_I_Statistic_WriterTask_t,            // writer type
                          Test_I_StatisticReport);                  // name

typedef Stream_Dev_Target_WavOut_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                   Test_I_ControlMessage_t,
                                   Test_I_Stream_Message,
                                   Test_I_Stream_SessionMessage,
                                   Stream_SessionId_t,
                                   struct Test_I_MP3Player_SessionData> Test_I_WavOutPlayer;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,         // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_target_wavout_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_WavOutPlayer);                          // writer type

//typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
//                                    Common_TimePolicy_t,
//                                    struct Test_I_MP3Player_ModuleHandlerConfiguration,
//                                    Test_I_ControlMessage_t,
//                                    Test_I_Stream_Message,
//                                    Test_I_Stream_SessionMessage,
//                                    Test_I_MP3Player_SessionData_t,
//                                    struct Test_I_MP3Player_SessionData,
//                                    struct _AMMediaType,
//                                    struct Stream_UserData> Test_I_WAVEncoder;
//DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,         // session data type
//                              enum Stream_SessionMessageType,           // session event type
//                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_dec_wav_encoder_module_name_string,
//                              Stream_INotify_t,                         // stream notification interface type
//                              Test_I_WAVEncoder);                          // writer type
//typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
//                                   Common_TimePolicy_t,
//                                   struct Test_I_MP3Player_ModuleHandlerConfiguration,
//                                   Test_I_ControlMessage_t,
//                                   Test_I_Stream_Message,
//                                   Test_I_Stream_SessionMessage> Test_I_FileWriter;
//DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,         // session data type
//                              enum Stream_SessionMessageType,           // session event type
//                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_file_sink_module_name_string,
//                              Stream_INotify_t,                         // stream notification interface type
//                              Test_I_FileWriter);                       // writer type

#endif
