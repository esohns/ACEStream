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

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
#include "stream_vis_gtk_cairo.h"
//#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"
#include "stream_vis_target_direct3d.h"
#else
#include "stream_dev_cam_source_v4l.h"
#endif
#include "stream_dec_avi_encoder.h"
#include "stream_file_sink.h"
#include "stream_misc_runtimestatistic.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_message.h"
#include "test_u_camsave_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_MUTEX,
//
//                                           Stream_CamSave_SessionMessage,
//                                           Stream_CamSave_Message,
//
//                                           Stream_CamSave_ModuleHandlerConfiguration,
//
//                                           Stream_State,
//
//                                           Stream_CamSave_SessionData,
//                                           Stream_CamSave_SessionData_t,
//
//                                           Stream_CamSave_StatisticData> Stream_CamSave_Module_Source;
typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_MUTEX,

                                                Stream_CamSave_SessionMessage,
                                                Stream_CamSave_Message,

                                                Stream_CamSave_ModuleHandlerConfiguration,

                                                int,
                                                int,
                                                Stream_State,

                                                Stream_CamSave_SessionData,
                                                Stream_CamSave_SessionData_t,

                                                Stream_CamSave_StatisticData> Stream_CamSave_Module_Source;
#else
typedef Stream_Module_CamSource_V4L_T<ACE_SYNCH_MUTEX,
                                      ////
                                      Stream_CamSave_SessionMessage,
                                      Stream_CamSave_Message,
                                      ////
                                      Stream_CamSave_ModuleHandlerConfiguration,
                                      ////
                                      int,
                                      int,
                                      Stream_State,
                                      ////
                                      Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t,
                                      ////
                                      Stream_CamSave_StatisticData> Stream_CamSave_Module_Source;
#endif
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                              // task synch type
                              Common_TimePolicy_t,                       // time policy
                              Stream_ModuleConfiguration,                // module configuration type
                              Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_Module_Source);             // writer type

typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Stream_CamSave_SessionMessage,
                                             Stream_CamSave_Message,
                                             Stream_CommandType_t,
                                             Stream_CamSave_StatisticData,
                                             Stream_CamSave_SessionData,
                                             Stream_CamSave_SessionData_t> Stream_CamSave_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Stream_CamSave_SessionMessage,
                                             Stream_CamSave_Message,
                                             Stream_CommandType_t,
                                             Stream_CamSave_StatisticData,
                                             Stream_CamSave_SessionData,
                                             Stream_CamSave_SessionData_t> Stream_CamSave_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                 // task synch type
                          Common_TimePolicy_t,                          // time policy type
                          Stream_ModuleConfiguration,                   // module configuration type
                          Stream_CamSave_ModuleHandlerConfiguration,    // module handler configuration type
                          Stream_CamSave_Module_Statistic_ReaderTask_t, // reader type
                          Stream_CamSave_Module_Statistic_WriterTask_t, // writer type
                          Stream_CamSave_Module_RuntimeStatistic);      // name

typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,

                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData> Stream_CamSave_Module_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<Stream_CamSave_SessionMessage,
                                               Stream_CamSave_Message,
                                              
                                               Stream_CamSave_ModuleHandlerConfiguration,
                                   
                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData> Stream_CamSave_Module_AVIEncoder_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                  // task synch type
                          Common_TimePolicy_t,                           // time policy type
                          Stream_ModuleConfiguration,                    // module configuration type
                          Stream_CamSave_ModuleHandlerConfiguration,     // module handler configuration type
                          Stream_CamSave_Module_AVIEncoder_ReaderTask_t, // reader type
                          Stream_CamSave_Module_AVIEncoder_WriterTask_t, // writer type
                          Stream_CamSave_Module_AVIEncoder);             // name

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Vis_Target_DirectShow_T<Stream_CamSave_SessionMessage,
//                                       Stream_CamSave_Message,
//                                       ///
//                                       Stream_CamSave_ModuleHandlerConfiguration,
//                                       ///
//                                       Stream_CamSave_SessionData,
//                                       Stream_CamSave_SessionData_t> Stream_CamSave_Module_Display;
//typedef Stream_Vis_Target_MediaFoundation_T<Stream_CamSave_SessionMessage,
//                                            Stream_CamSave_Message,
//                                            
//                                            Stream_CamSave_ModuleHandlerConfiguration,
//                                            
//                                            Stream_CamSave_SessionData,
//                                            Stream_CamSave_SessionData_t> Stream_CamSave_Module_Display;
typedef Stream_Vis_Target_MediaFoundation_2<Stream_CamSave_SessionMessage,
                                            Stream_CamSave_Message,

                                            Stream_CamSave_ModuleHandlerConfiguration,

                                            Stream_CamSave_SessionData,
                                            Stream_CamSave_SessionData_t> Stream_CamSave_Module_DisplayNull;
//typedef Stream_Vis_Target_Direct3D_T<Stream_CamSave_SessionMessage,
//                                     Stream_CamSave_Message,
//                                     /////
//                                     Stream_CamSave_ModuleHandlerConfiguration,
//                                     /////
//                                     Stream_CamSave_SessionData,
//                                     Stream_CamSave_SessionData_t> Stream_CamSave_Module_Display;
#endif
typedef Stream_Module_Vis_GTK_Cairo_T<Stream_CamSave_SessionMessage,
                                      Stream_CamSave_Message,
                                      ////
                                      Stream_CamSave_ModuleHandlerConfiguration,
                                      ////
                                      Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t> Stream_CamSave_Module_Display;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                              // task synch type
                              Common_TimePolicy_t,                       // time policy
                              Stream_ModuleConfiguration,                // module configuration type
                              Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_Module_Display);            // writer type
#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                              // task synch type
                              Common_TimePolicy_t,                       // time policy
                              Stream_ModuleConfiguration,                // module configuration type
                              Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_Module_DisplayNull);        // writer type
#endif

typedef Stream_Module_FileWriter_T<Stream_CamSave_SessionMessage,
                                   Stream_CamSave_Message,
                                   ///////
                                   Stream_CamSave_ModuleHandlerConfiguration,
                                   ///////
                                   Stream_CamSave_SessionData> Stream_CamSave_Module_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                              // task synch type
                              Common_TimePolicy_t,                       // time policy
                              Stream_ModuleConfiguration,                // module configuration type
                              Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_Module_FileWriter);         // writer type

#endif
