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

#ifndef Test_U_CameraFilter_OpenGL_GLUT_39_H
#define Test_U_CameraFilter_OpenGL_GLUT_39_H

#include <chrono>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_gl_shader.h"
#include "common_gl_texture.h"

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

extern void camera_filter_glut_39_reshape (int, int);
extern void camera_filter_glut_39_key (unsigned char, int, int);
extern void camera_filter_glut_39_mouse_button (int, int, int, int);
extern void camera_filter_glut_39_mouse_move (int, int);
extern void camera_filter_glut_39_draw (void);
extern void camera_filter_glut_39_idle (void);
extern void camera_filter_glut_39_close (void);
extern void camera_filter_glut_39_visible (int);

extern const char libacestream_default_module_opengl_glut_39_name_string[];

struct Test_U_CameraFilter_OpenGL_GLUT_39_WindowData
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
  unsigned int                                       depth;

  Stream_IStreamControlBase*                         stream;

  // mouse
  int                                                mouseX;
  int                                                mouseY;
  bool                                               mouse_LMB_pressed;

  // time
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::chrono::steady_clock::time_point              tp1;
#elif defined (ACE_LINUX)
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tp1;
#else
#error missing implementation, aborting
#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX

  // shader
  Common_GL_Shader                                   shader1;
  Common_GL_Shader                                   shader2;
  Common_GL_Shader                                   shader3;

  Common_GL_Texture                                  texture0; // camera image
  Common_GL_Texture                                  textureS1;
  Common_GL_Texture                                  textureS2;
  Common_GL_Texture                                  texture5; // random image

  GLuint                                             FBO1;
  GLuint                                             FBO2;

  GLuint                                             VAOId;
  GLuint                                             VBOId;

  GLint                                              S1resolutionLoc;
  GLint                                              S1timeLoc;
  GLint                                              S1frameLoc;
  GLint                                              S1mouseLoc;
  GLint                                              S1channel0Loc;
  GLint                                              S1channel1Loc;
  GLint                                              S1channel2Loc;

  GLint                                              S2resolutionLoc;
  GLint                                              S2mouseLoc;
  GLint                                              S2channel0Loc;

  GLint                                              S3resolutionLoc;
  GLint                                              S3channel0Loc;
};

class Test_U_CameraFilter_OpenGL_GLUT_39
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

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_CameraFilter_OpenGL_GLUT_39 (ISTREAM_T*); // stream handle
#else
   Test_U_CameraFilter_OpenGL_GLUT_39 (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_U_CameraFilter_OpenGL_GLUT_39 () {}

  // override (part of) Stream_IModuleHandler_T
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_U_DirectShow_Message_t*&, // data message handle
                                  bool&);                        // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_U_DirectShow_SessionMessage_t*&, // session message handle
                                     bool&);                               // return value: pass message downstream ?
#else
  virtual bool initialize (const struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_U_Message_t*&, // data message handle
                                  bool&);             // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_U_SessionMessage_t*&, // session message handle
                                     bool&);                    // return value: pass message downstream ?
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_OpenGL_GLUT_39 ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_OpenGL_GLUT_39 (const Test_U_CameraFilter_OpenGL_GLUT_39&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraFilter_OpenGL_GLUT_39& operator= (const Test_U_CameraFilter_OpenGL_GLUT_39&))

  // helper methods
  virtual int svc (void);

  struct Test_U_CameraFilter_OpenGL_GLUT_39_WindowData CBData_;
  bool                                                 inSession_;
  bool                                                 leftGLUTMainLoop_;
  int                                                  window_;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_module_opengl_glut_39_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_CameraFilter_OpenGL_GLUT_39);                               // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CameraFilter_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_module_opengl_glut_39_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_CameraFilter_OpenGL_GLUT_39);                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
