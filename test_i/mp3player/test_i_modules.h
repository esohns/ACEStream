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
#include "stream_lib_ffmpeg_common.h"
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#include "stream_dec_mp3_decoder.h"

#include "stream_stat_statistic_report.h"

//#include "stream_dec_wav_encoder.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_target_wavout.h"
#include "stream_dev_target_wasapi.h"
#include "stream_dev_target_xaudio2.h"
#else
#include "stream_dev_target_alsa.h"
#if defined (LIBPIPEWIRE_SUPPORT)
#include "stream_dev_target_pipewire.h"
#endif // LIBPIPEWIRE_SUPPORT

#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FAAD_SUPPORT)
#include "stream_dec_faad_decoder.h"
#endif // FAAD_SUPPORT
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_audio_decoder.h"
#endif // FFMPEG_SUPPORT
#if defined (SOX_SUPPORT)
#include "stream_dec_sox_resampler.h"
#endif // SOX_SUPPORT

#include "stream_file_source.h"
//#include "stream_file_sink.h"

#include "test_i_common.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Test_I_MP3Player_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_I_SessionManager_t;

// declare module(s)
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_I_Stream_Message,
                                    Test_I_Stream_SessionMessage,
                                    struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_I_MP3Player_StreamState,
                                    struct Stream_Statistic,
                                    Test_I_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Test_I_FileSource;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,      // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_FileSource);                       // writer type

#if defined (FFMPEG_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_LibAVAudioDecoder_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Stream_Message,
                                           Test_I_Stream_SessionMessage,
                                           Test_I_MP3Player_SessionData_t,
                                           struct _AMMediaType> Test_I_LibAVAudioDecoder;
#else
typedef Stream_Decoder_LibAVAudioDecoder_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Stream_Message,
                                           Test_I_Stream_SessionMessage,
                                           Test_I_MP3Player_SessionData_t,
                                           struct Stream_MediaFramework_ALSA_MediaType> Test_I_LibAVAudioDecoder;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,      // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_audio_decoder_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_LibAVAudioDecoder);                        // writer type
#endif // FFMPEG_SUPPORT

#if defined (FAAD_SUPPORT)
typedef Stream_Decoder_FAAD_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_I_MP3Player_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Test_I_Stream_Message,
                              Test_I_Stream_SessionMessage,
                              Test_I_MP3Player_SessionData_t,
                              struct _AMMediaType> Test_I_AACDecoder;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,      // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_faad_decoder_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_AACDecoder);                        // writer type
#endif // FAAD_SUPPORT

#if defined (MPG123_SUPPORT)
typedef Stream_Decoder_MP3Decoder_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_I_Stream_Message,
                                    Test_I_Stream_SessionMessage,
                                    struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_I_MP3Player_StreamState,
                                    struct Stream_Statistic,
                                    Test_I_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                    struct _AMMediaType> Test_I_MP3Decoder;
#else
                                    struct Stream_MediaFramework_ALSA_MediaType> Test_I_MP3Decoder;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,      // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_mp3_decoder_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_MP3Decoder);                        // writer type
#endif // MPG123_SUPPORT

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_MP3Player_SessionData,      // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                         // stream notification interface type
                          Test_I_Statistic_ReaderTask_t,            // reader type
                          Test_I_Statistic_WriterTask_t,            // writer type
                          Test_I_StatisticReport);                  // name

#if defined (SOX_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_SoXResampler_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Stream_Message,
                                      Test_I_Stream_SessionMessage,
                                      struct _AMMediaType> Test_I_SoXResampler;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration,        // module handler configuration type
                              libacestream_default_dec_sox_resampler_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_SoXResampler);                                      // name
#endif // ACE_WIN32 || ACE_WIN64
#endif // SOX_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Target_WavOut_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Stream_Message,
                                   Test_I_Stream_SessionMessage,
                                   struct Test_I_MP3Player_SessionData,
                                   struct _AMMediaType> Test_I_WavOutPlayer;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,         // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_target_wavout_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_WavOutPlayer);                          // writer type
typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Stream_Message,
                                   Test_I_Stream_SessionMessage,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   struct _AMMediaType> Test_I_WASAPIPlayer;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration,        // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_WASAPIPlayer);                                      // writer type
typedef Stream_Dev_Target_XAudio2_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_I_Stream_Message,
                                    Test_I_Stream_SessionMessage,
                                    struct _AMMediaType> Test_I_XAudio2Player;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration,         // module handler configuration type
                              libacestream_default_dev_target_xaudio2_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_XAudio2Player);                                      // writer type
#else
typedef Stream_Dev_Target_ALSA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Stream_Message,
                                 Test_I_Stream_SessionMessage,
                                 struct Test_I_MP3Player_SessionData> Test_I_ALSAPlayer;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,                     // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_dev_target_alsa_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_ALSAPlayer);                                      // writer type

#if defined (LIBPIPEWIRE_SUPPORT)
typedef Stream_Dev_Target_Pipewire_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_MP3Player_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_Stream_Message,
                                     Test_I_Stream_SessionMessage> Test_I_PipewirePlayer;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_MP3Player_ModuleHandlerConfiguration,          // module handler configuration type
                              libacestream_default_dev_target_pipewire_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_PipewirePlayer);                                      // writer type
#endif // LIBPIPEWIRE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

//typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
//                                    Common_TimePolicy_t,
//                                    struct Test_I_MP3Player_ModuleHandlerConfiguration,
//                                    Stream_ControlMessage_t,
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
//                                   Stream_ControlMessage_t,
//                                   Test_I_Stream_Message,
//                                   Test_I_Stream_SessionMessage> Test_I_FileWriter;
//DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_MP3Player_SessionData,         // session data type
//                              enum Stream_SessionMessageType,           // session event type
//                              struct Test_I_MP3Player_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_file_sink_module_name_string,
//                              Stream_INotify_t,                         // stream notification interface type
//                              Test_I_FileWriter);                       // writer type

#endif
