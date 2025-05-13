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
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_file_sink.h"

#if defined (FAAD_SUPPORT)
#include "stream_dec_faad_decoder.h"
#endif // FAAD_SUPPORT
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_audio_decoder.h"
#include "stream_dec_libav_decoder.h"
#include "stream_dec_libav_hw_decoder.h"
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_source.h"
#endif // FFMPEG_SUPPORT
#if defined (SOX_SUPPORT)
#include "stream_dec_sox_effect.h"
#endif // SOX_SUPPORT
#include "stream_dec_wav_encoder.h"

#include "stream_lib_tagger.h"

#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo.h"
#endif // GTK_SUPPORT

#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_extract_stream_common.h"

// declare module(s)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message_t,
                               Test_I_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_Message_t,
                                Test_I_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_I_TaskBaseAsynch_t;

typedef Stream_LibAV_Source_T<ACE_MT_SYNCH,
                              Stream_ControlMessage_t,
                              Test_I_Message_t,
                              Test_I_SessionMessage_t,
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                              enum Stream_ControlType,
                              enum Stream_SessionMessageType,
                              struct Test_I_ExtractStream_StreamState,
                              Test_I_ExtractStream_SessionData,
                              Test_I_ExtractStream_SessionData_t,
                              struct Stream_Statistic,
                              Common_Timer_Manager_t,
                              struct Stream_UserData> Test_I_Source;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVAudioDecoder_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Message_t,
                                           Test_I_SessionMessage_t,
                                           Test_I_ExtractStream_SessionData_t,
                                           struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVAudioDecoder;

typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Message_t,
                                      Test_I_SessionMessage_t,
                                      Test_I_ExtractStream_SessionData_t,
                                      struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVDecoder;
typedef Stream_LibAV_HW_Decoder_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                  Stream_ControlMessage_t,
                                  Test_I_Message_t,
                                  Test_I_SessionMessage_t,
                                  Test_I_ExtractStream_SessionData_t,
                                  struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVHWDecoder;

typedef Stream_Decoder_LibAVConverter_T<Test_I_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVConverter;
typedef Stream_Visualization_LibAVResize_T<Test_I_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_LibAVResize;
#endif // FFMPEG_SUPPORT

#if defined (FAAD_SUPPORT)
typedef Stream_Decoder_FAAD_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Test_I_Message_t,
                              Test_I_SessionMessage_t,
                              Test_I_ExtractStream_SessionData_t,
                              struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_AACDecoder;
#endif // FAAD_SUPPORT

typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message_t,
                               Test_I_SessionMessage_t,
                               STREAM_MEDIATYPE_AUDIO,
                               struct Stream_UserData> Test_I_AudioTagger;
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message_t,
                               Test_I_SessionMessage_t,
                               STREAM_MEDIATYPE_VIDEO,
                               struct Stream_UserData> Test_I_VideoTagger;

#if defined (SOX_SUPPORT)
typedef Stream_Decoder_SoXEffect_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Message_t,
                                   Test_I_SessionMessage_t,
                                   Test_I_ExtractStream_SessionData_t,
                                   Test_I_ExtractStream_SessionData,
                                   struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_AudioEffect;
#endif // SOX_SUPPORT

typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message_t,
                                                      Test_I_SessionMessage_t,
                                                      Test_I_ExtractStream_SessionData> Test_I_Distributor_Writer_t;

#if defined (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Message_t,
                                      Test_I_SessionMessage_t,
                                      Test_I_ExtractStream_SessionData,
                                      Test_I_ExtractStream_SessionData_t,
                                      struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_Vis_GTK_Cairo;
#endif // GTK_SUPPORT

typedef Stream_Decoder_WAVEncoder_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_I_Message_t,
                                    Test_I_SessionMessage_t,
                                    Test_I_ExtractStream_SessionData_t,
                                    Test_I_ExtractStream_SessionData,
                                    struct Stream_MediaFramework_FFMPEG_MediaType,
                                    struct Stream_UserData> Test_I_WAVEncoder;

typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Test_I_Message_t,
                                               Test_I_SessionMessage_t,
                                               Test_I_ExtractStream_SessionData_t,
                                               Test_I_ExtractStream_SessionData,
                                               struct Stream_MediaFramework_FFMPEG_MediaType,
                                               struct Stream_UserData> Test_I_AVIEncoder_Writer_t;

typedef Stream_Module_FileWriter_2<Common_TimePolicy_t,
                                   struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Message_t,
                                   Test_I_SessionMessage_t> Test_I_FileWriter;

typedef Stream_Module_MessageHandler_T <ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                                        Stream_ControlMessage_t,
                                        Test_I_Message_t,
                                        Test_I_SessionMessage_t,
                                        Test_I_ExtractStream_SessionData,
                                        struct Stream_UserData> Test_I_MessageHandler;

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_source_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_Source);                                            // module name

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_audio_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_LibAVAudioDecoder);                                 // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_LibAVDecoder);                                      // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_hw_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_LibAVHWDecoder);                                    // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_LibAVConverter);                                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_LibAVResize);                                      // writer type
#endif // FFMPEG_SUPPORT

#if defined (FAAD_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_faad_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_AACDecoder);                                 // writer type
#endif // FAAD_SUPPORT

#if defined (SOX_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_sox_effect_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_AudioEffect);                                    // writer type
#endif // SOX_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_AudioTagger);                                      // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_VideoTagger);                                      // writer type

DATASTREAM_MODULE_DUPLEX (Test_I_ExtractStream_SessionData,                         // session data type
                          enum Stream_SessionMessageType,                           // session event type
                          struct Test_I_ExtractStream_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                         // stream notification interface type
                          Test_I_Distributor_Writer_t::READER_TASK_T,               // reader type
                          Test_I_Distributor_Writer_t,                              // writer type
                          Test_I_Distributor);                                      // module name prefix

#if defined (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_Vis_GTK_Cairo);                                      // writer type
#endif // GTK_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_dec_wav_encoder_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Test_I_WAVEncoder);                                      // writer type

DATASTREAM_MODULE_DUPLEX (Test_I_ExtractStream_SessionData,                        // session data type
                          enum Stream_SessionMessageType,                          // session event type
                          struct Test_I_ExtractStream_ModuleHandlerConfiguration,  // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                        // stream notification interface type
                          Test_I_AVIEncoder_Writer_t::READER_T,                    // reader type
                          Test_I_AVIEncoder_Writer_t,                              // writer type
                          Test_I_AVIEncoder);                                      // name

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_FileWriter);                                     // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,      // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_MessageHandler);                                      // writer type

#endif
