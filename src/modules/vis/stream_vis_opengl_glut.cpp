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

#include "stream_vis_opengl_glut.h"

#include "stream_vis_defines.h"

const char libacestream_default_vis_opengl_glut_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING);

#define COS(X)   cos( (X) * 3.14159/180.0 )
#define SIN(X)   sin( (X) * 3.14159/180.0 )

#define RED 1
#define WHITE 2
#define CYAN 3

GLuint
libacestream_glut_make_ball (void)
{
  GLuint list;
  GLfloat a, b;
  GLfloat da = 18.0F, db = 18.0F;
  GLfloat radius = 1.0F;
  GLuint color;
  GLfloat x, y, z;

  list = glGenLists (1);

  glNewList (list, GL_COMPILE);

  color = 0;
  for (a = -90.0; a + da <= 90.0; a += da) {

    glBegin (GL_QUAD_STRIP);
    for (b = 0.0; b <= 360.0; b += db) {

      if (color) {
        glIndexi (RED);
        glColor3f (1, 0, 0);
      }
      else {
        glIndexi (WHITE);
        glColor3f (1, 1, 1);
      }

      x = radius * COS (b) * COS (a);
      y = radius * SIN (b) * COS (a);
      z = radius * SIN (a);
      glVertex3f (x, y, z);

      x = radius * COS (b) * COS (a + da);
      y = radius * SIN (b) * COS (a + da);
      z = radius * SIN (a + da);
      glVertex3f (x, y, z);

      color = 1 - color;
    }
    glEnd ();

  }

  glEndList ();

  return list;
}

void
libacestream_glut_reshape (int width, int height)
{
  float aspect = (float)width / (float)height;
  glViewport (0, 0, (GLint)width, (GLint)height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (-6.0 * aspect, 6.0 * aspect, -6.0, 6.0, -6.0, 6.0);
  glMatrixMode (GL_MODELVIEW);
}

void
libacestream_glut_key (unsigned char k, int x, int y)
{
  struct OpenGL_GLUT_WindowData* cb_data_p = 
    static_cast<struct OpenGL_GLUT_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (k) {
  case 27:  /* Escape */
    //exit (0);
    glutLeaveMainLoop ();

    //ACE_ASSERT (cb_data_p->queue);
    //ACE_Message_Block* message_block_p = NULL;
    //ACE_NEW_NORETURN (message_block_p,
    //                  ACE_Message_Block ());
    //ACE_ASSERT (message_block_p);
    //cb_data_p->queue->enqueue (message_block_p);

    break;
  }
}

void
libacestream_glut_draw (void)
{
  GLint i;

  glClear (GL_COLOR_BUFFER_BIT);

  glIndexi (CYAN);
  glColor3f (0, 1, 1);
  glBegin (GL_LINES);
  for (i = -5; i <= 5; i++) {
    glVertex2i (i, -5);
    glVertex2i (i, 5);
  }
  for (i = -5; i <= 5; i++) {
    glVertex2i (-5, i);
    glVertex2i (5, i);
  }
  for (i = -5; i <= 5; i++) {
    glVertex2i (i, -5);
    glVertex2f (i * 1.15, -5.9);
  }
  glVertex2f (-5.3, -5.35);
  glVertex2f (5.3, -5.35);
  glVertex2f (-5.75, -5.9);
  glVertex2f (5.75, -5.9);
  glEnd ();

  glPushMatrix ();
  glTranslatef (Xpos, Ypos, 0.0);
  glScalef (2.0, 2.0, 2.0);
  glRotatef (8.0, 0.0, 0.0, 1.0);
  glRotatef (90.0, 1.0, 0.0, 0.0);
  glRotatef (Zrot, 0.0, 0.0, 1.0);

  glCallList (Ball);

  glPopMatrix ();

  glFlush ();
  glutSwapBuffers ();
}

void
libacestream_glut_idle (void)
{
  static float vel0 = -100.0;
  static double t0 = -1.;
  double t, dt;
  t = glutGet (GLUT_ELAPSED_TIME) / 1000.;
  if (t0 < 0.)
    t0 = t;
  dt = t - t0;
  t0 = t;

  Zrot += Zstep * dt;

  Xpos += Xvel * dt;
  if (Xpos >= Xmax) {
    Xpos = Xmax;
    Xvel = -Xvel;
    Zstep = -Zstep;
  }
  if (Xpos <= Xmin) {
    Xpos = Xmin;
    Xvel = -Xvel;
    Zstep = -Zstep;
  }
  Ypos += Yvel * dt;
  Yvel += G * dt;
  if (Ypos < Ymin) {
    Ypos = Ymin;
    if (vel0 == -100.0)
      vel0 = fabs (Yvel);
    Yvel = vel0;
  }
  glutPostRedisplay ();
}

void
libacestream_glut_visible (int vis)
{
  if (vis == GLUT_VISIBLE)
    glutIdleFunc (libacestream_glut_idle);
  else
    glutIdleFunc (NULL);
}
