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

//#include "GL/glut.h"
#include "GL/freeglut.h"

#include "ace/Log_Msg.h"

#include "common_gl_defines.h"
#include "common_gl_tools.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Visualization_OpenGL_GLUT_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   MediaType>::Stream_Visualization_OpenGL_GLUT_T (ISTREAM_T* stream_in)
#else
                                   MediaType>::Stream_Visualization_OpenGL_GLUT_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , CBData_ ()
 , inSession_ (false)
 , window_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenGL_GLUT_T::Stream_Visualization_OpenGL_GLUT_T"));

  ACE_OS::memset (&CBData_.mediaType, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));
  CBData_.queue = inherited::msg_queue_;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Visualization_OpenGL_GLUT_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenGL_GLUT_T::initialize"));

  if (inherited::isInitialized_)
  {
    ACE_OS::memset (&CBData_.mediaType, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Visualization_OpenGL_GLUT_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenGL_GLUT_T::handleDataMessage"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Visualization_OpenGL_GLUT_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenGL_GLUT_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());

      inherited2::getMediaType (session_data_r.formats.back (),
                                CBData_.mediaType);

      window_ = glutCreateWindow ("Bounce");
      glutSetWindow (window_);
      glutSetWindowData (&CBData_);

      glClearColor (0.0F, 0.0F, 0.0F, 1.0F);              // Black Background
      COMMON_GL_ASSERT;
      //glClearDepth (1.0);                                 // Depth Buffer Setup
      //COMMON_GL_ASSERT;
      /* speedups */
      //  glDisable (GL_CULL_FACE);
      //  glEnable (GL_DITHER);
      //  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
      //  glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
      COMMON_GL_ASSERT;
      //glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      //COMMON_GL_ASSERT;
      //glEnable (GL_COLOR_MATERIAL);
      //COMMON_GL_ASSERT;
      //glEnable (GL_LIGHTING);
      //COMMON_GL_ASSERT;
      glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
      COMMON_GL_ASSERT;
      glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
      COMMON_GL_ASSERT;
      glDepthMask (GL_TRUE);
      COMMON_GL_ASSERT;
      glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
      COMMON_GL_ASSERT;
      glShadeModel (GL_SMOOTH);                           // Enable Smooth Shading
      COMMON_GL_ASSERT;
      glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
      COMMON_GL_ASSERT;
      glEnable (GL_BLEND);                                // Enable Semi-Transparency
      COMMON_GL_ASSERT;
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      COMMON_GL_ASSERT;
      glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing
      COMMON_GL_ASSERT;

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glutDisplayFunc (libacestream_glut_draw);
      glutReshapeFunc (libacestream_glut_reshape);
      glutVisibilityFunc (libacestream_glut_visible);
      glutKeyboardFunc (libacestream_glut_key);

      //Ball = libacestream_glut_make_ball ();

      ACE_ASSERT (!inSession_);
      inSession_ = true;

      break;

//error:
//      this->notify (STREAM_SESSION_MESSAGE_ABORT)

//      break;
    }
//    case STREAM_SESSION_MESSAGE_RESIZE:
//    {
//      break;
//    }
    case STREAM_SESSION_MESSAGE_END:
    { ACE_ASSERT (inSession_);
      inSession_ = false;

      if (window_)
        glutDestroyWindow (window_);

      //glutLeaveMainLoop ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
int
Stream_Visualization_OpenGL_GLUT_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenGL_GLUT_T::svc"));

  int result = -1;
  int result_2 = -1;
//  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
//  DataMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;

  //glutInitContextProfile (GLUT_CORE_PROFILE);
  //glutInitContextFlags (GLUT_DEBUG);
  glutInitWindowSize (600, 450);

  char* myargv[1];
  int myargc = 1;
  myargv[0] = ACE_OS::strdup ("Myappname");
  glutInit (&myargc, myargv);

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
        message_block_p->release ();
        message_block_p = NULL;

        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will not
        //         have been set at this stage

        // signal the controller ?
        if (!finished)
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("session aborted\n")));

          finished = true;
          inherited::finished ();

          continue;
        } // end IF

        break; // aborted
      } // end IF

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
    } // end IF
    else if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));

        // signal the controller ?
        if (!finished)
        {
          finished = true;
          inherited::finished ();
        } // end IF

        break;
      } // end IF
    } // end IF

    if (inSession_)
      glutMainLoop ();
    // *TODO*
  } while (true);

//done:
  return result_2;
}
