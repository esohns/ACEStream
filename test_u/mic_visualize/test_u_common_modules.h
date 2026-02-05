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

#ifndef TEST_U_COMMON_MODULES_H
#define TEST_U_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

#include "stream_dec_mp3_decoder.h"
#include "stream_dec_noise_source.h"
#if defined (SOX_SUPPORT)
#include "stream_dec_sox_resampler.h"
#endif // SOX_SUPPORT
#include "stream_dec_wav_encoder.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_mic_source_directshow.h"
#include "stream_dev_mic_source_mediafoundation.h"
#include "stream_dev_mic_source_wasapi.h"
#include "stream_dev_mic_source_wavein.h"
#include "stream_dev_target_wasapi.h"
#include "stream_dev_target_wavout.h"

#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#include "stream_lib_directshow_target.h"
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
#include "stream_lib_directshow_source.h"

#include "stream_lib_mediafoundation_mediasource.h"
#include "stream_lib_mediafoundation_source.h"
#include "stream_lib_mediafoundation_target.h"
#else
#if defined (SOX_SUPPORT)
#include "stream_dec_sox_effect.h"
#endif // SOX_SUPPORT

#include "stream_dev_mic_source_alsa.h"
#include "stream_dev_target_alsa.h"

#if defined (LIBPIPEWIRE_SUPPORT)
#include "stream_dev_mic_source_pipewire.h"
#endif // LIBPIPEWIRE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_misc_asynch.h"
#include "stream_misc_delay.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_file_sink.h"

#include "stream_stat_statistic_analysis.h"
#include "stream_stat_statistic_report.h"

#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo_spectrum_analyzer.h"
#endif // GTK_SUPPORT

#include "test_u_mic_visualize_common.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_U_MicVisualize_DirectShow_SessionData,
                                 struct Test_U_MicVisualize_Statistic,
                                 struct Stream_UserData> Test_U_DirectShow_SessionManager_t;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_U_MicVisualize_MediaFoundation_SessionData,
                                 struct Test_U_MicVisualize_Statistic,
                                 struct Stream_UserData> Test_U_MediaFoundation_SessionManager_t;

typedef Stream_Decoder_MP3Decoder_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_U_DirectShow_Message,
                                    Test_U_DirectShow_SessionMessage,
                                    struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_U_MicVisualize_DirectShow_StreamState,
                                    struct Test_U_MicVisualize_Statistic,
                                    Test_U_DirectShow_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData,
                                    struct _AMMediaType> Test_U_Dec_MP3Decoder_DirectShow;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_mp3_decoder_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dec_MP3Decoder_DirectShow);                               // writer type

typedef Stream_Decoder_MP3Decoder_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_U_MediaFoundation_Message,
                                    Test_U_MediaFoundation_SessionMessage,
                                    struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_U_MicVisualize_MediaFoundation_StreamState,
                                    struct Test_U_MicVisualize_Statistic,
                                    Test_U_MediaFoundation_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData,
                                    IMFMediaType*> Test_U_Dec_MP3Decoder_MediaFoundation;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_mp3_decoder_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dec_MP3Decoder_MediaFoundation);                          // writer type

typedef Stream_Dec_Noise_Source_T<ACE_MT_SYNCH,
                                  Stream_ControlMessage_t,
                                  Test_U_DirectShow_Message,
                                  Test_U_DirectShow_SessionMessage,
                                  struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Test_U_MicVisualize_DirectShow_StreamState,
                                  struct Test_U_MicVisualize_Statistic,
                                  Test_U_DirectShow_SessionManager_t,
                                  Common_Timer_Manager_t,
                                  struct _AMMediaType> Test_U_Dec_Noise_Source_DirectShow;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_noise_source_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dec_Noise_Source_DirectShow);                             // writer type

typedef Stream_Dec_Noise_Source_T<ACE_MT_SYNCH,
                                  Stream_ControlMessage_t,
                                  Test_U_MediaFoundation_Message,
                                  Test_U_MediaFoundation_SessionMessage,
                                  struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Test_U_MicVisualize_MediaFoundation_StreamState,
                                  struct Test_U_MicVisualize_Statistic,
                                  Test_U_MediaFoundation_SessionManager_t,
                                  Common_Timer_Manager_t,
                                  IMFMediaType*> Test_U_Dec_Noise_Source_MediaFoundation;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_noise_source_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_Dec_Noise_Source_MediaFoundation);                             // writer type

typedef Stream_Dev_Mic_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Test_U_DirectShow_Message,
                                           Test_U_DirectShow_SessionMessage,
                                           struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Test_U_MicVisualize_DirectShow_StreamState,
                                           struct Test_U_MicVisualize_Statistic,
                                           Test_U_DirectShow_SessionManager_t,
                                           Common_Timer_Manager_t> Test_U_Dev_Mic_Source_DirectShow;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_directshow_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dev_Mic_Source_DirectShow);                               // writer type

typedef Stream_Dev_Mic_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Test_U_MediaFoundation_Message,
                                                Test_U_MediaFoundation_SessionMessage,
                                                struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                Test_U_MicVisualize_MediaFoundation_StreamState,
                                                struct Test_U_MicVisualize_Statistic,
                                                Test_U_MediaFoundation_SessionManager_t,
                                                Common_Timer_Manager_t> Test_U_Dev_Mic_Source_MediaFoundation;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_Dev_Mic_Source_MediaFoundation);                               // writer type

typedef Stream_Dev_Mic_Source_WASAPI_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_U_DirectShow_Message,
                                       Test_U_DirectShow_SessionMessage,
                                       struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_U_MicVisualize_DirectShow_StreamState,
                                       struct Test_U_MicVisualize_Statistic,
                                       Test_U_DirectShow_SessionManager_t,
                                       Common_Timer_Manager_t,
                                       struct _AMMediaType> Test_U_Dev_Mic_Source_WASAPI;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_wasapi_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dev_Mic_Source_WASAPI);                                   // writer type

typedef Stream_Dev_Mic_Source_WaveIn_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_U_DirectShow_Message,
                                       Test_U_DirectShow_SessionMessage,
                                       struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_U_MicVisualize_DirectShow_StreamState,
                                       struct Test_U_MicVisualize_Statistic,
                                       Test_U_DirectShow_SessionManager_t,
                                       Common_Timer_Manager_t,
                                       struct _AMMediaType> Test_U_Dev_Mic_Source_WaveIn;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_wavein_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dev_Mic_Source_WaveIn);                                   // writer type

typedef Stream_Dev_Mic_Source_WASAPI_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_U_MediaFoundation_Message,
                                       Test_U_MediaFoundation_SessionMessage,
                                       struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_U_MicVisualize_MediaFoundation_StreamState,
                                       struct Test_U_MicVisualize_Statistic,
                                       Test_U_MediaFoundation_SessionManager_t,
                                       Common_Timer_Manager_t,
                                       IMFMediaType*> Test_U_Dev_Mic_Source_WASAPI2;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_wasapi_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dev_Mic_Source_WASAPI2);                                  // writer type

typedef Stream_Dev_Mic_Source_WaveIn_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_U_MediaFoundation_Message,
                                       Test_U_MediaFoundation_SessionMessage,
                                       struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_U_MicVisualize_MediaFoundation_StreamState,
                                       struct Test_U_MicVisualize_Statistic,
                                       Test_U_MediaFoundation_SessionManager_t,
                                       Common_Timer_Manager_t,
                                       IMFMediaType*> Test_U_Dev_Mic_Source_WaveIn2;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_wavein_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_Dev_Mic_Source_WaveIn2);                                       // writer type
#else
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_U_MicVisualize_SessionData,
                                 struct Test_U_MicVisualize_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

typedef Stream_Decoder_MP3Decoder_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_U_Message,
                                    Test_U_SessionMessage,
                                    struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_U_MicVisualize_StreamState,
                                    struct Test_U_MicVisualize_Statistic,
                                    Test_U_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData,
                                    struct Stream_MediaFramework_ALSA_MediaType> Test_U_Dec_MP3Decoder_ALSA;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_mp3_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_Dec_MP3Decoder_ALSA);                               // writer type

typedef Stream_Dec_Noise_Source_T<ACE_MT_SYNCH,
                                  Stream_ControlMessage_t,
                                  Test_U_Message,
                                  Test_U_SessionMessage,
                                  struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Test_U_MicVisualize_StreamState,
                                  struct Test_U_MicVisualize_Statistic,
                                  Test_U_SessionManager_t,
                                  Common_Timer_Manager_t,
                                  struct Stream_MediaFramework_ALSA_MediaType> Test_U_Dec_Noise_Source_ALSA;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_noise_source_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_Dec_Noise_Source_ALSA);                             // writer type

typedef Stream_Dev_Mic_Source_ALSA_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_U_Message,
                                     Test_U_SessionMessage,
                                     struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_U_MicVisualize_StreamState,
                                     struct Test_U_MicVisualize_Statistic,
                                     Test_U_SessionManager_t,
                                     Common_Timer_Manager_t> Test_U_Dev_Mic_Source_ALSA;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_alsa_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_U_Dev_Mic_Source_ALSA);                          // writer type
#if defined (LIBPIPEWIRE_SUPPORT)
typedef Stream_Dev_Mic_Source_Pipewire_T<ACE_MT_SYNCH,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         struct Test_U_MicVisualize_StreamState,
                                         struct Test_U_MicVisualize_Statistic,
                                         Test_U_SessionManager_t,
                                         Common_Timer_Manager_t> Test_U_Dev_Mic_Source_Pipewire;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_dev_mic_source_pipewire_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_Dev_Mic_Source_Pipewire);                                 // writer type
#endif // LIBPIPEWIRE_SUPPORT

typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Test_U_Message,
                              Test_U_SessionMessage,
                              struct Stream_MediaFramework_ALSA_MediaType,
                              struct Stream_UserData> Test_U_ALSA_Delay;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_delay_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_U_ALSA_Delay);                                   // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Asynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_DirectShow_Message,
                               Test_U_DirectShow_SessionMessage> Test_U_MicVisualize_DirectShow_Asynch;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_asynch_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_Asynch);                           // name

typedef Stream_Statistic_StatisticAnalysis_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                             Stream_ControlMessage_t,
                                             Test_U_DirectShow_Message,
                                             Test_U_DirectShow_SessionMessage,
                                             struct Test_U_MicVisualize_Statistic,
                                             Test_U_MicVisualize_DirectShow_SessionData,
                                             Test_U_MicVisualize_DirectShow_SessionData_t,
                                             struct _AMMediaType,
                                             float> Test_U_MicVisualize_DirectShow_StatisticAnalysis;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_stat_analysis_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_StatisticAnalysis);                // name

typedef Stream_Statistic_StatisticAnalysis_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                             Stream_ControlMessage_t,
                                             Test_U_MediaFoundation_Message,
                                             Test_U_MediaFoundation_SessionMessage,
                                             struct Test_U_MicVisualize_Statistic,
                                             Test_U_MicVisualize_MediaFoundation_SessionData,
                                             Test_U_MicVisualize_MediaFoundation_SessionData_t,
                                             IMFMediaType*,
                                             float> Test_U_MicVisualize_MediaFoundation_StatisticAnalysis;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_stat_analysis_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_StatisticAnalysis);                // name

//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_DirectShow_Message,
//                                                      Test_U_DirectShow_SessionMessage,
//                                                      Stream_CommandType_t,
//                                                      struct Test_U_MicVisualize_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_U_MicVisualize_DirectShow_SessionData,
//                                                      Test_U_MicVisualize_DirectShow_SessionData_t> Test_U_MicVisualize_DirectShow_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_DirectShow_Message,
//                                                      Test_U_DirectShow_SessionMessage,
//                                                      Stream_CommandType_t,
//                                                      struct Test_U_MicVisualize_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_U_MicVisualize_DirectShow_SessionData,
//                                                      Test_U_MicVisualize_DirectShow_SessionData_t> Test_U_MicVisualize_DirectShow_Statistic_WriterTask_t;
//DATASTREAM_MODULE_DUPLEX (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
//                          enum Stream_SessionMessageType,                                  // session event type
//                          struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                                // stream notification interface type
//                          Test_U_MicVisualize_DirectShow_Statistic_ReaderTask_t,            // reader type
//                          Test_U_MicVisualize_DirectShow_Statistic_WriterTask_t,            // writer type
//                          Test_U_MicVisualize_DirectShow_StatisticReport);                  // name
//
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_MediaFoundation_Message,
//                                                      Test_U_MediaFoundation_SessionMessage,
//                                                      Stream_CommandType_t,
//                                                      struct Test_U_MicVisualize_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_U_MicVisualize_MediaFoundation_SessionData,
//                                                      Test_U_MicVisualize_MediaFoundation_SessionData_t> Test_U_MicVisualize_MediaFoundation_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_MediaFoundation_Message,
//                                                      Test_U_MediaFoundation_SessionMessage,
//                                                      Stream_CommandType_t,
//                                                      struct Test_U_MicVisualize_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Test_U_MicVisualize_MediaFoundation_SessionData,
//                                                      Test_U_MicVisualize_MediaFoundation_SessionData_t> Test_U_MicVisualize_MediaFoundation_Statistic_WriterTask_t;
//DATASTREAM_MODULE_DUPLEX (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
//                          enum Stream_SessionMessageType,                                       // session event type
//                          struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                                     // stream notification interface type
//                          Test_U_MicVisualize_MediaFoundation_Statistic_ReaderTask_t,            // reader type
//                          Test_U_MicVisualize_MediaFoundation_Statistic_WriterTask_t,            // writer type
//                          Test_U_MicVisualize_MediaFoundation_StatisticReport);                  // name

typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_DirectShow_Message,
                                                      Test_U_DirectShow_SessionMessage,
                                                      Test_U_MicVisualize_DirectShow_SessionData_t> Test_U_MicVisualize_DirectShow_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_DirectShow_Message,
                                                      Test_U_DirectShow_SessionMessage,
                                                      Test_U_MicVisualize_DirectShow_SessionData_t> Test_U_MicVisualize_DirectShow_Distributor_Writer_t;
DATASTREAM_MODULE_DUPLEX (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                                  // session event type
                          struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                                // stream notification interface type
                          Test_U_MicVisualize_DirectShow_Distributor_Reader_t,              // reader task
                          Test_U_MicVisualize_DirectShow_Distributor_Writer_t,              // writer task
                          Test_U_MicVisualize_DirectShow_Distributor);                      // name
typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_MediaFoundation_Message,
                                                      Test_U_MediaFoundation_SessionMessage,
                                                      Test_U_MicVisualize_MediaFoundation_SessionData_t> Test_U_MicVisualize_MediaFoundation_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_MediaFoundation_Message,
                                                      Test_U_MediaFoundation_SessionMessage,
                                                      Test_U_MicVisualize_MediaFoundation_SessionData_t> Test_U_MicVisualize_MediaFoundation_Distributor_Writer_t;
DATASTREAM_MODULE_DUPLEX (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                                       // session event type
                          struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                                     // stream notification interface type
                          Test_U_MicVisualize_MediaFoundation_Distributor_Reader_t,              // reader task
                          Test_U_MicVisualize_MediaFoundation_Distributor_Writer_t,              // writer task
                          Test_U_MicVisualize_MediaFoundation_Distributor);                      // name
#else
typedef Stream_Statistic_StatisticAnalysis_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                             Stream_ControlMessage_t,
                                             Test_U_Message,
                                             Test_U_SessionMessage,
                                             struct Test_U_MicVisualize_Statistic,
                                             Test_U_MicVisualize_SessionData,
                                             Test_U_MicVisualize_SessionData_t,
                                             struct Stream_MediaFramework_ALSA_MediaType,
                                             float> Test_U_MicVisualize_StatisticAnalysis;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_stat_analysis_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_U_MicVisualize_StatisticAnalysis);                // name

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                                      ACE_Message_Block,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Test_U_MicVisualize_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_U_MicVisualize_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                                      ACE_Message_Block,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Test_U_MicVisualize_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_U_MicVisualize_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (Test_U_MicVisualize_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                       // session event type
                          struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                     // stream notification interface type
                          Test_U_MicVisualize_Module_Statistic_ReaderTask_t,     // reader type
                          Test_U_MicVisualize_Module_Statistic_WriterTask_t,     // writer type
                          Test_U_MicVisualize_StatisticReport);                  // name

typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Test_U_MicVisualize_SessionData_t> Test_U_MicVisualize_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Test_U_MicVisualize_SessionData_t> Test_U_MicVisualize_Distributor_Writer_t;
DATASTREAM_MODULE_DUPLEX (Test_U_MicVisualize_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                       // session event type
                          struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                     // stream notification interface type
                          Test_U_MicVisualize_Distributor_Reader_t,              // reader type
                          Test_U_MicVisualize_Distributor_Writer_t,              // writer type
                          Test_U_MicVisualize_Distributor);                      // module name prefix
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Delay_2<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Test_U_DirectShow_Message,
                              Test_U_DirectShow_SessionMessage,
                              struct _AMMediaType,
                              struct Stream_UserData> Test_U_MicVisualize_DirectShow_Delay;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_delay_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_Delay);                            // writer type

typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Test_U_MediaFoundation_Message,
                              Test_U_MediaFoundation_SessionMessage,
                              IMFMediaType*,
                              struct Stream_UserData> Test_U_MicVisualize_MediaFoundation_Delay;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_delay_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_Delay);                            // writer type

#if defined (GTK_SUPPORT)
typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                                          Stream_ControlMessage_t,
                                                          Test_U_DirectShow_Message,
                                                          Test_U_DirectShow_SessionMessage,
                                                          Test_U_MicVisualize_DirectShow_SessionData,
                                                          Test_U_MicVisualize_DirectShow_SessionData_t,
                                                          Common_Timer_Manager_t,
                                                          struct _AMMediaType,
                                                          float,
                                                          FFT_ALGORITHM_UNKNOWN> Test_U_MicVisualize_DirectShow_Vis_SpectrumAnalyzer;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_Vis_SpectrumAnalyzer);             // writer type
typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                                          Stream_ControlMessage_t,
                                                          Test_U_MediaFoundation_Message,
                                                          Test_U_MediaFoundation_SessionMessage,
                                                          Test_U_MicVisualize_MediaFoundation_SessionData,
                                                          Test_U_MicVisualize_MediaFoundation_SessionData_t,
                                                          Common_Timer_Manager_t,
                                                          IMFMediaType*,
                                                          float,
                                                          FFT_ALGORITHM_UNKNOWN> Test_U_MicVisualize_MediaFoundation_Vis_SpectrumAnalyzer;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_Vis_SpectrumAnalyzer);             // writer type
#endif // GTK_SUPPORT

#if defined (SOX_SUPPORT)
typedef Stream_Decoder_SoXResampler_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_U_DirectShow_Message,
                                      Test_U_DirectShow_SessionMessage,
                                      struct _AMMediaType> Test_U_MicVisualize_DirectShow_SoXResampler;
typedef Stream_Decoder_SoXResampler_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_U_MediaFoundation_Message,
                                      Test_U_MediaFoundation_SessionMessage,
                                      IMFMediaType*> Test_U_MicVisualize_MediaFoundation_SoXResampler;

DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_sox_resampler_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_SoXResampler);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_sox_resampler_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_SoXResampler);                     // writer type
#endif // SOX_SUPPORT

typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_U_DirectShow_Message,
                                    Test_U_DirectShow_SessionMessage,
                                    Test_U_MicVisualize_DirectShow_SessionData_t,
                                    Test_U_MicVisualize_DirectShow_SessionData,
                                    struct _AMMediaType,
                                    struct Stream_UserData> Test_U_MicVisualize_DirectShow_WAVEncoder;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_WAVEncoder);                       // writer type
typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_U_MediaFoundation_Message,
                                    Test_U_MediaFoundation_SessionMessage,
                                    Test_U_MicVisualize_MediaFoundation_SessionData_t,
                                    Test_U_MicVisualize_MediaFoundation_SessionData,
                                    IMFMediaType*,
                                    struct Stream_UserData> Test_U_MicVisualize_MediaFoundation_WAVEncoder;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_WAVEncoder);                       // writer type
#else
#if defined (SOX_SUPPORT)
typedef Stream_Decoder_SoXEffect_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_Message,
                                   Test_U_SessionMessage,
                                   Test_U_MicVisualize_SessionData_t,
                                   Test_U_MicVisualize_SessionData,
                                   struct Stream_MediaFramework_ALSA_MediaType> Test_U_MicVisualize_SoXEffect;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_dec_sox_effect_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_U_MicVisualize_SoXEffect);                          // writer type
#endif // SOX_SUPPORT

typedef Stream_Dev_Target_ALSA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_U_Message,
                                 Test_U_SessionMessage,
                                 Test_U_MicVisualize_SessionData> Test_U_MicVisualize_Target_ALSA;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dev_target_alsa_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_U_MicVisualize_Target_ALSA);                         // writer type
#if defined (GTK_SUPPORT)
typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                                          Stream_ControlMessage_t,
                                                          Test_U_Message,
                                                          Test_U_SessionMessage,
                                                          Test_U_MicVisualize_SessionData,
                                                          Test_U_MicVisualize_SessionData_t,
                                                          Common_Timer_Manager_t,
                                                          struct Stream_MediaFramework_ALSA_MediaType,
                                                          float> Test_U_MicVisualize_Vis_SpectrumAnalyzer;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                                // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,     // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_U_MicVisualize_Vis_SpectrumAnalyzer);                      // writer type
#endif // GTK_SUPPORT
typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_U_Message,
                                    Test_U_SessionMessage,
                                    Test_U_MicVisualize_SessionData_t,
                                    Test_U_MicVisualize_SessionData,
                                    struct Stream_MediaFramework_ALSA_MediaType,
                                    struct Test_U_MicVisualize_Statistic> Test_U_MicVisualize_ALSA_WAVEncoder;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_U_MicVisualize_ALSA_WAVEncoder);                     // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Test_U_DirectShow_Message,
                                                         struct Test_U_MicVisualize_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_U_MicVisualize_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Test_U_DirectShow_Message,
                                                                struct Test_U_MicVisualize_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_U_MicVisualize_AsynchDirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Target_T<Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                                                         Common_TimePolicy_t,
                                                                         struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                                                         Stream_ControlMessage_t,
                                                                         Test_U_DirectShow_Message,
                                                                         Test_U_DirectShow_SessionMessage,
                                                                         enum Stream_ControlType,
                                                                         enum Stream_SessionMessageType,
                                                                         struct Stream_UserData>,
                                                  struct Test_U_MicVisualize_DirectShow_FilterConfiguration,
                                                  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                  struct _AMMediaType,
                                                  Test_U_MicVisualize_DirectShowFilter_t> Test_U_MicVisualize_DirectShow_Target;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_directshow_target_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_Target);                           // writer type
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

typedef Stream_MediaFramework_DirectShow_Source_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Test_U_DirectShow_Message,
                                                  Test_U_DirectShow_SessionMessage,
                                                  Test_U_MicVisualize_DirectShow_SessionData,
                                                  struct _AMMediaType> Test_U_MicVisualize_DirectShow_Source;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_directshow_source_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_Source);                           // writer type

typedef Stream_MediaFramework_MediaFoundation_MediaSource_T<Common_TimePolicy_t,
                                                            Test_U_MediaFoundation_Message,
                                                            struct Stream_MediaFramework_MediaFoundation_Configuration> Test_U_MicVisualize_MediaFoundation_MediaSource_t;
typedef Stream_MediaFramework_MediaFoundation_Target_T<ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       Test_U_MediaFoundation_Message,
                                                       Test_U_MediaFoundation_SessionMessage,
                                                       Test_U_MicVisualize_MediaFoundation_SessionData,
                                                       Test_U_MicVisualize_MediaFoundation_SessionData_t,
                                                       IMFMediaType*> Test_U_MicVisualize_MediaFoundation_Target;
//DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                  // session data type
//                              enum Stream_SessionMessageType,                                  // session event type
//                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_lib_mediafoundation_target_module_name_string,
//                              Stream_INotify_t,                                                // stream notification interface type
//                              Test_U_MicVisualize_MediaFoundation_Target);                      // writer type

typedef Stream_MediaFramework_MediaFoundation_Source_T<ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       Test_U_MediaFoundation_Message,
                                                       Test_U_MediaFoundation_SessionMessage,
                                                       Test_U_MicVisualize_MediaFoundation_SessionData_t,
                                                       Test_U_MicVisualize_MediaFoundation_SessionData,
                                                       IMFMediaType*,
                                                       struct Stream_UserData> Test_U_MicVisualize_MediaFoundation_Source;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_mediafoundation_source_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_Source);                      // writer type

typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_DirectShow_Message,
                                   Test_U_DirectShow_SessionMessage,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   struct _AMMediaType> Test_U_MicVisualize_DirectShow_WASAPIOut;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_WASAPIOut);                        // writer type
typedef Stream_Dev_Target_WavOut_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_DirectShow_Message,
                                   Test_U_DirectShow_SessionMessage,
                                   Test_U_MicVisualize_DirectShow_SessionData,
                                   struct _AMMediaType> Test_U_MicVisualize_DirectShow_WavOut;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_target_wavout_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_WavOut);                           // writer type

typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_MediaFoundation_Message,
                                   Test_U_MediaFoundation_SessionMessage,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   IMFMediaType*> Test_U_MicVisualize_MediaFoundation_WASAPIOut;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_WASAPIOut);                        // writer type
typedef Stream_Dev_Target_WavOut_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_MediaFoundation_Message,
                                   Test_U_MediaFoundation_SessionMessage,
                                   Test_U_MicVisualize_MediaFoundation_SessionData,
                                   IMFMediaType*> Test_U_MicVisualize_MediaFoundation_WavOut;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_target_wavout_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_WavOut);                           // writer type

typedef Stream_Module_FileWriter_2<Common_TimePolicy_t,
                                   struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_DirectShow_Message,
                                   Test_U_DirectShow_SessionMessage> Test_U_MicVisualize_DirectShow_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_U_MicVisualize_DirectShow_FileWriter);                       // writer type
typedef Stream_Module_FileWriter_2<Common_TimePolicy_t,
                                   struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_MediaFoundation_Message,
                                   Test_U_MediaFoundation_SessionMessage> Test_U_MicVisualize_MediaFoundation_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_U_MicVisualize_MediaFoundation_FileWriter);                       // writer type

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_DirectShow_Message,
                                       Test_U_DirectShow_SessionMessage,
                                       Test_U_MicVisualize_DirectShow_SessionData,
                                       struct Stream_UserData> Test_U_DirectShow_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_MicVisualize_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_DirectShow_MessageHandler);                                // writer type
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_MediaFoundation_Message,
                                       Test_U_MediaFoundation_SessionMessage,
                                       Test_U_MicVisualize_MediaFoundation_SessionData,
                                       struct Stream_UserData> Test_U_MediaFoundation_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_MediaFoundation_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_MicVisualize_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_MediaFoundation_MessageHandler);                           // writer type
#else
//typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
//                                   Common_TimePolicy_t,
//                                   struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
//                                   Stream_ControlMessage_t,
//                                   Test_U_Message,
//                                   Test_U_SessionMessage> Test_U_MicVisualize_Module_FileWriter;
//DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                       // session data type
//                              enum Stream_SessionMessageType,                       // session event type
//                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_file_sink_module_name_string,
//                              Stream_INotify_t,                                     // stream notification interface type
//                              Test_U_MicVisualize_Module_FileWriter);                // writer type

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       Test_U_MicVisualize_SessionData,
                                       struct Stream_UserData> Test_U_ALSA_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_MicVisualize_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_U_MicVisualize_ALSA_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_U_ALSA_MessageHandler);                                 // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
