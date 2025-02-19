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

#include "common_gl_tools.h"

#include "stream_vis_defines.h"

const char libacestream_default_vis_opengl_glut_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING);

//#define COS(X)   cos( (X) * 3.14159/180.0 )
//#define SIN(X)   sin( (X) * 3.14159/180.0 )

//#define RED 1
//#define WHITE 2
//#define CYAN 3

//GLuint
//libacestream_glut_make_ball (void)
//{
//  GLuint list;
//  GLfloat a, b;
//  GLfloat da = 18.0F, db = 18.0F;
//  GLfloat radius = 1.0F;
//  GLuint color;
//  GLfloat x, y, z;
//
//  list = glGenLists (1);
//
//  glNewList (list, GL_COMPILE);
//
//  color = 0;
//  for (a = -90.0; a + da <= 90.0; a += da) {
//
//    glBegin (GL_QUAD_STRIP);
//    for (b = 0.0; b <= 360.0; b += db) {
//
//      if (color) {
//        glIndexi (RED);
//        glColor3f (1, 0, 0);
//      }
//      else {
//        glIndexi (WHITE);
//        glColor3f (1, 1, 1);
//      }
//
//      x = radius * COS (b) * COS (a);
//      y = radius * SIN (b) * COS (a);
//      z = radius * SIN (a);
//      glVertex3f (x, y, z);
//
//      x = radius * COS (b) * COS (a + da);
//      y = radius * SIN (b) * COS (a + da);
//      z = radius * SIN (a + da);
//      glVertex3f (x, y, z);
//
//      color = 1 - color;
//    }
//    glEnd ();
//
//  }
//
//  glEndList ();
//
//  return list;
//}

void
libacestream_glut_reshape (int width_in, int height_in)
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
                  0.1, 100.0);
  COMMON_GL_ASSERT;

  glMatrixMode (GL_MODELVIEW);
  COMMON_GL_ASSERT;
}

void
libacestream_glut_key (unsigned char key_in, int x_in, int y_in)
{
  struct OpenGL_GLUT_WindowData* cb_data_p =
    static_cast<struct OpenGL_GLUT_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);

  switch (key_in)
  {
    case 27: /* Escape */
    { ACE_ASSERT (cb_data_p->stream);
      cb_data_p->stream->stop (false,  // wait for completion ?
                               false,  // recurse upstream ?
                               false); // high priority ?
      glutLeaveMainLoop ();
      break;
    }
  } // end SWITCH
}

void
libacestream_glut_draw (void)
{
  static int frame_count_i = 1;

  struct OpenGL_GLUT_WindowData* cb_data_p =
    static_cast<struct OpenGL_GLUT_WindowData*> (glutGetWindowData ());
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->queue);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (cb_data_p->mediaType);
  unsigned int depth_i =
    Stream_MediaFramework_DirectShow_Tools::toFrameBits (cb_data_p->mediaType) / 8;
#else
  unsigned int depth_i =
    Stream_MediaFramework_Tools::ffmpegFormatToBitDepth (cb_data_p->mediaType.format) / 8;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Message_Block* message_block_p = NULL;

  message_block_p = NULL;
   cb_data_p->queue->dequeue_head (message_block_p,
                                   NULL);
  ACE_ASSERT (message_block_p);

  if (frame_count_i == 1)
  {
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, cb_data_p->textureId);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glGenerateMipmap (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, 0);
  } // end IF

  Common_GL_Tools::loadTexture (reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ()),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                resolution_s.cx, resolution_s.cy,
#else
                                cb_data_p->mediaType.resolution.width,
                                cb_data_p->mediaType.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                                depth_i,
                                cb_data_p->textureId,
                                frame_count_i == 1); // update ?
  message_block_p->release ();

  glBindTexture (GL_TEXTURE_2D, cb_data_p->textureId);

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity (); // Reset the transformation matrix.

  glTranslatef (0.0f, 0.0f, -5.0f); // Move back into the screen 7

  static GLfloat cube_rotation = 0.0f;
  glRotatef (cube_rotation, 1.0f, 1.0f, 1.0f); // Rotate The Cube On X, Y, and Z

  glBegin (GL_QUADS);

  // Front Face
  glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f, -1.0f, 1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f (1.0f, -1.0f, 1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f (1.0f, 1.0f, 1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f, 1.0f, 1.0f); // Top Left Of The Texture and Quad
  // Back Face
  glTexCoord2f (1.0f, 0.0f); glVertex3f (-1.0f, -1.0f, -1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f (-1.0f, 1.0f, -1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f (1.0f, 1.0f, -1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f (1.0f, -1.0f, -1.0f); // Bottom Left Of The Texture and Quad
  // Top Face
  glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f, 1.0f, -1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f, 1.0f, 1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f (1.0f, 1.0f, 1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f (1.0f, 1.0f, -1.0f); // Top Right Of The Texture and Quad
  // Bottom Face
  glTexCoord2f (1.0f, 1.0f); glVertex3f (-1.0f, -1.0f, -1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f (1.0f, -1.0f, -1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f (1.0f, -1.0f, 1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f (-1.0f, -1.0f, 1.0f); // Bottom Right Of The Texture and Quad
  // Right face
  glTexCoord2f (1.0f, 0.0f); glVertex3f (1.0f, -1.0f, -1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f (1.0f, 1.0f, -1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f (1.0f, 1.0f, 1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f (1.0f, -1.0f, 1.0f); // Bottom Left Of The Texture and Quad
  // Left Face
  glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f, -1.0f, -1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f (-1.0f, -1.0f, 1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f (-1.0f, 1.0f, 1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f, 1.0f, -1.0f); // Top Left Of The Texture and Quad

  glEnd ();
  // *TODO*: find out why this reports GL_INVALID_OPERATION
  //COMMON_GL_PRINT_ERROR;

  cube_rotation -= 1.0f; // Decrease The Rotation Variable For The Cube

  glBindTexture (GL_TEXTURE_2D, 0);
  //COMMON_GL_ASSERT;
//  glDeleteTextures (1, &cb_data_p->textureId);
//  COMMON_GL_ASSERT;

  glFlush ();
  //COMMON_GL_ASSERT;

  glutSwapBuffers ();

  ++frame_count_i;
}

void
libacestream_glut_idle (void)
{
  //static float vel0 = -100.0;
  //static double t0 = -1.;
  //double t, dt;
  //t = glutGet (GLUT_ELAPSED_TIME) / 1000.;
  //if (t0 < 0.)
  //  t0 = t;
  //dt = t - t0;
  //t0 = t;

  //Zrot += Zstep * dt;

  //Xpos += Xvel * dt;
  //if (Xpos >= Xmax) {
  //  Xpos = Xmax;
  //  Xvel = -Xvel;
  //  Zstep = -Zstep;
  //}
  //if (Xpos <= Xmin) {
  //  Xpos = Xmin;
  //  Xvel = -Xvel;
  //  Zstep = -Zstep;
  //}
  //Ypos += Yvel * dt;
  //Yvel += G * dt;
  //if (Ypos < Ymin) {
  //  Ypos = Ymin;
  //  if (vel0 == -100.0)
  //    vel0 = fabs (Yvel);
  //  Yvel = vel0;
  //}
  glutPostRedisplay ();
}

void
libacestream_glut_close ()
{ // pressing the 'x' is the same as hitting 'Escape'
  libacestream_glut_key (27, 0, 0);
}

void
libacestream_glut_visible (int vis)
{
  if (vis == GLUT_VISIBLE)
    glutIdleFunc (libacestream_glut_idle);
  else
    glutIdleFunc (NULL);
}
