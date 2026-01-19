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

#include "test_u_module_opengl_glut_36.h"

#include "GL/freeglut.h"

#include "ace/Log_Msg.h"

#include "common_gl_defines.h"
#include "common_gl_tools.h"

#include "stream_macros.h"

#include "test_u_camera_filter_defines.h"

const char libacestream_default_module_opengl_glut_36_name_string[] =
  ACE_TEXT_ALWAYS_CHAR ("OpenGL_GLUT 36");

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_OpenGL_GLUT_36::Test_U_CameraFilter_OpenGL_GLUT_36 (ISTREAM_T* stream_in)
#else
Test_U_CameraFilter_OpenGL_GLUT_36::Test_U_CameraFilter_OpenGL_GLUT_36 (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , CBData_ ()
 , inSession_ (false)
 , leftGLUTMainLoop_ (false)
 , window_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_36::Test_U_CameraFilter_OpenGL_GLUT_36"));

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
Test_U_CameraFilter_OpenGL_GLUT_36::initialize (const struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration& configuration_in,
#else
Test_U_CameraFilter_OpenGL_GLUT_36::initialize (const struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration& configuration_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_36::initialize"));

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
Test_U_CameraFilter_OpenGL_GLUT_36::handleDataMessage (Test_U_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraFilter_OpenGL_GLUT_36::handleDataMessage (Test_U_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_36::handleDataMessage"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_OpenGL_GLUT_36::handleSessionMessage (Test_U_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraFilter_OpenGL_GLUT_36::handleSessionMessage (Test_U_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_36::handleSessionMessage"));

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

      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                CBData_.mediaType);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      CBData_.resolution =
        Stream_MediaFramework_DirectShow_Tools::toResolution (CBData_.mediaType);
      CBData_.depth =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (CBData_.mediaType) / 8;

      CBData_.mouseX = CBData_.resolution.cx / 2;
      CBData_.mouseY = CBData_.resolution.cy / 2;
#else
      CBData_.resolution = CBData_.mediaType.resolution;
      CBData_.depth =
        Stream_MediaFramework_Tools::ffmpegFormatToBitDepth (CBData_.mediaType.format) / 8;

      CBData_.mouseX = CBData_.resolution.width / 2;
      CBData_.mouseY = CBData_.resolution.height / 2;
#endif // ACE_WIN32 || ACE_WIN64
      CBData_.mouse_LMB_pressed = false;

      CBData_.tp1 = std::chrono::high_resolution_clock::now ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      glutInitWindowSize (CBData_.resolution.cx, CBData_.resolution.cy);
#else
      glutInitWindowSize (CBData_.resolution.width, CBData_.resolution.height);
#endif // ACE_WIN32 || ACE_WIN64

      window_ = glutCreateWindow ("OpenGL GLUT 36");
      glutSetWindow (window_);
      glutSetWindowData (&CBData_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      std::vector<GLfloat> initial_values_a (CBData_.resolution.cx * CBData_.resolution.cy * 4, 0.0f);
#else
      std::vector<GLfloat> initial_values_a (CBData_.resolution.width * CBData_.resolution.height * 4, 0.0f);
#endif // ACE_WIN32 || ACE_WIN64

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

      glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

      glActiveTexture (GL_TEXTURE0);
      glGenTextures (1, &CBData_.texture0.id_);
      ACE_ASSERT (CBData_.texture0.id_);

      glClampColor (GL_CLAMP_READ_COLOR, GL_FALSE);
      glClampColor (GL_CLAMP_VERTEX_COLOR, GL_FALSE);
      glClampColor (GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
      glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei (GL_PACK_ALIGNMENT, 1);

      glActiveTexture (GL_TEXTURE1);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glGenTextures (1, &CBData_.textureS1.id_);
      ACE_ASSERT (CBData_.textureS1.id_);
      CBData_.textureS1.bind ();
      // *IMPORTANT NOTE*: generate a floating point-format texture to contain
      //                   the result of shader1
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, CBData_.resolution.cx, CBData_.resolution.cy, 0, GL_RGBA, GL_FLOAT, initial_values_a.data ());
#else
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, CBData_.resolution.width, CBData_.resolution.height, 0, GL_RGBA, GL_FLOAT, initial_values_a.data ());
#endif // ACE_WIN32 || ACE_WIN64
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glGenerateMipmap (GL_TEXTURE_2D);

      if (!CBData_.shader1.loadFromFile (ACE_TEXT_ALWAYS_CHAR (TEST_U_VERTEX_SHADER_36_1_FILENAME),
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_FRAGMENT_SHADER_36_1_FILENAME)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to load GL shader, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      if (!CBData_.shader2.loadFromFile (ACE_TEXT_ALWAYS_CHAR (TEST_U_VERTEX_SHADER_36_2_FILENAME),
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_FRAGMENT_SHADER_36_2_FILENAME)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to load GL shader, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      CBData_.shader1.use ();
      CBData_.S1resolutionLoc =
        glGetUniformLocation (CBData_.shader1.id_, ACE_TEXT_ALWAYS_CHAR ("iResolution"));
      ACE_ASSERT (CBData_.S1resolutionLoc != -1);
      CBData_.S1timeLoc =
        glGetUniformLocation (CBData_.shader1.id_, ACE_TEXT_ALWAYS_CHAR ("iTime"));
      ACE_ASSERT (CBData_.S1timeLoc != -1);
      CBData_.S1frameLoc =
        glGetUniformLocation (CBData_.shader1.id_, ACE_TEXT_ALWAYS_CHAR ("iFrame"));
      ACE_ASSERT (CBData_.S1frameLoc != -1);
      CBData_.S1channel0Loc =
        glGetUniformLocation (CBData_.shader1.id_, ACE_TEXT_ALWAYS_CHAR ("iChannel0"));
      ACE_ASSERT (CBData_.S1channel0Loc != -1);
      CBData_.S1channel1Loc =
        glGetUniformLocation (CBData_.shader1.id_, ACE_TEXT_ALWAYS_CHAR ("iChannel1"));
      ACE_ASSERT (CBData_.S1channel1Loc != -1);

      CBData_.shader2.use ();
      CBData_.S2resolutionLoc =
        glGetUniformLocation (CBData_.shader2.id_, ACE_TEXT_ALWAYS_CHAR ("iResolution"));
      ACE_ASSERT (CBData_.S2resolutionLoc != -1);
      CBData_.S2timeLoc =
        glGetUniformLocation (CBData_.shader2.id_, ACE_TEXT_ALWAYS_CHAR ("iTime"));
      ACE_ASSERT (CBData_.S2timeLoc != -1);
      CBData_.S2channel0Loc =
        glGetUniformLocation (CBData_.shader2.id_, ACE_TEXT_ALWAYS_CHAR ("iChannel0"));
      ACE_ASSERT (CBData_.S2channel0Loc != -1);

      glGenFramebuffers (1, &CBData_.FBO1);
      ACE_ASSERT (CBData_.FBO1);
      // draw render pass 1 to framebuffer object (--> textureS1)
      glBindFramebuffer (GL_FRAMEBUFFER, CBData_.FBO1);
      glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CBData_.textureS1.id_, 0);
      ACE_ASSERT (glCheckFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
      glBindFramebuffer (GL_FRAMEBUFFER, 0);

      glGenVertexArrays (1, &CBData_.VAOId);
      ACE_ASSERT (CBData_.VAOId);
      glGenBuffers (1, &CBData_.VBOId);
      ACE_ASSERT (CBData_.VBOId);

      glBindVertexArray (CBData_.VAOId);
      static GLfloat vertices_a[] = // (Typically type: GLfloat is used)
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
      glBindBuffer (GL_ARRAY_BUFFER, CBData_.VBOId);
      glBufferData (GL_ARRAY_BUFFER, sizeof (vertices_a), vertices_a, GL_STATIC_DRAW);

      glEnableVertexAttribArray (0);
      glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)0);
      //// color attribute
      //glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)(3 * sizeof (float)));
      //glEnableVertexAttribArray (2);
      // texture coord attribute
      glEnableVertexAttribArray (1);
      glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (float), (void*)(3 * sizeof (float)));

      glBindBuffer (GL_ARRAY_BUFFER, 0);
      glBindVertexArray (0);

      glutCloseFunc (camera_filter_glut_36_close);
      glutDisplayFunc (camera_filter_glut_36_draw);
      glutReshapeFunc (camera_filter_glut_36_reshape);
      glutVisibilityFunc (camera_filter_glut_36_visible);
      glutKeyboardFunc (camera_filter_glut_36_key);
      glutMouseFunc (camera_filter_glut_36_mouse_button);
      glutPassiveMotionFunc (camera_filter_glut_36_mouse_move);

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (CBData_.mediaType);
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    default:
      break;
  } // end SWITCH
}

int
Test_U_CameraFilter_OpenGL_GLUT_36::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_OpenGL_GLUT_36::svc"));

  int result = -1;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  bool stop_processing = false;

  //glutInitContextProfile (GLUT_CORE_PROFILE);
  //glutInitContextFlags (GLUT_DEBUG);

  char* myargv[1];
  int myargc = 1;
  myargv[0] = ACE_OS::strdup (ACE_TEXT_ALWAYS_CHAR ("Test_U_CameraFilter_OpenGL_GLUT_36"));
  ACE_ASSERT (myargv[0]);
  glutInit (&myargc, myargv);
  ACE_OS::free (myargv[0]);

  glutSetOption (GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

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
      ACE_UNUSED_ARG (error);
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
camera_filter_glut_36_reshape (int width_in, int height_in)
{
  glViewport (0, 0,
              static_cast<GLsizei> (width_in), static_cast<GLsizei> (height_in));
  // *TODO*: find out why this reports GL_INVALID_OPERATION
  COMMON_GL_PRINT_ERROR;

  //glMatrixMode (GL_PROJECTION);
  //COMMON_GL_ASSERT;

  //glLoadIdentity ();
  //COMMON_GL_ASSERT;

  //gluPerspective (45.0,
  //                static_cast<GLdouble> (width_in) / static_cast<GLdouble> (height_in),
  //                300.0, -1000.0);
  //COMMON_GL_ASSERT;

  //glMatrixMode (GL_MODELVIEW);
  //COMMON_GL_ASSERT;
}

void
camera_filter_glut_36_key (unsigned char key_in, int x_in, int y_in)
{
  struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData* cb_data_p =
    static_cast<struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (key_in)
  {
    case 27: /* Escape */
    {
      glutLeaveMainLoop ();

      // clean up GL resources
      cb_data_p->shader1.reset ();
      cb_data_p->shader2.reset ();

      cb_data_p->texture0.reset ();
      cb_data_p->textureS1.reset ();

      glDeleteFramebuffers (1, &cb_data_p->FBO1); cb_data_p->FBO1 = 0;

      glDeleteBuffers (1, &cb_data_p->VBOId); cb_data_p->VBOId = 0;
      glDeleteVertexArrays (1, &cb_data_p->VAOId); cb_data_p->VAOId = 0;

      ACE_ASSERT (cb_data_p->stream);
      cb_data_p->stream->stop (false,  // wait for completion ?
                               false,  // recurse upstream ?
                               false); // high priority ?

      break;
    }
  } // end SWITCH
}

void
camera_filter_glut_36_mouse_button (int button, int state, int x, int y)
{
  struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData* cb_data_p =
    static_cast<struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
    {
      cb_data_p->mouse_LMB_pressed = (state == GLUT_DOWN);
      break;
    }
    default:
      break;
  } // end IF
}

void
camera_filter_glut_36_mouse_move (int x, int y)
{
  struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData* cb_data_p =
    static_cast<struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  cb_data_p->mouseX = x;
  cb_data_p->mouseY = y;
}

void
camera_filter_glut_36_draw (void)
{
  static int frame_count_i = 1;

  struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData* cb_data_p =
    static_cast<struct Test_U_CameraFilter_OpenGL_GLUT_36_WindowData*> (glutGetWindowData ());
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

  if (unlikely (frame_count_i == 1))
  {
    glActiveTexture (GL_TEXTURE0);
    cb_data_p->texture0.bind ();
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    cb_data_p->texture0.unbind ();
  } // end IF
  if (unlikely (!cb_data_p->texture0.load (data_p,
                                           cb_data_p->resolution,
                                           cb_data_p->depth,
                                           frame_count_i > 1)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to load/update texture, returning\n")));
    return;
  } // end IF
  message_block_p->release ();
  cb_data_p->texture0.bind ();
  if (unlikely (frame_count_i == 1))
  {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap (GL_TEXTURE_2D);
  } // end IF

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // compute elapsed time
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::chrono::steady_clock::time_point tp2 =
    std::chrono::high_resolution_clock::now ();
#elif defined (ACE_LINUX)
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tp2 =
    std::chrono::high_resolution_clock::now ();
#else
#error missing implementation, aborting
#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX
  std::chrono::duration<float> elapsed_time = tp2 - cb_data_p->tp1;

  glBindVertexArray (cb_data_p->VAOId);

  // render pass 1
  cb_data_p->shader1.use ();

  // update uniforms
  glProgramUniform2f (cb_data_p->shader1.id_, cb_data_p->S1resolutionLoc,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      static_cast<GLfloat> (cb_data_p->resolution.cx),
                      static_cast<GLfloat> (cb_data_p->resolution.cy));
#else
                      static_cast<GLfloat> (cb_data_p->resolution.width),
                      static_cast<GLfloat> (cb_data_p->resolution.height));
#endif // ACE_WIN32 || ACE_WIN64

  glProgramUniform1f (cb_data_p->shader1.id_, cb_data_p->S1timeLoc,
                      static_cast<GLfloat> (elapsed_time.count ()));

  glProgramUniform1i (cb_data_p->shader1.id_, cb_data_p->S1frameLoc,
                      static_cast<GLint> (frame_count_i - 1));

  glProgramUniform1i (cb_data_p->shader1.id_, cb_data_p->S1channel0Loc,
                      1);
  glProgramUniform1i (cb_data_p->shader1.id_, cb_data_p->S1channel1Loc,
                      0);

  // draw render pass 1 to framebuffer object (--> textureS1)
  glBindFramebuffer (GL_FRAMEBUFFER, cb_data_p->FBO1);
  glDrawArrays (GL_TRIANGLES, 0, 6); // 2 triangles --> 6 vertices
  //glDrawArrays (GL_QUADS, 0, 4); // 1 quad --> 4 vertices
  glBindFramebuffer (GL_FRAMEBUFFER, 0);

  // render pass 2
  cb_data_p->shader2.use ();

  // update uniforms
  glProgramUniform2f (cb_data_p->shader2.id_, cb_data_p->S2resolutionLoc,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      static_cast<GLfloat> (cb_data_p->resolution.cx),
                      static_cast<GLfloat> (cb_data_p->resolution.cy));
#else
                      static_cast<GLfloat> (cb_data_p->resolution.width),
                      static_cast<GLfloat> (cb_data_p->resolution.height));
#endif // ACE_WIN32 || ACE_WIN64

  glProgramUniform1f (cb_data_p->shader2.id_, cb_data_p->S2timeLoc,
                      static_cast<GLfloat> (elapsed_time.count ()));

  glProgramUniform1i (cb_data_p->shader2.id_, cb_data_p->S2channel0Loc,
                      1);

  // draw render pass 2 to screen
  glDrawArrays (GL_TRIANGLES, 0, 6); // 2 triangles --> 6 vertices (see also: above)

  glBindVertexArray (0);

  glutSwapBuffers ();

  ++frame_count_i;
}

void
camera_filter_glut_36_idle (void)
{
  glutPostRedisplay ();
}

void
camera_filter_glut_36_close ()
{ // pressing the 'x' is the same as hitting 'Escape'
  camera_filter_glut_36_key (27, 0, 0);
}

void
camera_filter_glut_36_visible (int vis)
{
  if (vis == GLUT_VISIBLE)
    glutIdleFunc (camera_filter_glut_36_idle);
  else
    glutIdleFunc (NULL);
}
