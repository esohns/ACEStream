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
#include "stdafx.h"

#include "test_u_module_opengl_glut_3.h"

#include "GL/freeglut.h"

#include "ace/Log_Msg.h"

#include "common_gl_defines.h"
#include "common_gl_tools.h"

#include "stream_macros.h"

#include "test_u_camera_filter_defines.h"

const char libacestream_default_module_opengl_glut_3_name_string[] =
  ACE_TEXT_ALWAYS_CHAR ("OpenGL_GLUT 3");

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_OpenGL_GLUT_3::Test_U_CameraFilter_OpenGL_GLUT_3 (ISTREAM_T* stream_in)
#else
Test_U_CameraFilter_OpenGL_GLUT_3::Test_U_CameraFilter_OpenGL_GLUT_3 (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , CBData_ ()
 , inSession_ (false)
 , leftGLUTMainLoop_ (false)
 , window_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_3::Test_U_CameraFilter_OpenGL_GLUT_3"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&CBData_.mediaType, 0, sizeof (struct _AMMediaType));
#else
#if defined (FFMPEG_SUPPORT)
  ACE_OS::memset (&CBData_.mediaType, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  CBData_.queue = inherited::msg_queue_;
  CBData_.stream = dynamic_cast<Stream_IStreamControlBase*> (stream_in);
  ACE_ASSERT (CBData_.stream);
}

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_OpenGL_GLUT_3::initialize (const struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration& configuration_in,
#else
Test_U_CameraFilter_OpenGL_GLUT_3::initialize (const struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration& configuration_in,
#endif // ACE_WIN32 || ACE_WIN64
                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_3::initialize"));

  if (inherited::isInitialized_)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Stream_MediaFramework_DirectShow_Tools::free (CBData_.mediaType);
#else
#if defined (FFMPEG_SUPPORT)
    ACE_OS::memset (&CBData_.mediaType, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_OpenGL_GLUT_3::handleDataMessage (Test_U_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraFilter_OpenGL_GLUT_3::handleDataMessage (Test_U_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_3::handleDataMessage"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_OpenGL_GLUT_3::handleSessionMessage (Test_U_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraFilter_OpenGL_GLUT_3::handleSessionMessage (Test_U_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_3::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      const typename Test_U_DirectShow_SessionMessage_t::DATA_T::DATA_T& session_data_r =
#else
      const typename Test_U_SessionMessage_t::DATA_T::DATA_T& session_data_r =
#endif // ACE_WIN32 || ACE_WIN64
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());

      GLuint vertex_shader_id = -1, fragment_shader_id = -1;
      GLint success = 0;
      //GLuint VBO; //, EBO;

      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                CBData_.mediaType);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      CBData_.resolution =
        Stream_MediaFramework_DirectShow_Tools::toResolution (CBData_.mediaType);

      CBData_.mouseX = CBData_.resolution.cx / 2;
      CBData_.mouseY = CBData_.resolution.cy / 2;
#else
      CBData_.resolution = CBData_.mediaType.resolution;

      CBData_.mouseX = CBData_.resolution.width / 2;
      CBData_.mouseY = CBData_.resolution.height / 2;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      glutInitWindowSize (CBData_.resolution.cx, CBData_.resolution.cy);
#else
      glutInitWindowSize (CBData_.resolution.width, CBData_.resolution.height);
#endif // ACE_WIN32 || ACE_WIN64

      window_ = glutCreateWindow ("OpenGL GLUT 3");
      glutSetWindow (window_);
      glutSetWindowData (&CBData_);

      // initialize GLEW
      GLenum err = glewInit ();
      if (GLEW_OK != err)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to glewInit(): \"%s\", aborting\n"),
                    ACE_TEXT (glewGetErrorString (err))));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("using GLEW version: %s\n"),
                  ACE_TEXT (glewGetString (GLEW_VERSION))));

      //glClearColor (0.0F, 0.0F, 0.0F, 1.0F); // Black Background
      glClearColor (0.0F, 0.0F, 0.0F, 0.0F); // Black Background
      COMMON_GL_ASSERT;
      //glClearDepth (1.0);                                 // Depth Buffer Setup
      //COMMON_GL_ASSERT;
      /* speedups */
      //glEnable (GL_CULL_FACE);
      //glFrontFace (GL_CCW);
      //glCullFace (GL_BACK);
      //  glEnable (GL_DITHER);
      //  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
      //  glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
      //COMMON_GL_ASSERT;
      //glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      //COMMON_GL_ASSERT;
      //glEnable (GL_COLOR_MATERIAL);
      //COMMON_GL_ASSERT;
      //glEnable (GL_LIGHTING);
      //COMMON_GL_ASSERT;
      //glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
      //COMMON_GL_ASSERT;
      //glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
      //COMMON_GL_ASSERT;
      //glDepthMask (GL_TRUE);
      //COMMON_GL_ASSERT;
      glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
      COMMON_GL_ASSERT;
      //glShadeModel (GL_SMOOTH);                           // Enable Smooth Shading
      //COMMON_GL_ASSERT;
      //glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
      //COMMON_GL_ASSERT;
      glEnable (GL_BLEND);                                // Enable Semi-Transparency
      //COMMON_GL_ASSERT;
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      //glBlendFunc (GL_ONE_MINUS_DST_ALPHA, GL_ONE);
      COMMON_GL_ASSERT;
      glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing
      COMMON_GL_ASSERT;

      //glDisable (GL_CULL_FACE);
      //glEnable (GL_CULL_FACE);
      //glCullFace (GL_BACK);
      glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      COMMON_GL_ASSERT;
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      COMMON_GL_ASSERT;
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      COMMON_GL_ASSERT;
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      COMMON_GL_ASSERT;

      glGenTextures (1, &CBData_.textureId);
      COMMON_GL_ASSERT;

      // CBData_.scaleFactor = TEST_U_CANVAS_SCALE_FACTOR;
      // CBData_.columns = CBData_.resolution.cx / CBData_.scaleFactor;
      // CBData_.rows = CBData_.resolution.cy / CBData_.scaleFactor;

      if (!Common_GL_Tools::loadAndCompileShaderFile (ACE_TEXT_ALWAYS_CHAR (TEST_U_VERTEX_SHADER_3_FILENAME),
                                                      GL_VERTEX_SHADER,
                                                      vertex_shader_id))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to load GL shader, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      if (!Common_GL_Tools::loadAndCompileShaderFile (ACE_TEXT_ALWAYS_CHAR (TEST_U_FRAGMENT_SHADER_3_FILENAME),
                                                      GL_FRAGMENT_SHADER,
                                                      fragment_shader_id))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to load GL shader, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      CBData_.programId = glCreateProgram ();
      glAttachShader (CBData_.programId, vertex_shader_id);
      glAttachShader (CBData_.programId, fragment_shader_id);
      glLinkProgram (CBData_.programId);
      glGetProgramiv (CBData_.programId, GL_LINK_STATUS, &success);
      if (success == GL_FALSE)
      {
        GLchar info_log_a[BUFSIZ * 4];
        GLsizei buf_size_i = 0;
        glGetProgramInfoLog (CBData_.programId,
                             sizeof (GLchar) * BUFSIZ * 4,
                             &buf_size_i,
                             info_log_a);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to link GL program: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (info_log_a)));

        glDetachShader (CBData_.programId, vertex_shader_id);
        glDetachShader (CBData_.programId, fragment_shader_id);
        glDeleteShader (vertex_shader_id);
        glDeleteShader (fragment_shader_id);
        glDeleteProgram (CBData_.programId); CBData_.programId = -1;
        goto error;
      } // end IF
      glDetachShader (CBData_.programId, vertex_shader_id);
      glDetachShader (CBData_.programId, fragment_shader_id);
      glDeleteShader (vertex_shader_id);
      glDeleteShader (fragment_shader_id);

      glValidateProgram (CBData_.programId);
      glGetProgramiv (CBData_.programId, GL_VALIDATE_STATUS, &success);
      if (success == GL_FALSE)
      {
        GLchar info_log_a[BUFSIZ * 4];
        GLsizei buf_size_i = 0;
        glGetProgramInfoLog (CBData_.programId,
                             sizeof (GLchar) * BUFSIZ * 4,
                             &buf_size_i,
                             info_log_a);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to validate GL program: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (info_log_a)));
        glDeleteProgram (CBData_.programId); CBData_.programId = -1;
        goto error;
      } // end IF

      //glEnableVertexAttribArray (0);
      //glEnableVertexAttribArray (1);
      //glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * 4, 0);
      //glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 2 * 4, reinterpret_cast<void*> (12));
      //glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof (float) * 2, (void*)(sizeof(float)*2));
      //glEnableVertexAttribArray (1);

      glUseProgram (CBData_.programId);

      CBData_.resolutionLoc =
        glGetUniformLocation (CBData_.programId, ACE_TEXT_ALWAYS_CHAR ("canvasSize"));
      //ACE_ASSERT (CBData_.resolutionLoc != -1);
      CBData_.mouseLoc =
        glGetUniformLocation (CBData_.programId, ACE_TEXT_ALWAYS_CHAR ("mouse"));
      //ACE_ASSERT (CBData_.mouseLoc != -1);
      CBData_.timeLoc =
        glGetUniformLocation (CBData_.programId, ACE_TEXT_ALWAYS_CHAR ("time"));
      ACE_ASSERT (CBData_.timeLoc != -1);
      CBData_.textureLoc =
        glGetUniformLocation (CBData_.programId, ACE_TEXT_ALWAYS_CHAR ("tex0"));
      ACE_ASSERT (CBData_.textureLoc != -1);

      //GLfloat vertices[] = {
      //  // positions        // texture coords
      //  -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // left top
      //  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // left bottom
      //   1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // right top

      //   1.0f,  1.0f, 0.0f, 1.0f, 1.0f  // right top
      //  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // left bottom
      //   1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // right bottom

      //   0.0f,   0.75f, 0.0f, 0.5f, 1.0f,  // middle top ... 0
      //  -0.75f, -0.75f, 0.0f, 0.0f, 0.0f,  // left bottom ... 1
      //   0.75f, -0.75f, 0.0f, 1.0f, 0.0f  // right bottom .. 2		 
      //};
      //unsigned int indices[] = {
      //  0, 1, 3, // first triangle
      //  1, 2, 3  // second triangle
      //};
      glGenVertexArrays (1, &CBData_.VAOId);
      glGenBuffers (1, &CBData_.VBOId);
      //glGenElementArryBuffers (1, &EBO);

      //glBindVertexArray (CBData_.VAOId);
      //glBindBuffer (GL_ARRAY_BUFFER, CBData_.VBOId);
      //glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

      //glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, EBO);
      //glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (indices), indices, GL_STATIC_DRAW);

      // position attribute
      //glEnableVertexAttribArray (0);
      //glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float),
      //                       (void*)0);
      ////// color attribute
      ////glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)(3 * sizeof (float)));
      ////glEnableVertexAttribArray (2);
      //// texture coord attribute
      //glEnableVertexAttribArray (1);
      //glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)(3 * sizeof (float)));

      // START TIMING
      //CBData_.tp1 = std::chrono::high_resolution_clock::now ();

      glutCloseFunc (camera_filter_glut_3_close);
      glutDisplayFunc (camera_filter_glut_3_draw);
      glutReshapeFunc (camera_filter_glut_3_reshape);
      glutVisibilityFunc (camera_filter_glut_3_visible);
      glutKeyboardFunc (camera_filter_glut_3_key);

      ACE_ASSERT (!inSession_);
      inSession_ = true;

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
//    case STREAM_SESSION_MESSAGE_RESIZE:
//    {
//      break;
//    }
    case STREAM_SESSION_MESSAGE_END:
    { ACE_ASSERT (inSession_);
      inSession_ = false;

      if (likely (window_))
      {
        glutDestroyWindow (window_); window_ = 0;
      } // end IF

      if (!leftGLUTMainLoop_)
        glutLeaveMainLoop ();

      inherited::stop (false,  // wait for completion ?
                       false); // high priority ?

      break;
    }
    default:
      break;
  } // end SWITCH
}

int
Test_U_CameraFilter_OpenGL_GLUT_3::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_3::svc"));

  int result = -1;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  bool stop_processing = false;

  //glutInitContextProfile (GLUT_CORE_PROFILE);
  //glutInitContextFlags (GLUT_DEBUG);

  char* myargv[1];
  int myargc = 1;
  myargv[0] = ACE_OS::strdup ("Myappname");
  glutInit (&myargc, myargv);

  glutSetOption (GLUT_ACTION_ON_WINDOW_CLOSE,
                 GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  //  glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE);
  glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

  // step1: start processing data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result == 0)
    {
      ACE_ASSERT (message_block_p);
      ACE_Message_Block::ACE_Message_Type message_type =
        message_block_p->msg_type ();
      if (message_type == ACE_Message_Block::MB_STOP)
      {
        // clean up
        message_block_p->release (); message_block_p = NULL;

        result_2 = 0; // OK

        break; // aborted
      } // end IF

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
    } // end IF
    else if (result == -1)
    {
      int error = ACE_OS::last_error ();
      //if (error != EWOULDBLOCK) // Win32: 10035
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
      //  break;
      //} // end IF
    } // end IF

    if (inSession_ && !leftGLUTMainLoop_)
    {
      glutMainLoop ();
      leftGLUTMainLoop_ = true;
      window_ = 0;
    } // end IF
  } while (true);

//done:
  return result_2;
}

//////////////////////////////////////////

void
camera_filter_glut_3_reshape (int width_in, int height_in)
{
  glViewport (0, 0,
              static_cast<GLsizei> (width_in), static_cast<GLsizei> (height_in));
  // *TODO*: find out why this reports GL_INVALID_OPERATION
  COMMON_GL_PRINT_ERROR;

  glMatrixMode (GL_PROJECTION);
  COMMON_GL_ASSERT;

  glLoadIdentity ();
  COMMON_GL_ASSERT;

  gluPerspective (45.0,
                  static_cast<GLdouble> (width_in) / static_cast<GLdouble> (height_in),
                  300.0, -1000.0);
  COMMON_GL_ASSERT;

  glMatrixMode (GL_MODELVIEW);
  COMMON_GL_ASSERT;
}

void
camera_filter_glut_3_key (unsigned char key_in, int x_in, int y_in)
{
  struct Test_U_CameraFilter_OpenGL_GLUT_3_WindowData* cb_data_p =
    static_cast<struct Test_U_CameraFilter_OpenGL_GLUT_3_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (key_in)
  {
    case 27: /* Escape */
    {
      glutLeaveMainLoop ();

      ACE_ASSERT (cb_data_p->stream);
      cb_data_p->stream->stop (false,  // wait for completion ?
                               false,  // recurse upstream ?
                               false); // high priority ?

      break;
    }
  } // end SWITCH
}

void
camera_filter_glut_3_draw (void)
{
  static int frame_count_i = 1;

  struct Test_U_CameraFilter_OpenGL_GLUT_3_WindowData* cb_data_p =
    static_cast<struct Test_U_CameraFilter_OpenGL_GLUT_3_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->queue);
  ACE_Message_Block* message_block_p = NULL;

  message_block_p = NULL;
  cb_data_p->queue->dequeue_head (message_block_p,
                                  NULL);
  ACE_ASSERT (message_block_p);
  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ());
  if (unlikely (!data_p))
  { // most likely a session message (*TODO*: should not happen)
    cb_data_p->queue->enqueue_tail (message_block_p, NULL);
    return;
  } // end IF

  Common_GL_Tools::loadTexture (data_p,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                cb_data_p->resolution.cx,
                                cb_data_p->resolution.cy,
#else
                                cb_data_p->mediaType.resolution.width,
                                cb_data_p->mediaType.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                                cb_data_p->textureId,
                                frame_count_i == 1);
  message_block_p->release ();

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //COMMON_GL_ASSERT;

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity (); // Reset the transformation matrix.
  //COMMON_GL_ASSERT;

  // set the camera
  gluLookAt (0.0, 0.0, 700.0,
             0.0, 0.0, 0.0,
             0.0, 1.0, 0.0);

  glBindTexture (GL_TEXTURE_2D, cb_data_p->textureId);
  glBindVertexArray (cb_data_p->VAOId);
  static GLfloat vertices[] = // (Typically type: GLfloat is used)
  {
    // Note:  "Being able to store the vertex attributes for that vertex only once is very economical, as a vertex's attribute...
    // data is generally around 32 bytes, while indices are usually 2-4 bytes in size." (Hence, the next tutorial uses: glDrawElements())
    
    // Rectangle vertices (Texture Coordinates: 0,0 = bottom left... 1,1 = top right)
    //-------------------------
    -1.0f ,  1.0f, 0.0f,  0.0f, 1.0f, // left top ......... 0 // Draw this rectangle's six vertices 1st.
    -1.0f , -1.0f, 0.0f,  0.0f, 0.0f, // left bottom ... 1 // Use NDC coordinates [-1, +1] to completely fill the display window.
     1.0f ,  1.0f, 0.0f,  1.0f, 1.0f, // right top ....... 2
 
     1.0f ,  1.0f, 0.0f,  1.0f, 1.0f, // right top ........ 3
    -1.0f , -1.0f, 0.0f,  0.0f, 0.0f, // left bottom ..... 4
     1.0f , -1.0f, 0.0f,  1.0f, 0.0f, // right bottom ... 5
 
     // Triangle vertices (Drawing this 2nd allows some of the rectangle's fragment colour to be added to this triangle via transparency)
     // ----------------------
    // 0.0f,   0.75f, 0.0f,  0.5f, 1.0f, // middle top ... 0
    //-0.75f, -0.75f, 0.0f,  0.0f, 0.0f, // left bottom ... 1
    // 0.75f, -0.75f, 0.0f,  1.0f, 0.0f  // right bottom .. 2
  };
  glBindBuffer (GL_ARRAY_BUFFER, cb_data_p->VBOId);
  glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)0);
  //// color attribute
  //glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)(3 * sizeof (float)));
  //glEnableVertexAttribArray (2);
  // texture coord attribute
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)(3 * sizeof (float)));

  //glBegin (GL_LINES);
  //glColor3f (1.0f, 0.0f, 0.0f); glVertex3i (0, 0, 0); glVertex3i (100, 0, 0);
  //glColor3f (0.0f, 1.0f, 0.0f); glVertex3i (0, 0, 0); glVertex3i (0, 100, 0);
  //glColor3f (0.0f, 0.0f, 1.0f); glVertex3i (0, 0, 0); glVertex3i (0, 0, 100);
  //glEnd ();

  // update uniforms
  glProgramUniform2f (cb_data_p->programId, cb_data_p->resolutionLoc,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      static_cast<GLfloat> (cb_data_p->resolution.cx),
                      static_cast<GLfloat> (cb_data_p->resolution.cy));
#else
                      static_cast<GLfloat> (cb_data_p->resolution.width),
                      static_cast<GLfloat> (cb_data_p->resolution.height));
#endif // ACE_WIN32 || ACE_WIN64
  glProgramUniform2f (cb_data_p->programId, cb_data_p->mouseLoc,
                      static_cast<GLfloat> (cb_data_p->mouseX),
                      static_cast<GLfloat> (cb_data_p->mouseY));
//  // compute elapsed time
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  std::chrono::steady_clock::time_point tp2 =
//    std::chrono::high_resolution_clock::now ();
//#elif defined (ACE_LINUX)
//  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tp2 =
//    std::chrono::high_resolution_clock::now ();
//#else
//#error missing implementation, aborting
//#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX
//  std::chrono::duration<float> elapsed_time = tp2 - cb_data_p->tp1;
//  glProgramUniform1f (cb_data_p->programId, cb_data_p->timeLoc,
//                      elapsed_time.count ());
  glProgramUniform1f (cb_data_p->programId, cb_data_p->timeLoc,
                      frame_count_i / 60.0f);
  glProgramUniform1i (cb_data_p->programId, cb_data_p->textureLoc,
                      0);

  //glColor3f (1.0f, 1.0f, 1.0f);

  glDrawArrays (GL_TRIANGLES, 0, sizeof (vertices) / sizeof (vertices[0]));

  //glTranslatef (static_cast<GLfloat> (-cb_data_p->resolution.cx / 2.0f),
  //              static_cast<GLfloat> ( cb_data_p->resolution.cy / 2.0f),
  //              0.0f);

  //for (int y = 0; y < cb_data_p->rows - 1; ++y)
  //{
  //  glBegin (GL_TRIANGLE_STRIP);
  //  for (int x = 0; x < cb_data_p->columns; ++x)
  //  {
  //    glVertex2f (static_cast<float> (x * cb_data_p->scaleFactor), -static_cast<float> ( y      * cb_data_p->scaleFactor));
  //    glVertex2f (static_cast<float> (x * cb_data_p->scaleFactor), -static_cast<float> ((y + 1) * cb_data_p->scaleFactor));
  //  } // end FOR
  //  glEnd ();
  //} // end FOR

  glBindVertexArray (0);
  glBindTexture (GL_TEXTURE_2D, 0);

  glutSwapBuffers ();

  ++frame_count_i;
}

void
camera_filter_glut_3_idle (void)
{
  glutPostRedisplay ();
}

void
camera_filter_glut_3_close ()
{ // pressing the 'x' is the same as hitting 'Escape'
  camera_filter_glut_3_key (27, 0, 0);
}

void
camera_filter_glut_3_visible (int vis)
{
  if (vis == GLUT_VISIBLE)
    glutIdleFunc (camera_filter_glut_3_idle);
  else
    glutIdleFunc (NULL);
}
