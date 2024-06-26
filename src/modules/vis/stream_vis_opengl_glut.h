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

#ifndef STREAM_VIS_OPENGL_GLUT_T_H
#define STREAM_VIS_OPENGL_GLUT_T_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

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

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

//GLuint Ball;
//GLfloat Zrot = 0.0F, Zstep = 180.0F;
//GLfloat Xpos = 0.0F, Ypos = 1.0F;
//GLfloat Xvel = 2.0F, Yvel = 0.0F;
//GLfloat Xmin = -4.0F, Xmax = 4.0F;
//GLfloat Ymin = -3.8F, Ymax = 4.0F;
//GLfloat G = -9.8F;

//extern GLuint libacestream_glut_make_ball (void);
extern void libacestream_glut_reshape (int, int);
extern void libacestream_glut_key (unsigned char, int, int);
extern void libacestream_glut_draw (void);
extern void libacestream_glut_idle (void);
extern void libacestream_glut_close (void);
extern void libacestream_glut_visible (int);

extern const char libacestream_default_vis_opengl_glut_module_name_string[];

struct OpenGL_GLUT_WindowData
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                                mediaType;
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType mediaType;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Message_Queue_Base*                            queue;
  Stream_IStreamControlBase*                         stream;
  GLuint                                             textureId;
};

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          typename MediaType> // session data-
class Stream_Visualization_OpenGL_GLUT_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
   Stream_Visualization_OpenGL_GLUT_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Visualization_OpenGL_GLUT_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_OpenGL_GLUT_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_OpenGL_GLUT_T (const Stream_Visualization_OpenGL_GLUT_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_OpenGL_GLUT_T& operator= (const Stream_Visualization_OpenGL_GLUT_T&))

  // helper methods
  virtual int svc (void);

  struct OpenGL_GLUT_WindowData CBData_;
  bool                          inSession_;
  bool                          leftGLUTMainLoop_;
  int                           window_;
};

// include template definition
#include "stream_vis_opengl_glut.inl"

#endif
