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

#ifndef TEST_U_IMAGESCREEN_COMMON_MODULES_H
#define TEST_U_IMAGESCREEN_COMMON_MODULES_H

#include "ace/config-lite.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_decoder.h"

#include "stream_dev_cam_source_v4l.h"
#include "stream_dec_avi_encoder.h"

#include "stream_file_source.h"

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_gtk_cairo.h"

#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"
#else
#include "stream_vis_libav_resize.h"
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_USE)
#include "stream_vis_gtk_pixbuf.h"
#elif defined (WXWIDGETS_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64
#endif
#endif // GUI_SUPPORT

#include "test_u_imagescreen_common.h"
#include "test_u_imagescreen_message.h"
#include "test_u_imagescreen_session_message.h"

// declare module(s)
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Stream_ImageScreen_Message_t,
                                    Stream_ImageScreen_SessionMessage_t,
                                    struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Stream_ImageScreen_StreamState,
                                    Stream_ImageScreen_SessionData,
                                    Stream_ImageScreen_SessionData_t,
                                    struct Stream_Statistic,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Stream_ImageScreen_Source;

typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_ImageScreen_Message_t,
                                      Stream_ImageScreen_SessionMessage_t,
                                      Stream_ImageScreen_SessionData_t,
                                      struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_LibAVDecoder;

typedef Stream_Visualization_LibAVResize_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Stream_ImageScreen_Message_t,
                                           Stream_ImageScreen_SessionMessage_t,
                                           Stream_ImageScreen_SessionData_t,
                                           struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_LibAVResize;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_ImageScreen_Message_t,
                                                      Stream_ImageScreen_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Stream_ImageScreen_SessionData,
                                                      Stream_ImageScreen_SessionData_t> Stream_ImageScreen_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_ImageScreen_Message_t,
                                                      Stream_ImageScreen_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      Stream_ImageScreen_SessionData,
                                                      Stream_ImageScreen_SessionData_t> Stream_ImageScreen_Statistic_WriterTask_t;

#if defined (GTK_USE)
//typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
//                                      Common_TimePolicy_t,
//                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
//                                      Stream_ControlMessage_t,
//                                      Stream_ImageScreen_Message_t,
//                                      Stream_ImageScreen_SessionMessage_t,
//                                      Stream_ImageScreen_SessionData,
//                                      Stream_ImageScreen_SessionData_t,
//                                      struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Display;
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_ImageScreen_Message_t,
                                       Stream_ImageScreen_SessionMessage_t,
                                       Stream_ImageScreen_SessionData_t,
                                       struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Display;
#elif defined (WXWIDGETS_USE)
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_ImageScreen_Message_t,
                                       Stream_ImageScreen_SessionMessage_t,
                                       Stream_ImageScreen_SessionData_t,
                                       struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Display;
#endif

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_ImageScreen_Message_t,
                                       Stream_ImageScreen_SessionMessage_t,
                                       Stream_SessionId_t,
                                       Stream_ImageScreen_SessionData,
                                       struct Stream_ImageScreen_UserData> Stream_ImageScreen_MessageHandler;

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Source);                       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_LibAVDecoder);                     // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_LibAVResize);                      // writer type

DATASTREAM_MODULE_DUPLEX (Stream_ImageScreen_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_ImageScreen_Statistic_ReaderTask_t,            // reader type
                          Stream_ImageScreen_Statistic_WriterTask_t,            // writer type
                          Stream_ImageScreen_StatisticReport);                  // name

#if defined (GTK_USE)
//DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                       // session data type
//                              enum Stream_SessionMessageType,                       // session event type
//                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_cairo_module_name_string,
//                              Stream_INotify_t,                                     // stream notification interface type
//                              Stream_ImageScreen_Display);                              // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Display);                          // writer type
#elif defined (WXWIDGETS_USE)
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Display);                          // writer type
#endif

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_ImageScreen_MessageHandler);                       // writer type

#endif
