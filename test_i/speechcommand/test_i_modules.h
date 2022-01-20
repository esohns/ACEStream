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

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_defines.h"
#include "stream_misc_messagehandler.h"

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

#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source.h"
#include "stream_lib_directshow_source_filter.h"
#include "stream_lib_directshow_target.h"

#include "stream_lib_mediafoundation_mediasource.h"
#include "stream_lib_mediafoundation_source.h"
#include "stream_lib_mediafoundation_target.h"
#else
#include "stream_dev_mic_source_alsa.h"
#include "stream_dev_target_alsa.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_misc_distributor.h"

#include "stream_file_sink.h"

#include "stream_stat_statistic_analysis.h"
#include "stream_stat_statistic_report.h"

#include "stream_dec_deepspeech_decoder.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo_spectrum_analyzer.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "test_i_message.h"
#include "test_i_speechcommand_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Mic_Source_WaveIn_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_I_DirectShow_Message,
                                       Test_I_DirectShow_SessionMessage_t,
                                       struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_I_SpeechCommand_DirectShow_StreamState,
                                       Test_I_SpeechCommand_DirectShow_SessionData,
                                       Test_I_SpeechCommand_DirectShow_SessionData_t,
                                       struct Test_I_Statistic,
                                       Common_Timer_Manager_t,
                                       struct _AMMediaType> Test_I_Mic_Source_DirectShow_WaveIn;
typedef Stream_Dev_Mic_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Test_I_DirectShow_Message,
                                           Test_I_DirectShow_SessionMessage_t,
                                           struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Test_I_SpeechCommand_DirectShow_StreamState,
                                           Test_I_SpeechCommand_DirectShow_SessionData,
                                           Test_I_SpeechCommand_DirectShow_SessionData_t,
                                           struct Test_I_Statistic,
                                           Common_Timer_Manager_t> Test_I_Mic_Source_DirectShow;
typedef Stream_Dev_Mic_Source_WASAPI_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_I_DirectShow_Message,
                                       Test_I_DirectShow_SessionMessage_t,
                                       struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_I_SpeechCommand_DirectShow_StreamState,
                                       Test_I_SpeechCommand_DirectShow_SessionData,
                                       Test_I_SpeechCommand_DirectShow_SessionData_t,
                                       struct Test_I_Statistic,
                                       Common_Timer_Manager_t,
                                       struct _AMMediaType> Test_I_Mic_Source_DirectShow_WASAPI;

typedef Stream_Dev_Mic_Source_WaveIn_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_I_MediaFoundation_Message,
                                       Test_I_MediaFoundation_SessionMessage_t,
                                       struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_I_SpeechCommand_MediaFoundation_StreamState,
                                       Test_I_SpeechCommand_MediaFoundation_SessionData,
                                       Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                       struct Test_I_Statistic,
                                       Common_Timer_Manager_t,
                                       IMFMediaType*> Test_I_Mic_Source_MediaFoundation_WaveIn;
typedef Stream_Dev_Mic_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Test_I_MediaFoundation_Message,
                                                Test_I_MediaFoundation_SessionMessage_t,
                                                struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                Test_I_SpeechCommand_MediaFoundation_StreamState,
                                                Test_I_SpeechCommand_MediaFoundation_SessionData,
                                                Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                                struct Test_I_Statistic,
                                                Common_Timer_Manager_t> Test_I_Mic_Source_MediaFoundation;
typedef Stream_Dev_Mic_Source_WASAPI_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Test_I_MediaFoundation_Message,
                                       Test_I_MediaFoundation_SessionMessage_t,
                                       struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Test_I_SpeechCommand_MediaFoundation_StreamState,
                                       Test_I_SpeechCommand_MediaFoundation_SessionData,
                                       Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                       struct Test_I_Statistic,
                                       Common_Timer_Manager_t,
                                       IMFMediaType*> Test_I_Mic_Source_MediaFoundation_WASAPI;

//////////////////////////////////////////

#if defined (SOX_SUPPORT)
typedef Stream_Decoder_SoXResampler_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_DirectShow_Message,
                                      Test_I_DirectShow_SessionMessage_t,
                                      Test_I_SpeechCommand_DirectShow_SessionData_t,
                                      Test_I_SpeechCommand_DirectShow_SessionData,
                                      struct _AMMediaType> Test_I_DirectShow_SoXResampler;
typedef Stream_Decoder_SoXResampler_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_DirectShow_Message,
                                      Test_I_MediaFoundation_SessionMessage_t,
                                      Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                      Test_I_SpeechCommand_MediaFoundation_SessionData,
                                      IMFMediaType*> Test_I_MediaFoundation_SoXResampler;
#endif // SOX_SUPPORT

//////////////////////////////////////////

typedef Stream_Statistic_StatisticAnalysis_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                             Stream_ControlMessage_t,
                                             Test_I_DirectShow_Message,
                                             Test_I_DirectShow_SessionMessage_t,
                                             struct Test_I_Statistic,
                                             Test_I_SpeechCommand_DirectShow_SessionData,
                                             Test_I_SpeechCommand_DirectShow_SessionData_t,
                                             struct _AMMediaType,
                                             double> Test_I_DirectShow_StatisticAnalysis;
typedef Stream_Statistic_StatisticAnalysis_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                             Stream_ControlMessage_t,
                                             Test_I_DirectShow_Message,
                                             Test_I_MediaFoundation_SessionMessage_t,
                                             struct Test_I_Statistic,
                                             Test_I_SpeechCommand_MediaFoundation_SessionData,
                                             Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                             IMFMediaType*,
                                             double> Test_I_MediaFoundation_StatisticAnalysis;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_DirectShow_Message,
                                                      Test_I_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Test_I_SpeechCommand_DirectShow_SessionData,
                                                      Test_I_SpeechCommand_DirectShow_SessionData_t> Test_I_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_DirectShow_Message,
                                                      Test_I_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Test_I_SpeechCommand_DirectShow_SessionData,
                                                      Test_I_SpeechCommand_DirectShow_SessionData_t> Test_I_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_MediaFoundation_Message,
                                                      Test_I_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Test_I_SpeechCommand_MediaFoundation_SessionData,
                                                      Test_I_SpeechCommand_MediaFoundation_SessionData_t> Test_I_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_MediaFoundation_Message,
                                                      Test_I_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Test_I_SpeechCommand_MediaFoundation_SessionData,
                                                      Test_I_SpeechCommand_MediaFoundation_SessionData_t> Test_I_MediaFoundation_Statistic_WriterTask_t;

//////////////////////////////////////////

typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Test_I_DirectShow_Message,
                                                         struct Stream_MediaFramework_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_I_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Test_I_DirectShow_Message,
                                                                struct Stream_MediaFramework_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_I_AsynchDirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Target_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Test_I_DirectShow_Message,
                                                  Test_I_DirectShow_SessionMessage_t,
                                                  Test_I_SpeechCommand_DirectShow_SessionData,
                                                  struct Stream_MediaFramework_DirectShow_FilterConfiguration,
                                                  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                  struct _AMMediaType,
                                                  Test_I_DirectShowFilter_t> Test_I_DirectShow_Target;
typedef Stream_MediaFramework_DirectShow_Source_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Test_I_DirectShow_Message,
                                                  Test_I_DirectShow_SessionMessage_t,
                                                  Test_I_SpeechCommand_DirectShow_SessionData,
                                                  struct _AMMediaType> Test_I_DirectShow_Source;

typedef Stream_MediaFramework_MediaFoundation_MediaSource_T<Common_TimePolicy_t,
                                                            Test_I_MediaFoundation_Message,
                                                            struct Stream_MediaFramework_MediaFoundation_Configuration> Test_I_MediaFoundation_MediaSource_t;
typedef Stream_MediaFramework_MediaFoundation_Target_T<ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       Test_I_MediaFoundation_Message,
                                                       Test_I_MediaFoundation_SessionMessage_t,
                                                       Test_I_SpeechCommand_MediaFoundation_SessionData,
                                                       Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                                       IMFMediaType*> Test_I_MediaFoundation_Target;
typedef Stream_MediaFramework_MediaFoundation_Source_T<ACE_MT_SYNCH,
                                                       Common_TimePolicy_t,
                                                       struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                                       Stream_ControlMessage_t,
                                                       Test_I_MediaFoundation_Message,
                                                       Test_I_MediaFoundation_SessionMessage_t,
                                                       Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                                       Test_I_SpeechCommand_MediaFoundation_SessionData,
                                                       IMFMediaType*,
                                                       struct Stream_UserData> Test_I_MediaFoundation_Source;

//////////////////////////////////////////

typedef Stream_Miscellaneous_Distributor_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_DirectShow_Message,
                                           Test_I_DirectShow_SessionMessage_t,
                                           Test_I_SpeechCommand_DirectShow_SessionData_t> Test_I_DirectShow_Distributor;

typedef Stream_Miscellaneous_Distributor_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_MediaFoundation_Message,
                                           Test_I_MediaFoundation_SessionMessage_t,
                                           Test_I_SpeechCommand_MediaFoundation_SessionData_t> Test_I_MediaFoundation_Distributor;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                                          Stream_ControlMessage_t,
                                                          Test_I_DirectShow_Message,
                                                          Test_I_DirectShow_SessionMessage_t,
                                                          Test_I_SpeechCommand_DirectShow_SessionData,
                                                          Test_I_SpeechCommand_DirectShow_SessionData_t,
                                                          Common_Timer_Manager_t,
                                                          struct _AMMediaType,
                                                          double> Test_I_DirectShow_Vis_SpectrumAnalyzer;

typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                                          Stream_ControlMessage_t,
                                                          Test_I_MediaFoundation_Message,
                                                          Test_I_MediaFoundation_SessionMessage_t,
                                                          Test_I_SpeechCommand_MediaFoundation_SessionData,
                                                          Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                                          Common_Timer_Manager_t,
                                                          IMFMediaType*,
                                                          double> Test_I_MediaFoundation_Vis_SpectrumAnalyzer;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

//////////////////////////////////////////

typedef Stream_Decoder_DeepSpeechDecoder_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_DirectShow_Message,
                                           Test_I_DirectShow_SessionMessage_t,
                                           Test_I_SpeechCommand_DirectShow_SessionData_t,
                                           struct _AMMediaType> Test_I_DirectShow_DeepSpeechDecoder;

typedef Stream_Decoder_DeepSpeechDecoder_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_MediaFoundation_Message,
                                           Test_I_MediaFoundation_SessionMessage_t,
                                           Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                           IMFMediaType*> Test_I_MediaFoundation_DeepSpeechDecoder;

//////////////////////////////////////////

typedef Stream_Dev_Target_WavOut_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_DirectShow_Message,
                                   Test_I_DirectShow_SessionMessage_t,
                                   Test_I_SpeechCommand_DirectShow_SessionData,
                                   struct _AMMediaType> Test_I_DirectShow_Target_WaveOut;
typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_DirectShow_Message,
                                   Test_I_DirectShow_SessionMessage_t,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   struct _AMMediaType> Test_I_DirectShow_Target_WASAPI;

typedef Stream_Dev_Target_WavOut_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_MediaFoundation_Message,
                                   Test_I_MediaFoundation_SessionMessage_t,
                                   Test_I_SpeechCommand_MediaFoundation_SessionData,
                                   IMFMediaType*> Test_I_MediaFoundation_Target_WaveOut;
typedef Stream_Dev_Target_WASAPI_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_MediaFoundation_Message,
                                   Test_I_MediaFoundation_SessionMessage_t,
                                   enum Stream_ControlType,
                                   enum Stream_SessionMessageType,
                                   struct Stream_UserData,
                                   IMFMediaType*> Test_I_MediaFoundation_Target_WASAPI;

//////////////////////////////////////////

typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_I_DirectShow_Message,
                                    Test_I_DirectShow_SessionMessage_t,
                                    Test_I_SpeechCommand_DirectShow_SessionData_t,
                                    Test_I_SpeechCommand_DirectShow_SessionData,
                                    struct _AMMediaType,
                                    struct Stream_UserData> Test_I_DirectShow_WAVEncoder;

typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_I_MediaFoundation_Message,
                                    Test_I_MediaFoundation_SessionMessage_t,
                                    Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                    Test_I_SpeechCommand_MediaFoundation_SessionData,
                                    IMFMediaType*,
                                    struct Stream_UserData> Test_I_MediaFoundation_WAVEncoder;

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_DirectShow_Message,
                                   Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_FileWriter;

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_MediaFoundation_Message,
                                   Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_FileWriter;

//////////////////////////////////////////

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_DirectShow_Message,
                                       Test_I_DirectShow_SessionMessage_t,
                                       Test_I_SpeechCommand_DirectShow_SessionData,
                                       struct Stream_UserData> Test_I_DirectShow_MessageHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_MediaFoundation_Message,
                                       Test_I_MediaFoundation_SessionMessage_t,
                                       Test_I_SpeechCommand_MediaFoundation_SessionData,
                                       struct Stream_UserData> Test_I_MediaFoundation_MessageHandler;
#else
typedef Stream_Dev_Mic_Source_ALSA_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_ALSA_SessionMessage_t,
                                     struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_SpeechCommand_ALSA_StreamState,
                                     Test_I_SpeechCommand_ALSA_SessionData,
                                     Test_I_SpeechCommand_ALSA_SessionData_t,
                                     struct Test_I_Statistic,
                                     Common_Timer_Manager_t> Test_I_Mic_Source_ALSA;

//////////////////////////////////////////

#if defined (SOX_SUPPORT)
typedef Stream_Decoder_SoXResampler_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Message,
                                      Test_I_ALSA_SessionMessage_t,
                                      Test_I_SpeechCommand_ALSA_SessionData_t,
                                      Test_I_SpeechCommand_ALSA_SessionData,
                                      struct Stream_MediaFramework_ALSA_MediaType> Test_I_ALSA_SoXResampler;
#endif // SOX_SUPPORT

//////////////////////////////////////////

typedef Stream_Statistic_StatisticAnalysis_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                             Stream_ControlMessage_t,
                                             Test_I_Message,
                                             Test_I_ALSA_SessionMessage_t,
                                             struct Test_I_Statistic,
                                             Test_I_SpeechCommand_ALSA_SessionData,
                                             Test_I_SpeechCommand_ALSA_SessionData_t,
                                             struct Stream_MediaFramework_ALSA_MediaType,
                                             double> Test_I_ALSA_StatisticAnalysis;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                                      ACE_Message_Block,
                                                      Test_I_Message,
                                                      Test_I_ALSA_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Test_I_SpeechCommand_ALSA_SessionData,
                                                      Test_I_SpeechCommand_ALSA_SessionData_t> Test_I_ALSA_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message,
                                                      Test_I_ALSA_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Test_I_SpeechCommand_ALSA_SessionData,
                                                      Test_I_SpeechCommand_ALSA_SessionData_t> Test_I_ALSA_Statistic_WriterTask_t;

//////////////////////////////////////////

typedef Stream_Miscellaneous_Distributor_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_ALSA_SessionMessage_t,
                                           Test_I_SpeechCommand_ALSA_SessionData_t> Test_I_ALSA_Distributor;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
typedef Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_MT_SYNCH,
                                                          Common_TimePolicy_t,
                                                          struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                                          Stream_ControlMessage_t,
                                                          Test_I_Message,
                                                          Test_I_ALSA_SessionMessage_t,
                                                          Test_I_SpeechCommand_ALSA_SessionData,
                                                          Test_I_SpeechCommand_ALSA_SessionData_t,
                                                          Common_Timer_Manager_t,
                                                          struct Stream_MediaFramework_ALSA_MediaType,
                                                          double> Test_I_ALSA_Vis_SpectrumAnalyzer;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

//////////////////////////////////////////

typedef Stream_Decoder_DeepSpeechDecoder_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_ALSA_SessionMessage_t,
                                           Test_I_SpeechCommand_ALSA_SessionData_t,
                                           struct Stream_MediaFramework_ALSA_MediaType> Test_I_ALSA_DeepSpeechDecoder;

//////////////////////////////////////////

typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_ALSA_SessionMessage_t,
                                    Test_I_SpeechCommand_ALSA_SessionData_t,
                                    Test_I_SpeechCommand_ALSA_SessionData,
                                    struct Stream_MediaFramework_ALSA_MediaType,
                                    struct Stream_Statistic> Test_I_ALSA_WAVEncoder;

//////////////////////////////////////////

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_ALSA_SessionMessage_t,
                                       Test_I_SpeechCommand_ALSA_SessionData,
                                       struct Stream_UserData> Test_I_ALSA_MessageHandler;

//////////////////////////////////////////

typedef Stream_Dev_Target_ALSA_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Message,
                                 Test_I_ALSA_SessionMessage_t,
                                 Test_I_SpeechCommand_ALSA_SessionData> Test_I_Target_ALSA;
#endif // ACE_WIN32 || ACE_WIN64

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_Input_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       ACE_Message_Block,
                                       Stream_SessionMessageBase_T<enum Stream_SessionMessageType,
                                                                   Stream_SessionData_T<struct Stream_SessionData>,
                                                                   struct Stream_UserData>,
                                       struct Stream_SessionData,
                                       struct Stream_UserData> Test_I_InputMessageHandler;

//////////////////////////////////////////
// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                               // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,         // module handler configuration type
                              libacestream_default_dev_mic_source_wavein_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Mic_Source_DirectShow_WaveIn);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                                   // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,             // module handler configuration type
                              libacestream_default_dev_mic_source_directshow_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Mic_Source_DirectShow);                                   // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                               // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,         // module handler configuration type
                              libacestream_default_dev_mic_source_wasapi_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Mic_Source_DirectShow_WASAPI);                        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dev_mic_source_wavein_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Mic_Source_MediaFoundation_WaveIn);                   // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                                   // session data type
                              enum Stream_SessionMessageType,                                       // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,             // module handler configuration type
                              libacestream_default_dev_mic_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                     // stream notification interface type
                              Test_I_Mic_Source_MediaFoundation);                                   // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dev_mic_source_wasapi_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Mic_Source_MediaFoundation_WASAPI);                   // writer type

//////////////////////////////////////////

#if defined (SOX_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                    // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_sox_resampler_module_name_string,
                              Stream_INotify_t,                                                  // stream notification interface type
                              Test_I_DirectShow_SoXResampler);                                   // name

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                    // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_sox_resampler_module_name_string,
                              Stream_INotify_t,                                                  // stream notification interface type
                              Test_I_MediaFoundation_SoXResampler);                                   // name
#endif // SOX_SUPPORT

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                      // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_stat_analysis_module_name_string,
                              Stream_INotify_t,                                    // stream notification interface type
                              Test_I_DirectShow_StatisticAnalysis);                // name
DATASTREAM_MODULE_DUPLEX (Test_I_SpeechCommand_DirectShow_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                      // session event type
                          struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                    // stream notification interface type
                          Test_I_DirectShow_Statistic_ReaderTask_t,            // reader type
                          Test_I_DirectShow_Statistic_WriterTask_t,            // writer type
                          Test_I_DirectShow_StatisticReport);                  // name

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_stat_analysis_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_MediaFoundation_StatisticAnalysis);                // name
DATASTREAM_MODULE_DUPLEX (Test_I_SpeechCommand_MediaFoundation_SessionData,                       // session data type
                          enum Stream_SessionMessageType,                           // session event type
                          struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                         // stream notification interface type
                          Test_I_MediaFoundation_Statistic_ReaderTask_t,            // reader type
                          Test_I_MediaFoundation_Statistic_WriterTask_t,            // writer type
                          Test_I_MediaFoundation_StatisticReport);                  // name

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_lib_directshow_target_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_DirectShow_Target);                                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_lib_directshow_source_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_DirectShow_Source);                                     // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                                     // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_lib_mediafoundation_target_module_name_string,
                              Stream_INotify_t,                                                   // stream notification interface type
                              Test_I_MediaFoundation_Target);                                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                                     // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_lib_mediafoundation_source_module_name_string,
                              Stream_INotify_t,                                                   // stream notification interface type
                              Test_I_MediaFoundation_Source);                                     // writer type

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_misc_distributor_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_DirectShow_Distributor);                           // name
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_distributor_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_MediaFoundation_Distributor);                      // name

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_DirectShow_Vis_SpectrumAnalyzer);                       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_MediaFoundation_Vis_SpectrumAnalyzer);                  // writer type
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_dec_deepspeech_decoder_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_DirectShow_DeepSpeechDecoder);                                // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_dec_deepspeech_decoder_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_MediaFoundation_DeepSpeechDecoder);                                // writer type

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_dev_target_wavout_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_DirectShow_Target_WaveOut);                         // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_DirectShow_Target_WASAPI);                          // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_dev_target_wavout_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_MediaFoundation_Target_WaveOut);                    // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_dev_target_wasapi_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_MediaFoundation_Target_WASAPI);                     // writer type

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_DirectShow_WAVEncoder);                          // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_MediaFoundation_WAVEncoder);                       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                      // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                    // stream notification interface type
                              Test_I_DirectShow_FileWriter);                       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_MediaFoundation_FileWriter);                       // writer type

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_DirectShow_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_DirectShow_MessageHandler);                         // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_MediaFoundation_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_MediaFoundation_MessageHandler);                    // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                                   // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,             // module handler configuration type
                              libacestream_default_dev_mic_source_alsa_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_Mic_Source_ALSA);                                   // writer type

//////////////////////////////////////////

#if defined (SOX_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                    // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_sox_resampler_module_name_string,
                              Stream_INotify_t,                                                  // stream notification interface type
                              Test_I_ALSA_SoXResampler);                                   // name
#endif // SOX_SUPPORT

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                              // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,        // module handler configuration type
                              libacestream_default_stat_analysis_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_I_ALSA_StatisticAnalysis);                       // name
DATASTREAM_MODULE_DUPLEX (Test_I_SpeechCommand_ALSA_SessionData,                             // session data type
                          enum Stream_SessionMessageType,                      // session event type
                          struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,       // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                    // stream notification interface type
                          Test_I_ALSA_Statistic_ReaderTask_t,                  // reader type
                          Test_I_ALSA_Statistic_WriterTask_t,                  // writer type
                          Test_I_ALSA_StatisticReport);                        // name

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_misc_distributor_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_ALSA_Distributor);                                // name

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                                       // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,                 // module handler configuration type
                              libacestream_default_vis_spectrum_analyzer_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_ALSA_Vis_SpectrumAnalyzer);                             // writer type
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_dec_deepspeech_decoder_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_ALSA_DeepSpeechDecoder);                                // writer type

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_ALSA_WAVEncoder);                                 // writer type

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                     // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_ALSA_MessageHandler);                               // writer type

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_SpeechCommand_ALSA_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,           // module handler configuration type
                              libacestream_default_dev_target_alsa_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_Target_ALSA);                                     // writer type
#endif // ACE_WIN32 || ACE_WIN64

DATASTREAM_MODULE_INPUT_ONLY (struct Stream_SessionData,                                   // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Stream_Input_ModuleHandlerConfiguration,              // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_InputMessageHandler);                                 // writer type

#endif
