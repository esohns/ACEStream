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

#include "test_u_camerascreen_module_video_wall.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "test_u_camerascreen_defines.h"
#include "test_u_camerascreen_message.h"
#include "test_u_camerascreen_session_message.h"

const char libacestream_default_video_wall_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR ("VideoWall");

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_VideoWall::Test_U_CameraScreen_VideoWall (ISTREAM_T* stream_in)
#else
Test_U_CameraScreen_VideoWall::Test_U_CameraScreen_VideoWall (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , bytesPerPixel_ (3)
 , messages_ ()
 , resolution_ ()
 , thumbnailResolution_ ()
 , numTrailingBlackPixelsPerRow_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_VideoWall::Test_U_CameraScreen_VideoWall"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_VideoWall::handleDataMessage (Stream_CameraScreen_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraScreen_VideoWall::handleDataMessage (Stream_CameraScreen_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_VideoWall::handleDataMessage"));

  passMessageDownstream_out = false;

  static int numb_thumbnails_i =
    TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_X * TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_Y;
  std::deque<ACE_Message_Block*>::iterator iterator;
  if (messages_.size () == static_cast<size_t> (numb_thumbnails_i))
  {
    iterator = messages_.end ();
    --iterator;
    (*iterator)->release ();
    messages_.erase (iterator);
  } // end ELSE
  messages_.push_front (message_inout);
  ACE_ASSERT (static_cast<int> (messages_.size ()) <= numb_thumbnails_i);

  size_t message_size_i =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    resolution_.cx * resolution_.cy * bytesPerPixel_;
#else
    resolution_.width * resolution_.height * bytesPerPixel_;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_CameraScreen_DirectShow_Message_t* message_p =
#else
  Stream_CameraScreen_Message_t* message_p =
#endif // ACE_WIN32 || ACE_WIN64
    inherited::allocateMessage (message_size_i);
  ACE_ASSERT (message_p);
  ACE_OS::memset (reinterpret_cast<void*> (message_p->wr_ptr ()), 0, message_size_i); // initialize to black

  int index_x, index_y, index_i, result;
  char* data_src_p;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  for (int y = 0; y < resolution_.cy; y++)
#else
  for (int y = 0; y < static_cast<int> (resolution_.height); y++)
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    index_y = y / thumbnailResolution_.cy;
#else
    index_y = y / thumbnailResolution_.height;
#endif // ACE_WIN32 || ACE_WIN64
    for (index_x = 0; index_x < TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_X;
         index_x++)
    {
      index_i = (index_y * TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_X) + index_x;
      if (index_i >= static_cast<int> (messages_.size ()))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        message_p->wr_ptr (thumbnailResolution_.cx * bytesPerPixel_);
#else
        message_p->wr_ptr (thumbnailResolution_.width * bytesPerPixel_);
#endif // ACE_WIN32 || ACE_WIN64
        continue;
      } // end IF
      iterator = messages_.begin ();
      std::advance (iterator, index_i);

      data_src_p = reinterpret_cast<char*> ((*iterator)->rd_ptr ());      
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      data_src_p += ((y % thumbnailResolution_.cy) * (thumbnailResolution_.cx * bytesPerPixel_));
      result = message_p->copy (data_src_p,
                                static_cast<size_t> (thumbnailResolution_.cx * bytesPerPixel_));
#else
      data_src_p += ((y % thumbnailResolution_.height) * (thumbnailResolution_.width * bytesPerPixel_));
      result = message_p->copy (data_src_p,
                                static_cast<size_t> (thumbnailResolution_.width * bytesPerPixel_));
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (result == 0);
    } // end FOR

    if (numTrailingBlackPixelsPerRow_)
    { // *NOTE*: no need to memset; it's already black (see above)
      //ACE_OS::memset (message_p->wr_ptr (), 0, numTrailingBlackPixelsPerRow_ * bytesPerPixel_);
      message_p->wr_ptr (numTrailingBlackPixelsPerRow_ * bytesPerPixel_);
    } // end IF
  } // end FOR

  result = inherited::put_next (message_p, NULL);
  ACE_ASSERT (result == 0);

  return;

//error:
  if (message_p)
    message_p->release ();
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_VideoWall::handleSessionMessage (Stream_CameraScreen_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraScreen_VideoWall::handleSessionMessage (Stream_CameraScreen_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_VideoWall::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      typename Stream_CameraScreen_DirectShow_SessionMessage_t::DATA_T::DATA_T& session_data_r =
        const_cast<typename Stream_CameraScreen_DirectShow_SessionMessage_t::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
#else
      typename Stream_CameraScreen_SessionMessage_t::DATA_T::DATA_T& session_data_r =
        const_cast<typename Stream_CameraScreen_SessionMessage_t::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (!session_data_r.formats.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      const struct _AMMediaType& media_type_r = session_data_r.formats.front ();
      const struct _AMMediaType& media_type_2 = session_data_r.formats.back ();
      ACE_ASSERT (Stream_MediaFramework_Tools::isRGB (media_type_2.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW));

      bytesPerPixel_ =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_2) / 8;
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_r);
      thumbnailResolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_2);
      numTrailingBlackPixelsPerRow_ =
        resolution_.cx - ((resolution_.cx / TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_X) * TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_X);

      struct _AMMediaType media_type_3;
      ACE_OS::memset (&media_type_3, 0, sizeof (struct _AMMediaType));
      Stream_MediaFramework_DirectShow_Tools::copy (media_type_r,
                                                    media_type_3);
#else
      const struct Stream_MediaFramework_V4L_MediaType& media_type_r =
        session_data_r.formats.front ();
      const struct Stream_MediaFramework_V4L_MediaType& media_type_2 =
        session_data_r.formats.back ();
      ACE_ASSERT (Stream_MediaFramework_Tools::isRGB (media_type_2.format.pixelformat));

      bytesPerPixel_ =
        Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_2.format.pixelformat) / 8;
      resolution_.width = media_type_r.format.width;
      resolution_.height = media_type_r.format.height;
      thumbnailResolution_.width = media_type_2.format.width;
      thumbnailResolution_.height = media_type_2.format.height;
      numTrailingBlackPixelsPerRow_ =
        resolution_.width - ((resolution_.width / TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_X) * TEST_U_MODULE_VIDEOWALL_DEFAULT_RESOLUTION_X);

      struct Stream_MediaFramework_V4L_MediaType media_type_3 = media_type_r;
#endif // ACE_WIN32 || ACE_WIN64

      ACE_ASSERT (session_data_r.lock);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited2::setFormat (MEDIASUBTYPE_RGB24, media_type_3);
#else
      inherited2::setFormat (V4L2_PIX_FMT_RGB24, media_type_3);
#endif // ACE_WIN32 || ACE_WIN64
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.formats.push_back (media_type_3);
      } // end lock scope

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      for (std::deque<ACE_Message_Block*>::const_iterator iterator = messages_.begin ();
           iterator != messages_.end ();
           ++iterator)
        (*iterator)->release ();
      messages_.clear ();

      break;
    }
    default:
      break;
  } // end SWITCH
}
