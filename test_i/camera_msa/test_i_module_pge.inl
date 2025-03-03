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

#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE

#include "stream_macros.h"
#include "stream_tools.h"

template <typename TaskType,
          typename MediaType>
Test_I_Module_PGE_T<TaskType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                    MediaType>::Test_I_Module_PGE_T (typename TaskType::ISTREAM_T* stream_in)
#else
                    MediaType>::Test_I_Module_PGE_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , previousImage_ (NULL)
 , currentImage_ (NULL)
 , fluidImage_ (NULL)
 , solver_ (0, 0)
 , aspectRatio2_ (0.0f)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::Test_I_Module_PGE_T"));

}

template <typename TaskType,
          typename MediaType>
void
Test_I_Module_PGE_T<TaskType,
                    MediaType>::handleDataMessage (typename inherited::DATA_MESSAGE_T*& message_inout,
                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::handleDataMessage"));

  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  static int screen_width_i = inherited3::ScreenWidth (), screen_height_i = inherited3::ScreenHeight ();

  // backup previous frame data
  ACE_OS::memmove (previousImage_, currentImage_, sizeof (char) * screen_width_i * screen_height_i * 3);

  // store current image data
  ACE_OS::memcpy (currentImage_, data_p, sizeof (char) * screen_width_i * screen_height_i * 3);

  // update frame data
  for (int y = 0; y < screen_height_i; y++)
    for (int x = 0; x < screen_width_i; x++)
    {
      // draw image
      inherited3::Draw (x, y, olc::Pixel (data_p[0], data_p[1], data_p[2], 128U)); // *NOTE*: leave some alpha for MSA

      data_p += 3;
    } // end FOR
}

template <typename TaskType,
          typename MediaType>
void
Test_I_Module_PGE_T<TaskType,
                    MediaType>::handleSessionMessage (typename inherited::SESSION_MESSAGE_T*& message_inout,
                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::handleSessionMessage"));

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
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      delete [] previousImage_; previousImage_ = NULL;
      delete [] currentImage_; currentImage_ = NULL;

      break;
    }
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

      previousImage_ = new char[
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                resolution_s.cx * resolution_s.cy
#else
                                resolution_s.width * resolution_s.height
#endif // ACE_WIN32 || ACE_WIN64
                                * 3];
      currentImage_ = new char[
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               resolution_s.cx * resolution_s.cy
#else
                               resolution_s.width * resolution_s.height
#endif // ACE_WIN32 || ACE_WIN64
                               * 3];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_OS::memset (previousImage_, 0, sizeof (char) * resolution_s.cx * resolution_s.cy * 3);
      ACE_OS::memset (currentImage_, 0, sizeof (char) * resolution_s.cx * resolution_s.cy * 3);
#else
      ACE_OS::memset (previousImage_, 0, sizeof (char) * resolution_s.width * resolution_s.height * 3);
      ACE_OS::memset (currentImage_, 0, sizeof (char) * resolution_s.width * resolution_s.height * 3);
#endif // ACE_WIN32 || ACE_WIN64

      olc::rcode result =
        inherited3::Construct (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               resolution_s.cx, resolution_s.cy,
#else
                               resolution_s.width, resolution_s.height,
#endif // ACE_WIN32 || ACE_WIN64
                               1, 1,
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
      delete [] previousImage_; previousImage_ = NULL;
      delete [] currentImage_; currentImage_ = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_Module_PGE_T<TaskType,
                    MediaType>::OnUserCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::OnUserCreate"));

  //olc::PixelGameEngine::SetPixelMode (olc::Pixel::ALPHA);

  float aspect_ratio_f =
    olc::PixelGameEngine::ScreenWidth () / static_cast<float> (olc::PixelGameEngine::ScreenHeight ());
  aspectRatio2_ = aspect_ratio_f * aspect_ratio_f;

  solver_.setup (FLUID_DEFAULT_FLUID_WIDTH,
                 (int)(FLUID_DEFAULT_FLUID_WIDTH * olc::PixelGameEngine::ScreenHeight () / static_cast<float> (olc::PixelGameEngine::ScreenWidth ())));

  fluidImage_ = new olc::Pixel[solver_.getWidth () * solver_.getHeight ()];

  return true;
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_Module_PGE_T<TaskType,
                    MediaType>::OnUserUpdate (float fElapsedTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::OnUserUpdate"));

  static int frame_count_i = 1;
  static int width_i = olc::PixelGameEngine::ScreenWidth ();
  static int height_i = olc::PixelGameEngine::ScreenHeight ();

  // process next message
  if (!processNextMessage ())
    return false; // done

  for (int i = 0; i < solver_.numCells_; i++)
    fluidImage_[i] = olc::PixelF (solver_.r_[i], solver_.g_[i], solver_.b_[i], 1.0f);
  // project fluid image to screen
  static float ratio_x = solver_.getWidth () / static_cast<float> (width_i);
  static float ratio_y = solver_.getHeight () / static_cast<float> (height_i);
  olc::Pixel* p = olc::PixelGameEngine::GetDrawTarget ()->GetData ();
  for (int y = 0; y < height_i; y++)
    for (int x = 0; x < width_i; x++)
    {
      int index_x = static_cast<int> (x * ratio_x);
      int index_y = static_cast<int> (y * ratio_y);
      // blend pixels from camera and fluid images
      olc::Pixel a = p[y * width_i + x];
      olc::Pixel b = fluidImage_[index_y * solver_.getWidth () + index_x];
      float r, g, b_2;
      float foreground_alpha_f = a.a / 255.0f;
      r   = ((a.r / 255.0f) * foreground_alpha_f) + ((b.r / 255.0f) * (1.0f - foreground_alpha_f));
      g   = ((a.g / 255.0f) * foreground_alpha_f) + ((b.g / 255.0f) * (1.0f - foreground_alpha_f));
      b_2 = ((a.b / 255.0f) * foreground_alpha_f) + ((b.b / 255.0f) * (1.0f - foreground_alpha_f));
      p[y * width_i + x] = olc::PixelF (r, g, b_2, 1.0f);
    } // end FOR

  std::vector<flow_zone> flow_zones_a =
    calculateFlow (previousImage_, currentImage_, width_i, height_i);
  for (typename std::vector<flow_zone>::iterator iterator = flow_zones_a.begin ();
       iterator != flow_zones_a.end ();
       ++iterator)
    if ((*iterator).UVMeanAboveLimit (solver_.UVCutoff_))
    {
      addForce ((*iterator).x_ / static_cast<float> (width_i), (*iterator).y_ / static_cast<float> (height_i),
                (*iterator).u_ / static_cast<float> (width_i), (*iterator).v_ / static_cast<float> (height_i),
                frame_count_i);
      //(*iterator).draw (this);
    } // end IF

  solver_.update ();

  ++frame_count_i;

  return !inherited3::GetKey (olc::Key::ESCAPE).bPressed;
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_Module_PGE_T<TaskType,
                    MediaType>::OnUserDestroy ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::OnUserDestroy"));

  delete [] fluidImage_; fluidImage_ = NULL;
  solver_.destroy ();

  return true;
}

template <typename TaskType,
          typename MediaType>
int
Test_I_Module_PGE_T<TaskType,
                    MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::svc"));

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
      stop_processing = true;

      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

#if defined (GTK_USE)
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false,  // wait for completion ?
                                                          false); // N/A
#endif // GTK_USE
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
Test_I_Module_PGE_T<TaskType,
                    MediaType>::processNextMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::processNextMessage"));

  ACE_Message_Block* message_block_p = NULL;
  static ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int result = inherited::getq (message_block_p, &no_wait);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (likely ((error == 0)          || // *TODO*: why does this happen ?
                (error == EWOULDBLOCK)))
      return true; // continue PGE
    if (unlikely (error != ESHUTDOWN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: worker thread %t failed to ACE_Task::getq(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    return false; // stop PGE
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
    return false; // stop PGE
  } // end IF

  // process manually
  bool stop_processing = false;
  inherited::handleMessage (message_block_p,
                            stop_processing);
  if (unlikely (stop_processing))
  {
    inherited::stop (false, // wait ?
                     true); // high priority ?
    return false; // stop PGE
  } // end IF

  return true; // continue PGE
}

template <typename TaskType,
          typename MediaType>
std::vector<typename Test_I_Module_PGE_T<TaskType, MediaType>::flow_zone>
Test_I_Module_PGE_T<TaskType,
                    MediaType>::calculateFlow (char* oldImage, char* newImage, int width, int height)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::calculateFlow"));

  std::vector<flow_zone> zones;

  int winStep = solver_.step_ * 2 + 1;
  int address_i;
  int luminance_i, luminance_2;
  int gradX, gradY, gradT;
  int A2, A1B2, B1, C1, C2;
  uint8_t red_i, red_2, green_i, green_2, blue_i, blue_2;
  float u, v /*, uu, vv*/;
  //uu = vv = 0.0f;
  int wMax = width - solver_.step_ - 1;
  int hMax = height - solver_.step_ - 1;
  int globalY, globalX, localY, localX;
  int delta_i;
  for (globalY = solver_.step_ + 1; globalY < hMax; globalY += winStep)
    for (globalX = solver_.step_ + 1; globalX < wMax; globalX += winStep)
    {
      A2 = A1B2 = B1 = C1 = C2 = 0;

      for (localY = -solver_.step_; localY <= solver_.step_; localY++)
        for (localX = -solver_.step_; localX <= solver_.step_; localX++)
        {
          address_i = (globalY + localY) * width + globalX + localX;

          blue_i  = static_cast<uint8_t> (newImage[(address_i - 1) * 3]);
          green_i = static_cast<uint8_t> (newImage[(address_i - 1) * 3 + 1]);
          red_i   = static_cast<uint8_t> (newImage[(address_i - 1) * 3 + 2]);
          blue_2  = static_cast<uint8_t> (newImage[(address_i + 1) * 3]);
          green_2 = static_cast<uint8_t> (newImage[(address_i + 1) * 3 + 1]);
          red_2   = static_cast<uint8_t> (newImage[(address_i + 1) * 3 + 2]);
          luminance_i =
            static_cast<int> (0.2990f * red_i + 0.5870f * green_i + 0.1140f * blue_i);
          luminance_2 =
            static_cast<int> (0.2990f * red_2 + 0.5870f * green_2 + 0.1140f * blue_2);
          gradX = luminance_i - luminance_2;
            //(newImage[(address_i - 1) * 3]) - (newImage[(address_i + 1) * 3]);

          blue_i  = static_cast<uint8_t> (newImage[(address_i - width) * 3]);
          green_i = static_cast<uint8_t> (newImage[(address_i - width) * 3 + 1]);
          red_i   = static_cast<uint8_t> (newImage[(address_i - width) * 3 + 2]);
          blue_2  = static_cast<uint8_t> (newImage[(address_i + width) * 3]);
          green_2 = static_cast<uint8_t> (newImage[(address_i + width) * 3 + 1]);
          red_2   = static_cast<uint8_t> (newImage[(address_i + width) * 3 + 2]);
          luminance_i =
            static_cast<int> (0.2990f * red_i + 0.5870f * green_i + 0.1140f * blue_i);
          luminance_2 =
            static_cast<int> (0.2990f * red_2 + 0.5870f * green_2 + 0.1140f * blue_2);
          gradY = luminance_i - luminance_2;
            //(newImage[(address_i - width) * 3]) - (newImage[(address_i + width) * 3]);

          blue_i  = static_cast<uint8_t> (oldImage[address_i * 3]);
          green_i = static_cast<uint8_t> (oldImage[address_i * 3 + 1]);
          red_i   = static_cast<uint8_t> (oldImage[address_i * 3 + 2]);
          blue_2  = static_cast<uint8_t> (newImage[address_i * 3]);
          green_2 = static_cast<uint8_t> (newImage[address_i * 3 + 1]);
          red_2   = static_cast<uint8_t> (newImage[address_i * 3 + 2]);
          luminance_i =
            static_cast<int> (0.2990f * red_i + 0.5870f * green_i + 0.1140f * blue_i);
          luminance_2 =
            static_cast<int> (0.2990f * red_2 + 0.5870f * green_2 + 0.1140f * blue_2);
          gradT = luminance_i - luminance_2;
            //(oldImage[address_i * 3]) - (newImage[address_i * 3]);

          A2 += gradX * gradX;
          A1B2 += gradX * gradY;
          B1 += gradY * gradY;
          C2 += gradX * gradT;
          C1 += gradY * gradT;
        } // end FOR

      delta_i = (A1B2 * A1B2 - A2 * B1);
      if (delta_i != 0)
      {
        /* system is not singular - solving by Kramer method */
        float Idelta = solver_.step_ / static_cast<float> (delta_i);
        int deltaX = -(C1 * A1B2 - C2 * B1);
        int deltaY = -(A1B2 * C2 - A2 * C1);
        u = deltaX * Idelta;
        v = deltaY * Idelta;
      } // end IF
      else
      {
        /* singular system - find optical flow in gradient direction */
        int norm = (A1B2 + A2) * (A1B2 + A2) + (B1 + A1B2) * (B1 + A1B2);
        if (norm != 0)
        {
          float IGradNorm = solver_.step_ / static_cast<float> (norm);
          float temp = -(C1 + C2) * IGradNorm;
          u = (A1B2 + A2) * temp;
          v = (B1 + A1B2) * temp;
        } // end IF
        else
        {
          u = v = 0.0f;
        } // end ELSE
      } // end ELSE

      if (static_cast<float> (-winStep) < u && u < static_cast<float> (winStep) &&
          static_cast<float> (-winStep) < v && v < static_cast<float> (winStep))
      {
        //uu += u;
        //vv += v;
        zones.push_back (flow_zone (globalX, globalY, u, v));
      } // end IF
    } // end FOR

  return zones;
}

template <typename TaskType,
          typename MediaType>
void
Test_I_Module_PGE_T<TaskType,
                    MediaType>::addForce (float x, float y, float dx, float dy, int frameCount)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::addForce"));

  float speed =
    dx * dx + dy * dy * aspectRatio2_; // balance the x and y components of
                                        // speed with the screen aspect ratio
  if (speed > 0.0f)
  {
    int index = solver_.getIndexForNormalizedPosition (x, y);

    float hue = std::fmod ((x + y) * 180.0f + frameCount, 360.0f);
    float r, g, b;
    Common_Image_Tools::HSVToRGB (hue, 1.0f, 1.0f, r, g, b);

    solver_.rOld_[index] += r * solver_.colorMultiplier_;
    solver_.gOld_[index] += g * solver_.colorMultiplier_;
    solver_.bOld_[index] += b * solver_.colorMultiplier_;

    solver_.uOld_[index] += dx * solver_.velocityMultiplier_;
    solver_.vOld_[index] += dy * solver_.velocityMultiplier_;
  } // end IF
}

