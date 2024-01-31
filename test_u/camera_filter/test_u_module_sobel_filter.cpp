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

#include "test_u_module_sobel_filter.h"

#include <utility>

#include "ace/Log_Msg.h"

#include "common_image_tools.h"

#include "stream_macros.h"

const char libacestream_default_sobel_filter_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR ("SobelFilter");

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_Sobel_Filter::Test_U_CameraFilter_Sobel_Filter (ISTREAM_T* stream_in)
#else
Test_U_CameraFilter_Sobel_Filter::Test_U_CameraFilter_Sobel_Filter (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , buffer_ (NULL)
 , bytesPerPixel_ (4)
 , frameCount_ (0)
 , resolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_Sobel_Filter::Test_U_CameraFilter_Sobel_Filter"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_Sobel_Filter::handleDataMessage (Test_U_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraFilter_Sobel_Filter::handleDataMessage (Test_U_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_Sobel_Filter::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  int index_i;
  uint8_t tlr, tlg, tlb, lr, lg, lb, blr, blg, blb, tr, tg, tb, br, bg, bb, trr, trg, trb, rr, rg, rb, brr, brg, brb;
  float tl, l, bl, t, b, tr_2, r, br_2;
  float Gx, Gy, G;
  float theta_f;
  float hue_f;
  float r_f, g_f, b_f;
  for (int x = 1; x < resolution_.cx - 1; ++x)
    for (int y = 1; y < resolution_.cy - 1; ++y)
    {
      index_i = ((x - 1) + ((y - 1) * resolution_.cx)) * bytesPerPixel_;
      tlr = data_p[index_i];
      tlg = data_p[index_i + 1];
      tlb = data_p[index_i + 2];

      index_i = ((x - 1) + (y * resolution_.cx)) * bytesPerPixel_;
      lr = data_p[index_i];
      lg = data_p[index_i + 1];
      lb = data_p[index_i + 2];

      index_i = ((x - 1) + ((y + 1) * resolution_.cx)) * bytesPerPixel_;
      blr = data_p[index_i];
      blg = data_p[index_i + 1];
      blb = data_p[index_i + 2];

      index_i = (x + ((y - 1) * resolution_.cx)) * bytesPerPixel_;
      tr = data_p[index_i];
      tg = data_p[index_i + 1];
      tb = data_p[index_i + 2];

      index_i = (x + ((y + 1) * resolution_.cx)) * bytesPerPixel_;
      br = data_p[index_i];
      bg = data_p[index_i + 1];
      bb = data_p[index_i + 2];

      index_i = ((x + 1) + ((y - 1) * resolution_.cx)) * bytesPerPixel_;
      trr = data_p[index_i];
      trg = data_p[index_i + 1];
      trb = data_p[index_i + 2];

      index_i = ((x + 1) + (y * resolution_.cx)) * bytesPerPixel_;
      rr = data_p[index_i];
      rg = data_p[index_i + 1];
      rb = data_p[index_i + 2];

      index_i = ((x + 1) + ((y + 1) * resolution_.cx)) * bytesPerPixel_;
      brr = data_p[index_i];
      brg = data_p[index_i + 1];
      brb = data_p[index_i + 2];

      tl = (tlr + tlg + tlb) / 3.0f;
      l = (lr + lg + lb) / 3.0f;
      bl = (blr + blg + blb) / 3.0f;
      t = (tr + tg + tb) / 3.0f;
      b = (br + bg + bb) / 3.0f;
      tr_2 = (trr + trg + trb) / 3.0f;
      r = (rr + rg + rb) / 3.0f;
      br_2 = (brr + brg + brb) / 3.0f;

      Gx = tl + 2.0f * l + bl - tr_2 - 2.0f * r - br_2;
      Gy = tl + 2.0f * t + tr_2 - bl - 2.0f * b - br_2;
      G = std::sqrt ((Gx * Gx) + (Gy * Gy));
      theta_f = std::atan (Gy / Gx);
      hue_f =
        (std::fmod ((theta_f + static_cast<float> (M_PI_2)) + (frameCount_ / 12.0f), static_cast<float> (M_PI)) / static_cast<float> (M_PI)) * 360.0f;
      Common_Image_Tools::HSVToRGB (hue_f, 1.0f, G / 255.0f, r_f, g_f, b_f);

      index_i = (x + (y * resolution_.cx)) * bytesPerPixel_;
      buffer_[index_i]     = static_cast<uint8_t> (r_f * 255.0f);
      buffer_[index_i + 1] = static_cast<uint8_t> (g_f * 255.0f);
      buffer_[index_i + 2] = static_cast<uint8_t> (b_f * 255.0f);
    } // end FOR
  ACE_OS::memcpy (data_p, buffer_, resolution_.cx * resolution_.cy * bytesPerPixel_ * sizeof (uint8_t));

  ++frameCount_;
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_Sobel_Filter::handleSessionMessage (Test_U_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraFilter_Sobel_Filter::handleSessionMessage (Test_U_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_Sobel_Filter::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename Test_U_DirectShow_SessionMessage_t::DATA_T::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
      const struct _AMMediaType& media_type_r = session_data_r.formats.back ();
      ACE_ASSERT (Stream_MediaFramework_Tools::isRGB (media_type_r.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW));

      bytesPerPixel_ =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_r) / 8;
      frameCount_ = 0;
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_r);

      buffer_ = new uint8_t[resolution_.cx * resolution_.cy * bytesPerPixel_];

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      delete [] buffer_; buffer_ = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}
