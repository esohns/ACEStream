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

#ifndef TEST_I_AVSAVE_COMMON_MODULES_H
#define TEST_I_AVSAVE_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"

#include "stream_dev_mic_source_wavein.h"
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_decoder.h"
#endif // FFMPEG_SUPPORT

#include "stream_dev_cam_source_v4l.h"
#include "stream_dev_mic_source_alsa.h"
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_encoder.h"
#endif // FFMPEG_SUPPORT


#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_tagger.h"

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

//#include "stream_stat_statistic_report.h"

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"

#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo.h"
#endif // GTK_SUPPORT
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT

#include "stream_vis_x11_window.h"
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_pixbuf.h"
#endif // GTK_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#include "test_i_avsave_common.h"
#include "test_i_avsave_message.h"
#include "test_i_avsave_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Mic_Source_WaveIn_T<ACE_MT_SYNCH,
                                       Stream_ControlMessage_t,
                                       Stream_AVSave_DirectShow_Message_t,
                                       Stream_AVSave_DirectShow_SessionMessage_t,
                                       struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                                       enum Stream_ControlType,
                                       enum Stream_SessionMessageType,
                                       struct Stream_AVSave_DirectShow_StreamState,
                                       Stream_AVSave_DirectShow_SessionData,
                                       Stream_AVSave_DirectShow_SessionData_t,
                                       struct Stream_AVSave_StatisticData,
                                       Common_Timer_Manager_t> Stream_AVSave_WaveIn_Source;
typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Stream_AVSave_DirectShow_Message_t,
                                           Stream_AVSave_DirectShow_SessionMessage_t,
                                           struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Stream_AVSave_DirectShow_StreamState,
                                           Stream_AVSave_DirectShow_SessionData,
                                           Stream_AVSave_DirectShow_SessionData_t,
                                           struct Stream_AVSave_StatisticData,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData> Stream_AVSave_DirectShow_Source;

typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Stream_AVSave_MediaFoundation_Message_t,
                                                Stream_AVSave_MediaFoundation_SessionMessage_t,
                                                struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Stream_AVSave_MediaFoundation_StreamState,
                                                Stream_AVSave_MediaFoundation_SessionData,
                                                Stream_AVSave_MediaFoundation_SessionData_t,
                                                struct Stream_AVSave_StatisticData,
                                                Common_Timer_Manager_t,
                                                struct Stream_UserData> Stream_AVSave_MediaFoundation_Source;
#else
typedef Stream_Dev_Mic_Source_ALSA_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Stream_AVSave_Message_t,
                                     Stream_AVSave_ALSA_SessionMessage_t,
                                     struct Stream_AVSave_ALSA_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Stream_AVSave_ALSA_StreamState,
                                     Stream_AVSave_ALSA_SessionData,
                                     Stream_AVSave_ALSA_SessionData_t,
                                     struct Stream_AVSave_StatisticData,
                                     Common_Timer_Manager_t> Stream_AVSave_ALSA_Source;
typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Stream_AVSave_Message_t,
                                      Stream_AVSave_V4L_SessionMessage_t,
                                      struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Stream_AVSave_V4L_StreamState,
                                      Stream_AVSave_V4L_SessionData,
                                      Stream_AVSave_V4L_SessionData_t,
                                      struct Stream_AVSave_StatisticData,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Stream_AVSave_V4L_Source;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_AVSave_Message_t,
                                      Stream_AVSave_V4L_SessionMessage_t,
                                      Stream_AVSave_V4L_SessionData_t,
                                      struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_LibAVDecoder;
typedef Stream_Decoder_LibAVConverter_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                                        Stream_ControlMessage_t,
                                        Stream_AVSave_Message_t,
                                        Stream_AVSave_V4L_SessionMessage_t,
                                        Stream_AVSave_V4L_SessionData_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_LibAVConverter;

//typedef Stream_Visualization_LibAVResize_T<ACE_MT_SYNCH,
//                                           Common_TimePolicy_t,
//                                           struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
//                                           Stream_ControlMessage_t,
//                                           Stream_AVSave_Message_t,
//                                           Stream_AVSave_V4L_SessionMessage_t,
//                                           Stream_AVSave_V4L_SessionData_t,
//                                           struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_LibAVResize;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_AVSave_DirectShow_Message_t,
//                                                      Stream_AVSave_DirectShow_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_AVSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_AVSave_DirectShow_SessionData,
//                                                      Stream_AVSave_DirectShow_SessionData_t> Stream_AVSave_DirectShow_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_AVSave_DirectShow_Message_t,
//                                                      Stream_AVSave_DirectShow_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_AVSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_AVSave_DirectShow_SessionData,
//                                                      Stream_AVSave_DirectShow_SessionData_t> Stream_AVSave_DirectShow_Statistic_WriterTask_t;
//
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_AVSave_MediaFoundation_Message_t,
//                                                      Stream_AVSave_MediaFoundation_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_AVSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_AVSave_MediaFoundation_SessionData,
//                                                      Stream_AVSave_MediaFoundation_SessionData_t> Stream_AVSave_MediaFoundation_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_AVSave_MediaFoundation_Message_t,
//                                                      Stream_AVSave_MediaFoundation_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_AVSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_AVSave_MediaFoundation_SessionData,
//                                                      Stream_AVSave_MediaFoundation_SessionData_t> Stream_AVSave_MediaFoundation_Statistic_WriterTask_t;
#else
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_AVSave_Message_t,
//                                                      Stream_AVSave_V4L_SessionMessage_t,
//                                                      enum Stream_MediaType_Type,
//                                                      struct Stream_AVSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_AVSave_V4L_SessionData,
//                                                      Stream_AVSave_V4L_SessionData_t> Stream_AVSave_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_AVSave_Message_t,
//                                                      Stream_AVSave_V4L_SessionMessage_t,
//                                                      enum Stream_MediaType_Type,
//                                                      struct Stream_AVSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_AVSave_V4L_SessionData,
//                                                      Stream_AVSave_V4L_SessionData_t> Stream_AVSave_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_AVSave_DirectShow_Message_t,
                               Stream_AVSave_DirectShow_SessionMessage_t,
                               STREAM_MEDIATYPE_AUDIO,
                               struct _AMMediaType> Stream_AVSave_WaveIn_Tagger;
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_AVSave_DirectShow_Message_t,
                               Stream_AVSave_DirectShow_SessionMessage_t,
                               STREAM_MEDIATYPE_VIDEO,
                               struct _AMMediaType> Stream_AVSave_DirectShow_Video_Tagger;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVEncoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_AVSave_DirectShow_Message_t,
                                      Stream_AVSave_DirectShow_SessionMessage_t,
                                      Stream_AVSave_DirectShow_SessionData_t,
                                      struct _AMMediaType> Stream_AVSave_DirectShow_Encoder;
#endif // FFMPEG_SUPPORT
#else
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_AVSave_ALSA_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_AVSave_Message_t,
                               Stream_AVSave_ALSA_SessionMessage_t,
                               STREAM_MEDIATYPE_AUDIO,
                               struct Stream_MediaFramework_ALSA_MediaType> Stream_AVSave_ALSA_Tagger;
typedef Stream_Module_Tagger_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_AVSave_Message_t,
                               Stream_AVSave_V4L_SessionMessage_t,
                               STREAM_MEDIATYPE_VIDEO,
                               struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_V4L_Tagger;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVEncoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_AVSave_Message_t,
                                      Stream_AVSave_V4L_SessionMessage_t,
                                      Stream_AVSave_V4L_SessionData_t,
                                      struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_V4L_Encoder;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
//                                     Common_TimePolicy_t,
//                                     struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
//                                     Stream_ControlMessage_t,
//                                     Stream_AVSave_DirectShow_Message_t,
//                                     Stream_AVSave_DirectShow_SessionMessage_t,
//                                     Stream_AVSave_DirectShow_SessionData,
//                                     Stream_AVSave_DirectShow_SessionData_t,
//                                     struct _AMMediaType> Stream_AVSave_DirectShow_Direct3DDisplay;
//typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
//                                     Common_TimePolicy_t,
//                                     struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
//                                     Stream_ControlMessage_t,
//                                     Stream_AVSave_MediaFoundation_Message_t,
//                                     Stream_AVSave_MediaFoundation_SessionMessage_t,
//                                     Stream_AVSave_MediaFoundation_SessionData,
//                                     Stream_AVSave_MediaFoundation_SessionData_t,
//                                     IMFMediaType*> Stream_AVSave_MediaFoundation_Direct3DDisplay;
//
//struct Stream_AVSave_DirectShow_FilterConfiguration
// : Stream_MediaFramework_DirectShow_FilterConfiguration
//{
//  Stream_AVSave_DirectShow_FilterConfiguration ()
//   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
//   , module (NULL)
//   , pinConfiguration (NULL)
//  {}
//
//  Stream_Module_t*                                                module; // handle
//  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
//};
//typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Common_TimePolicy_t,
//                                                         Stream_AVSave_DirectShow_SessionMessage_t,
//                                                         Stream_AVSave_DirectShow_Message_t,
//                                                         struct Stream_AVSave_DirectShow_FilterConfiguration,
//                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
//                                                         struct _AMMediaType> Stream_AVSave_DirectShowFilter_t;
//typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Common_TimePolicy_t,
//                                                                Stream_AVSave_DirectShow_SessionMessage_t,
//                                                                Stream_AVSave_DirectShow_Message_t,
//                                                                struct Stream_AVSave_DirectShow_FilterConfiguration,
//                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
//                                                                struct _AMMediaType> Stream_AVSave_AsynchDirectShowFilter_t;
//typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_AVSave_DirectShow_Message_t,
//                                       Stream_AVSave_DirectShow_SessionMessage_t,
//                                       Stream_AVSave_DirectShow_SessionData_t,
//                                       Stream_AVSave_DirectShow_SessionData,
//                                       struct Stream_AVSave_DirectShow_FilterConfiguration,
//                                       struct Stream_AVSave_DirectShow_PinConfiguration,
//                                       Stream_AVSave_DirectShowFilter_t> Stream_AVSave_DirectShow_DirectShowDisplay;
//
//typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
//                                            Stream_ControlMessage_t,
//                                            Stream_AVSave_MediaFoundation_Message_t,
//                                            Stream_AVSave_MediaFoundation_SessionMessage_t,
//                                            Stream_AVSave_MediaFoundation_SessionData,
//                                            Stream_AVSave_MediaFoundation_SessionData_t,
//                                            struct Stream_UserData> Stream_AVSave_MediaFoundation_MediaFoundationDisplay;
//typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
//                                            Stream_ControlMessage_t,
//                                            Stream_AVSave_MediaFoundation_Message_t,
//                                            Stream_AVSave_MediaFoundation_SessionMessage_t,
//                                            Stream_AVSave_MediaFoundation_SessionData,
//                                            Stream_AVSave_MediaFoundation_SessionData_t> Stream_AVSave_MediaFoundation_MediaFoundationDisplayNull;

//typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
//                                      Common_TimePolicy_t,
//                                      struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
//                                      Stream_ControlMessage_t,
//                                      Stream_AVSave_DirectShow_Message_t,
//                                      Stream_AVSave_DirectShow_SessionMessage_t,
//                                      Stream_AVSave_DirectShow_SessionData,
//                                      Stream_AVSave_DirectShow_SessionData_t,
//                                      struct _AMMediaType> Stream_AVSave_DirectShow_GTKCairoDisplay;
//typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
//                                      Common_TimePolicy_t,
//                                      struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
//                                      Stream_ControlMessage_t,
//                                      Stream_AVSave_MediaFoundation_Message_t,
//                                      Stream_AVSave_MediaFoundation_SessionMessage_t,
//                                      Stream_AVSave_MediaFoundation_SessionData,
//                                      Stream_AVSave_MediaFoundation_SessionData_t,
//                                      IMFMediaType*> Stream_AVSave_MediaFoundation_GTKCairoDisplay;
#else
//typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
//                                      Common_TimePolicy_t,
//                                      struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
//                                      Stream_ControlMessage_t,
//                                      Stream_AVSave_Message_t,
//                                      Stream_AVSave_V4L_SessionMessage_t,
//                                      Stream_AVSave_V4L_SessionData,
//                                      Stream_AVSave_V4L_SessionData_t,
//                                      struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_Display;
//typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_AVSave_Message_t,
//                                       Stream_AVSave_V4L_SessionMessage_t,
//                                       Stream_AVSave_V4L_SessionData_t,
//                                       struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_Display;
//typedef Stream_Module_Vis_GTK_Window_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_AVSave_Message_t,
//                                       Stream_AVSave_V4L_SessionMessage_t,
//                                       struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_Display_2;
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_AVSave_Message_t,
                                       Stream_AVSave_V4L_SessionMessage_t,
                                       Stream_AVSave_V4L_SessionData_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Stream_AVSave_Display;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandlerA_T <ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Stream_AVSave_DirectShow_Message_t,
                                         Stream_AVSave_DirectShow_SessionMessage_t,
                                         Stream_AVSave_DirectShow_SessionData,
                                         struct Stream_UserData> Stream_AVSave_DirectShow_MessageHandler;

typedef Stream_Module_MessageHandlerA_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
                                        Stream_ControlMessage_t,
                                        Stream_AVSave_MediaFoundation_Message_t,
                                        Stream_AVSave_MediaFoundation_SessionMessage_t,
                                        Stream_AVSave_MediaFoundation_SessionData,
                                        struct Stream_UserData> Stream_AVSave_MediaFoundation_MessageHandler;
#else
typedef Stream_Module_MessageHandlerA_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                                        Stream_ControlMessage_t,
                                        Stream_AVSave_Message_t,
                                        Stream_AVSave_V4L_SessionMessage_t,
                                        Stream_AVSave_V4L_SessionData,
                                        struct Stream_UserData> Stream_AVSave_MessageHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_wavein_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_WaveIn_Source);                // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_directshow_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_DirectShow_Source);                // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_MediaFoundation_Source);           // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_ALSA_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_mic_source_alsa_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_ALSA_Source);                       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_V4L_Source);                       // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_LibAVDecoder);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_LibAVConverter);                   // writer type

//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_libav_resize_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//DATASTREAM_MODULE_DUPLEX (Stream_AVSave_DirectShow_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                   // session event type
//                          struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                 // stream notification interface type
//                          Stream_AVSave_DirectShow_Statistic_ReaderTask_t, // reader type
//                          Stream_AVSave_DirectShow_Statistic_WriterTask_t, // writer type
//                          Stream_AVSave_DirectShow_StatisticReport);       // name
//
//DATASTREAM_MODULE_DUPLEX (Stream_AVSave_MediaFoundation_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                   // session event type
//                          struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                 // stream notification interface type
//                          Stream_AVSave_MediaFoundation_Statistic_ReaderTask_t, // reader type
//                          Stream_AVSave_MediaFoundation_Statistic_WriterTask_t, // writer type
//                          Stream_AVSave_MediaFoundation_StatisticReport);  // name
#else
//DATASTREAM_MODULE_DUPLEX (Stream_AVSave_V4L_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                   // session event type
//                          struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                 // stream notification interface type
//                          Stream_AVSave_Statistic_ReaderTask_t,            // reader type
//                          Stream_AVSave_Statistic_WriterTask_t,            // writer type
//                          Stream_AVSave_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_WaveIn_Tagger);           // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_DirectShow_Video_Tagger);           // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_encoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_DirectShow_Encoder);                // writer type
#endif // FFMPEG_SUPPORT
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_ALSA_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_ALSA_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_ALSA_Tagger);                       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_lib_tagger_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_V4L_Tagger);                        // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_encoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_V4L_Encoder);                       // writer type
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_directshow_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_DirectShow_Direct3DDisplay);       // writer type
//
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_MediaFoundation_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_mediafoundation_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_MediaFoundation_Direct3DDisplay);  // writer type
//
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_directshow_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_DirectShow_DirectShowDisplay);     // writer type
//
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_MediaFoundation_SessionData,                      // session data type
//                              enum Stream_SessionMessageType,                         // session event type
//                              struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_mediafoundation_module_name_string,
//                              Stream_INotify_t,                                       // stream notification interface type
//                              Stream_AVSave_MediaFoundation_MediaFoundationDisplay); // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_MediaFoundation_SessionData,                          // session data type
//                              enum Stream_SessionMessageType,                             // session event type
//                              struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_mediafoundation_module_name_string,
//                              Stream_INotify_t,                                           // stream notification interface type
//                              Stream_AVSave_MediaFoundation_MediaFoundationDisplayNull); // writer type

//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_DirectShow_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_cairo_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_DirectShow_GTKCairoDisplay);       // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_MediaFoundation_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_cairo_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_MediaFoundation_GTKCairoDisplay);  // writer type
#else
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                       // session data type
//                              enum Stream_SessionMessageType,                       // session event type
//                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_cairo_module_name_string,
//                              Stream_INotify_t,                                     // stream notification interface type
//                              Stream_AVSave_Display);                              // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_pixbuf_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_Display);                          // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_window_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_AVSave_Display_2);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_AVSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_AVSave_Display);                           // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY_A (Stream_AVSave_DirectShow_SessionData,                // session data type
                                enum Stream_SessionMessageType,                   // session event type
                                struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                                libacestream_default_misc_messagehandler_module_name_string,
                                Stream_INotify_t,                                 // stream notification interface type
                                Stream_AVSave_DirectShow_MessageHandler);        // writer type

DATASTREAM_MODULE_INPUT_ONLY_A (Stream_AVSave_MediaFoundation_SessionData,                // session data type
                                enum Stream_SessionMessageType,                   // session event type
                                struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                                libacestream_default_misc_messagehandler_module_name_string,
                                Stream_INotify_t,                                 // stream notification interface type
                                Stream_AVSave_MediaFoundation_MessageHandler);   // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY_A (Stream_AVSave_V4L_SessionData,                           // session data type
                                enum Stream_SessionMessageType,                       // session event type
                                struct Stream_AVSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                                libacestream_default_misc_messagehandler_module_name_string,
                                Stream_INotify_t,                                     // stream notification interface type
                                Stream_AVSave_MessageHandler);                       // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
