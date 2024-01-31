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

#ifndef Test_U_CameraFilter_OpenGL_GLUT_H
#define Test_U_CameraFilter_OpenGL_GLUT_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_control_message.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_asynch.h"

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gl/GL.h"
#else
#include "GL/gl.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_mediatype_converter.h"

#include "test_u_camera_filter_common.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

extern void camera_filter_glut_reshape (int, int);
extern void camera_filter_glut_key (unsigned char, int, int);
extern void camera_filter_glut_draw (void);
extern void camera_filter_glut_idle (void);
extern void camera_filter_glut_close (void);
extern void camera_filter_glut_visible (int);

extern const char libacestream_default_module_opengl_glut_name_string[];

struct Test_U_CameraFilter_OpenGL_GLUT_WindowData
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                                mediaType;
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType mediaType;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Message_Queue_Base*                            queue;
  Common_Image_Resolution_t                          resolution;
  Stream_IStreamControlBase*                         stream;

  // canvas
  int                                                columns;
  int                                                rows;
  int                                                scaleFactor;

  // shader
  GLuint                                             programId;
  GLuint                                             textureId;
  GLuint                                             VAOId;
  GLuint                                             VBOId;

  GLint                                              textureLoc;
  GLint                                              textureSizeLoc;
};

class Test_U_CameraFilter_OpenGL_GLUT
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
{
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

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_CameraFilter_OpenGL_GLUT (ISTREAM_T*); // stream handle
#else
   Test_U_CameraFilter_OpenGL_GLUT (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_U_CameraFilter_OpenGL_GLUT () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_U_DirectShow_Message_t*&, // data message handle
                                  bool&);                        // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_U_DirectShow_SessionMessage_t*&, // session message handle
                                     bool&);                               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_OpenGL_GLUT ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_OpenGL_GLUT (const Test_U_CameraFilter_OpenGL_GLUT&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_OpenGL_GLUT& operator= (const Test_U_CameraFilter_OpenGL_GLUT&))

  // helper methods
  virtual int svc (void);

  struct Test_U_CameraFilter_OpenGL_GLUT_WindowData CBData_;
  bool                                              inSession_;
  bool                                              leftGLUTMainLoop_;
  int                                               window_;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_module_opengl_glut_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_CameraFilter_OpenGL_GLUT);                                // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_module_opengl_glut_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_CameraFilter_OpenGL_GLUT);                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
