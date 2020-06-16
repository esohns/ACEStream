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

#ifndef TEST_U_CAMSCREEN_COMMON_MODULES_H
#define TEST_U_CAMSCREEN_COMMON_MODULES_H

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
#include "stream_dec_libav_converter.h"
//#include "stream_dec_libav_decoder.h"

#include "stream_dev_cam_source_v4l.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#include "stream_vis_gtk_window.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"
#else
#include "stream_vis_libav_resize.h"
#include "stream_vis_wayland_window.h"
#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_vis_opengl_glut.h"

#include "test_u_camerascreen_common.h"
#include "test_u_camerascreen_message.h"
#include "test_u_camerascreen_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Stream_CameraScreen_DirectShow_Message_t,
                                           Stream_CameraScreen_DirectShow_SessionMessage_t,
                                           struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Stream_CameraScreen_DirectShow_StreamState,
                                           Stream_CameraScreen_DirectShow_SessionData,
                                           Stream_CameraScreen_DirectShow_SessionData_t,
                                           struct Stream_CameraScreen_StatisticData,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData> Stream_CameraScreen_DirectShow_Source;

typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Stream_CameraScreen_MediaFoundation_Message_t,
                                                Stream_CameraScreen_MediaFoundation_SessionMessage_t,
                                                struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Stream_CameraScreen_MediaFoundation_StreamState,
                                                Stream_CameraScreen_MediaFoundation_SessionData,
                                                Stream_CameraScreen_MediaFoundation_SessionData_t,
                                                struct Stream_CameraScreen_StatisticData,
                                                Common_Timer_Manager_t,
                                                struct Stream_UserData> Stream_CameraScreen_MediaFoundation_Source;
#else
typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Stream_CameraScreen_Message_t,
                                      Stream_CameraScreen_SessionMessage_t,
                                      struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Stream_CameraScreen_StreamState,
                                      Stream_CameraScreen_V4L_SessionData,
                                      Stream_CameraScreen_V4L_SessionData_t,
                                      struct Stream_CameraScreen_StatisticData,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Stream_CameraScreen_V4L_Source;

typedef Stream_Decoder_LibAVConverter_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                        Stream_ControlMessage_t,
                                        Stream_CameraScreen_Message_t,
                                        Stream_CameraScreen_SessionMessage_t,
                                        Stream_CameraScreen_V4L_SessionData_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Stream_CameraScreen_LibAVConvert;

typedef Stream_Visualization_LibAVResize_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Stream_CameraScreen_Message_t,
                                           Stream_CameraScreen_SessionMessage_t,
                                           Stream_CameraScreen_V4L_SessionData_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Stream_CameraScreen_LibAVResize;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraScreen_DirectShow_Message_t,
                                                      Stream_CameraScreen_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraScreen_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CameraScreen_DirectShow_SessionData,
                                                      Stream_CameraScreen_DirectShow_SessionData_t> Stream_CameraScreen_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraScreen_DirectShow_Message_t,
                                                      Stream_CameraScreen_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraScreen_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CameraScreen_DirectShow_SessionData,
                                                      Stream_CameraScreen_DirectShow_SessionData_t> Stream_CameraScreen_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraScreen_MediaFoundation_Message_t,
                                                      Stream_CameraScreen_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraScreen_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CameraScreen_MediaFoundation_SessionData,
                                                      Stream_CameraScreen_MediaFoundation_SessionData_t> Stream_CameraScreen_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraScreen_MediaFoundation_Message_t,
                                                      Stream_CameraScreen_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraScreen_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CameraScreen_MediaFoundation_SessionData,
                                                      Stream_CameraScreen_MediaFoundation_SessionData_t> Stream_CameraScreen_MediaFoundation_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraScreen_Message_t,
                                                      Stream_CameraScreen_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraScreen_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CameraScreen_V4L_SessionData,
                                                      Stream_CameraScreen_V4L_SessionData_t> Stream_CameraScreen_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraScreen_Message_t,
                                                      Stream_CameraScreen_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraScreen_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CameraScreen_V4L_SessionData,
                                                      Stream_CameraScreen_V4L_SessionData_t> Stream_CameraScreen_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CameraScreen_DirectShow_Message_t,
                                     Stream_CameraScreen_DirectShow_SessionMessage_t,
                                     Stream_CameraScreen_DirectShow_SessionData,
                                     Stream_CameraScreen_DirectShow_SessionData_t,
                                     struct _AMMediaType> Stream_CameraScreen_DirectShow_Direct3DDisplay;
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CameraScreen_MediaFoundation_Message_t,
                                     Stream_CameraScreen_MediaFoundation_SessionMessage_t,
                                     Stream_CameraScreen_MediaFoundation_SessionData,
                                     Stream_CameraScreen_MediaFoundation_SessionData_t,
                                     IMFMediaType*> Stream_CameraScreen_MediaFoundation_Direct3DDisplay;

struct Stream_CameraScreen_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Stream_CameraScreen_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Common_TimePolicy_t,
                                                         Stream_CameraScreen_DirectShow_SessionMessage_t,
                                                         Stream_CameraScreen_DirectShow_Message_t,
                                                         struct Stream_CameraScreen_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                         struct _AMMediaType> Stream_CameraScreen_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Common_TimePolicy_t,
                                                                Stream_CameraScreen_DirectShow_SessionMessage_t,
                                                                Stream_CameraScreen_DirectShow_Message_t,
                                                                struct Stream_CameraScreen_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                                struct _AMMediaType> Stream_CameraScreen_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CameraScreen_DirectShow_Message_t,
                                       Stream_CameraScreen_DirectShow_SessionMessage_t,
                                       Stream_CameraScreen_DirectShow_SessionData_t,
                                       Stream_CameraScreen_DirectShow_SessionData,
                                       struct Stream_CameraScreen_DirectShow_FilterConfiguration,
                                       struct Stream_CameraScreen_DirectShow_PinConfiguration,
                                       Stream_CameraScreen_DirectShowFilter_t> Stream_CameraScreen_DirectShow_Display;

typedef Stream_Visualization_OpenGL_GLUT_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Stream_CameraScreen_DirectShow_Message_t,
                                           Stream_CameraScreen_DirectShow_SessionMessage_t,
                                           Stream_CameraScreen_DirectShow_SessionData_t,
                                           struct _AMMediaType> Stream_CameraScreen_OpenGL_Display;

typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Stream_CameraScreen_MediaFoundation_Message_t,
                                            Stream_CameraScreen_MediaFoundation_SessionMessage_t,
                                            Stream_CameraScreen_MediaFoundation_SessionData,
                                            Stream_CameraScreen_MediaFoundation_SessionData_t,
                                            struct Stream_UserData> Stream_CameraScreen_MediaFoundation_Display;
typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Stream_CameraScreen_MediaFoundation_Message_t,
                                            Stream_CameraScreen_MediaFoundation_SessionMessage_t,
                                            Stream_CameraScreen_MediaFoundation_SessionData,
                                            Stream_CameraScreen_MediaFoundation_SessionData_t> Stream_CameraScreen_MediaFoundation_DisplayNull;
#else
typedef Stream_Module_Vis_GTK_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CameraScreen_Message_t,
                                       Stream_CameraScreen_SessionMessage_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Stream_CameraScreen_GTK_Display;
typedef Stream_Module_Vis_Wayland_Window_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Stream_CameraScreen_Message_t,
                                           Stream_CameraScreen_SessionMessage_t,
                                           Stream_CameraScreen_V4L_SessionData_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Stream_CameraScreen_Wayland_Display;
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CameraScreen_Message_t,
                                       Stream_CameraScreen_SessionMessage_t,
                                       Stream_CameraScreen_V4L_SessionData_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Stream_CameraScreen_X11_Display;
typedef Stream_Visualization_OpenGL_GLUT_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Stream_CameraScreen_Message_t,
                                           Stream_CameraScreen_SessionMessage_t,
                                           Stream_CameraScreen_V4L_SessionData_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Stream_CameraScreen_OpenGL_Display;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CameraScreen_DirectShow_Message_t,
                                       Stream_CameraScreen_DirectShow_SessionMessage_t,
                                       Stream_SessionId_t,
                                       Stream_CameraScreen_DirectShow_SessionData,
                                       struct Stream_UserData> Stream_CameraScreen_DirectShow_MessageHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CameraScreen_MediaFoundation_Message_t,
                                       Stream_CameraScreen_MediaFoundation_SessionMessage_t,
                                       Stream_SessionId_t,
                                       Stream_CameraScreen_MediaFoundation_SessionData,
                                       struct Stream_UserData> Stream_CameraScreen_MediaFoundation_MessageHandler;
#else
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_CameraScreen_Message_t,
                                       Stream_CameraScreen_SessionMessage_t,
                                       Stream_SessionId_t,
                                       Stream_CameraScreen_V4L_SessionData,
                                       struct Stream_UserData> Stream_CameraScreen_MessageHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_DirectShow_Source);                // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_MediaFoundation_Source);           // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_V4L_Source);                       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_LibAVResize);                      // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Stream_CameraScreen_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CameraScreen_DirectShow_Statistic_ReaderTask_t, // reader type
                          Stream_CameraScreen_DirectShow_Statistic_WriterTask_t, // writer type
                          Stream_CameraScreen_DirectShow_StatisticReport);       // name

DATASTREAM_MODULE_DUPLEX (Stream_CameraScreen_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CameraScreen_MediaFoundation_Statistic_ReaderTask_t, // reader type
                          Stream_CameraScreen_MediaFoundation_Statistic_WriterTask_t, // writer type
                          Stream_CameraScreen_MediaFoundation_StatisticReport);  // name
#else
DATASTREAM_MODULE_DUPLEX (Stream_CameraScreen_V4L_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CameraScreen_Statistic_ReaderTask_t,            // reader type
                          Stream_CameraScreen_Statistic_WriterTask_t,            // writer type
                          Stream_CameraScreen_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_DirectShow_Direct3DDisplay);       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_MediaFoundation_Direct3DDisplay);  // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_DirectShow_Display);     // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_DirectShow_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_opengl_glut_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_OpenGL_Display);              // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_MediaFoundation_SessionData,                      // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Stream_CameraScreen_MediaFoundation_Display); // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_MediaFoundation_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Stream_CameraScreen_MediaFoundation_DisplayNull); // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_GTK_Display);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_wayland_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_Wayland_Display);                          // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_X11_Display);                          // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_opengl_glut_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_OpenGL_Display);              // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_DirectShow_MessageHandler);        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraScreen_MediaFoundation_MessageHandler);   // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_CameraScreen_MessageHandler);                       // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
