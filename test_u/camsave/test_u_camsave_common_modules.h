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

#ifndef TEST_U_CAMSAVE_COMMON_MODULES_H
#define TEST_U_CAMSAVE_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (GUI_SUPPORT)
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_decoder.h"
#endif // FFMPEG_SUPPORT
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_avi_encoder.h"
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_avi_encoder.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
#include "stream_dev_cam_source_vfw.h"
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
#else
#include "stream_dev_cam_source_v4l.h"
#if defined (LIBCAMERA_SUPPORT)
#include "stream_dev_cam_source_libcamera.h"
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_file_sink.h"

#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo.h"
#include "stream_vis_gtk_pixbuf.h"
#endif // GTK_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_direct3d_11.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"
#else
//#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT
#endif // GUI_SUPPORT

#include "test_u_camsave_common.h"
#include "test_u_camsave_message.h"
#include "test_u_camsave_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_CamSave_DirectShow_Message_t,
                               Stream_CamSave_DirectShow_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_DirectShow_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_CamSave_DirectShow_Message_t,
                                Stream_CamSave_DirectShow_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_DirectShow_TaskBaseAsynch_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_CamSave_MediaFoundation_Message_t,
                               Stream_CamSave_MediaFoundation_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_MediaFoundation_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_CamSave_MediaFoundation_Message_t,
                                Stream_CamSave_MediaFoundation_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_MediaFoundation_TaskBaseAsynch_t;
#else
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_CamSave_V4L_Message_t,
                               Stream_CamSave_V4L_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_V4L_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_CamSave_V4L_Message_t,
                                Stream_CamSave_V4L_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_V4L_TaskBaseAsynch_t;

#if defined (LIBCAMERA_SUPPORT)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_CamSave_LibCamera_Message_t,
                               Stream_CamSave_LibCamera_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_LibCamera_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_CamSave_LibCamera_Message_t,
                                Stream_CamSave_LibCamera_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_LibCamera_TaskBaseAsynch_t;
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Cam_Source_VfW_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Stream_CamSave_DirectShow_Message_t,
                                    Stream_CamSave_DirectShow_SessionMessage_t,
                                    struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Stream_CamSave_DirectShow_StreamState,
                                    Stream_CamSave_DirectShow_SessionData,
                                    Stream_CamSave_DirectShow_SessionData_t,
                                    struct Stream_CamSave_StatisticData,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData,
                                    struct _AMMediaType> Stream_CamSave_VfW_Source;
typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Stream_CamSave_DirectShow_Message_t,
                                           Stream_CamSave_DirectShow_SessionMessage_t,
                                           struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Stream_CamSave_DirectShow_StreamState,
                                           Stream_CamSave_DirectShow_SessionData,
                                           Stream_CamSave_DirectShow_SessionData_t,
                                           struct Stream_CamSave_StatisticData,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData,
                                           struct _AMMediaType,
                                           false> Stream_CamSave_DirectShow_Source;

typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Stream_CamSave_MediaFoundation_Message_t,
                                                Stream_CamSave_MediaFoundation_SessionMessage_t,
                                                struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Stream_CamSave_MediaFoundation_StreamState,
                                                Stream_CamSave_MediaFoundation_SessionData,
                                                Stream_CamSave_MediaFoundation_SessionData_t,
                                                struct Stream_CamSave_StatisticData,
                                                Common_Timer_Manager_t,
                                                struct Stream_UserData,
                                                IMFMediaType*> Stream_CamSave_MediaFoundation_Source;

typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_DirectShow_Message_t,
                                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                                      Stream_CamSave_DirectShow_SessionData> Stream_CamSave_DirectShow_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_DirectShow_Message_t,
                                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                                      Stream_CamSave_DirectShow_SessionData> Stream_CamSave_DirectShow_Distributor_Writer_t;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CamSave_DirectShow_Message_t,
                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                      Stream_CamSave_DirectShow_SessionData_t,
                                      struct _AMMediaType> Stream_CamSave_DirectShow_LibAVDecoder;

typedef Stream_Decoder_LibAVConverter_T<Test_U_DirectShow_TaskBaseSynch_t,
                                        struct _AMMediaType> Stream_CamSave_DirectShow_LibAVConverter;

typedef Stream_Visualization_LibAVResize_T<Test_U_DirectShow_TaskBaseSynch_t,
                                           struct _AMMediaType> Stream_CamSave_DirectShow_LibAVResize;
#endif // FFMPEG_SUPPORT
#else
#if defined (LIBCAMERA_SUPPORT)
typedef Stream_Module_CamSource_LibCamera_T<ACE_MT_SYNCH,
                                            Stream_ControlMessage_t,
                                            Stream_CamSave_LibCamera_Message_t,
                                            Stream_CamSave_LibCamera_SessionMessage_t,
                                            struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                            enum Stream_ControlType,
                                            enum Stream_SessionMessageType,
                                            struct Stream_CamSave_LibCamera_StreamState,
                                            Stream_CamSave_LibCamera_SessionData,
                                            Stream_CamSave_LibCamera_SessionData_t,
                                            struct Stream_CamSave_StatisticData,
                                            Common_Timer_Manager_t,
                                            struct Stream_UserData> Stream_CamSave_LibCamera_Source;
#endif // LIBCAMERA_SUPPORT

typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Stream_CamSave_V4L_Message_t,
                                      Stream_CamSave_V4L_SessionMessage_t,
                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Stream_CamSave_V4L_StreamState,
                                      Stream_CamSave_V4L_SessionData,
                                      Stream_CamSave_V4L_SessionData_t,
                                      struct Stream_CamSave_StatisticData,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Stream_CamSave_V4L_Source;

#if defined (LIBCAMERA_SUPPORT)
#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CamSave_LibCamera_Message_t,
                                      Stream_CamSave_LibCamera_SessionMessage_t,
                                      Stream_CamSave_LibCamera_SessionData_t,
                                      struct Stream_MediaFramework_LibCamera_MediaType> Stream_CamSave_LibCamera_LibAVDecoder;
#endif // FFMPEG_SUPPORT

typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_LibCamera_Message_t,
                                                      Stream_CamSave_LibCamera_SessionMessage_t,
                                                      Stream_CamSave_LibCamera_SessionData> Stream_CamSave_LibCamera_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_LibCamera_Message_t,
                                                      Stream_CamSave_LibCamera_SessionMessage_t,
                                                      Stream_CamSave_LibCamera_SessionData> Stream_CamSave_LibCamera_Distributor_Writer_t;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_LibCamera_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_LibCamera_MediaType> Stream_CamSave_LibCamera_LibAVConverter;
typedef Stream_Visualization_LibAVResize_T<Test_U_LibCamera_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_LibCamera_MediaType> Stream_CamSave_LibCamera_LibAVResize;
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CamSave_V4L_Message_t,
                                      Stream_CamSave_V4L_SessionMessage_t,
                                      Stream_CamSave_V4L_SessionData_t,
                                      struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_V4L_LibAVDecoder;
#endif // FFMPEG_SUPPORT

typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_V4L_Message_t,
                                                      Stream_CamSave_V4L_SessionMessage_t,
                                                      Stream_CamSave_V4L_SessionData> Stream_CamSave_V4L_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_V4L_Message_t,
                                                      Stream_CamSave_V4L_SessionMessage_t,
                                                      Stream_CamSave_V4L_SessionData> Stream_CamSave_V4L_Distributor_Writer_t;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_V4L_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_V4L_LibAVConverter;
typedef Stream_Visualization_LibAVResize_T<Test_U_V4L_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_V4L_LibAVResize;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_DirectShow_Message_t,
                                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_DirectShow_SessionData,
                                                      Stream_CamSave_DirectShow_SessionData_t> Stream_CamSave_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_DirectShow_Message_t,
                                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_DirectShow_SessionData,
                                                      Stream_CamSave_DirectShow_SessionData_t> Stream_CamSave_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_MediaFoundation_Message_t,
                                                      Stream_CamSave_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_MediaFoundation_SessionData,
                                                      Stream_CamSave_MediaFoundation_SessionData_t> Stream_CamSave_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CamSave_MediaFoundation_Message_t,
                                                      Stream_CamSave_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_MediaFoundation_SessionData,
                                                      Stream_CamSave_MediaFoundation_SessionData_t> Stream_CamSave_MediaFoundation_Statistic_WriterTask_t;
#else
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_CamSave_Message_t,
//                                                      Stream_CamSave_V4L_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_CamSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_CamSave_V4L_SessionData,
//                                                      Stream_CamSave_V4L_SessionData_t> Stream_CamSave_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_CamSave_Message_t,
//                                                      Stream_CamSave_V4L_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_CamSave_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_CamSave_V4L_SessionData,
//                                                      Stream_CamSave_V4L_SessionData_t> Stream_CamSave_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_DirectShow_Message_t,
                                               Stream_CamSave_DirectShow_SessionMessage_t,
                                               Stream_CamSave_DirectShow_SessionData_t,
                                               Stream_CamSave_DirectShow_SessionData,
                                               struct _AMMediaType,
                                               struct Stream_UserData> Stream_CamSave_DirectShow_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_DirectShow_Message_t,
                                               Stream_CamSave_DirectShow_SessionMessage_t,
                                               Stream_CamSave_DirectShow_SessionData_t,
                                               Stream_CamSave_DirectShow_SessionData,
                                               struct _AMMediaType,
                                               struct Stream_UserData> Stream_CamSave_DirectShow_AVIEncoder_WriterTask_t;

typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_MediaFoundation_Message_t,
                                               Stream_CamSave_MediaFoundation_SessionMessage_t,
                                               Stream_CamSave_MediaFoundation_SessionData_t,
                                               Stream_CamSave_MediaFoundation_SessionData,
                                               IMFMediaType*,
                                               struct Stream_UserData> Stream_CamSave_MediaFoundation_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_MediaFoundation_Message_t,
                                               Stream_CamSave_MediaFoundation_SessionMessage_t,
                                               Stream_CamSave_MediaFoundation_SessionData_t,
                                               Stream_CamSave_MediaFoundation_SessionData,
                                               IMFMediaType*,
                                               struct Stream_UserData> Stream_CamSave_MediaFoundation_AVIEncoder_WriterTask_t;
#else
#if defined (LIBCAMERA_SUPPORT)
#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_LibCamera_Message_t,
                                               Stream_CamSave_LibCamera_SessionMessage_t,
                                               Stream_CamSave_LibCamera_SessionData_t,
                                               Stream_CamSave_LibCamera_SessionData,
                                               struct Stream_MediaFramework_LibCamera_MediaType,
                                               struct Stream_UserData> Stream_CamSave_LibCamera_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_LibCamera_Message_t,
                                               Stream_CamSave_LibCamera_SessionMessage_t,
                                               Stream_CamSave_LibCamera_SessionData_t,
                                               Stream_CamSave_LibCamera_SessionData,
                                               struct Stream_MediaFramework_LibCamera_MediaType,
                                               struct Stream_UserData> Stream_CamSave_LibCamera_AVIEncoder_WriterTask_t;
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_V4L_Message_t,
                                               Stream_CamSave_V4L_SessionMessage_t,
                                               Stream_CamSave_V4L_SessionData_t,
                                               Stream_CamSave_V4L_SessionData,
                                               struct Stream_MediaFramework_V4L_MediaType,
                                               struct Stream_UserData> Stream_CamSave_V4L_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                               Stream_ControlMessage_t,
                                               Stream_CamSave_V4L_Message_t,
                                               Stream_CamSave_V4L_SessionMessage_t,
                                               Stream_CamSave_V4L_SessionData_t,
                                               Stream_CamSave_V4L_SessionData,
                                               struct Stream_MediaFramework_V4L_MediaType,
                                               struct Stream_UserData> Stream_CamSave_V4L_AVIEncoder_WriterTask_t;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CamSave_DirectShow_Message_t,
                                     Stream_CamSave_DirectShow_SessionMessage_t,
                                     Stream_CamSave_DirectShow_SessionData,
                                     Stream_CamSave_DirectShow_SessionData_t,
                                     struct _AMMediaType> Stream_CamSave_DirectShow_Direct3DDisplay;
typedef Stream_Vis_Target_Direct3D11_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_DirectShow_Message_t,
                                       Stream_CamSave_DirectShow_SessionMessage_t,
                                       Stream_CamSave_DirectShow_SessionData,
                                       Stream_CamSave_DirectShow_SessionData_t,
                                       struct _AMMediaType> Stream_CamSave_DirectShow_Direct3D11Display;
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CamSave_MediaFoundation_Message_t,
                                     Stream_CamSave_MediaFoundation_SessionMessage_t,
                                     Stream_CamSave_MediaFoundation_SessionData,
                                     Stream_CamSave_MediaFoundation_SessionData_t,
                                     IMFMediaType*> Stream_CamSave_MediaFoundation_Direct3DDisplay;

struct Stream_CamSave_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Stream_CamSave_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Stream_CamSave_DirectShow_Message_t,
                                                         struct Stream_CamSave_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Stream_CamSave_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Stream_CamSave_DirectShow_Message_t,
                                                                struct Stream_CamSave_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Stream_CamSave_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_DirectShow_Message_t,
                                       Stream_CamSave_DirectShow_SessionMessage_t,
                                       Stream_CamSave_DirectShow_SessionData_t,
                                       Stream_CamSave_DirectShow_SessionData,
                                       struct Stream_CamSave_DirectShow_FilterConfiguration,
                                       struct Stream_CamSave_DirectShow_PinConfiguration,
                                       Stream_CamSave_DirectShowFilter_t,
                                       struct _AMMediaType> Stream_CamSave_DirectShow_DirectShowDisplay;
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Stream_CamSave_MediaFoundation_Message_t,
                                            Stream_CamSave_MediaFoundation_SessionMessage_t,
                                            Stream_CamSave_MediaFoundation_SessionData,
                                            Stream_CamSave_MediaFoundation_SessionData_t,
                                            struct Stream_UserData> Stream_CamSave_MediaFoundation_MediaFoundationDisplay;
typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Stream_CamSave_MediaFoundation_Message_t,
                                            Stream_CamSave_MediaFoundation_SessionMessage_t,
                                            Stream_CamSave_MediaFoundation_SessionData,
                                            Stream_CamSave_MediaFoundation_SessionData_t,
                                            IMFMediaType*> Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull;

#if (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_DirectShow_Message_t,
                                       Stream_CamSave_DirectShow_SessionMessage_t,
                                       Stream_CamSave_DirectShow_SessionData_t,
                                       struct _AMMediaType> Stream_CamSave_DirectShow_GTKPixbufDisplay;
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CamSave_DirectShow_Message_t,
                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                      Stream_CamSave_DirectShow_SessionData,
                                      Stream_CamSave_DirectShow_SessionData_t,
                                      struct _AMMediaType> Stream_CamSave_DirectShow_GTKCairoDisplay;
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CamSave_MediaFoundation_Message_t,
                                      Stream_CamSave_MediaFoundation_SessionMessage_t,
                                      Stream_CamSave_MediaFoundation_SessionData,
                                      Stream_CamSave_MediaFoundation_SessionData_t,
                                      IMFMediaType*> Stream_CamSave_MediaFoundation_GTKCairoDisplay;
#endif // GTK_SUPPORT
#else
#if (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CamSave_V4L_Message_t,
                                      Stream_CamSave_V4L_SessionMessage_t,
                                      Stream_CamSave_V4L_SessionData,
                                      Stream_CamSave_V4L_SessionData_t,
                                      struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_V4L_GTKCairoDisplay;
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_V4L_Message_t,
                                       Stream_CamSave_V4L_SessionMessage_t,
                                       Stream_CamSave_V4L_SessionData_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_V4L_GTKPixbufDisplay;
//typedef Stream_Module_Vis_GTK_Window_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_CamSave_Message_t,
//                                       Stream_CamSave_V4L_SessionMessage_t,
//                                       struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_V4L_GTKWindowDisplay;

#if defined (LIBCAMERA_SUPPORT)
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_LibCamera_Message_t,
                                       Stream_CamSave_LibCamera_SessionMessage_t,
                                       Stream_CamSave_LibCamera_SessionData_t,
                                       struct Stream_MediaFramework_LibCamera_MediaType> Stream_CamSave_LibCamera_GTKPixbufDisplay;
#endif // LIBCAMERA_SUPPORT
#endif // GTK_SUPPORT
//#if defined (LIBCAMERA_SUPPORT)
//typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_CamSave_LibCamera_Message_t,
//                                       Stream_CamSave_LibCamera_SessionMessage_t,
//                                       Stream_CamSave_LibCamera_SessionData_t,
//                                       struct Stream_MediaFramework_LibCamera_MediaType> Stream_CamSave_LibCamera_X11Display;
//#endif // LIBCAMERA_SUPPORT
//typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_CamSave_V4L_Message_t,
//                                       Stream_CamSave_V4L_SessionMessage_t,
//                                       Stream_CamSave_V4L_SessionData_t,
//                                       struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_V4L_X11Display;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Stream_CamSave_DirectShow_Message_t,
                                   Stream_CamSave_DirectShow_SessionMessage_t> Stream_CamSave_DirectShow_FileWriter;

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Stream_CamSave_MediaFoundation_Message_t,
                                   Stream_CamSave_MediaFoundation_SessionMessage_t> Stream_CamSave_MediaFoundation_FileWriter;
#else
//typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
//                                   Common_TimePolicy_t,
//                                   struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
//                                   Stream_ControlMessage_t,
//                                   Stream_CamSave_Message_t,
//                                   Stream_CamSave_V4L_SessionMessage_t> Stream_CamSave_FileWriter;
#if defined (LIBCAMERA_SUPPORT)
typedef Stream_Module_FileWriter_2<Common_TimePolicy_t,
                                   struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Stream_CamSave_LibCamera_Message_t,
                                   Stream_CamSave_LibCamera_SessionMessage_t> Stream_CamSave_LibCamera_FileWriter;
#endif // LIBCAMERA_SUPPORT
typedef Stream_Module_FileWriter_2<Common_TimePolicy_t,
                                   struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Stream_CamSave_V4L_Message_t,
                                   Stream_CamSave_V4L_SessionMessage_t> Stream_CamSave_V4L_FileWriter;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_DirectShow_Message_t,
                                       Stream_CamSave_DirectShow_SessionMessage_t,
                                       Stream_CamSave_DirectShow_SessionData,
                                       struct Stream_UserData> Stream_CamSave_DirectShow_MessageHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_MediaFoundation_Message_t,
                                       Stream_CamSave_MediaFoundation_SessionMessage_t,
                                       Stream_CamSave_MediaFoundation_SessionData,
                                       struct Stream_UserData> Stream_CamSave_MediaFoundation_MessageHandler;
#else
#if defined (LIBCAMERA_SUPPORT)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_LibCamera_Message_t,
                                       Stream_CamSave_LibCamera_SessionMessage_t,
                                       Stream_CamSave_LibCamera_SessionData,
                                       struct Stream_UserData> Stream_CamSave_LibCamera_MessageHandler;
#endif // LIBCAMERA_SUPPORT
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CamSave_V4L_Message_t,
                                       Stream_CamSave_V4L_SessionMessage_t,
                                       Stream_CamSave_V4L_SessionData,
                                       struct Stream_UserData> Stream_CamSave_V4L_MessageHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_vfw_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_VfW_Source);                                 // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_directshow_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_Source);                // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_Source);           // writer type

DATASTREAM_MODULE_DUPLEX (Stream_CamSave_DirectShow_SessionData,            // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_DirectShow_Distributor_Reader_t,   // reader type
                          Stream_CamSave_DirectShow_Distributor_Writer_t,   // writer type
                          Stream_CamSave_DirectShow_Distributor);           // name

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,            // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_LibAVDecoder);          // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,            // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_LibAVConverter);        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,            // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_LibAVResize);           // writer type
#endif // FFMPEG_SUPPORT
#else
#if defined (LIBCAMERA_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_LibCamera_SessionData,             // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_libcamera_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibCamera_Source);                 // writer type
#endif // LIBCAMERA_SUPPORT
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_V4L_Source);                       // writer type

#if defined (LIBCAMERA_SUPPORT)
#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_LibCamera_SessionData,             // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibCamera_LibAVDecoder);           // writer type
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT
#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_V4L_LibAVDecoder);                 // writer type
#endif // FFMPEG_SUPPORT

#if defined (LIBCAMERA_SUPPORT)
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_LibCamera_SessionData,             // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_LibCamera_Distributor_Reader_t,    // reader type
                          Stream_CamSave_LibCamera_Distributor_Writer_t,    // writer type
                          Stream_CamSave_LibCamera_Distributor);            // module name prefix

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_LibCamera_SessionData,             // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibCamera_LibAVConverter);         // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_LibCamera_SessionData,             // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibCamera_LibAVResize);            // writer type
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT

DATASTREAM_MODULE_DUPLEX (Stream_CamSave_V4L_SessionData,                   // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_V4L_Distributor_Reader_t,          // reader type
                          Stream_CamSave_V4L_Distributor_Writer_t,          // writer type
                          Stream_CamSave_V4L_Distributor);                  // module name prefix

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_V4L_LibAVConverter);               // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_V4L_LibAVResize);                  // writer type
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_DirectShow_Statistic_ReaderTask_t, // reader type
                          Stream_CamSave_DirectShow_Statistic_WriterTask_t, // writer type
                          Stream_CamSave_DirectShow_StatisticReport);       // name

DATASTREAM_MODULE_DUPLEX (Stream_CamSave_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_MediaFoundation_Statistic_ReaderTask_t, // reader type
                          Stream_CamSave_MediaFoundation_Statistic_WriterTask_t, // writer type
                          Stream_CamSave_MediaFoundation_StatisticReport);  // name
#else
//DATASTREAM_MODULE_DUPLEX (Stream_CamSave_V4L_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                   // session event type
//                          struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                 // stream notification interface type
//                          Stream_CamSave_Statistic_ReaderTask_t,            // reader type
//                          Stream_CamSave_Statistic_WriterTask_t,            // writer type
//                          Stream_CamSave_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_DirectShow_SessionData,                           // session data type
                          enum Stream_SessionMessageType,                              // session event type
                          struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                            // stream notification interface type
                          Stream_CamSave_DirectShow_AVIEncoder_ReaderTask_t,           // reader type
                          Stream_CamSave_DirectShow_AVIEncoder_WriterTask_t,           // writer type
                          Stream_CamSave_DirectShow_AVIEncoder);                       // name

DATASTREAM_MODULE_DUPLEX (Stream_CamSave_MediaFoundation_SessionData,                                // session data type
                          enum Stream_SessionMessageType,                                   // session event type
                          struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                                 // stream notification interface type
                          Stream_CamSave_MediaFoundation_AVIEncoder_ReaderTask_t,           // reader type
                          Stream_CamSave_MediaFoundation_AVIEncoder_WriterTask_t,           // writer type
                          Stream_CamSave_MediaFoundation_AVIEncoder);                       // name
#else
#if defined (LIBCAMERA_SUPPORT)
#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_LibCamera_SessionData,             // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_LibCamera_AVIEncoder_ReaderTask_t, // reader type
                          Stream_CamSave_LibCamera_AVIEncoder_WriterTask_t, // writer type
                          Stream_CamSave_LibCamera_AVIEncoder);             // name
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT
#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_V4L_SessionData,                   // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_V4L_AVIEncoder_ReaderTask_t,       // reader type
                          Stream_CamSave_V4L_AVIEncoder_WriterTask_t,       // writer type
                          Stream_CamSave_V4L_AVIEncoder);                   // name
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_Direct3DDisplay);       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct3d11_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Stream_CamSave_DirectShow_Direct3D11Display);                // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_Direct3DDisplay);  // writer type

#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Stream_CamSave_DirectShow_DirectShowDisplay);                // writer type
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_MediaFoundation_SessionData,                      // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Stream_CamSave_MediaFoundation_MediaFoundationDisplay); // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_MediaFoundation_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull); // writer type

#if (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,            // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_GTKPixbufDisplay);      // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,            // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_GTKCairoDisplay);       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_MediaFoundation_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_GTKCairoDisplay);  // writer type
#endif // GTK_SUPPORT
#else
#if (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_CamSave_V4L_GTKCairoDisplay);                  // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_V4L_GTKPixbufDisplay);             // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_window_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_CamSave_Display_V4L_GTKWindowDisplay);     // writer type
#endif // GTK_SUPPORT
#if defined (LIBCAMERA_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_LibCamera_SessionData,             // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibCamera_GTKPixbufDisplay);       // writer type
#endif // LIBCAMERA_SUPPORT
//DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_x11_window_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_CamSave_V4L_X11Display);                   // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_FileWriter);            // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_FileWriter);       // writer type
#else
#if defined (LIBCAMERA_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_LibCamera_SessionData,             // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibCamera_FileWriter);             // writer type
#endif // LIBCAMERA_SUPPORT
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_V4L_FileWriter);                   // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_MessageHandler);        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_MessageHandler);   // writer type
#else
#if defined (LIBCAMERA_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_LibCamera_SessionData,                 // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_CamSave_LibCamera_MessageHandler);             // writer type
#endif // LIBCAMERA_SUPPORT
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_CamSave_V4L_MessageHandler);                   // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
