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

#include "test_u_module_perlin_noise_filter.h"

#include "ace/Log_Msg.h"

#include "common_gl_tools.h"

#include "common_image_tools.h"

#include "stream_macros.h"

const char libacestream_default_perlin_noise_filter_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR ("PerlinNoiseFilter");

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_PerlinNoise_Filter::Test_U_CameraFilter_PerlinNoise_Filter (ISTREAM_T* stream_in)
#else
Test_U_CameraFilter_PerlinNoise_Filter::Test_U_CameraFilter_PerlinNoise_Filter (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , bytesPerPixel_ (4)
 , xOffset_ (0.0f)
 , yOffset_ (0.0f)
 , zOffset_ (0.0f)
 , resolution_ ()
 , noise_ ()
 , noise2_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_PerlinNoise_Filter::Test_U_CameraFilter_PerlinNoise_Filter"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_PerlinNoise_Filter::handleDataMessage (Test_U_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraFilter_PerlinNoise_Filter::handleDataMessage (Test_U_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_PerlinNoise_Filter::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  
  float r_f, g_f, b_f, hue_f, saturation_f, value_f, x_f, y_f, z_f, value_2;
  uint8_t r, g, b;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  for (int i = 0; i < resolution_.cx; i++)
    for (int j = 0; j < resolution_.cy; j++)
#else
  for (int i = 0; i < static_cast<int> (resolution_.width); i++)
    for (int j = 0; j < static_cast<int> (resolution_.height); j++)
#endif // ACE_WIN32 || ACE_WIN64
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      r = data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 0];
      g = data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 1];
      b = data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 2];
#else
      r = data_p[(j * resolution_.width + i) * bytesPerPixel_ + 0];
      g = data_p[(j * resolution_.width + i) * bytesPerPixel_ + 1];
      b = data_p[(j * resolution_.width + i) * bytesPerPixel_ + 2];
#endif // ACE_WIN32 || ACE_WIN64

      r_f = r / 255.0f;
      g_f = g / 255.0f;
      b_f = b / 255.0f;
      Common_Image_Tools::RGBToHSV (r_f, g_f, b_f,
                                    hue_f, saturation_f, value_f);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      x_f = i / static_cast<float> (resolution_.cx) * ACESTREAM_PNF_ZOOM_FACTOR_F;
      y_f = j / static_cast<float> (resolution_.cy) * ACESTREAM_PNF_ZOOM_FACTOR_F;
#else
      x_f = i / static_cast<float> (resolution_.width) * ACESTREAM_PNF_ZOOM_FACTOR_F;
      y_f = j / static_cast<float> (resolution_.height) * ACESTREAM_PNF_ZOOM_FACTOR_F;
#endif // ACE_WIN32 || ACE_WIN64
      z_f = 0.0f;
      value_2 =
        //Common_GL_Tools::map (static_cast<float> (noise_.GetValue (x_f + xOffset_, y_f + yOffset_, z_f + zOffset_)),
        //                      -1.0f, 1.0f, 0.0f, 1.0f);
        Common_GL_Tools::map (static_cast<float> (noise2_.Evaluate (x_f + xOffset_, y_f + yOffset_, z_f + zOffset_)),
                              -1.0f, 1.0f, 0.0f, 1.0f);

      //hue_f *= value_2;
      //saturation_f *= value_2;
      value_f *= value_2;
      Common_Image_Tools::HSVToRGB (hue_f, saturation_f, value_f,
                                    r_f, g_f, b_f);
      r = static_cast<uint8_t> (255.0f * r_f);
      g = static_cast<uint8_t> (255.0f * g_f);
      b = static_cast<uint8_t> (255.0f * b_f);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 0] = r;
      data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 1] = g;
      data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 2] = b;
#else
      data_p[(j * resolution_.width + i) * bytesPerPixel_ + 0] = r;
      data_p[(j * resolution_.width + i) * bytesPerPixel_ + 1] = g;
      data_p[(j * resolution_.width + i) * bytesPerPixel_ + 2] = b;
#endif // ACE_WIN32 || ACE_WIN64
    } // end FOR

  xOffset_ += 0.01f;
  yOffset_ += 0.01f;
  zOffset_ += 0.03f;
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_PerlinNoise_Filter::handleSessionMessage (Test_U_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraFilter_PerlinNoise_Filter::handleSessionMessage (Test_U_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_PerlinNoise_Filter::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      const typename Test_U_DirectShow_SessionMessage_t::DATA_T::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
#else
      const typename Test_U_SessionMessage_t::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      const struct _AMMediaType& media_type_r = session_data_r.formats.back ();
      ACE_ASSERT (Stream_MediaFramework_Tools::isRGB (media_type_r.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW));

      bytesPerPixel_ =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_r) / 8;
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_r);
#else
      const struct Stream_MediaFramework_V4L_MediaType& media_type_r =
        session_data_r.formats.back ();
      ACE_ASSERT (Stream_MediaFramework_Tools::isRGB (media_type_r.format.pixelformat));

      bytesPerPixel_ =
        Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_r.format.pixelformat) / 8;
      resolution_.width = media_type_r.format.width;
      resolution_.height = media_type_r.format.height;
#endif // ACE_WIN32 || ACE_WIN64

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}
