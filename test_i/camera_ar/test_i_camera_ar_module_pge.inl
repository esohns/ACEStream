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

#include "ace/Log_Msg.h"

#include "stream_macros.h"
#include "stream_tools.h"

template <typename TaskType,
          typename MediaType>
Test_I_CameraAR_Module_PGE_T<TaskType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             MediaType>::Test_I_CameraAR_Module_PGE_T (typename TaskType::ISTREAM_T* stream_in)
#else
                             MediaType>::Test_I_CameraAR_Module_PGE_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , previousImage (NULL)
 , currentImage (NULL)
 , previousFilteredImage (NULL)
 , currentFilteredImage (NULL)
 , previousMotionImage (NULL)
 , currentMotionImage (NULL)
 , flowFieldX (NULL)
 , flowFieldY (NULL)
 , ballX (0.0F)
 , ballY (0.0F)
 , ballVelocityX (0.0F)
 , ballVelocityY (0.0F)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::Test_I_CameraAR_Module_PGE_T"));

}

template <typename TaskType,
          typename MediaType>
void
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::handleDataMessage (typename inherited::DATA_MESSAGE_T*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::handleDataMessage"));

  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  float red_f, green_f, blue_f, luminance_f, difference_f;
  int screen_width_i = inherited3::ScreenWidth (), screen_height_i = inherited3::ScreenHeight ();

  // backup previous frame data
  ACE_OS::memmove (previousImage, currentImage, sizeof (float) * screen_width_i * screen_height_i);
  ACE_OS::memmove (previousFilteredImage, currentFilteredImage, sizeof (float) * screen_width_i * screen_height_i);
  ACE_OS::memmove (previousMotionImage, currentMotionImage, sizeof (float) * screen_width_i * screen_height_i);

  // update frame data
  for (int y = 0; y < screen_height_i; y++)
    for (int x = 0; x < screen_width_i; x++)
    {
      // store luminance values
      red_f = (float)data_p[2] / 255.0f;
      green_f = (float)data_p[1] / 255.0f;
      blue_f = (float)data_p[0] / 255.0f;
      luminance_f = 0.2987f * red_f + 0.5870f * green_f + 0.1140f * blue_f;
      currentImage[y * screen_width_i + x] = luminance_f;

      // low-pass filter camera image
      currentFilteredImage[y*screen_width_i + x] += (currentImage[y*screen_width_i + x] - currentFilteredImage[y*screen_width_i + x]) * 0.8f;

      // compute difference between two successive camera frames
      difference_f =
        fabs (currentFilteredImage[y*screen_width_i + x] - previousFilteredImage[y*screen_width_i + x]);
      // add threshold to filter out camera noise
      currentMotionImage[y * screen_width_i + x] =
        (difference_f >= 0.05f) ? difference_f : 0.0f;

      // draw image
      inherited3::Draw (x, y, olc::Pixel (data_p[0], data_p[1], data_p[2], 255U));

      data_p += 3;
    } // end FOR

  // compute flow vector map
  int patch_size_i = 9;
  int search_size_i = 7;

  float patch_difference_max_f/*, patch_difference_x_f, patch_difference_y_f*/;
  float accumulated_difference_f, patch_pixel_f, base_pixel_f;
  int search_vector_x_i, search_vector_y_i, patch_pixel_x_i, patch_pixel_y_i;
  int base_pixel_x_i, base_pixel_y_i;
  for (int x = 0; x < screen_width_i; x++)
    for (int y = 0; y < screen_height_i; y++)
    {
      patch_difference_max_f = INFINITY;
//      patch_difference_x_f = 0.0f;
//      patch_difference_y_f = 0.0f;
      flowFieldX[y * screen_width_i + x] = 0.0f;
      flowFieldY[y * screen_width_i + x] = 0.0f;

      // search over a given rectangular area for a "patch" of old image
      // that "resembles" a patch of the new image
      for (int sx = 0; sx < search_size_i; sx++)
        for (int sy = 0; sy < search_size_i; sy++)
        {
          // search vector is centre of patch test
          search_vector_x_i = x + (sx - search_size_i / 2);
          search_vector_y_i = y + (sy - search_size_i / 2);

          accumulated_difference_f = 0.0f;

          // for each pixel in search patch, accumulate difference with base patch
          for (int px = 0; px < patch_size_i; px++)
            for (int py = 0; py < patch_size_i; py++)
            {
              // Work out search patch offset indices
              patch_pixel_x_i = search_vector_x_i + (px - patch_size_i / 2);
              patch_pixel_y_i = search_vector_y_i + (py - patch_size_i / 2);

              // Work out base patch indices
              base_pixel_x_i = x + (px - patch_size_i / 2);
              base_pixel_y_i = y + (py - patch_size_i / 2);

              // Get adjacent values for each patch
              patch_pixel_f = getPixel (currentImage, patch_pixel_x_i, patch_pixel_y_i);
              base_pixel_f = getPixel (previousImage, base_pixel_x_i, base_pixel_y_i);

              // Accumulate difference
              accumulated_difference_f += fabs (patch_pixel_f - base_pixel_f);
            } // end FOR

          // Record the vector offset for the search patch that is the
          // least different to the base patch
          if (accumulated_difference_f <= patch_difference_max_f)
          {
            patch_difference_max_f = accumulated_difference_f;
            flowFieldX[y * screen_width_i + x] = (float)(search_vector_x_i - x);
            flowFieldY[y * screen_width_i + x] = (float)(search_vector_y_i - y);
          } // end IF
        } // end FOR
    } // end FOR

  // modulate flow vector map with motion map, to remove vectors that
  // erroneously indicate large local motion
  for (int i = 0; i < screen_width_i * screen_height_i; i++)
  {
    flowFieldX[i] *= currentMotionImage[i] > 0 ? 1.0f : 0.0f;
    flowFieldY[i] *= currentMotionImage[i] > 0 ? 1.0f : 0.0f;
  } // end FOR
}

template <typename TaskType,
          typename MediaType>
void
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::handleSessionMessage (typename inherited::SESSION_MESSAGE_T*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  const typename inherited::SESSION_MESSAGE_T::DATA_T& session_data_container_r =
    message_inout->getR ();
  typename inherited::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
    const_cast<typename inherited::SESSION_MESSAGE_T::DATA_T::DATA_T&> (session_data_container_r.getR ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      Common_Image_Resolution_t resolution_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
#else
      resolution_s = media_type_s.resolution;
#endif // ACE_WIN32 || ACE_WIN64

      olc::rcode result =
        inherited3::Construct (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               resolution_s.cx, resolution_s.cy,
#else
                               resolution_s.width, resolution_s.height,
#endif // ACE_WIN32 || ACE_WIN64
                               8, 8,
                               false,  // fullscreen ?
                               false,  // vsync ?
                               false); // cohesion ?
      if (unlikely (result == olc::FAIL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to olc::PixelGameEngine::Construct(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::OnUserCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::OnUserCreate"));

  previousImage = new float[ScreenWidth () * ScreenHeight ()];
  currentImage = new float[ScreenWidth () * ScreenHeight ()];
  previousFilteredImage = new float[ScreenWidth () * ScreenHeight ()];
  currentFilteredImage = new float[ScreenWidth () * ScreenHeight ()];
  previousMotionImage = new float[ScreenWidth () * ScreenHeight ()];
  currentMotionImage = new float[ScreenWidth () * ScreenHeight ()];
  flowFieldX = new float[ScreenWidth () * ScreenHeight ()];
  flowFieldY = new float[ScreenWidth () * ScreenHeight ()];

  ACE_OS::memset (previousImage, 0, sizeof (float) * ScreenWidth () * ScreenHeight ());
  ACE_OS::memset (currentImage, 0, sizeof (float) * ScreenWidth () * ScreenHeight ());
  ACE_OS::memset (previousFilteredImage, 0, sizeof (float) * ScreenWidth () * ScreenHeight ());
  ACE_OS::memset (currentFilteredImage, 0, sizeof (float) * ScreenWidth () * ScreenHeight ());
  ACE_OS::memset (previousMotionImage, 0, sizeof (float) * ScreenWidth () * ScreenHeight ());
  ACE_OS::memset (currentMotionImage, 0, sizeof (float) * ScreenWidth () * ScreenHeight ());
  ACE_OS::memset (flowFieldX, 0, sizeof (float) * ScreenWidth () * ScreenHeight ());
  ACE_OS::memset (flowFieldY,	0, sizeof (float) * ScreenWidth () * ScreenHeight ());

  ballX = inherited3::ScreenWidth () / 2.0f;
  ballY = inherited3::ScreenHeight () / 2.0f;

  return true;
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::OnUserUpdate (float fElapsedTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::OnUserUpdate"));

  // process next message
  if (processNextMessage ())
    return false; // done

  // move ball
  // ball velocity is updated by flow vector map
  ballVelocityX +=
    100.0f * flowFieldX[(int)ballY * inherited3::ScreenWidth () + (int)ballX] * fElapsedTime_in;
  ballVelocityY +=
    100.0f * flowFieldY[(int)ballY * inherited3::ScreenWidth () + (int)ballX] * fElapsedTime_in;
  // ball position is updated by velocity
  ballX += 1.0f * ballVelocityX * fElapsedTime_in;
  ballY += 1.0f * ballVelocityY * fElapsedTime_in;

  // add "drag" effect to ball velocity
  ballVelocityX *= 0.85f;
  ballVelocityY *= 0.85f;

  // wrap ball around screen
  while (ballX >= inherited3::ScreenWidth ()) ballX -= (float)inherited3::ScreenWidth ();
  while (ballY >= inherited3::ScreenHeight ()) ballY -= (float)inherited3::ScreenHeight ();
  while (ballX < 0) ballX += (float)inherited3::ScreenWidth ();
  while (ballY < 0) ballY += (float)inherited3::ScreenHeight ();

  // draw ball
  inherited3::DrawRect (static_cast<int32_t> (ballX - 4), static_cast<int32_t> (ballY - 4), 8, 8, olc::RED);

  return !inherited3::GetKey (olc::Key::ESCAPE).bPressed;
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::OnUserDestroy ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::OnUserDestroy"));

  delete [] previousImage;
  delete [] currentImage;
  delete [] previousFilteredImage;
  delete [] currentFilteredImage;
  delete [] previousMotionImage;
  delete [] currentMotionImage;
  delete [] flowFieldX;
  delete [] flowFieldY;

  return true;
}

template <typename TaskType,
          typename MediaType>
int
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  int error = -1;
  bool stop_processing = false;
  bool start_pge = false;
  olc::rcode result_2 = olc::FAIL;

  do
  {
    result = inherited::getq (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      if (unlikely (error != ESHUTDOWN))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        return -1;
      } // end IF
      result = 0; // OK, queue has been deactivate()d
      break;
    } // end IF

    ACE_ASSERT (message_block_p);
    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      if (unlikely (inherited::thr_count_ > 1))
      {
        result = inherited::putq (message_block_p, NULL);
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          return -1;
        } // end IF
        message_block_p = NULL;
      } // end IF
      // clean up ?
      if (message_block_p)
      {
        message_block_p->release (); message_block_p = NULL;
      } // end IF
      break; // done
    } // end IF

    switch (message_block_p->msg_type ())
    {
      case STREAM_MESSAGE_CONTROL:
        break;
      case STREAM_MESSAGE_SESSION:
      {
        typename TaskType::SESSION_MESSAGE_T* session_message_p =
          static_cast<typename TaskType::SESSION_MESSAGE_T*> (message_block_p);
        if (session_message_p->type () == STREAM_SESSION_MESSAGE_BEGIN)
          start_pge = true;
        break;
      }
      case STREAM_MESSAGE_DATA:
      case STREAM_MESSAGE_OBJECT:
        break;
      case ACE_Message_Block::MB_USER:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown message (type was: \"%s\"), continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (message_block_p->msg_type ())).c_str ())));
        break;
      }
    } // end SWITCH

    // process manually
    inherited::handleMessage (message_block_p,
                              stop_processing);
    if (unlikely (stop_processing))
      inherited::stop (false, // wait ?
                       true); // high priority ?

    if (unlikely (start_pge && !stop_processing))
    {
      start_pge = false;
      result_2 = inherited3::Start ();
      if (unlikely (result_2 != olc::OK))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to olc::PixelGameEngine::Start(), aborting\n"),
                    inherited::mod_->name ()));
        return -1;
      } // end IF
    } // end IF

    message_block_p = NULL;
  } while (true);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::processNextMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::processNextMessage"));

  ACE_Message_Block* message_block_p = NULL;
  static ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int result = inherited::getq (message_block_p, &no_wait);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (likely (error == EWOULDBLOCK))
      return false; // continue PGE
    if (unlikely (error != ESHUTDOWN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: worker thread %t failed to ACE_Task::getq(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    return true; // stop PGE
  } // end IF
  ACE_ASSERT (message_block_p);

  if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
  {
    result = inherited::putq (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_block_p->release ();
    } // end IF
    return true; // stop PGE
  } // end IF

  // process manually
  bool stop_processing = false;
  inherited::handleMessage (message_block_p,
                            stop_processing);
  if (unlikely (stop_processing))
  {
    inherited::stop (false, // wait ?
                     true); // high priority ?
    return true; // stop PGE
  } // end IF

  return false; // continue PGE
}
