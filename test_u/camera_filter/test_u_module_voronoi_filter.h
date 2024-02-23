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

#ifndef TEST_U_MODULE_VORONOI_FILTER_H
#define TEST_U_MODULE_VORONOI_FILTER_H

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#undef OK
#if defined (OLC_PGE_SUPPORT)
#include "olcPixelGameEngine.h"
#endif // OLC_PGE_SUPPORT
#include "jc_voronoi.h"

#include "ace/Basic_Types.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_control_message.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_asynch.h"

#include "stream_lib_mediatype_converter.h"

#include "test_u_camera_filter_common.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

//////////////////////////////////////////

#define ACESTREAM_VORONOI_FILTER_DEFAULT_NUMBER_OF_POINTS 500
#define ACESTREAM_VORONOI_FILTER_DEFAULT_ALPHA_DECAY      15

//////////////////////////////////////////

extern const char libacestream_default_voronoi_filter_module_name_string[];

class Test_U_CameraFilter_Voronoi_Filter
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 : public Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                  Stream_ControlMessage_t,
                                  Test_U_DirectShow_Message_t,
                                  Test_U_DirectShow_SessionMessage_t,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<struct _AMMediaType>
#else
 : public Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                  Stream_ControlMessage_t,
                                  Test_U_Message_t,
                                  Test_U_SessionMessage_t,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<struct Stream_MediaFramework_V4L_MediaType>
#endif // ACE_WIN32 || ACE_WIN64
 , public olc::PixelGameEngine
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_U_DirectShow_Message_t,
                                 Test_U_DirectShow_SessionMessage_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
 typedef Stream_MediaFramework_MediaTypeConverter_T<struct _AMMediaType> inherited2;
#else
 typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_U_Message_t,
                                 Test_U_SessionMessage_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
 typedef Stream_MediaFramework_MediaTypeConverter_T<struct Stream_MediaFramework_V4L_MediaType> inherited2;
#endif // ACE_WIN32 || ACE_WIN64
 typedef olc::PixelGameEngine inherited3;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_CameraFilter_Voronoi_Filter (ISTREAM_T*); // stream handle
#else
  Test_U_CameraFilter_Voronoi_Filter (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_U_CameraFilter_Voronoi_Filter () {}

  // implement (part of) Stream_ITaskBase_T
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void handleDataMessage (Test_U_DirectShow_Message_t*&, // data message handle
                                  bool&);                        // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_U_DirectShow_SessionMessage_t*&, // session message handle
                                     bool&);                               // return value: pass message downstream ?
#else
  virtual void handleDataMessage (Test_U_Message_t*&, // data message handle
                                  bool&);             // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_U_SessionMessage_t*&, // session message handle
                                     bool&);                    // return value: pass message downstream ?
#endif // ACE_WIN32 || ACE_WIN64

  virtual bool OnUserCreate ();
  virtual bool OnUserUpdate (float); // elapsed time
  virtual bool OnUserDestroy ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_Voronoi_Filter ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_Voronoi_Filter (const Test_U_CameraFilter_Voronoi_Filter&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_Voronoi_Filter& operator= (const Test_U_CameraFilter_Voronoi_Filter&))

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  bool processNextMessage (); // return value: stop PGE ?

  int closestPoint (jcv_diagram&, olc::vf2d&);

  uint8_t                   bytesPerPixel_;
  jcv_point*                points_;
  Common_Image_Resolution_t resolution_;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_voronoi_filter_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_CameraFilter_Voronoi_Filter);                                // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_voronoi_filter_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_CameraFilter_Voronoi_Filter);                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
