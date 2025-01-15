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

#include "test_u_mic_visualize_common.h"
#include "test_u_mic_visualize_defines.h"

void
test_u_glut_reshape (int width_in, int height_in)
{
  glViewport (0, 0, width_in, height_in);

  glMatrixMode (GL_PROJECTION);

  glLoadIdentity ();

  ACE_ASSERT (height_in);
  gluPerspective (45.0,
                  width_in / static_cast<GLdouble> (height_in),
                  150.0, -150.0);
  //glOrtho (static_cast<GLdouble> (-width_in / 2.0), static_cast<GLdouble> (width_in / 2.0),
  //         static_cast<GLdouble> (height_in / 2.0), static_cast<GLdouble> (-height_in / 2.0), 150.0, -150.0);

  glMatrixMode (GL_MODELVIEW);
}

void
test_u_glut_key (unsigned char key_in,
                     int x,
                     int y)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (key_in)
  {
    case 27: /* Escape */
      glutLeaveMainLoop ();
      break;
  } // end SWITCH
}

void
test_u_glut_key_special (int key_in,
                             int x,
                             int y)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (key_in)
  {
    case GLUT_KEY_LEFT:
      break;
    case GLUT_KEY_RIGHT:
      break;
    case GLUT_KEY_UP:
      cb_data_p->camera.position_.x = 0.0f;
      cb_data_p->camera.position_.y = 0.0f;
      cb_data_p->camera.position_.z = 1000.0f;
      break;
  } // end SWITCH
}

void
test_u_glut_menu (int entry_in)
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
test_u_glut_mouse_button (int button, int state, int x, int y)
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
    default:
      break;
  } // end IF
}

void
test_u_glut_mouse_move (int x, int y)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  cb_data_p->mouseX = x;
  cb_data_p->mouseY = y;
}

void
test_u_glut_timer (int v)
{
  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  glutPostRedisplay ();

  glutTimerFunc (1000 / TEST_U_GLUT_DEFAULT_FPS,
                 test_u_glut_timer,
                 v);
}

void
test_u_glut_draw (void)
{
  static int frame_count_i = 1;
  float x1, y1, z1;
  int i = 0;
  float r, g, b;
  std::vector<float> spectrum_a;

  struct Test_U_GLUT_CBData* cb_data_p =
    static_cast<struct Test_U_GLUT_CBData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);
  if (!cb_data_p->fft) // stream currently not running ?
    goto continue_;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // reset transformations
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  // set the camera
  gluLookAt (cb_data_p->camera.position_.x, cb_data_p->camera.position_.y, cb_data_p->camera.position_.z,
             cb_data_p->camera.looking_at_.x, cb_data_p->camera.looking_at_.y, cb_data_p->camera.looking_at_.z,
             cb_data_p->camera.up_.x, cb_data_p->camera.up_.y, cb_data_p->camera.up_.z);

  glPolygonMode (GL_FRONT_AND_BACK,
                 cb_data_p->wireframe ? GL_LINE : GL_FILL);

  // draw a red x-axis, a green y-axis, and a blue z-axis. Each of the
  // axes are 100 units long
  glBegin (GL_LINES);
  glColor3f (1.0f, 0.0f, 0.0f); glVertex3i (0, 0, 0); glVertex3i (100, 0, 0);
  glColor3f (0.0f, 1.0f, 0.0f); glVertex3i (0, 0, 0); glVertex3i (0, 100, 0);
  glColor3f (0.0f, 0.0f, 1.0f); glVertex3i (0, 0, 0); glVertex3i (0, 0, 100);
  glEnd ();

  //glColor3f (1.0f, 1.0f, 1.0f);

  glTranslatef (0.0f, -250.0f, 0.0f);
  glRotatef (static_cast<float> (M_PI_4) * (180.0f / static_cast<float> (M_PI)), 1.0f, 0.0f, 0.0f);
  glRotatef (frame_count_i / 10.0f * (180.0f / static_cast<float> (M_PI)), 0.0f, 0.0f, 1.0f);

  spectrum_a = cb_data_p->fft->Spectrum ();
  for (int k = 0; k < TEST_U_GLUT_DEFAULT_LAYERS; k++)
  {
    Common_Image_Tools::HSVToRGB (std::fmod (k * 18.0f, 360.0f),
                                  80 / 100.0f,
                                  100 / 100.0f,
                                  r, g, b);
    glColor3f (r, g, b);

    glBegin (GL_LINE_STRIP);
    for (float a = 0.0f; a < 2.0f * static_cast<float> (M_PI); a += 1.0f / static_cast<float> (k))
    {
      float x = TEST_U_GLUT_DEFAULT_D * k * std::cos (a);
      float y = TEST_U_GLUT_DEFAULT_D * k * std::sin (a);
      float z = static_cast<float> (k); //*std::cos (frame_count_i);
      // z *= noise(x/30,y/30);
      if (i < static_cast<int> (spectrum_a.size ()))
      {
        z -= spectrum_a[i++] * TEST_U_GLUT_DEFAULT_AMP_FACTOR;
        // stroke(255, 144, random(0,255));
      } // end IF

      if (unlikely (a == 0.0f))
      {
        x1 = x;
        y1 = y;
        z1 = z;
      } // end IF

      glVertex3f (x, y, z);
    } // end FOR
    glVertex3f (x1, y1, z1);
    glEnd ();
  } // end FOR

continue_:
  glutSwapBuffers ();

  ++frame_count_i;
}

void
test_u_glut_idle (void)
{
  //glutPostRedisplay ();
}

void
test_u_glut_visible (int vis)
{
  if (vis == GLUT_VISIBLE)
    ;// glutIdleFunc (test_u_glut_idle);
  else
    glutIdleFunc (NULL);
}
