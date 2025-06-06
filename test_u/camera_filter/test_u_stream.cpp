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

#include "test_u_stream.h"

#include "ace/Log_Msg.h"

#include "common_os_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_tools.h"

#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_stat_defines.h"

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_DirectShow_Stream::Test_U_DirectShow_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING))
 , statisticReport_ (this,
                     ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 , convert_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING))
 , resize_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING))
 , sobelFilter_ (this,
                 ACE_TEXT_ALWAYS_CHAR ("SobelFilter"))
#if defined (OLC_PGE_SUPPORT)
 , marchingSquaresFilter_ (this,
                           ACE_TEXT_ALWAYS_CHAR ("MarchingSquaresFilter"))
#endif // OLC_PGE_SUPPORT
#if defined (LIBNOISE_SUPPORT)
 , perlinNoiseFilter_ (this,
                       ACE_TEXT_ALWAYS_CHAR ("PerlinNoiseFilter"))
#endif // LIBNOISE_SUPPORT
#if defined (GTK_SUPPORT)
 , GTKDisplay_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_WINDOW_DEFAULT_NAME_STRING))
#endif // GTK_SUPPORT
 , GDIDisplay_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GDI_DEFAULT_NAME_STRING))
 , Direct2DDisplay_ (this,
                     ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT2D_DEFAULT_NAME_STRING))
 , Direct3DDisplay_ (this,
                     ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING))
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
 , DirectShowDisplay_ (this,
                       ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING))
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
#if defined (GLUT_SUPPORT)
 , OpenGLDisplay_ (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_ (this,
                 ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_2 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_3 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_4 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_5 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_6 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_7 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_8 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_9 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_10 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_11 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_12 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_13 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_14 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_15 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_16 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_17 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_18 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_19 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_20 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_21 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_22 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_23 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_24 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_25 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_26 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_27 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_28 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_29 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_30 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_31 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_32 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_33 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_34 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_35 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_36 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_37 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_38 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_39 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_40 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_41 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_42 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_43 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_44 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_45 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_46 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_47 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_48 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_49 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_50 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
#endif // GLUT_SUPPORT
#if defined (JC_VORONOI_SUPPORT) && defined (OLC_PGE_SUPPORT)
 , weightedVoronoiStippleFilter_ (this,
                                  ACE_TEXT_ALWAYS_CHAR ("WeightedVoronoiStippleFilter"))
#endif // JC_VORONOI_SUPPORT && OLC_PGE_SUPPORT
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_Stream::Test_U_DirectShow_Stream"));

}

bool
Test_U_DirectShow_Stream::load (Stream_ILayout* layout_in,
                                bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  layout_in->append (&source_, NULL, 0);
  layout_in->append (&convert_, NULL, 0);
  //layout_in->append (&statisticReport_, NULL, 0);

  bool add_renderer_b = true;
  switch (inherited::configuration_->configuration_->mode)
  {
    case TEST_U_MODE_SOBEL:
    {
      layout_in->append (&sobelFilter_, NULL, 0);
      break;
    }
#if defined (LIBNOISE_SUPPORT)
    case TEST_U_MODE_PERLIN_NOISE:
    {
      layout_in->append (&perlinNoiseFilter_, NULL, 0);
      break;
    }
#endif // LIBNOISE_SUPPORT
#if defined (OLC_PGE_SUPPORT)
    case TEST_U_MODE_MARCHING_SQUARES:
    {
      layout_in->append (&marchingSquaresFilter_, NULL, 0);
      add_renderer_b = false;
      break;
    }
#endif // OLC_PGE_SUPPORT
#if defined (JC_VORONOI_SUPPORT) && defined (OLC_PGE_SUPPORT)
    case TEST_U_MODE_WEIGHTED_VORONOI_STIPPLE:
    {
      layout_in->append (&weightedVoronoiStippleFilter_, NULL, 0);
      add_renderer_b = false;
      break;
    }
#endif // JC_VORONOI_SUPPORT && OLC_PGE_SUPPORT
#if defined (GLUT_SUPPORT)
    case TEST_U_MODE_GLUT:
    {
      //layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
      layout_in->append (&GLUTDisplay_, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_2:
    {
      layout_in->append (&GLUTDisplay_2, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_3:
    {
      layout_in->append (&GLUTDisplay_3, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_4:
    {
      layout_in->append (&GLUTDisplay_4, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_5:
    {
      layout_in->append (&GLUTDisplay_5, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_6:
    {
      layout_in->append (&GLUTDisplay_6, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_7:
    {
      layout_in->append (&GLUTDisplay_7, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_8:
    {
      layout_in->append (&GLUTDisplay_8, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_9:
    {
      layout_in->append (&GLUTDisplay_9, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_10:
    {
      layout_in->append (&GLUTDisplay_10, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_11:
    {
      layout_in->append (&GLUTDisplay_11, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_12:
    {
      layout_in->append (&GLUTDisplay_12, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_13:
    {
      layout_in->append (&GLUTDisplay_13, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_14:
    {
      layout_in->append (&GLUTDisplay_14, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_15:
    {
      layout_in->append (&GLUTDisplay_15, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_16:
    {
      layout_in->append (&GLUTDisplay_16, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_17:
    {
      layout_in->append (&GLUTDisplay_17, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_18:
    {
      layout_in->append (&GLUTDisplay_18, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_19:
    {
      layout_in->append (&GLUTDisplay_19, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_20:
    {
      layout_in->append (&GLUTDisplay_20, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_21:
    {
      layout_in->append (&GLUTDisplay_21, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_22:
    {
      layout_in->append (&GLUTDisplay_22, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_23:
    {
      layout_in->append (&GLUTDisplay_23, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_24:
    {
      layout_in->append (&GLUTDisplay_24, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_25:
    {
      layout_in->append (&GLUTDisplay_25, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_26:
    {
      layout_in->append (&GLUTDisplay_26, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_27:
    {
      layout_in->append (&GLUTDisplay_27, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_28:
    {
      layout_in->append (&GLUTDisplay_28, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_29:
    {
      layout_in->append (&GLUTDisplay_29, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_30:
    {
      layout_in->append (&GLUTDisplay_30, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_31:
    {
      layout_in->append (&GLUTDisplay_31, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_32:
    {
      layout_in->append (&GLUTDisplay_32, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_33:
    {
      layout_in->append (&GLUTDisplay_33, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_34:
    {
      layout_in->append (&GLUTDisplay_34, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_35:
    {
      layout_in->append (&GLUTDisplay_35, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_36:
    {
      layout_in->append (&GLUTDisplay_36, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_37:
    {
      layout_in->append (&GLUTDisplay_37, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_38:
    {
      layout_in->append (&GLUTDisplay_38, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_39:
    {
      layout_in->append (&GLUTDisplay_39, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_40:
    {
      layout_in->append (&GLUTDisplay_40, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_41:
    {
      layout_in->append (&GLUTDisplay_41, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_42:
    {
      layout_in->append (&GLUTDisplay_42, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_43:
    {
      layout_in->append (&GLUTDisplay_43, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_44:
    {
      layout_in->append (&GLUTDisplay_44, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_45:
    {
      layout_in->append (&GLUTDisplay_45, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_46:
    {
      layout_in->append (&GLUTDisplay_46, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_47:
    {
      layout_in->append (&GLUTDisplay_47, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_48:
    {
      layout_in->append (&GLUTDisplay_48, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_49:
    {
      layout_in->append (&GLUTDisplay_49, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_50:
    {
      layout_in->append (&GLUTDisplay_50, NULL, 0);
      add_renderer_b = false;
      break;
    }
#endif // GLUT_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown mode (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->mode));
      return false;
    }
  } // end SWITCH
  if (!add_renderer_b)
    goto continue_;

  switch (inherited::configuration_->configuration_->renderer)
  {
#if defined (GTK_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW:
    {
      layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
      layout_in->append (&GTKDisplay_, NULL, 0);
      break;
    }
#endif // GTK_SUPPORT
    case STREAM_VISUALIZATION_VIDEORENDERER_GDI:
    {
      layout_in->append (&GDIDisplay_, NULL, 0);
      break;
    }
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D:
    {
      layout_in->append (&Direct2DDisplay_, NULL, 0);
      break;
    }
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
    {
      layout_in->append (&Direct3DDisplay_, NULL, 0);
      break;
    }
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTSHOW:
    {
      layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
      layout_in->append (&DirectShowDisplay_, NULL, 0);
      break;
    }
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
#if defined (GLUT_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT:
#endif // GLUT_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->renderer));
      return false;
    }
  } // end SWITCH

continue_:
  return true;
}

bool
Test_U_DirectShow_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_U_CameraFilter_DirectShow_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator, iterator_2;
  Test_U_DirectShow_Source* source_impl_p = NULL;
  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  ISampleGrabber* isample_grabber_p = NULL;
  std::string log_file_name;
  struct _AMMediaType media_type_s;

  iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  iterator_2 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (Stream_Visualization_Tools::rendererToModuleName (configuration_in.configuration_->renderer));
  // sanity check(s)
  ACE_ASSERT (iterator != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());
  ACE_ASSERT (iterator_2 != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());

  // ---------------------------------------------------------------------------
  // step1: set up directshow filter graph
  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED     |
                              COINIT_DISABLE_OLE1DDE   |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF
  COM_initialized = true;

  if ((*iterator).second.second->builder)
  {
    // *NOTE*: Stream_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    if (!Stream_MediaFramework_DirectShow_Tools::reset ((*iterator).second.second->builder,
                                                        CLSID_VideoInputDeviceCategory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::reset(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

    if (!Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation ((*iterator).second.second->builder,
                                                                       STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L,
                                                                       buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation(), continuing\n"),
                  ACE_TEXT (stream_name_string_)));
      //goto error;
    } // end IF
    //ACE_ASSERT (buffer_negotiation_p);

    goto continue_;
  } // end IF

  ACE_ASSERT ((*iterator).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph ((*iterator).second.second->deviceIdentifier,
                                                        CLSID_VideoInputDeviceCategory,
                                                        (*iterator).second.second->builder,
                                                        buffer_negotiation_p,
                                                        stream_config_p,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second->deviceIdentifier.identifier._string)));
    goto error;
  } // end IF
  ACE_ASSERT ((*iterator).second.second->builder);
  //ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (stream_config_p);
  stream_config_p->Release (); stream_config_p = NULL;

continue_:
  if (!Stream_Device_DirectShow_Tools::setCaptureFormat ((*iterator).second.second->builder,
                                                         CLSID_VideoInputDeviceCategory,
                                                         configuration_in.configuration_->captureFormat))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  //// sanity check(s)
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration);

  //if (!Stream_Device_Tools::getDirect3DDevice (*(*iterator).second.second->direct3DConfiguration,
  //                                                    direct3D_manager_p,
  //                                                    reset_token))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_Device_Tools::getDirect3DDevice(), aborting\n"),
  //              ACE_TEXT (stream_name_string_)));
  //  goto error;
  //} // end IF
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration->handle);
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration->handle);
  //ACE_ASSERT (direct3D_manager_p);
  //ACE_ASSERT (reset_token);
  //direct3D_manager_p->Release (); direct3D_manager_p = NULL;

  if (!Stream_Module_Decoder_Tools::loadVideoRendererGraph (CLSID_VideoInputDeviceCategory,
                                                            configuration_in.configuration_->captureFormat,
                                                            configuration_in.configuration_->outputFormat,
                                                            NULL, // use NULL-VideoRenderer
                                                            //(*iterator).second.second->window,
                                                            (*iterator).second.second->builder,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadVideoRendererGraph(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  result_2 =
    (*iterator).second.second->builder->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                                         &filter_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                       (void**)&isample_grabber_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (isample_grabber_p);
  filter_p->Release (); filter_p = NULL;

  result_2 = isample_grabber_p->SetBufferSamples (false);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 = isample_grabber_p->SetCallback (source_impl_p, 0);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  isample_grabber_p->Release (); isample_grabber_p = NULL;

  if (buffer_negotiation_p)
  {
    ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
    // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
    //         if this is -1/0 (why ?)
    allocator_properties.cbAlign = 1;
    allocator_properties.cbBuffer =
      configuration_in.configuration_->allocatorConfiguration->defaultBufferSize;
    allocator_properties.cbPrefix = -1; // <-- use default
    allocator_properties.cBuffers =
      STREAM_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
    result_2 =
        buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
    if (FAILED (result_2)) // E_UNEXPECTED: 0x8000FFFF --> graph already connected
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      goto error;
    } // end IF
  } // end IF

  if (!Stream_MediaFramework_DirectShow_Tools::connect ((*iterator).second.second->builder,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  //         --> reconnect the AVI decompressor to the (connected) sample
  //             grabber; this seems to work
  if (!Stream_MediaFramework_DirectShow_Tools::connected ((*iterator).second.second->builder,
                                                          STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reconnecting...\n"),
                ACE_TEXT (stream_name_string_)));

    if (!Stream_MediaFramework_DirectShow_Tools::connectFirst ((*iterator).second.second->builder,
                                                               STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connectFirst(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::connected ((*iterator).second.second->builder,
                                                                 STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L));

  if (buffer_negotiation_p)
  {
#if defined (_DEBUG)
    ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
    result_2 =
        buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
    if (FAILED (result_2)) // E_FAIL (0x80004005)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s/%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      //goto error;
    } // end IF
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: negotiated allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
                  ACE_TEXT (stream_name_string_),
                  allocator_properties.cBuffers,
                  allocator_properties.cbBuffer,
                  allocator_properties.cbAlign,
                  allocator_properties.cbPrefix));
#endif // _DEBUG
    buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;
  } // end IF

  result_2 =
    (*iterator).second.second->builder->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result_2 = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release (); media_filter_p = NULL;

  // ---------------------------------------------------------------------------
  // step3: allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration);

  session_data_p =
    &const_cast<Test_U_CameraFilter_DirectShow_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  //if ((*iterator).second.second->direct3DConfiguration->handle)
  //{
  //  (*iterator).second.second->direct3DConfiguration->handle->AddRef ();
  //  session_data_p->direct3DDevice =
  //    (*iterator).second.second->direct3DConfiguration->handle;
  //} // end IF
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------
  // step4: initialize module(s)

  // ******************* Camera Source ************************
  source_impl_p =
    dynamic_cast<Test_U_DirectShow_Source*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_CamSave_DirectShow_Source> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  // ---------------------------------------------------------------------------
  // step5: update session data
  session_data_p->formats.push_back (configuration_in.configuration_->outputFormat);
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  if (!Stream_MediaFramework_DirectShow_Tools::getOutputFormat ((*iterator).second.second->builder,
                                                                STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                                                media_type_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getOutputFormat(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L)));
    goto error;
  } // end IF
  session_data_p->formats.push_back (media_type_s);
  //ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::matchMediaType (*session_data_p->sourceFormat, *(*iterator).second.second->sourceFormat));

  // ---------------------------------------------------------------------------
  // step6: initialize head module
  //source_impl_p->setP (&(inherited::state_));
  ////fileReader_impl_p->reset ();
  //// *NOTE*: push()ing the module will open() it
  ////         --> set the argument that is passed along (head module expects a
  ////             handle to the session data)
  //source_.arg (inherited::sessionData_);

  // step7: assemble stream
  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_r.fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  if ((*iterator).second.second->builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF
  if (session_data_p)
  {
    if (session_data_p->direct3DDevice)
    {
      session_data_p->direct3DDevice->Release (); session_data_p->direct3DDevice = NULL;
    } // end IF
    Stream_MediaFramework_DirectShow_Tools::free (session_data_p->formats);
    session_data_p->resetToken = 0;
  } // end IF

  if (COM_initialized)
    CoUninitialize ();

  return false;
}

//////////////////////////////////////////

Test_U_MediaFoundation_Stream::Test_U_MediaFoundation_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING))
 //, statisticReport_ (this,
 //                    ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 , display_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING))
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
 , mediaSession_ (NULL)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::Test_U_MediaFoundation_Stream"));

}

Test_U_MediaFoundation_Stream::~Test_U_MediaFoundation_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::~Test_U_MediaFoundation_Stream"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result) &&
        (result != MF_E_SHUTDOWN)) // already shut down...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

const Stream_Module_t*
Test_U_MediaFoundation_Stream::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::find"));

  //if (!ACE_OS::strcmp (name_in.c_str (),
  //                     ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_RENDERER_NULL_MODULE_NAME)))
  //  return const_cast<Test_U_MediaFoundation_DisplayNull_Module*> (&displayNull_);

  return inherited::find (name_in);
}

void
Test_U_MediaFoundation_Stream::start ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::start"));

  // sanity check(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (mediaSession_);

  struct _GUID GUID_s = GUID_NULL;
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  HRESULT result = mediaSession_->Start (&GUID_s,      // time format
                                         &property_s); // start position
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::Start(): \"%s\", returning\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    PropVariantClear (&property_s);
    return;
  } // end IF
  PropVariantClear (&property_s);

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  inherited::start ();
}

void
Test_U_MediaFoundation_Stream::stop (bool waitForCompletion_in,
                                                  bool recurseUpstream_in,
                                                  bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::stop"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  inherited::stop (waitForCompletion_in,
                   recurseUpstream_in,
                   highPriority_in);
}

HRESULT
Test_U_MediaFoundation_Stream::QueryInterface (const IID& IID_in,
                                                            void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (Test_U_MediaFoundation_Stream, IMFAsyncCallback),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}
ULONG
Test_U_MediaFoundation_Stream::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
ULONG
Test_U_MediaFoundation_Stream::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0);
  //delete this;

  return count;
}

HRESULT
Test_U_MediaFoundation_Stream::GetParameters (DWORD* flags_out,
                                                           DWORD* queue_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::GetParameters"));

  ACE_UNUSED_ARG (flags_out);
  ACE_UNUSED_ARG (queue_out);

  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  return E_NOTIMPL;
}

HRESULT
Test_U_MediaFoundation_Stream::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::Invoke"));

  HRESULT result = E_FAIL;
  IMFMediaEvent* media_event_p = NULL;
  MediaEventType event_type = MEUnknown;
  HRESULT status = E_FAIL;
  struct tagPROPVARIANT value;
  PropVariantInit (&value);

  // sanity check(s)
  ACE_ASSERT (result_in);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (inherited::sessionData_);

  //Test_U_SessionData& session_data_r =
  //  const_cast<Test_U_SessionData&> (inherited::sessionData_->get ());

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (media_event_p);
  result = media_event_p->GetType (&event_type);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetStatus (&status);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetValue (&value);
  ACE_ASSERT (SUCCEEDED (result));
  switch (event_type)
  {
    case MEEndOfPresentation:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MEEndOfPresentation\n"),
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MEError:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: received MEError: \"%s\"\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionClosed:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                  ACE_TEXT (stream_name_string_)));
      //IMFMediaSource* media_source_p = NULL;
      //if (!Stream_Device_Tools::getMediaSource (mediaSession_,
      //                                                 media_source_p))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Device_Tools::getMediaSource(), continuing\n")));
      //  goto continue_;
      //} // end IF
      //ACE_ASSERT (media_source_p);
      //result = media_source_p->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSource::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      //media_source_p->Release (); media_source_p = NULL;
  //continue_:
      // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
      //result = mediaSession_->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case MESessionEnded:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionEnded, closing sesion\n"),
                  ACE_TEXT (stream_name_string_)));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      result = mediaSession_->Close ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                    ACE_TEXT (stream_name_string_),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      break;
    }
    case MESessionCapabilitiesChanged:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionCapabilitiesChanged\n"),
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MESessionNotifyPresentationTime:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionNotifyPresentationTime\n"),
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MESessionStarted:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStarted\n"),
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MESessionStopped:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStopped, stopping\n"),
                  ACE_TEXT (stream_name_string_)));
      if (isRunning ())
        stop (false, // wait ?
              true,  // high priority ?
              true); // locked access ?
      break;
    }
    case MESessionTopologySet:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologySet (status was: \"%s\")\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionTopologyStatus:
    {
      UINT32 attribute_value = 0;
      result = media_event_p->GetUINT32 (MF_EVENT_TOPOLOGY_STATUS,
                                         &attribute_value);
      ACE_ASSERT (SUCCEEDED (result));
      MF_TOPOSTATUS topology_status =
        static_cast<MF_TOPOSTATUS> (attribute_value);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologyStatus: \"%s\"\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (topology_status).c_str ())));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: received unknown/invalid media session event (type was: %d), continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  event_type));
      break;
    }
  } // end SWITCH
  PropVariantClear (&value);
  media_event_p->Release (); media_event_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  return S_OK;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
error:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}

bool
Test_U_MediaFoundation_Stream::load (Stream_ILayout* layout_in,
                                                  bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // *NOTE*: one problem is that any module that was NOT enqueued onto the
  //         stream (e.g. because initialize() failed) needs to be explicitly
  //         close()d
  layout_in->append (&source_, NULL, 0);
  //modules_out.push_back (&statisticReport_);
  layout_in->append (&display_, NULL, 0);

  return true;
}

bool
Test_U_MediaFoundation_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_U_CameraFilter_MediaFoundation_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator;
  Test_U_MediaFoundation_Source* source_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
    &const_cast<Test_U_CameraFilter_MediaFoundation_SessionData&> (inherited::sessionData_->getR ());
  iterator =
      const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  // *TODO*: remove type inferences
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Camera Source ************************
  source_impl_p =
    dynamic_cast<Test_U_MediaFoundation_Source*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_CamSave_MediaFoundation_CamSource> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  IMFTopology* topology_p = NULL;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFMediaType* media_type_p = NULL;
  TOPOID node_id = 0, node_id_2 = 0;

  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  COM_initialized = true;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if ((*iterator).second.second->session)
  {
    ULONG reference_count = (*iterator).second.second->session->AddRef ();
    mediaSession_ = (*iterator).second.second->session;

    if (!Stream_MediaFramework_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

    // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
    //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
    //         --> (try to) wait for the next MESessionTopologySet event
    // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
    //         still fails with MF_E_INVALIDREQUEST)
    do
    {
      result_2 = mediaSession_->GetFullTopology (flags,
                                                 0,
                                                 &topology_p);
    } while (result_2 == MF_E_INVALIDREQUEST);
    if (FAILED (result_2)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    ACE_ASSERT (topology_p);

    //if ((*iterator).second.second->sampleGrabberNodeId)
    //  goto continue_;
    if (!Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                              node_id))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    ACE_ASSERT (node_id);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    goto continue_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  if (!Stream_Module_Decoder_Tools::loadVideoRendererTopology ((*iterator).second.second->deviceIdentifier,
                                                               configuration_in.configuration_->format,
                                                               source_impl_p,
                                                               NULL,
                                                               //configuration_p->window,
                                                               node_id,
                                                               node_id_2,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadVideoRendererTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;
#if defined (_DEBUG)
  Stream_MediaFramework_MediaFoundation_Tools::dump (topology_p);
#endif // _DEBUG

continue_:
  if (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                              configuration_in.configuration_->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: capture format: \"%s\"\n"),
              ACE_TEXT (stream_name_string_),
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (configuration_in.configuration_->format).c_str ())));
#endif // _DEBUG

  ACE_ASSERT (session_data_p->formats.empty ());
  media_type_p =
    Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration_->format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  session_data_p->formats.push_back (media_type_p);
  media_type_p = NULL;

  if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                     node_id,
                                                                     media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  session_data_p->formats.push_back (media_type_p);
  media_type_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  //HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
  ACE_ASSERT (!mediaSession_);
  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 mediaSession_,
                                                                 true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (!(*iterator).second.second->session)
  {
    ULONG reference_count = mediaSession_->AddRef ();
    (*iterator).second.second->session = mediaSession_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  source_.arg (inherited::sessionData_);

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (session_data_p->direct3DDevice)
  {
    session_data_p->direct3DDevice->Release (); session_data_p->direct3DDevice = NULL;
  } // end IF
  session_data_p->direct3DManagerResetToken = 0;
  Stream_MediaFramework_MediaFoundation_Tools::free (session_data_p->formats);
  if (session_data_p->session)
  {
    session_data_p->session->Release (); session_data_p->session = NULL;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  if (COM_initialized)
    CoUninitialize ();

  return false;
}
#else
Test_U_Stream::Test_U_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING))
 , statisticReport_ (this,
                     ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 , convert_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING))
 , resize_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING))
 , sobelFilter_ (this,
                 ACE_TEXT_ALWAYS_CHAR ("SobelFilter"))
#if defined (OLC_PGE_SUPPORT)
 , marchingSquaresFilter_ (this,
                           ACE_TEXT_ALWAYS_CHAR ("MarchingSquaresFilter"))
#endif // OLC_PGE_SUPPORT
#if defined (LIBNOISE_SUPPORT)
 , perlinNoiseFilter_ (this,
                       ACE_TEXT_ALWAYS_CHAR ("PerlinNoiseFilter"))
#endif // LIBNOISE_SUPPORT
#if defined (GTK_SUPPORT)
 , GTKDisplay_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_WINDOW_DEFAULT_NAME_STRING))
#endif // GTK_SUPPORT
 , WaylandDisplay_ (this,
                    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_WAYLAND_WINDOW_DEFAULT_NAME_STRING))
 , X11Display_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING))
#if defined (GLUT_SUPPORT)
 , OpenGLDisplay_ (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_ (this,
                 ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_2 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_3 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_4 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_5 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_6 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_7 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_8 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_9 (this,
                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_10 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_11 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_12 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_13 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_14 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_15 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_16 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_17 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_18 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_19 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_20 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_21 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_22 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_23 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_24 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_25 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_26 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_27 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_28 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_29 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_30 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_31 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_32 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_33 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_34 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_35 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_36 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_37 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_38 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_39 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
 , GLUTDisplay_40 (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_OPENGL_GLUT_DEFAULT_NAME_STRING))
#endif // GLUT_SUPPORT
#if defined (JC_VORONOI_SUPPORT)
 , weightedVoronoiStippleFilter_ (this,
                                  ACE_TEXT_ALWAYS_CHAR ("WeightedVoronoiStippleFilter"))
#endif // JC_VORONOI_SUPPORT
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::Test_U_Stream"));

}

bool
Test_U_Stream::load (Stream_ILayout* layout_in,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);

  layout_in->append (&source_, NULL, 0);
  //layout_in->append (&statisticReport_, NULL, 0);
  layout_in->append (&convert_, NULL, 0);

  bool add_renderer_b = true;
  switch (inherited::configuration_->configuration_->mode)
  {
    case TEST_U_MODE_SOBEL:
    {
      layout_in->append (&sobelFilter_, NULL, 0);
      break;
    }
#if defined (LIBNOISE_SUPPORT)
    case TEST_U_MODE_PERLIN_NOISE:
    {
      layout_in->append (&perlinNoiseFilter_, NULL, 0);
      break;
    }
#endif // LIBNOISE_SUPPORT
#if defined (OLC_PGE_SUPPORT)
    case TEST_U_MODE_MARCHING_SQUARES:
    {
      layout_in->append (&marchingSquaresFilter_, NULL, 0);
      add_renderer_b = false;
      break;
    }
#endif // OLC_PGE_SUPPORT
#if defined (JC_VORONOI_SUPPORT) && defined (OLC_PGE_SUPPORT)
    case TEST_U_MODE_WEIGHTED_VORONOI_STIPPLE:
    {
      layout_in->append (&weightedVoronoiStippleFilter_, NULL, 0);
      add_renderer_b = false;
      break;
    }
#endif // JC_VORONOI_SUPPORT && OLC_PGE_SUPPORT
#if defined (GLUT_SUPPORT)
    case TEST_U_MODE_GLUT:
    {
      //layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
      layout_in->append (&OpenGLDisplay_, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_2:
    {
      layout_in->append (&GLUTDisplay_2, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_3:
    {
      layout_in->append (&GLUTDisplay_3, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_4:
    {
      layout_in->append (&GLUTDisplay_4, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_5:
    {
      layout_in->append (&GLUTDisplay_5, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_6:
    {
      layout_in->append (&GLUTDisplay_6, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_7:
    {
      layout_in->append (&GLUTDisplay_7, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_8:
    {
      layout_in->append (&GLUTDisplay_8, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_9:
    {
      layout_in->append (&GLUTDisplay_9, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_10:
    {
      layout_in->append (&GLUTDisplay_10, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_11:
    {
      layout_in->append (&GLUTDisplay_11, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_12:
    {
      layout_in->append (&GLUTDisplay_12, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_13:
    {
      layout_in->append (&GLUTDisplay_13, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_14:
    {
      layout_in->append (&GLUTDisplay_14, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_15:
    {
      layout_in->append (&GLUTDisplay_15, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_16:
    {
      layout_in->append (&GLUTDisplay_16, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_17:
    {
      layout_in->append (&GLUTDisplay_17, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_18:
    {
      layout_in->append (&GLUTDisplay_18, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_19:
    {
      layout_in->append (&GLUTDisplay_19, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_20:
    {
      layout_in->append (&GLUTDisplay_20, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_21:
    {
      layout_in->append (&GLUTDisplay_21, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_22:
    {
      layout_in->append (&GLUTDisplay_22, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_23:
    {
      layout_in->append (&GLUTDisplay_23, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_24:
    {
      layout_in->append (&GLUTDisplay_24, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_25:
    {
      layout_in->append (&GLUTDisplay_25, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_26:
    {
      layout_in->append (&GLUTDisplay_26, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_27:
    {
      layout_in->append (&GLUTDisplay_27, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_28:
    {
      layout_in->append (&GLUTDisplay_28, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_29:
    {
      layout_in->append (&GLUTDisplay_29, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_30:
    {
      layout_in->append (&GLUTDisplay_30, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_31:
    {
      layout_in->append (&GLUTDisplay_31, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_32:
    {
      layout_in->append (&GLUTDisplay_32, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_33:
    {
      layout_in->append (&GLUTDisplay_33, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_34:
    {
      layout_in->append (&GLUTDisplay_34, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_35:
    {
      layout_in->append (&GLUTDisplay_35, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_36:
    {
      layout_in->append (&GLUTDisplay_36, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_37:
    {
      layout_in->append (&GLUTDisplay_37, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_38:
    {
      layout_in->append (&GLUTDisplay_38, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_39:
    {
      layout_in->append (&GLUTDisplay_39, NULL, 0);
      add_renderer_b = false;
      break;
    }
    case TEST_U_MODE_GLUT_40:
    {
      layout_in->append (&GLUTDisplay_40, NULL, 0);
      add_renderer_b = false;
      break;
    }
#endif // GLUT_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown mode (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->mode));
      return false;
    }
  } // end SWITCH
  if (!add_renderer_b)
    goto continue_;

  // layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
  switch (inherited::configuration_->configuration_->renderer)
  {
#if defined (GTK_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW:
      layout_in->append (&GTKDisplay_, NULL, 0);
      break;
#endif // GTK_SUPPORT
    case STREAM_VISUALIZATION_VIDEORENDERER_WAYLAND:
      layout_in->append (&WaylandDisplay_, NULL, 0);
      break;
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
      layout_in->append (&X11Display_, NULL, 0);
      break;
#if defined (GLUT_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT:
      layout_in->append (&OpenGLDisplay_, NULL, 0);
      break;
#endif // GLUT_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->renderer));
      return false;
    }
  } // end SWITCH

continue_:
  return true;
}

bool
Test_U_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  ACE_ASSERT (configuration_in.configuration_);
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_U_CameraFilter_V4L_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
//  Test_U_V4L_Source* source_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
    &const_cast<Test_U_CameraFilter_V4L_SessionData&> (inherited::sessionData_->getR ());
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  // *TODO*: remove type inferences
  ACE_ASSERT (session_data_p->formats.empty ());
  session_data_p->formats.push_back (configuration_in.configuration_->format);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
#endif // ACE_WIN32 || ACE_WIN64
