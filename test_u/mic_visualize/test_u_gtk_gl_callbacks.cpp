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

#include "test_u_gtk_gl_callbacks.h"

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gl/GL.h"
#include "gl/GLU.h"
#else
#include "GL/gl.h"
#include "GL/glu.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "common_gl_defines.h"
#include "common_gl_tools.h"

#include "stream_macros.h"

#include "test_u_mic_visualize_common.h"

void
processInstructions (struct Test_U_MicVisualize_UI_CBDataBase* CBDataBase_in)
{
  STREAM_TRACE (ACE_TEXT ("::processInstructions"));

  // sanity check(s)
  ACE_ASSERT (CBDataBase_in);
  ACE_ASSERT (CBDataBase_in->OpenGLInstructions);
  ACE_ASSERT (CBDataBase_in->OpenGLInstructionsLock);

  struct Stream_Visualization_GTKGL_Instruction* instruction_p = NULL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *CBDataBase_in->OpenGLInstructionsLock);
    if (CBDataBase_in->OpenGLInstructions->empty ())
      return;

    do
    {
      instruction_p = &CBDataBase_in->OpenGLInstructions->front ();
      switch (instruction_p->type)
      {
        case STREAM_VISUALIZATION_INSTRUCTION_CHANGE_ROTATION:
        {
          CBDataBase_in->objectRotationStep *= -1.0f;
          break;
        }
        case STREAM_VISUALIZATION_INSTRUCTION_ROTATE:
        {
          CBDataBase_in->objectRotation += 0.01f;
          break;
        }
        case STREAM_VISUALIZATION_INSTRUCTION_SET_COLOR_BG:
        {
#if GTK_CHECK_VERSION (3,0,0)
          glClearColor ((float)instruction_p->color.red,
                        (float)instruction_p->color.green,
                        (float)instruction_p->color.blue,
                        1.0F);
#else
          glClearColor ((float)instruction_p->color.red   / (float)std::numeric_limits<guint16>::max (),
                        (float)instruction_p->color.green / (float)std::numeric_limits<guint16>::max (),
                        (float)instruction_p->color.blue  / (float)std::numeric_limits<guint16>::max (),
                        1.0F);
#endif // GTK_CHECK_VERSION (3,0,0)
          break;
        }
        case STREAM_VISUALIZATION_INSTRUCTION_SET_COLOR_FG:
        {
#if GTK_CHECK_VERSION (3,0,0)
          glColor4f ((float)instruction_p->color.red,
                     (float)instruction_p->color.green,
                     (float)instruction_p->color.blue,
                     1.0F);
#else
          glColor4f ((float)instruction_p->color.red   / (float)std::numeric_limits<guint16>::max (),
                     (float)instruction_p->color.green / (float)std::numeric_limits<guint16>::max (),
                     (float)instruction_p->color.blue  / (float)std::numeric_limits<guint16>::max (),
                     1.0F);
#endif // GTK_CHECK_VERSION (3,0,0)
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown OpenGL effect (was: %d), continuing\n"),
                      ACE_TEXT (instruction_p->type)));
          break;
        }
      } // end SWITCH
      CBDataBase_in->OpenGLInstructions->pop_front ();
    } while (!CBDataBase_in->OpenGLInstructions->empty ());
  } // end lock scope
}

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
glarea_realize_cb (GtkWidget* widget_in,
                   gpointer   userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_realize_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (userData_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
#endif // ACE_WIN32 || ACE_WIN64

  GLuint* texture_id_p = NULL;
  GLuint* VAO_p = NULL;
  GLuint* VBO_p = NULL;
  GLuint* EBO_p = NULL;
  Common_GL_Shader* shader_p = NULL;
  GtkAllocation allocation;
  std::string path_root, vertex_shader_file_path, fragment_shader_file_path;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      VAO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->VAO;
      VBO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->VBO;
      EBO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->EBO;
      shader_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->shader;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      VAO_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->VAO;
      VBO_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->VBO;
      EBO_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->EBO;
      shader_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->shader;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &((*modulehandler_configuration_iterator).second.second->OpenGLTextureId);
  VAO_p =
    &((*modulehandler_configuration_iterator).second.second->VAO);
  VBO_p =
    &((*modulehandler_configuration_iterator).second.second->VBO);
  EBO_p =
    &((*modulehandler_configuration_iterator).second.second->EBO);
  shader_p =
    &((*modulehandler_configuration_iterator).second.second->shader);

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  // sanity check(s)
  ACE_ASSERT (widget_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;

  // sanity check(s)
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p && VAO_p && VBO_p && EBO_p && shader_p);

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea* gl_area_p = GTK_GL_AREA (widget_in);
  ACE_ASSERT (gl_area_p);
  // NOTE*: the OpenGL context has been created at this point
  GdkGLContext* context_p = gtk_gl_area_get_context (gl_area_p);
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    return;
  } // end IF

  // load the texture
  gtk_gl_area_attach_buffers (gl_area_p);
  gdk_gl_context_make_current (context_p);

  // sanity check(s)
  ACE_ASSERT (gtk_gl_area_get_has_depth_buffer (gl_area_p));
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
    return;
#else
  GdkGLContext* context_p = gtk_gl_area_get_context (GTK_GL_AREA (widget_in));
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    goto error;
  } // end IF

  bool result = gdk_gl_drawable_make_current (drawable_p,
                                              context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  result = gdk_gl_drawable_gl_begin (drawable_p,
                                     context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if defined (GLEW_SUPPORT)
  GLenum err = glewInit ();
  if (GLEW_OK != err)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glewInit(): \"%s\", continuing\n"),
                ACE_TEXT (glewGetErrorString (err))));
#endif // GLEW_SUPPORT

  // load texture
  if (!*texture_id_p)
  {
    std::string filename =
      Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                                        ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_U_SUBDIRECTORY),
                                                        false); // data
    filename += ACE_DIRECTORY_SEPARATOR_CHAR;
    filename +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_OPENGL_DEFAULT_TEXTURE_FILE);
    *texture_id_p = Common_GL_Tools::loadTexture (filename);
    if (!*texture_id_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_GL_Tools::loadTexture(\"%s\"), returning\n"),
                  ACE_TEXT (filename.c_str ())));
      return;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("OpenGL texture id: %u\n"),
                *texture_id_p));
  } // end IF

  // initialize perspective
  gtk_widget_get_allocation (widget_in,
                             &allocation);
  glViewport (0, 0,
              static_cast<GLsizei> (allocation.width), static_cast<GLsizei> (allocation.height));

  //glMatrixMode (GL_PROJECTION);
  //glLoadIdentity (); // Reset The Projection Matrix

#if defined (GLU_SUPPORT)
  ACE_ASSERT (allocation.height);
  gluPerspective (45.0,
                  allocation.width / (GLdouble)allocation.height,
                  0.1,
                  100.0); // Calculate The Aspect Ratio Of The Window
#else
  GLdouble fW, fH;

  //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
  fH = tan (45.0 / 360.0 * M_PI) * 0.1;
  fW = fH * (allocation.width / (GLdouble)allocation.height);

  glFrustum (-fW, fW, -fH, fH, 0.1, 100.0);
#endif // GLU_SUPPORT

  //glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
  glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
  //glShadeModel (GL_SMOOTH);                           // Enable Smooth Shading
  //glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);

  glEnable (GL_BLEND);                                // Enable Semi-Transparency
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing
  //glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
  //glDepthMask (GL_TRUE);

  path_root = Common_File_Tools::getWorkingDirectory ();
  vertex_shader_file_path = path_root;
  vertex_shader_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  vertex_shader_file_path += COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY;
  vertex_shader_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  vertex_shader_file_path +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_OPENGL_DEFAULT_VS_FILE);
  fragment_shader_file_path = path_root;
  fragment_shader_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  fragment_shader_file_path += COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY;
  fragment_shader_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  fragment_shader_file_path +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_OPENGL_DEFAULT_FS_FILE);
  if (!shader_p->loadFromFile (vertex_shader_file_path,
                               fragment_shader_file_path))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_GL_Shader::loadFromFile(\"%s\",\"%s\"), returning\n"),
                ACE_TEXT (vertex_shader_file_path.c_str ()),
                ACE_TEXT (fragment_shader_file_path.c_str ())));
    return;
  } // end IF
  shader_p->use ();

  glGenVertexArrays (1, VAO_p);
  ACE_ASSERT (*VAO_p);
  glGenBuffers (1, VBO_p);
  ACE_ASSERT (*VBO_p);
  glGenBuffers (1, EBO_p);
  ACE_ASSERT (*EBO_p);

  glBindVertexArray (*VAO_p);

  glBindBuffer (GL_ARRAY_BUFFER, *VBO_p);

  static GLfloat cube_strip_color_texcoords[] = {
   // x       y      z         r    g     b     a         u    v
    -0.5f, -0.5f,  0.5f,     1.0f, 0.0f, 0.0f, 0.5f,    0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,     0.0f, 1.0f, 0.0f, 0.5f,    0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 1.0f, 0.5f,    1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,     1.0f, 1.0f, 1.0f, 0.5f,    1.0f, 1.0f,

     0.5f, -0.5f,  0.5f,     1.0f, 0.0f, 0.0f, 0.5f,    0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,     0.0f, 1.0f, 0.0f, 0.5f,    0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 1.0f, 0.5f,    1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,     1.0f, 1.0f, 1.0f, 0.5f,    1.0f, 1.0f,

     0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f, 0.5f,    0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,     0.0f, 1.0f, 0.0f, 0.5f,    0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 1.0f, 0.5f,    1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,     1.0f, 1.0f, 1.0f, 0.5f,    1.0f, 1.0f,

    -0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f, 0.5f,    0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,     0.0f, 1.0f, 0.0f, 0.5f,    0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 1.0f, 0.5f,    1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,     1.0f, 1.0f, 1.0f, 0.5f,    1.0f, 1.0f,

    -0.5f, -0.5f, -0.5f,     1.0f, 0.0f, 0.0f, 0.5f,    0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,     0.0f, 1.0f, 0.0f, 0.5f,    0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 1.0f, 0.5f,    1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,     1.0f, 1.0f, 1.0f, 0.5f,    1.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,     1.0f, 0.0f, 0.0f, 0.5f,    0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,     0.0f, 1.0f, 0.0f, 0.5f,    0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 1.0f, 0.5f,    1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,     1.0f, 1.0f, 1.0f, 0.5f,    1.0f, 1.0f
  };
  glBufferData (GL_ARRAY_BUFFER, sizeof (cube_strip_color_texcoords), cube_strip_color_texcoords, GL_STATIC_DRAW);

  // position attribute
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof (GLfloat), (void*)0);

  // color attribute
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(3 * sizeof (GLfloat)));

  // texture coord attribute
  glEnableVertexAttribArray (2);
  glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof (GLfloat), (void*)(7 * sizeof (GLfloat)));

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, *EBO_p);

  static GLubyte cube_indices[34] = {
    0,  1,  2,  3,  3,      // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
    4,  4,  5,  6,  7,  7,  // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
    8,  8,  9,  10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
    12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
    16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
    20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
  };
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (cube_indices), cube_indices, GL_STATIC_DRAW);

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  glBindVertexArray (0);

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  gdk_gl_drawable_gl_end (drawable_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
} // glarea_realize_cb

void
glarea_unrealize_cb (GtkWidget* widget_in,
                     gpointer   userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_unrealize_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (userData_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
#endif // ACE_WIN32 || ACE_WIN64

  GLuint* texture_id_p = NULL;
  GLuint* VAO_p = NULL;
  GLuint* VBO_p = NULL;
  GLuint* EBO_p = NULL;
  Common_GL_Shader* shader_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      VAO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->VAO;
      VBO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->VBO;
      EBO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->EBO;
      shader_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->shader;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      VAO_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->VAO;
      VBO_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->VBO;
      EBO_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->EBO;
      shader_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->shader;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &((*modulehandler_configuration_iterator).second.second->OpenGLTextureId);
  VAO_p =
    &(*modulehandler_configuration_iterator).second.second->VAO;
  VBO_p =
    &(*modulehandler_configuration_iterator).second.second->VBO;
  EBO_p =
    &(*modulehandler_configuration_iterator).second.second->EBO;
  shader_p =
    &((*modulehandler_configuration_iterator).second.second->shader);

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  // sanity check(s)
  ACE_ASSERT (widget_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;

  // sanity check(s)
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p && VAO_p && VBO_p && EBO_p && shader_p);

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea* gl_area_p = GTK_GL_AREA (widget_in);
  ACE_ASSERT (gl_area_p);
  // NOTE*: the OpenGL context has been created at this point
  GdkGLContext* context_p = gtk_gl_area_get_context (gl_area_p);
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    return;
  } // end IF

  // load the texture
  gtk_gl_area_attach_buffers (gl_area_p);
  gdk_gl_context_make_current (context_p);

  // sanity check(s)
  ACE_ASSERT (gtk_gl_area_get_has_depth_buffer (gl_area_p));
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
    return;
#else
  GdkGLContext* context_p = gtk_gl_area_get_context (GTK_GL_AREA (widget_in));
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    return;
  } // end IF

  bool result = gdk_gl_drawable_make_current (drawable_p,
                                              context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  result = gdk_gl_drawable_gl_begin (drawable_p,
                                     context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)

  if (*texture_id_p > 0)
  {
    glDeleteTextures (1, texture_id_p);
    *texture_id_p = 0;
  } // end IF
  glDeleteVertexArrays (1, VAO_p);
  glDeleteBuffers (1, VBO_p);
  glDeleteBuffers (1, EBO_p);
  shader_p->reset ();

#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  gdk_gl_drawable_gl_end (drawable_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
} // glarea_unrealize_cb

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
GdkGLContext*
glarea_create_context_cb (GtkGLArea* GLArea_in,
                          gpointer userData_in)
{
  // sanity check(s)
  ACE_ASSERT (GLArea_in);
  ACE_ASSERT (userData_in);
  ACE_ASSERT (!gtk_gl_area_get_context (GLArea_in));

  GdkGLContext* result_p = NULL;

  GError* error_p = NULL;
  // *TODO*: this currently fails on Wayland (Gnome 3.22.24)
  // *WORKAROUND*: set GDK_BACKEND=x11 environment to force XWayland
  result_p =
    gdk_window_create_gl_context (gtk_widget_get_window (GTK_WIDGET (GLArea_in)),
                                  &error_p);
  if (!result_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_window_create_gl_context(): \"%s\", aborting\n"),
                ACE_TEXT (error_p->message)));
    gtk_gl_area_set_error (GLArea_in, error_p);
    g_error_free (error_p); error_p = NULL;
    return NULL;
  } // end IF

  gdk_gl_context_set_required_version (result_p,
                                       3, 2);
#if defined (_DEBUG)
  gdk_gl_context_set_debug_enabled (result_p,
                                    TRUE);
#endif // _DEBUG
  //gdk_gl_context_set_forward_compatible (result_p,
  //                                       FALSE);
  gdk_gl_context_set_use_es (result_p,
                             -1); // auto-detect

  if (!gdk_gl_context_realize (result_p,
                               &error_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to realize OpenGL context: \"%s\", continuing\n"),
                ACE_TEXT (error_p->message)));
    gtk_gl_area_set_error (GLArea_in, error_p);
    g_error_free (error_p); error_p = NULL;
    return NULL;
  } // end IF

  gdk_gl_context_make_current (result_p);

  // initialize options
  glClearColor (0.0F, 0.0F, 0.0F, 1.0F);              // Black Background
  //glClearDepth (1.0);                                 // Depth Buffer Setup
  /* speedups */
  //  glDisable (GL_CULL_FACE);
  //  glEnable (GL_DITHER);
  //  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  //  glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
  //glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  //glEnable (GL_COLOR_MATERIAL);
  //glEnable (GL_LIGHTING);
  // glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
  // glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
  // glDepthMask (GL_TRUE);
  glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
  // glShadeModel (GL_SMOOTH);                           // Enable Smooth Shading
  // glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  // glEnable (GL_BLEND);                                // Enable Semi-Transparency
  // glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing

  return result_p;
}

gboolean
glarea_render_cb (GtkGLArea* GLArea_in,
                  GdkGLContext* context_in,
                  gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_render_cb"));

  // sanity check(s)
  ACE_ASSERT (GLArea_in);
  ACE_UNUSED_ARG (context_in);
  ACE_ASSERT (userData_in);
  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
  GLuint* VAO_p = NULL;
  GLuint* EBO_p = NULL;
  Common_GL_Shader* shader_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  struct Test_U_MicVisualize_UI_CBData* ui_cb_data_p =
      static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &(*modulehandler_configuration_iterator).second.second->OpenGLTextureId;
  VAO_p =
    &((*modulehandler_configuration_iterator).second.second->VAO);
  EBO_p =
    &((*modulehandler_configuration_iterator).second.second->EBO);
  shader_p =
    &((*modulehandler_configuration_iterator).second.second->shader);
#endif
  ACE_ASSERT (texture_id_p && VAO_p && EBO_p && shader_p);

  static bool is_first = true;
  if (unlikely (is_first))
  {
    is_first = false;

    // initialize options
    glClearColor (0.0F, 0.0F, 0.0F, 1.0F);              // Black Background
    //glClearDepth (1.0);                                 // Depth Buffer Setup
    /* speedups */
    //  glDisable (GL_CULL_FACE);
    //  glEnable (GL_DITHER);
    //  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //  glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
    // glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
    // glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
    // glDepthMask (GL_TRUE);
    glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
    // glShadeModel (GL_SMOOTH);                           // Enable Smooth Shading
    // glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glDisable (GL_BLEND);
    // glEnable (GL_BLEND);                                // Enable Semi-Transparency
    // glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glBlendFunc (GL_ONE, GL_ZERO);
//    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing

    // glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    // glEnable (GL_COLOR_MATERIAL);
    // glEnable (GL_NORMALIZE);
//    glEnable (GL_LIGHTING);

    // initialize texture ?
    if (!*texture_id_p)
    {
      std::string filename = Common_File_Tools::getWorkingDirectory ();
      filename += ACE_DIRECTORY_SEPARATOR_CHAR;
      filename += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_DATA_SUBDIRECTORY);
      filename += ACE_DIRECTORY_SEPARATOR_CHAR;
      filename +=
        ACE_TEXT_ALWAYS_CHAR (TEST_U_OPENGL_DEFAULT_TEXTURE_FILE);
      *texture_id_p = Common_GL_Tools::loadTexture (filename);
      if (!*texture_id_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_GL_Tools::load(\"%s\"), returning\n"),
                    ACE_TEXT (filename.c_str ())));
        return FALSE;
      } // end IF
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("OpenGL texture id: %u\n"),
                *texture_id_p));
  } // end IF

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindTexture (GL_TEXTURE_2D, *texture_id_p);

  // //static GLfloat rot_x = 0.0f;
  // //static GLfloat rot_y = 0.0f;
  // //static GLfloat rot_z = 0.0f;
  // //glRotatef (rot_x, 1.0f, 0.0f, 0.0f); // Rotate On The X Axis
  // //glRotatef (rot_y, 0.0f, 1.0f, 0.0f); // Rotate On The Y Axis
  // //glRotatef (rot_z, 0.0f, 0.0f, 1.0f); // Rotate On The Z Axis

  // ui_cb_data_base_p->objectRotationStep -= 1.0f; // Decrease The Rotation Variable For The Cube

  shader_p->use ();

#if defined (GLM_SUPPORT)
  glm::mat4 model_matrix = glm::mat4 (1.0f); // make sure to initialize matrix to identity matrix first
  model_matrix = glm::translate (model_matrix,
                                 glm::vec3 (0.0f, 0.0f, -3.0f));
  model_matrix = glm::rotate (model_matrix,
                              glm::radians (ui_cb_data_base_p->objectRotation),
                              glm::vec3 (1.0f, 1.0f, 1.0f));
  glm::mat4 view_matrix = glm::lookAt (glm::vec3 (0.0f, 0.0f, 0.0f),
                                       glm::vec3 (0.0f, 0.0f, -1.0f),
                                       glm::vec3 (0.0f, 1.0f, 0.0f));
  GtkAllocation allocation;
  gtk_widget_get_allocation (GTK_WIDGET (GLArea_in),
                             &allocation);
  glm::mat4 projection_matrix =
    glm::perspective (glm::radians (45.0f),
                      allocation.width / static_cast<float> (allocation.height),
                      0.1f, 100.0f);
#else
#error this program requires glm, aborting compilation
#endif // GLM_SUPPORT

#if defined (GLM_SUPPORT)
  shader_p->setMat4 (ACE_TEXT_ALWAYS_CHAR ("model"), model_matrix);
  shader_p->setMat4 (ACE_TEXT_ALWAYS_CHAR ("view"), view_matrix);
  shader_p->setMat4 (ACE_TEXT_ALWAYS_CHAR ("projection"), projection_matrix);
#endif // GLM_SUPPORT
  shader_p->setInt (ACE_TEXT_ALWAYS_CHAR ("texture1"), 0); // *IMPORTANT NOTE*: <-- texture unit (!) not -id

  glBindVertexArray (*VAO_p);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, *EBO_p);

  glDrawElements (GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_BYTE, NULL);

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray (0);

  glBindTexture (GL_TEXTURE_2D, 0);

  processInstructions (ui_cb_data_base_p);

  shader_p->unuse ();

  gtk_gl_area_queue_render (GLArea_in);

  return FALSE;
}

void
glarea_resize_cb (GtkGLArea* GLArea_in,
                  gint width_in,
                  gint height_in,
                  gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_resize_cb"));

  // sanity check(s)
  ACE_ASSERT (GLArea_in);
  ACE_ASSERT (userData_in);

  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
//  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
//    NULL;
//  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
//  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
//  if (data_base_p->useMediaFoundation)
//  {
//    mediafoundation_ui_cb_data_p =
//      static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
//    // sanity check(s)
//    ACE_ASSERT (mediafoundation_ui_cb_data_p);
//    ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
//
//    mediafoundation_modulehandler_configuration_iterator =
//      mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//    ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
//  } // end IF
//  else
//  {
//    directshow_ui_cb_data_p =
//      static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
//    // sanity check(s)
//    ACE_ASSERT (directshow_ui_cb_data_p);
//    ACE_ASSERT (directshow_ui_cb_data_p->configuration);
//
//    directshow_modulehandler_configuration_iterator =
//      directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//    ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
//  } // end ELSE
//#else
//  struct Test_U_MicVisualize_UI_CBData* data_p =
//      static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->configuration);
//
//  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
//    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
//#endif

  //gtk_gl_area_make_current (GLArea_in);

  glViewport (0, 0,
              static_cast<GLsizei> (width_in), static_cast<GLsizei> (height_in));

  glMatrixMode (GL_PROJECTION);

  glLoadIdentity ();

  ACE_ASSERT (height_in);
  gluPerspective (45.0,
                  static_cast<GLdouble> (width_in) / static_cast<GLdouble> (height_in),
                  0.1, 100.0);

  glMatrixMode (GL_MODELVIEW);
}
#else
#if defined (GTKGLAREA_SUPPORT)
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  ACE_ASSERT (widget_in);
  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return;

  glViewport (0, 0,
              event_in->configure.width, event_in->configure.height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glMatrixMode (GL_PROJECTION);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glLoadIdentity (); // Reset The Projection Matrix
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  gluPerspective (45.0,
                  event_in->configure.width / (GLdouble)event_in->configure.height,
                  0.1,
                  100.0); // Calculate The Aspect Ratio Of The Window
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glMatrixMode (GL_MODELVIEW);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
}

gboolean
glarea_expose_event_cb (GtkWidget* widget_in,
                        GdkEvent* event_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_expose_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_UNUSED_ARG (event_in);
  ACE_ASSERT (userData_in);

  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
  GLuint* VAO_p = NULL;
  GLuint* EBO_p = NULL;
  Common_GL_Shader* shader_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      VAO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->VAO;
      EBO_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->EBO;
      shader_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->shader;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return FALSE;
    }
  } // end SWITCH
#else
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &(*modulehandler_configuration_iterator).second.second->OpenGLTextureId;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p && VAO_p && EBO_p && shader_p);

  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glBindVertexArray (*VAO_p);
  COMMON_GL_ASSERT;

  glBindTexture (GL_TEXTURE_2D, *texture_id_p);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

#if defined (GLM_SUPPORT)
  glm::mat4 model_matrix = glm::mat4 (1.0f); // make sure to initialize matrix to identity matrix first
  model_matrix = glm::translate (model_matrix,
                                 glm::vec3 (0.0f, 0.0f, -3.0f));
  model_matrix = glm::rotate (model_matrix,
                              glm::radians (ui_cb_data_base_p->objectRotation),
                              glm::vec3 (1.0f, 1.0f, 1.0f));
  glm::mat4 view_matrix = glm::lookAt (glm::vec3 (0.0f, 0.0f, 0.0f),
                                       glm::vec3 (0.0f, 0.0f, -1.0f),
                                       glm::vec3 (0.0f, 1.0f, 0.0f));
  GtkAllocation allocation;
  gtk_widget_get_allocation (widget_in,
                             &allocation);
  glm::mat4 projection_matrix =
    glm::perspective (glm::radians (45.0f),
                      allocation.width / static_cast<float> (allocation.height),
                      0.1f, 100.0f);
#else
#error this program requires glm, aborting compilation
#endif // GLM_SUPPORT

  shader_p->use ();
#if defined (GLM_SUPPORT)
  shader_p->setMat4 (ACE_TEXT_ALWAYS_CHAR ("model"), model_matrix);
  shader_p->setMat4 (ACE_TEXT_ALWAYS_CHAR ("view"), view_matrix);
  shader_p->setMat4 (ACE_TEXT_ALWAYS_CHAR ("projection"), projection_matrix);
#endif // GLM_SUPPORT
  shader_p->setInt (ACE_TEXT_ALWAYS_CHAR ("texture1"), 0); // *IMPORTANT NOTE*: <-- texture unit (!) not -id

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, *EBO_p);
  COMMON_GL_ASSERT;

  glDisable (GL_DEPTH_TEST);
  COMMON_GL_ASSERT;
  glDrawElements (GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_BYTE, (void*)0); // see: cube_indices
  COMMON_GL_ASSERT;
  glEnable (GL_DEPTH_TEST);
  COMMON_GL_ASSERT;

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
  COMMON_GL_ASSERT;

  glBindTexture (GL_TEXTURE_2D, 0);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glBindVertexArray (0);
  COMMON_GL_ASSERT;

  //ui_cb_data_base_p->objectRotation += ui_cb_data_base_p->objectRotationStep;

  processInstructions (ui_cb_data_base_p);

  ggla_area_swap_buffers (GGLA_AREA (widget_in));

  glUseProgram (0);
  COMMON_GL_ASSERT;

  return TRUE;
}
#else // !GTKGLAREA_SUPPORT
void
glarea_size_allocate_event_cb (GtkWidget* widget_in,
                               GdkRectangle* allocation_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_size_allocate_event_cb"));

  // sanity check(s)
  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  if (ui_cb_data_base_p->useMediaFoundation)
  {
    mediafoundation_ui_cb_data_p =
      static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
    ACE_ASSERT (mediafoundation_ui_cb_data_p);
    ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
  } // end IF
  else
  {
    directshow_ui_cb_data_p =
      static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
    ACE_ASSERT (directshow_ui_cb_data_p);
    ACE_ASSERT (directshow_ui_cb_data_p->configuration);
  } // end ELSE
#else
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);

  if (!gdk_gl_drawable_make_current (drawable_p,
                                     context_p))
    return;
#endif // ACE_WIN32 || ACE_WIN64

  glViewport (0, 0,
              allocation_in->width, allocation_in->height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glMatrixMode (GL_PROJECTION);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glLoadIdentity (); // Reset The Projection Matrix
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  gluPerspective (45.0,
                  allocation_in->width / (GLdouble)allocation_in->height,
                  0.1,
                  100.0); // Calculate The Aspect Ratio Of The Window
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glMatrixMode (GL_MODELVIEW);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
}

gboolean
glarea_draw_cb (GtkWidget* widget_in,
                cairo_t* context_in,
                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_draw_cb"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (userData_in);

  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &(*modulehandler_configuration_iterator).second.second->OpenGLTextureId;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (false); // *TODO*
#else
  GdkGLDrawable* drawable_p =
    (*modulehandler_configuration_iterator).second.GdkWindow3D;
  GdkGLContext* context_p =
    (*modulehandler_configuration_iterator).second.OpenGLContext;

  // sanity check(s)
  ACE_ASSERT (drawable_p);
  ACE_ASSERT (context_p);

  bool result = gdk_gl_drawable_make_current (drawable_p,
                                              context_p);
  if (!result)
    return FALSE;
  result = gdk_gl_drawable_gl_begin (drawable_p,
                                     context_p);
  if (!result)
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glLoadIdentity (); // Reset the transformation matrix.
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glTranslatef (0.0F, 0.0F, -5.0F); // Move back into the screen 5 units
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glBindTexture (GL_TEXTURE_2D, *texture_id_p);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  static GLfloat rotation = 0.0F;
  glRotatef (rotation, 1.0F, 1.0F, 1.0F); // Rotate On The X,Y,Z Axis
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  static GLfloat vertices[] = {
    -0.5f, 0.0f, 0.5f,   0.5f, 0.0f, 0.5f,   0.5f, 1.0f, 0.5f,  -0.5f, 1.0f, 0.5f,
    -0.5f, 1.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f,
    0.5f, 0.0f, 0.5f,   0.5f, 0.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 1.0f, 0.5f,
    -0.5f, 0.0f, -0.5f,  -0.5f, 0.0f, 0.5f,  -0.5f, 1.0f, 0.5f, -0.5f, 1.0f, -0.5f};
  static GLfloat texture_coordinates[] = {
    0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
    0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
    0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
    0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0 };
  static GLubyte cube_indices[24] = {
    0,1,2,3, 4,5,6,7, 3,2,5,4, 7,6,1,0,
    8,9,10,11, 12,13,14,15};

  glTexCoordPointer (2, GL_FLOAT, 0, texture_coordinates);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glVertexPointer (3, GL_FLOAT, 0, vertices);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glDrawElements (GL_QUADS, 24, GL_UNSIGNED_BYTE, cube_indices);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  rotation -= 1.0f; // Decrease The Rotation Variable For The Cube

  gdk_gl_drawable_gl_end (drawable_p);
  gdk_gl_drawable_swap_buffers (drawable_p);
#endif // ACE_WIN32 || ACE_WIN64

  return TRUE;
}
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64
  // sanity check(s)
  ACE_ASSERT (widget_in);

  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
    return;

  glViewport (0, 0,
              event_in->configure.width, event_in->configure.height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glMatrixMode (GL_PROJECTION);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glLoadIdentity (); // Reset The Projection Matrix
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

#if defined (GLU_SUPPORT)
  gluPerspective (45.0,
                  event_in->configure.width / (GLdouble)event_in->configure.height,
                  0.1,
                  100.0); // Calculate The Aspect Ratio Of The Window
#else
  GLdouble fW, fH;

  //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
  fH = tan (45.0 / 360 * M_PI) * 0.1;
  fW = fH * (event_in->configure.width / (GLdouble)event_in->configure.height);

  glFrustum (-fW, fW, -fH, fH, 0.1, 100.0);
#endif // GLU_SUPPORT
  COMMON_GL_ASSERT;

  glMatrixMode (GL_MODELVIEW);
  COMMON_GL_ASSERT;
}

gboolean
glarea_expose_event_cb (GtkWidget* widget_in,
                        GdkEvent* event_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_expose_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      // sanity check(s)
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*directshow_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // sanity check(s)
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      texture_id_p =
        &(*mediafoundation_modulehandler_configuration_iterator).second.second->OpenGLTextureId;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return FALSE;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &((*modulehandler_configuration_iterator).second.second->OpenGLTextureId);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p);

  if (!gtk_gl_area_begingl (GTK_GL_AREA (widget_in)))
    return FALSE;

  processInstructions (ui_cb_data_base_p);

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  COMMON_GL_ASSERT;
  glLoadIdentity (); // Reset the transformation matrix
  COMMON_GL_ASSERT;
  glTranslatef (0.0F, 0.0F, -5.0F); // Move back into the screen 5 units
  COMMON_GL_ASSERT;
  static GLfloat rotation = 0.0f;
  glRotatef (rotation, 1.0f, 1.0f, 1.0f); // Rotate around the X,Y,Z axis'
  COMMON_GL_ASSERT;
  rotation -= ui_cb_data_base_p->objectRotationStep; // Modify rotation angle

  static GLfloat vertices[] = {
    -0.5f, 0.0f,  0.5f,  0.5f, 0.0f,  0.5f,  0.5f, 1.0f,  0.5f, -0.5f, 1.0f,  0.5f,
    -0.5f, 1.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f,
     0.5f, 0.0f,  0.5f,  0.5f, 0.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 1.0f,  0.5f,
    -0.5f, 0.0f, -0.5f, -0.5f, 0.0f,  0.5f, -0.5f, 1.0f,  0.5f, -0.5f, 1.0f, -0.5f};
  static GLfloat texture_coordinates[] = {
     0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
     0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
     0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
     0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0};
  static GLubyte cube_indices[24] = {
     0,1,2,3, 4,5,6,7, 3,2,5,4, 7,6,1,0, 8,9,10,11, 12,13,14,15};

  glBindTexture (GL_TEXTURE_2D, *texture_id_p);
  COMMON_GL_ASSERT;
  glTexCoordPointer (2, GL_FLOAT, 0, texture_coordinates);
  COMMON_GL_ASSERT;
  glVertexPointer (3, GL_FLOAT, 0, vertices);
  COMMON_GL_ASSERT;
  glDrawElements (GL_QUADS, 24, GL_UNSIGNED_BYTE, cube_indices);
  COMMON_GL_ASSERT;

  gtk_gl_area_swap_buffers (GTK_GL_AREA (widget_in));

  gtk_gl_area_endgl (GTK_GL_AREA (widget_in));

  return TRUE;
}
#else
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  //// sanity check(s)
  //ACE_ASSERT (widget_in);

  //GdkGLDrawable* gl_drawable_p = gtk_widget_get_gl_drawable (widget_in);
  //ACE_ASSERT (gl_drawable_p);
  //GdkGLContext* gl_context_p = gtk_widget_get_gl_context (widget_in);
  //ACE_ASSERT (gl_context_p);

  glViewport (0, 0,
              event_in->configure.width, event_in->configure.height);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glMatrixMode (GL_PROJECTION);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glLoadIdentity (); // Reset The Projection Matrix
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  gluPerspective (45.0,
                  event_in->configure.width / (GLdouble)event_in->configure.height,
                  0.1,
                  100.0); // Calculate The Aspect Ratio Of The Window
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glMatrixMode (GL_MODELVIEW);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
}

gboolean
glarea_expose_event_cb (GtkWidget* widget_in,
                        GdkEvent* event_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_expose_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_UNUSED_ARG (event_in);
  struct Test_U_MicVisualize_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_MicVisualize_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MicVisualize_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_MicVisualize_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_MicVisualize_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_MicVisualize_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_MicVisualize_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_MicVisualize_UI_CBData* data_p =
    static_cast<struct Test_U_MicVisualize_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_MicVisualize_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  GLuint texture_id = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      texture_id =
        (*directshow_modulehandler_configuration_iterator).second.OpenGLTextureId;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      texture_id =
        (*mediafoundation_modulehandler_configuration_iterator).second.OpenGLTextureId;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  texture_id =
    (*modulehandler_configuration_iterator).second.OpenGLTextureId;
#endif // ACE_WIN32 || ACE_WIN64
  // sanity check(s)
  if (texture_id == 0)
    return FALSE; // --> still waiting for the first frame

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  ACE_ASSERT (window_p);

  GdkGLDrawable* gl_drawable_p = gtk_widget_get_gl_drawable (widget_in);
  ACE_ASSERT (gl_drawable_p);
  GdkGLContext* gl_context_p = gtk_widget_get_gl_context (widget_in);
  ACE_ASSERT (gl_context_p);

  gdk_gl_drawable_gl_begin (gl_drawable_p,
                            gl_context_p);

//  gdk_gl_drawable_swap_buffers (gl_drawable_p);
  gdk_gl_drawable_gl_end (gl_drawable_p);

  return TRUE;
} // glarea_draw_cb
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,0,0) */
#ifdef __cplusplus
}
#endif /* __cplusplus */
