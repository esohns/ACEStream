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
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
#else
#include "stream_dev_cam_source_v4l.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_converter.h"
#endif // FFMPEG_SUPPORT

#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_window.h"
#endif // GTK_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct2d.h"
#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_gdi.h"
#include "stream_vis_target_mediafoundation.h"
#else
#include "stream_vis_wayland_window.h"
#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GLUT_SUPPORT)
#include "stream_vis_opengl_glut.h"
#endif // GLUT_SUPPORT

#include "test_u_camera_filter_common.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_DirectShow_Message_t,
                               Test_U_DirectShow_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_DirectShow_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_DirectShow_Message_t,
                                Test_U_DirectShow_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_DirectShow_TaskBaseAsynch_t;

typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Test_U_DirectShow_Message_t,
                                           Test_U_DirectShow_SessionMessage_t,
                                           struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Test_U_DirectShow_StreamState,
                                           Test_U_CameraFilter_DirectShow_SessionData,
                                           Test_U_CameraFilter_DirectShow_SessionData_t,
                                           struct Test_U_StatisticData,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData,
                                           struct _AMMediaType,
                                           false> Test_U_DirectShow_Source;

typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Test_U_MediaFoundation_Message_t,
                                                Test_U_MediaFoundation_SessionMessage_t,
                                                struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Test_U_MediaFoundation_StreamState,
                                                Test_U_CameraFilter_MediaFoundation_SessionData,
                                                Test_U_CameraFilter_MediaFoundation_SessionData_t,
                                                struct Test_U_StatisticData,
                                                Common_Timer_Manager_t,
                                                struct Stream_UserData,
                                                IMFMediaType*> Test_U_MediaFoundation_Source;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_DirectShow_TaskBaseSynch_t,
                                        struct _AMMediaType> Test_U_DirectShow_LibAVConvert;

typedef Stream_Visualization_LibAVResize_T<Test_U_DirectShow_TaskBaseSynch_t,
                                           struct _AMMediaType> Test_U_DirectShow_LibAVResize;
#endif // FFMPEG_SUPPORT
#else
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message_t,
                               Test_U_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_Message_t,
                                Test_U_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_TaskBaseAsynch_t;

typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Test_U_Message_t,
                                      Test_U_SessionMessage_t,
                                      struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Test_U_StreamState,
                                      Test_U_CameraFilter_V4L_SessionData,
                                      Test_U_CameraFilter_V4L_SessionData_t,
                                      struct Test_U_StatisticData,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Test_U_V4L_Source;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Test_U_LibAVConvert;

typedef Stream_Visualization_LibAVResize_T<Test_U_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Test_U_LibAVResize;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_DirectShow_Message_t,
                                                      Test_U_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CameraFilter_DirectShow_SessionData,
                                                      Test_U_CameraFilter_DirectShow_SessionData_t> Test_U_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_DirectShow_Message_t,
                                                      Test_U_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CameraFilter_DirectShow_SessionData,
                                                      Test_U_CameraFilter_DirectShow_SessionData_t> Test_U_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_MediaFoundation_Message_t,
                                                      Test_U_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CameraFilter_MediaFoundation_SessionData,
                                                      Test_U_CameraFilter_MediaFoundation_SessionData_t> Test_U_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_MediaFoundation_Message_t,
                                                      Test_U_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CameraFilter_MediaFoundation_SessionData,
                                                      Test_U_CameraFilter_MediaFoundation_SessionData_t> Test_U_MediaFoundation_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message_t,
                                                      Test_U_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CameraFilter_V4L_SessionData,
                                                      Test_U_CameraFilter_V4L_SessionData_t> Test_U_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message_t,
                                                      Test_U_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CameraFilter_V4L_SessionData,
                                                      Test_U_CameraFilter_V4L_SessionData_t> Test_U_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct2D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_U_DirectShow_Message_t,
                                     Test_U_DirectShow_SessionMessage_t,
                                     Test_U_CameraFilter_DirectShow_SessionData,
                                     Test_U_CameraFilter_DirectShow_SessionData_t,
                                     struct _AMMediaType> Test_U_DirectShow_Direct2D_Display;

typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_U_DirectShow_Message_t,
                                     Test_U_DirectShow_SessionMessage_t,
                                     Test_U_CameraFilter_DirectShow_SessionData,
                                     Test_U_CameraFilter_DirectShow_SessionData_t,
                                     struct _AMMediaType> Test_U_DirectShow_Direct3D_Display;

struct Test_U_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Test_U_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Test_U_DirectShow_Message_t,
                                                         struct Test_U_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_U_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Test_U_DirectShow_Message_t,
                                                                struct Test_U_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration> Test_U_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_DirectShow_Message_t,
                                       Test_U_DirectShow_SessionMessage_t,
                                       Test_U_CameraFilter_DirectShow_SessionData_t,
                                       Test_U_CameraFilter_DirectShow_SessionData,
                                       struct Test_U_DirectShow_FilterConfiguration,
                                       struct Test_U_DirectShow_PinConfiguration,
                                       Test_U_DirectShowFilter_t,
                                       struct _AMMediaType> Test_U_DirectShow_Display;
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

#if defined (GLUT_SUPPORT)
typedef Stream_Visualization_OpenGL_GLUT_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_U_DirectShow_Message_t,
                                           Test_U_DirectShow_SessionMessage_t,
                                           Test_U_CameraFilter_DirectShow_SessionData_t,
                                           struct _AMMediaType> Test_U_DirectShow_OpenGL_Display;
#endif // GLUT_SUPPORT

typedef Stream_Vis_Target_GDI_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_DirectShow_Message_t,
                                Test_U_DirectShow_SessionMessage_t,
                                Test_U_CameraFilter_DirectShow_SessionData,
                                Test_U_CameraFilter_DirectShow_SessionData_t,
                                struct _AMMediaType> Test_U_DirectShow_GDI_Display;

#if defined (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_DirectShow_Message_t,
                                       Test_U_DirectShow_SessionMessage_t,
                                       struct _AMMediaType> Test_U_DirectShow_GTK_Display;
#endif // GTK_SUPPORT

typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_U_MediaFoundation_Message_t,
                                     Test_U_MediaFoundation_SessionMessage_t,
                                     Test_U_CameraFilter_MediaFoundation_SessionData,
                                     Test_U_CameraFilter_MediaFoundation_SessionData_t,
                                     IMFMediaType*> Test_U_MediaFoundation_Direct3D_Display;

typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_U_MediaFoundation_Message_t,
                                            Test_U_MediaFoundation_SessionMessage_t,
                                            Test_U_CameraFilter_MediaFoundation_SessionData,
                                            Test_U_CameraFilter_MediaFoundation_SessionData_t,
                                            struct Stream_UserData> Test_U_MediaFoundation_Display;
typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_U_MediaFoundation_Message_t,
                                            Test_U_MediaFoundation_SessionMessage_t,
                                            Test_U_CameraFilter_MediaFoundation_SessionData,
                                            Test_U_CameraFilter_MediaFoundation_SessionData_t,
                                            IMFMediaType*> Test_U_MediaFoundation_DisplayNull;

#if defined (GLUT_SUPPORT)
typedef Stream_Visualization_OpenGL_GLUT_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_U_MediaFoundation_Message_t,
                                           Test_U_MediaFoundation_SessionMessage_t,
                                           Test_U_CameraFilter_MediaFoundation_SessionData_t,
                                           IMFMediaType*> Test_U_MediaFoundation_OpenGL_Display;
#endif // GLUT_SUPPORT

typedef Stream_Vis_Target_GDI_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_MediaFoundation_Message_t,
                                Test_U_MediaFoundation_SessionMessage_t,
                                Test_U_CameraFilter_MediaFoundation_SessionData,
                                Test_U_CameraFilter_MediaFoundation_SessionData_t,
                                IMFMediaType*> Test_U_MediaFoundation_GDI_Display;
#else
#if defined (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message_t,
                                       Test_U_SessionMessage_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Test_U_GTK_Display;
#endif // GTK_SUPPORT
typedef Stream_Module_Vis_Wayland_Window_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_U_Message_t,
                                           Test_U_SessionMessage_t,
                                           Test_U_CameraFilter_V4L_SessionData_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Test_U_Wayland_Display;
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message_t,
                                       Test_U_SessionMessage_t,
                                       Test_U_CameraFilter_V4L_SessionData_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Test_U_X11_Display;
#if defined (GLUT_SUPPORT)
typedef Stream_Visualization_OpenGL_GLUT_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_U_Message_t,
                                           Test_U_SessionMessage_t,
                                           Test_U_CameraFilter_V4L_SessionData_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Test_U_OpenGL_Display;
#endif // GLUT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_DirectShow_Message_t,
                                       Test_U_DirectShow_SessionMessage_t,
                                       Test_U_CameraFilter_DirectShow_SessionData,
                                       struct Stream_UserData> Test_U_DirectShow_MessageHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_MediaFoundation_Message_t,
                                       Test_U_MediaFoundation_SessionMessage_t,
                                       Test_U_CameraFilter_MediaFoundation_SessionData,
                                       struct Stream_UserData> Test_U_MediaFoundation_MessageHandler;
#else
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message_t,
                                       Test_U_SessionMessage_t,
                                       Test_U_CameraFilter_V4L_SessionData,
                                       struct Stream_UserData> Test_U_MessageHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_Source);                // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_MediaFoundation_Source);           // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_V4L_Source);                       // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Test_U_CameraFilter_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_U_DirectShow_Statistic_ReaderTask_t, // reader type
                          Test_U_DirectShow_Statistic_WriterTask_t, // writer type
                          Test_U_DirectShow_StatisticReport);       // name

DATASTREAM_MODULE_DUPLEX (Test_U_CameraFilter_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_U_MediaFoundation_Statistic_ReaderTask_t, // reader type
                          Test_U_MediaFoundation_Statistic_WriterTask_t, // writer type
                          Test_U_MediaFoundation_StatisticReport);  // name
#else
DATASTREAM_MODULE_DUPLEX (Test_U_CameraFilter_V4L_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_U_Statistic_ReaderTask_t,            // reader type
                          Test_U_Statistic_WriterTask_t,            // writer type
                          Test_U_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct2d_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_DirectShow_Direct2D_Display);                 // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_DirectShow_Direct3D_Display);                 // writer type

#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_DirectShow_Display);                                       // writer type
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

#if defined (GLUT_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_opengl_glut_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_OpenGL_Display);   // writer type
#endif // GLUT_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gdi_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_GDI_Display);                 // writer type

#if defined (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_GTK_Display);                        // writer type
#endif // GTK_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_MediaFoundation_Direct3D_Display);  // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_MediaFoundation_SessionData,                      // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_U_MediaFoundation_Display); // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_MediaFoundation_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_U_MediaFoundation_DisplayNull); // writer type

#if defined (GLUT_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_MediaFoundation_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_opengl_glut_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_MediaFoundation_OpenGL_Display);   // writer type
#endif // GLUT_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_MediaFoundation_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gdi_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_MediaFoundation_GDI_Display); // writer type
#else
#if defined (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_GTK_Display);                        // writer type
#endif // GTK_SUPPORT
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_X11_Display);                                       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_wayland_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_Wayland_Display);                          // writer type
#if defined (GLUT_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_opengl_glut_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_OpenGL_Display);              // writer type
#endif // GLUT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_MessageHandler);        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_MediaFoundation_MessageHandler);   // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_U_MessageHandler);                       // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
