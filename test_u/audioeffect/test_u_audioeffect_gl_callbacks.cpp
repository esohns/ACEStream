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

#include "test_u_audioeffect_gl_callbacks.h"

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

void
processInstructions (struct Test_U_AudioEffect_UI_CBDataBase* CBDataBase_in)
{
  STREAM_TRACE (ACE_TEXT ("::processInstructions"));

  // sanity check(s)
  ACE_ASSERT (CBDataBase_in);
  ACE_ASSERT (CBDataBase_in->UIState);

  struct Stream_Visualization_OpenGL_Instruction* instruction_p = NULL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBDataBase_in->UIState->lock);
    if (CBDataBase_in->OpenGLInstructions.empty ())
      return;

    do
    {
      instruction_p = &CBDataBase_in->OpenGLInstructions.front ();
      switch (instruction_p->type)
      {
        case STREAM_VISUALIZATION_OPENGL_INSTRUCTION_CHANGE_ROTATION:
        {
          CBDataBase_in->objectRotation *= -1;
          break;
        }
        case STREAM_VISUALIZATION_OPENGL_INSTRUCTION_SET_COLOR_BG:
        {
          glClearColor (static_cast<GLclampf> (instruction_p->color.red),
                        static_cast<GLclampf> (instruction_p->color.green),
                        static_cast<GLclampf> (instruction_p->color.blue),
                        1.0F);
          break;
        }
        case STREAM_VISUALIZATION_OPENGL_INSTRUCTION_SET_COLOR_FG:
        {
          glColor4f (static_cast<GLclampf> (instruction_p->color.red),
                     static_cast<GLclampf> (instruction_p->color.green),
                     static_cast<GLclampf> (instruction_p->color.blue),
                     1.0F);
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
      CBDataBase_in->OpenGLInstructions.pop_front ();
    } while (!CBDataBase_in->OpenGLInstructions.empty ());
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
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
  GtkAllocation allocation;
  //// set up light colors (ambient, diffuse, specular)
  //GLfloat light_ambient[] = {1.0F, 1.0F, 1.0F, 1.0F};
  //GLfloat light_diffuse[] = {0.3F, 0.3F, 0.3F, 1.0F};
  //GLfloat light_specular[] = {1.0F, 1.0F, 1.0F, 1.0F};
  //// position the light in eye space
  //GLfloat light0_position[] = {0.0F,
  //                             5.0F * 2,
  //                             5.0F * 2,
  //                             0.0F}; // --> directional light

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &((*modulehandler_configuration_iterator).second.second->OpenGLTextureId);

#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (widget_in);
#else
  // sanity check(s)
  ACE_ASSERT (widget_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,16,0)
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
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p);

#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
  GtkGLArea* gl_area_p = GTK_GL_AREA (widget_in);
  ACE_ASSERT (gl_area_p);
  // NOTE*: the OpenGL context has been created at this point
  GdkGLContext* context_p = gtk_gl_area_get_context (gl_area_p);
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_gl_area_get_context(%@), returning\n"),
                gl_area_p));
    goto error;
  } // end IF

  // load the texture
  gtk_gl_area_attach_buffers (gl_area_p);
  gdk_gl_context_make_current (context_p);

  // sanity check(s)
  ACE_ASSERT (gtk_gl_area_get_has_depth_buffer (gl_area_p));
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
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
#endif // GTKGLAREA_SUPPORT
    return;
#endif // GTK_CHECK_VERSION(3,0,0)

#if GTK_CHECK_VERSION(3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  result = gdk_gl_drawable_gl_begin (drawable_p,
                                     context_p);
  if (!result)
    return;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)

  // load texture
  if (*texture_id_p > 0)
  {
    glDeleteTextures (1, texture_id_p);
    COMMON_GL_ASSERT;
    *texture_id_p = 0;
  } // end IF
  static GLubyte* image_p = NULL;
  if (!image_p)
  {
    std::string filename = Common_File_Tools::getWorkingDirectory ();
    filename += ACE_DIRECTORY_SEPARATOR_CHAR;
    filename += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
    filename += ACE_DIRECTORY_SEPARATOR_CHAR;
    filename +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_IMAGE_FILE);
    *texture_id_p = Common_GL_Tools::loadTexture (filename);
    if (!*texture_id_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_GL_Tools::loadTexture(\"%s\"), returning\n"),
                  ACE_TEXT (filename.c_str ())));
      goto error;
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
  COMMON_GL_ASSERT;

  glMatrixMode (GL_PROJECTION);
  COMMON_GL_ASSERT;
  glLoadIdentity (); // Reset The Projection Matrix
  COMMON_GL_ASSERT;

#if defined (GLU_SUPPORT)
  gluPerspective (45.0,
                  allocation.width / (GLdouble)allocation.height,
                  0.1,
                  100.0); // Calculate The Aspect Ratio Of The Window
#else
  GLdouble fW, fH;

  //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
  fH = tan (45.0 / 360 * M_PI) * 0.1;
  fW = fH * (allocation.width / (GLdouble)allocation.height);

  glFrustum (-fW, fW, -fH, fH, 0.1, 100.0);
#endif // GLU_SUPPORT
  COMMON_GL_ASSERT;

  glMatrixMode (GL_MODELVIEW);
  COMMON_GL_ASSERT;
  glLoadIdentity (); // reset the projection matrix
  COMMON_GL_ASSERT;

  /* light */
//  GLfloat light_positions[2][4]   = { 50.0, 50.0, 0.0, 0.0,
//                                     -50.0, 50.0, 0.0, 0.0 };
//  GLfloat light_colors[2][4] = { .6, .6,  .6, 1.0,   /* white light */
//                                 .4, .4, 1.0, 1.0 }; /* cold blue light */
//  glLightfv (GL_LIGHT0, GL_POSITION, light_positions[0]);
//  glLightfv (GL_LIGHT0, GL_DIFFUSE,  light_colors[0]);
//  glLightfv (GL_LIGHT1, GL_POSITION, light_positions[1]);
//  glLightfv (GL_LIGHT1, GL_DIFFUSE,  light_colors[1]);
//  glEnable (GL_LIGHT0);
//  glEnable (GL_LIGHT1);
//  glEnable (GL_LIGHTING);

  // set up light colors (ambient, diffuse, specular)
  //glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
  //COMMON_GL_ASSERT;
  //glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  //COMMON_GL_ASSERT;
  //glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
  //COMMON_GL_ASSERT;
  //glLightfv (GL_LIGHT0, GL_POSITION, light0_position);
  //COMMON_GL_ASSERT;
  //glEnable (GL_LIGHT0);
  //COMMON_GL_ASSERT;

  //glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
  //COMMON_GL_ASSERT;
  glEnable (GL_TEXTURE_2D);                           // Enable Texture Mapping
  COMMON_GL_ASSERT;
  //glShadeModel (GL_SMOOTH);                           // Enable Smooth Shading
  //COMMON_GL_ASSERT;
  //glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  //COMMON_GL_ASSERT;

  glEnable (GL_BLEND);                                // Enable Semi-Transparency
  COMMON_GL_ASSERT;
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  COMMON_GL_ASSERT;
  //glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing
  //COMMON_GL_ASSERT;
  //glDepthFunc (GL_LESS);                              // The Type Of Depth Testing To Do
  //COMMON_GL_ASSERT;
  //glDepthMask (GL_TRUE);
  //COMMON_GL_ASSERT;

#if GTK_CHECK_VERSION(3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
  gdk_gl_drawable_gl_end (drawable_p);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)

  return;

error:
  return;
} // glarea_realize_cb

#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
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
                                       2, 1);
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
  COMMON_GL_ASSERT;
  //glClearDepth (1.0);                                 // Depth Buffer Setup
  //COMMON_GL_ASSERT;
  /* speedups */
  //  glDisable (GL_CULL_FACE);
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

  return result_p;
}

gboolean
glarea_render_cb (GtkGLArea* GLArea_in,
                  GdkGLContext* context_in,
                  gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_render_cb"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (GLArea_in);
  ACE_ASSERT (userData_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  struct Stream_Visualization_OpenGL_Instruction* instruction_p = NULL;

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
//  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second->OpenGLTextureId);

  texture_id_p =
    &(*modulehandler_configuration_iterator).second.second->OpenGLTextureId;
#endif
  ACE_ASSERT (texture_id_p);

  static bool is_first = true;
  if (is_first)
  {
    is_first = false;

    // initialize options
    glClearColor (0.0F, 0.0F, 0.0F, 1.0F);              // Black Background
    COMMON_GL_ASSERT;
    //glClearDepth (1.0);                                 // Depth Buffer Setup
    //COMMON_GL_ASSERT;
    /* speedups */
    //  glDisable (GL_CULL_FACE);
    //  glEnable (GL_DITHER);
    //  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //  glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
//    COMMON_GL_ASSERT;
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
//    glBlendFunc (GL_ONE, GL_ZERO);
//    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    COMMON_GL_ASSERT;
    glEnable (GL_DEPTH_TEST);                           // Enables Depth Testing
    COMMON_GL_ASSERT;

    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    COMMON_GL_ASSERT;
    glEnable (GL_COLOR_MATERIAL);
    COMMON_GL_ASSERT;
    glEnable (GL_NORMALIZE);
    COMMON_GL_ASSERT;
//    glEnable (GL_LIGHTING);
//    COMMON_GL_ASSERT;

    // initialize texture
    std::string filename = Common_File_Tools::getWorkingDirectory ();
    filename += ACE_DIRECTORY_SEPARATOR_CHAR;
    filename += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
    filename += ACE_DIRECTORY_SEPARATOR_CHAR;
    filename +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_IMAGE_FILE);
    *texture_id_p = Common_GL_Tools::loadTexture (filename);
    if (!*texture_id_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_GL_Texture::load(\"%s\"), returning\n"),
                  ACE_TEXT (filename.c_str ())));
      return FALSE;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("OpenGL texture id: %u\n"),
                *texture_id_p));
  } // end IF

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // *TODO*: find out why this reports GL_INVALID_OPERATION
  COMMON_GL_PRINT_ERROR;

  glBindTexture (GL_TEXTURE_2D, *texture_id_p);
  COMMON_GL_ASSERT;

  glLoadIdentity ();				// Reset the transformation matrix.
  COMMON_GL_ASSERT;

  glTranslatef (0.0f, 0.0f, -5.0f);		// Move back into the screen 7
  COMMON_GL_ASSERT;

  static GLfloat cube_rotation = 0.0f;
  glRotatef (cube_rotation, 1.0f, 1.0f, 1.0f);		// Rotate The Cube On X, Y, and Z
  COMMON_GL_ASSERT;

  //static GLfloat rot_x = 0.0f;
  //static GLfloat rot_y = 0.0f;
  //static GLfloat rot_z = 0.0f;
  //glRotatef (rot_x, 1.0f, 0.0f, 0.0f); // Rotate On The X Axis
  //glRotatef (rot_y, 0.0f, 1.0f, 0.0f); // Rotate On The Y Axis
  //glRotatef (rot_z, 0.0f, 0.0f, 1.0f); // Rotate On The Z Axis

  glBegin (GL_QUADS);

  // Front Face
  glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f, -1.0f,  1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.0f, -1.0f,  1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.0f,  1.0f,  1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f,  1.0f,  1.0f); // Top Left Of The Texture and Quad
  // Back Face
  glTexCoord2f (1.0f, 0.0f); glVertex3f (-1.0f, -1.0f, -1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f (-1.0f,  1.0f, -1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f ( 1.0f,  1.0f, -1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f ( 1.0f, -1.0f, -1.0f); // Bottom Left Of The Texture and Quad
  // Top Face
  glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f,  1.0f, -1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f,  1.0f,  1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.0f,  1.0f,  1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.0f,  1.0f, -1.0f); // Top Right Of The Texture and Quad
  // Bottom Face
  glTexCoord2f (1.0f, 1.0f); glVertex3f (-1.0f, -1.0f, -1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f ( 1.0f, -1.0f, -1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f ( 1.0f, -1.0f,  1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f (-1.0f, -1.0f,  1.0f); // Bottom Right Of The Texture and Quad
  // Right face
  glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.0f, -1.0f, -1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.0f,  1.0f, -1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f ( 1.0f,  1.0f,  1.0f); // Top Left Of The Texture and Quad
  glTexCoord2f (0.0f, 0.0f); glVertex3f ( 1.0f, -1.0f,  1.0f); // Bottom Left Of The Texture and Quad
  // Left Face
  glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f, -1.0f, -1.0f); // Bottom Left Of The Texture and Quad
  glTexCoord2f (1.0f, 0.0f); glVertex3f (-1.0f, -1.0f,  1.0f); // Bottom Right Of The Texture and Quad
  glTexCoord2f (1.0f, 1.0f); glVertex3f (-1.0f,  1.0f,  1.0f); // Top Right Of The Texture and Quad
  glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f,  1.0f, -1.0f); // Top Left Of The Texture and Quad

  glEnd ();
  COMMON_GL_ASSERT;

  cube_rotation -= 1.0f;					// Decrease The Rotation Variable For The Cube

  processInstructions (ui_cb_data_base_p);

continue_:
  //rot_x += 0.3f;
  //rot_y += 0.20f;
  //rot_z += 0.4f;

  //GLuint vertex_array_id = 0;
  //glGenVertexArrays (1, &vertex_array_id);
  //glBindVertexArray (vertex_array_id);

  //static const GLfloat vertex_buffer_data[] = {
  //  -1.0f, -1.0f, 0.0f,
  //  1.0f, -1.0f, 0.0f,
  //  -1.0f,  1.0f, 0.0f,
  //  -1.0f,  1.0f, 0.0f,
  //  1.0f, -1.0f, 0.0f,
  //  1.0f,  1.0f, 0.0f,
  //};

  //GLuint vertex_buffer;
  //glGenBuffers (1, &vertex_buffer);
  //glBindBuffer (GL_ARRAY_BUFFER, vertex_buffer);
  //glBufferData (GL_ARRAY_BUFFER,
  //              sizeof (vertex_buffer_data), vertex_buffer_data,
  //              GL_STATIC_DRAW);

  ////GLuint program_id = LoadShaders ("Passthrough.vertexshader",
  ////                                 "SimpleTexture.fragmentshader");
  ////GLuint tex_id = glGetUniformLocation (program_id, "renderedTexture");
  ////GLuint time_id = glGetUniformLocation (program_id, "time");

  //glBindFramebuffer (GL_FRAMEBUFFER, 0);
  //glViewport (0, 0,
  //            data_p->area3D.width, data_p->area3D.height);

  //gtk_gl_area_queue_render (GLArea_in);

  return TRUE;
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

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
//  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
//    NULL;
//  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
//  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
//  if (data_base_p->useMediaFoundation)
//  {
//    mediafoundation_ui_cb_data_p =
//      static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
//      static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
//    // sanity check(s)
//    ACE_ASSERT (directshow_ui_cb_data_p);
//    ACE_ASSERT (directshow_ui_cb_data_p->configuration);
//
//    directshow_modulehandler_configuration_iterator =
//      directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//    ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
//  } // end ELSE
//#else
//  struct Test_U_AudioEffect_UI_CBData* data_p =
//      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->configuration);
//
//  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
//    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
//#endif

  //gtk_gl_area_make_current (GLArea_in);

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
#else
#if defined (GTKGLAREA_SUPPORT)
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
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

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &(*modulehandler_configuration_iterator).second.second->OpenGLTextureId;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p);

  if (!ggla_area_make_current (GGLA_AREA (widget_in)))
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glLoadIdentity (); // Reset the transformation matrix.
  ACE_ASSERT (glGetError () == GL_NO_ERROR);
  glTranslatef (0.0F, 0.0F, -5.0F); // Move back into the screen 5 units
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

  glBindTexture (GL_TEXTURE_2D, *texture_id_p);
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

//  static GLfloat rot_x = 0.0f;
//  static GLfloat rot_y = 0.0f;
//  static GLfloat rot_z = 0.0f;
//  glRotatef (rot_x, 1.0f, 0.0f, 0.0f); // Rotate On The X Axis
//  glRotatef (rot_y, 0.0f, 1.0f, 0.0f); // Rotate On The Y Axis
//  glRotatef (rot_z, 0.0f, 0.0f, 1.0f); // Rotate On The Z Axis
  static GLfloat rotation = 0.0F;
  glRotatef (rotation, 1.0F, 1.0F, 1.0F); // Rotate On The X,Y,Z Axis
  ACE_ASSERT (glGetError () == GL_NO_ERROR);

//  glBegin (GL_QUADS);

//  glTexCoord2i (0, 0); glVertex3f (  0.0f,   0.0f, 0.0f);
//  glTexCoord2i (0, 1); glVertex3f (  0.0f, 100.0f, 0.0f);
//  glTexCoord2i (1, 1); glVertex3f (100.0f, 100.0f, 0.0f);
//  glTexCoord2i (1, 0); glVertex3f (100.0f,   0.0f, 0.0f);

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

//  rot_x += 0.3f;
//  rot_y += 0.20f;
//  rot_z += 0.4f;
  rotation -= 1.0f; // Decrease The Rotation Variable For The Cube

  //GLuint vertex_array_id = 0;
  //glGenVertexArrays (1, &vertex_array_id);
  //glBindVertexArray (vertex_array_id);

  //static const GLfloat vertex_buffer_data[] = {
  //  -1.0f, -1.0f, 0.0f,
  //  1.0f, -1.0f, 0.0f,
  //  -1.0f,  1.0f, 0.0f,
  //  -1.0f,  1.0f, 0.0f,
  //  1.0f, -1.0f, 0.0f,
  //  1.0f,  1.0f, 0.0f,
  //};

  //GLuint vertex_buffer;
  //glGenBuffers (1, &vertex_buffer);
  //glBindBuffer (GL_ARRAY_BUFFER, vertex_buffer);
  //glBufferData (GL_ARRAY_BUFFER,
  //              sizeof (vertex_buffer_data), vertex_buffer_data,
  //              GL_STATIC_DRAW);

  ////GLuint program_id = LoadShaders ("Passthrough.vertexshader",
  ////                                 "SimpleTexture.fragmentshader");
  ////GLuint tex_id = glGetUniformLocation (program_id, "renderedTexture");
  ////GLuint time_id = glGetUniformLocation (program_id, "time");

  //glBindFramebuffer (GL_FRAMEBUFFER, 0);
  //glViewport (0, 0,
  //            data_p->area3D.width, data_p->area3D.height);

  ggla_area_swap_buffers (GGLA_AREA (widget_in));

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
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  if (ui_cb_data_base_p->useMediaFoundation)
  {
    mediafoundation_ui_cb_data_p =
      static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
    ACE_ASSERT (mediafoundation_ui_cb_data_p);
    ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
  } // end IF
  else
  {
    directshow_ui_cb_data_p =
      static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
    ACE_ASSERT (directshow_ui_cb_data_p);
    ACE_ASSERT (directshow_ui_cb_data_p->configuration);
  } // end ELSE
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
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

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
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

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
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

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_UNUSED_ARG (event_in);
  ACE_ASSERT (userData_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  GLuint* texture_id_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  texture_id_p =
    &((*modulehandler_configuration_iterator).second.second->OpenGLTextureId);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (texture_id_p);

  // sanity check(s)
  ACE_ASSERT (widget_in);

  if (!gtk_gl_area_make_current (GTK_GL_AREA (widget_in)))
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  COMMON_GL_ASSERT;
  glLoadIdentity (); // Reset the transformation matrix.
  COMMON_GL_ASSERT;
  glTranslatef (0.0F, 0.0F, -5.0F); // Move back into the screen 5 units
  COMMON_GL_ASSERT;

  glBindTexture (GL_TEXTURE_2D, *texture_id_p);
  COMMON_GL_ASSERT;

//  static GLfloat rot_x = 0.0f;
//  static GLfloat rot_y = 0.0f;
//  static GLfloat rot_z = 0.0f;
//  glRotatef (rot_x, 1.0f, 0.0f, 0.0f); // Rotate On The X Axis
//  glRotatef (rot_y, 0.0f, 1.0f, 0.0f); // Rotate On The Y Axis
//  glRotatef (rot_z, 0.0f, 0.0f, 1.0f); // Rotate On The Z Axis
  static GLfloat rotation = 0.0F;
  glRotatef (rotation, 1.0F, 1.0F, 1.0F); // Rotate On The X,Y,Z Axis
  COMMON_GL_ASSERT;

//  glBegin (GL_QUADS);

//  glTexCoord2i (0, 0); glVertex3f (  0.0f,   0.0f, 0.0f);
//  glTexCoord2i (0, 1); glVertex3f (  0.0f, 100.0f, 0.0f);
//  glTexCoord2i (1, 1); glVertex3f (100.0f, 100.0f, 0.0f);
//  glTexCoord2i (1, 0); glVertex3f (100.0f,   0.0f, 0.0f);

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
  COMMON_GL_ASSERT;
  glVertexPointer (3, GL_FLOAT, 0, vertices);
  COMMON_GL_ASSERT;
  glDrawElements (GL_QUADS, 24, GL_UNSIGNED_BYTE, cube_indices);
  COMMON_GL_ASSERT;

//  rot_x += 0.3f;
//  rot_y += 0.20f;
//  rot_z += 0.4f;
  rotation -= (1.0f * ui_cb_data_base_p->objectRotation); // modify The Rotation Angle For The Cube

  //GLuint vertex_array_id = 0;
  //glGenVertexArrays (1, &vertex_array_id);
  //glBindVertexArray (vertex_array_id);

  //static const GLfloat vertex_buffer_data[] = {
  //  -1.0f, -1.0f, 0.0f,
  //  1.0f, -1.0f, 0.0f,
  //  -1.0f,  1.0f, 0.0f,
  //  -1.0f,  1.0f, 0.0f,
  //  1.0f, -1.0f, 0.0f,
  //  1.0f,  1.0f, 0.0f,
  //};

  //GLuint vertex_buffer;
  //glGenBuffers (1, &vertex_buffer);
  //glBindBuffer (GL_ARRAY_BUFFER, vertex_buffer);
  //glBufferData (GL_ARRAY_BUFFER,
  //              sizeof (vertex_buffer_data), vertex_buffer_data,
  //              GL_STATIC_DRAW);

  ////GLuint program_id = LoadShaders ("Passthrough.vertexshader",
  ////                                 "SimpleTexture.fragmentshader");
  ////GLuint tex_id = glGetUniformLocation (program_id, "renderedTexture");
  ////GLuint time_id = glGetUniformLocation (program_id, "time");

  //glBindFramebuffer (GL_FRAMEBUFFER, 0);
  //glViewport (0, 0,
  //            data_p->area3D.width, data_p->area3D.height);

  processInstructions (ui_cb_data_base_p);

  gtk_gl_area_swap_buffers (GTK_GL_AREA (widget_in));

  return TRUE;
}
#else
void
glarea_configure_event_cb (GtkWidget* widget_in,
                           GdkEvent* event_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::glarea_configure_event_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
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
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
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
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
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
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
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
