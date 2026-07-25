#include "stdafx.h"

#include "test_u_glut_callbacks.h"

#include "GL/glew.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gl/GL.h"
#else
#include "GL/gl.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "GL/freeglut.h"

#include "glm/gtc/matrix_transform.hpp"

#include "ace/Assert.h"
#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_image_tools.h"

#include "common_gl_tools.h"

#include "test_u_mic_visualize_common.h"
#include "test_u_mic_visualize_defines.h"

//#if defined (ACE_LINUX)
//#else
//void* timer_cb_data_p = NULL;
//#endif // ACE_LINUX

void
acestream_projectm_preset_switch_cb (bool isHardCut_in,
                                     unsigned int index_in,
                                     void* userData_in)
{
  struct Stream_Visualization_ProjectM_Configuration* cb_data_p =
    static_cast<struct Stream_Visualization_ProjectM_Configuration*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->playlist);

  cb_data_p->current = index_in;

  char* preset_name_p = projectm_playlist_item (cb_data_p->playlist, index_in);
  if (!preset_name_p) return;
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("preset switch: %s\n"),
              ACE_TEXT (preset_name_p)));

  // clean up
  projectm_playlist_free_string (preset_name_p);
}

//////////////////////////////////////////

void
test_u_projectm_glut_close ()
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  // glutDestroyWindow (cb_data_p->windowId);
  cb_data_p->windowId = -1;

  glutLeaveMainLoop ();
}

void
test_u_projectm_glut_reshape (int width_in,
                              int height_in)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->projectMConfiguration);
  ACE_ASSERT (cb_data_p->projectMConfiguration->handle);

  glViewport (0, 0, width_in, height_in);

  glMatrixMode (GL_PROJECTION);

  glLoadIdentity ();

  ACE_ASSERT (height_in);
  gluPerspective (TEST_U_OPENGL_PERSPECTIVE_FOVY_D,
                  width_in / static_cast<GLdouble> (height_in),
                  TEST_U_OPENGL_PERSPECTIVE_ZNEAR_D, TEST_U_OPENGL_PERSPECTIVE_ZFAR_D);
  //glOrtho (static_cast<GLdouble> (-width_in / 2.0), static_cast<GLdouble> (width_in / 2.0),
  //         static_cast<GLdouble> (height_in / 2.0), static_cast<GLdouble> (-height_in / 2.0), 150.0, -150.0);

  glMatrixMode (GL_MODELVIEW);

  projectm_set_window_size (cb_data_p->projectMConfiguration->handle,
                            width_in, height_in);
}

void
test_u_projectm_glut_key (unsigned char key_in,
                          int x,
                          int y)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->projectMConfiguration);
  ACE_ASSERT (cb_data_p->projectMConfiguration->playlist);

  switch (key_in)
  {
    case 27: /* Escape */
      glutLeaveMainLoop ();
      break;
    case ' ':
      cb_data_p->projectMConfiguration->current =
        projectm_playlist_play_next (cb_data_p->projectMConfiguration->playlist, false);
      break;
  } // end SWITCH
}

void
test_u_projectm_glut_key_special (int key_in,
                                  int x,
                                  int y)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->projectMConfiguration);
  ACE_ASSERT (cb_data_p->projectMConfiguration->handle);
  ACE_ASSERT (cb_data_p->projectMConfiguration->playlist);

  int modifiers_i = glutGetModifiers ();
  cb_data_p->shiftPressed = (modifiers_i & GLUT_ACTIVE_SHIFT) != 0;

  switch (key_in)
  {
    case GLUT_KEY_F1:
    {
      char* preset_name_p =
        projectm_playlist_item (cb_data_p->projectMConfiguration->playlist,
                                cb_data_p->projectMConfiguration->current);
      if (!preset_name_p) break;
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("current preset: %s\n"),
                  ACE_TEXT (preset_name_p)));

      // clean up
      projectm_playlist_free_string (preset_name_p);
      break;
    }
    case GLUT_KEY_F2:
    {
      projectm_set_preset_locked (cb_data_p->projectMConfiguration->handle, true);
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("current preset LOCKED\n")));
      break;
    }
    case GLUT_KEY_F3:
    {
      projectm_set_preset_locked (cb_data_p->projectMConfiguration->handle, false);
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("current preset UNLOCKED\n")));
      break;
    }
    case GLUT_KEY_F12:
    {
      std::string path = Common_File_Tools::getTempDirectory ();
      path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
      path += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_SCREENSHOT_FILE);
      Common_GL_Tools::screenShot (path);
      break;
    }
    case GLUT_KEY_LEFT:
      cb_data_p->projectMConfiguration->current =
        projectm_playlist_play_previous (cb_data_p->projectMConfiguration->playlist, false);
      break;
    case GLUT_KEY_RIGHT:
      cb_data_p->projectMConfiguration->current =
        projectm_playlist_play_next (cb_data_p->projectMConfiguration->playlist, false);
      break;
    case GLUT_KEY_UP:
      break;
    case GLUT_KEY_DOWN:
      break;
    case GLUT_KEY_PAGE_UP:
      break;
    case GLUT_KEY_PAGE_DOWN:
      break;
    case GLUT_KEY_HOME:
      break;
  } // end SWITCH

  cb_data_p->shiftPressed = false;
}

void
test_u_projectm_glut_menu (int entry_in)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (entry_in)
  {
    case 0:
      cb_data_p->wireframe = !cb_data_p->wireframe;
      break;
    default:
      break;
  } // end SWITCH
}

void
test_u_projectm_glut_mouse_button (int button,
                                   int state,
                                   int x,
                                   int y)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
    {
      cb_data_p->mouseLMBPressed = (state == GLUT_DOWN);
      break;
    }
    case 3:
    {
      if (state == GLUT_DOWN)
        cb_data_p->camera.updatePosition (Common_GL_Camera::Direction::FORWARD,
                                          0.25f);
      break;
    }
    case 4:
    {
      if (state == GLUT_DOWN)
        cb_data_p->camera.updatePosition (Common_GL_Camera::Direction::BACKWARD,
                                          0.25f);
      break;
    }
    default:
      break;
  } // end IF
}

void
test_u_projectm_glut_mouse_move (int x,
                                 int y)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  cb_data_p->mouseX = x;
  cb_data_p->mouseY = y;
}

void
test_u_projectm_glut_timer (int value_in)
{
  // sanity check(s)
  struct Test_U_GLUT_CBData* cb_data_p = NULL;
#if defined (ACE_LINUX)
  ACE_ASSERT (ACE_SIZEOF_INT == 4);
  uint64_t value_i = static_cast<unsigned int> (value_in) + 0x7FFF00000000;
  cb_data_p = reinterpret_cast<struct Test_U_GLUT_CBData*> (value_i);
#else
  ACE_ASSERT (timer_cb_data_p);
  cb_data_p =
    reinterpret_cast<struct Test_U_GLUT_CBData*> (timer_cb_data_p);
#endif // ACE_LINUX
  ACE_ASSERT (cb_data_p);

  if (likely (cb_data_p->windowId != -1))
  {
    glutPostRedisplay ();

    glutTimerFunc (1000 / TEST_U_GLUT_DEFAULT_FPS, test_u_glut_timer, value_in);
  } // end IF
}

void
test_u_projectm_glut_draw (void)
{
  static int frame_count_i = 1;

  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->projectMConfiguration);
  ACE_ASSERT (cb_data_p->projectMConfiguration->handle);

  glClear (GL_COLOR_BUFFER_BIT /* | GL_DEPTH_BUFFER_BIT*/);

  projectm_opengl_render_frame (cb_data_p->projectMConfiguration->handle);

  glutSwapBuffers ();

  ++frame_count_i;
}

void
test_u_projectm_glut_idle (void)
{
  //glutPostRedisplay ();
}

void
test_u_projectm_glut_visible (int vis)
{
  // glutIdleFunc ((vis == GLUT_VISIBLE) ? test_u_glut_idle : NULL);
}
