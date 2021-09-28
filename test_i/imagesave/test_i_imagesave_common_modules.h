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

#include "stream_dec_mpeg_4_decoder.h"
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_decoder.h"
#endif // FFMPEG_SUPPORT

#include "stream_file_source.h"

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

//#include "stream_stat_statistic_report.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct3d.h"
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT

//#include "stream_vis_gtk_pixbuf.h"
#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#include "test_i_imagesave_common.h"
#include "test_i_imagesave_message.h"
#include "test_i_imagesave_session_message.h"

// declare module(s)
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_I_Message,
                                    Test_I_SessionMessage_t,
                                    struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_I_ImageSave_StreamState,
                                    Test_I_ImageSave_SessionData,
                                    Test_I_ImageSave_SessionData_t,
                                    struct Test_I_StatisticData,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Test_I_Source;

typedef Stream_Decoder_MPEG_4_Decoder_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                        Stream_ControlMessage_t,
                                        Test_I_Message,
                                        Test_I_SessionMessage_t,
                                        Test_I_ImageSave_SessionData_t> Test_I_MP4Decoder;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Message,
                                      Test_I_SessionMessage_t,
                                      Test_I_ImageSave_SessionData_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                      struct _AMMediaType> Test_I_LibAVDecoder;
#else
                                      struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_I_LibAVDecoder;
#endif // ACE_WIN32 || ACE_WIN64
#endif // FFMPEG_SUPPORT

//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_DirectShow_Message_t,
//                                                      Test_I_DirectShow_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Test_I_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_DirectShow_SessionData,
//                                                      Test_I_DirectShow_SessionData_t> Test_I_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Test_I_DirectShow_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_I_DirectShow_Message_t,
//                                                      Test_I_DirectShow_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Test_I_StatisticData,
//                                                      Common_Timer_Manager_t,
//                                                      Test_I_DirectShow_SessionData,
//                                                      Test_I_DirectShow_SessionData_t> Test_I_Statistic_WriterTask_t;

typedef Stream_Miscellaneous_Distributor_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_SessionMessage_t,
                                           Test_I_ImageSave_SessionData> Test_I_Distributor;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
//typedef Stream_Decoder_LibAVConverter_T<ACE_MT_SYNCH,
//                                        Common_TimePolicy_t,
//                                        struct Test_I_ImageSave_ModuleHandlerConfiguration,
//                                        Stream_ControlMessage_t,
//                                        Test_I_Message,
//                                        Test_I_SessionMessage_t,
//                                        Test_I_ImageSave_SessionData_t,
//                                        struct _AMMediaType> Test_I_LibAVConverter;
typedef Stream_Visualization_LibAVResize_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_I_Message,
                                           Test_I_SessionMessage_t,
                                           Test_I_ImageSave_SessionData_t,
                                           struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_I_LibAVResize;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage_t,
                                     Test_I_ImageSave_SessionData,
                                     Test_I_ImageSave_SessionData_t,
                                     struct _AMMediaType> Test_I_Display;
#else
//typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
//                                      Common_TimePolicy_t,
//                                      struct Test_I_V4L_ModuleHandlerConfiguration,
//                                      Stream_ControlMessage_t,
//                                      Test_I_Message_t,
//                                      Test_I_V4L_SessionMessage_t,
//                                      Test_I_V4L_SessionData,
//                                      Test_I_V4L_SessionData_t,
//                                      struct Stream_MediaFramework_V4L_MediaType> Test_I_Display;
//typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Test_I_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Test_I_Message_t,
//                                       Test_I_V4L_SessionMessage_t,
//                                       Test_I_V4L_SessionData_t,
//                                       struct Stream_MediaFramework_V4L_MediaType> Test_I_Display;
//typedef Stream_Module_Vis_GTK_Window_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Test_I_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Test_I_Message_t,
//                                       Test_I_V4L_SessionMessage_t,
//                                       struct Stream_MediaFramework_V4L_MediaType> Test_I_Display_2;
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_SessionMessage_t,
                                       Test_I_ImageSave_SessionData_t,
                                       struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_I_Display;
#endif // ACE_WIN32 || ACE_WIN64

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_ImageSave_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Message,
                                       Test_I_SessionMessage_t,
                                       Test_I_ImageSave_SessionData,
                                       struct Stream_UserData> Test_I_MessageHandler;

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                      // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                                    // stream notification interface type
                              Test_I_Source);                                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                      // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                                    // stream notification interface type
                              Test_I_MP4Decoder);                                  // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                              // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration,        // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_LibAVDecoder);                                      // writer type
#endif // FFMPEG_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration,       // module handler configuration type
                              libacestream_default_misc_distributor_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_I_Distributor);                                      // writer type

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Test_I_ImageSave_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_dec_libav_converter_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Test_I_LibAVConverter);                   // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

//DATASTREAM_MODULE_DUPLEX (Test_I_DirectShow_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                   // session event type
//                          struct Test_I_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                 // stream notification interface type
//                          Test_I_DirectShow_Statistic_ReaderTask_t, // reader type
//                          Test_I_DirectShow_Statistic_WriterTask_t, // writer type
//                          Test_I_DirectShow_StatisticReport);       // name

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_I_Display);                                      // writer type
#else
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_V4L_SessionData,                       // session data type
//                              enum Stream_SessionMessageType,                       // session event type
//                              struct Test_I_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_cairo_module_name_string,
//                              Stream_INotify_t,                                     // stream notification interface type
//                              Test_I_Display);                              // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_V4L_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Test_I_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_pixbuf_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Test_I_Display);                          // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_V4L_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Test_I_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_vis_gtk_window_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Test_I_Display_2);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration,     // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_Display);                                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ImageSave_SessionData,                                // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_ImageSave_ModuleHandlerConfiguration,          // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_MessageHandler);                                      // writer type

#endif
