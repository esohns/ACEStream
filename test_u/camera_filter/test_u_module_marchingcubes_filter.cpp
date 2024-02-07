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

#define OLC_PGE_APPLICATION
#include "test_u_module_marchingcubes_filter.h"

#if defined (GLM_SUPPORT)
#include "glm/glm.hpp"
#endif // GLM_SUPPORT

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_gl_tools.h"

#include "common_image_tools.h"

#include "stream_macros.h"

const char libacestream_default_marchingcubes_filter_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR ("MarchingCubesFilter");

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_MarchingCubes_Filter::Test_U_CameraFilter_MarchingCubes_Filter (ISTREAM_T* stream_in)
#else
Test_U_CameraFilter_MarchingCubes_Filter::Test_U_CameraFilter_MarchingCubes_Filter (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , bytesPerPixel_ (0)
 , resolution_ ()
 , imgWidth_ (0)
 , minPixX_ (0), maxPixX_ (0)
 , minPixY_ (0), maxPixY_ (0)
 , stepW_ (0.0f)
 , areaWidth_ (0.0f)
 , initX_ (0.0f), initY_ (0.0f)
 , palette_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::Test_U_CameraFilter_MarchingCubes_Filter"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_MarchingCubes_Filter::handleDataMessage (Test_U_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraFilter_MarchingCubes_Filter::handleDataMessage (Test_U_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());

  std::vector<std::vector<glm::vec2> > positions_a;
  std::vector<std::vector<uint8_t> > colors_a;
  for (int i = 0; i < ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM; ++i)
  {
    std::vector<glm::vec2> temp_a;
    positions_a.push_back (temp_a);
    std::vector<uint8_t> temp_2;
    colors_a.push_back (temp_2);
    int x, y, x2, y2, r, g, b, index_i;
    float r_2, g_2, b_2; 
    for (int j = 0; j < ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM; ++j)
    {
      x =
        Common_GL_Tools::map (i, 0, ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM, minPixX_, maxPixX_);
      y =
        Common_GL_Tools::map (j, 0, ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM, minPixY_, maxPixY_);
      x2 =
        Common_GL_Tools::map (i + 1, 0, ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM, minPixX_, maxPixX_);
      y2 =
        Common_GL_Tools::map (j + 1, 0, ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM, minPixY_, maxPixY_);

      r = g = b = 0;
      for (int k = x; k < x2; ++k)
        for (int l = y; l < y2; ++l)
        {
          index_i =
            k * bytesPerPixel_ + l * resolution_.cx * bytesPerPixel_;
          r += data_p[index_i];
          g += data_p[index_i + 1];
          b += data_p[index_i + 2];
        } // end FOR
      r_2 = r / static_cast<float> ((x2 - x) * (y2 - y));
      g_2 = g / static_cast<float> ((x2 - x) * (y2 - y));
      b_2 = b / static_cast<float> ((x2 - x) * (y2 - y));

      colors_a[i].push_back (static_cast<uint8_t> ((r_2 + g_2 + b_2) / 3.0f));
      positions_a[i].push_back (glm::vec2 (initX_ + stepW_ * i, initY_ + stepW_ * j));
    } // end FOR
  } // end FOR

  std::vector<std::vector<bool> > filled_a;
  for (int x = 0; x < ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM - 1; ++x)
  {
    std::vector<bool> temp_a;
    temp_a.assign (ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM - 1, false);
    filled_a.push_back (temp_a);
  } // end FOR

  std::vector<std::vector<std::vector<struct acestream_mc_filter_state> > > states_a;
  std::vector<std::vector<struct acestream_mc_filter_state> > temp_a;
  states_a.assign (ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD, temp_a);
  for (int i = ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD - 1; i >= 0; --i)
    for (int x = 0; x < ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM - 1; ++x)
    {
      std::vector<struct acestream_mc_filter_state> temp_3;
      states_a[i].push_back (temp_3);
      struct acestream_mc_filter_state state_s, state_2;
      state_s.state = 0;
      for (int y = 0; y < ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM - 1; ++y)
      {
        states_a[i][x].push_back (state_s);

        if (filled_a[x][y] == false)
        {
          state_2.n1 =
            colors_a[x][y] / 255.0f - (i + 0.5f) / static_cast<float> (ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD);
          state_2.n2 =
            colors_a[x + 1][y] / 255.0f - (i + 0.5f) / static_cast<float> (ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD);
          state_2.n3 =
            colors_a[x + 1][y + 1] / 255.0f - (i + 0.5f) / static_cast<float> (ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD);
          state_2.n4 =
            colors_a[x][y + 1] / 255.0f - (i + 0.5f) / static_cast<float> (ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD);
          state_2.state = getState (static_cast<int> (std::ceil (state_2.n1)),
                                    static_cast<int> (std::ceil (state_2.n2)),
                                    static_cast<int> (std::ceil (state_2.n3)),
                                    static_cast<int> (std::ceil (state_2.n4)));
          states_a[i][x][y] = state_2;
          if (state_2.state == 15)
            filled_a[x][y] = true;
        } // end IF
      } // end FOR
    } // end FOR

  palette_.push_back (palette_[0]);
  palette_.erase (palette_.begin ());
  for (int i = 0; i < ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD; ++i)
    for (int x = 0; x < ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM - 1; ++x)
      for (int y = 0; y < ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM - 1; ++y)
        if (states_a[i][x][y].state != 0 && states_a[i][x][y].state != 15)
          marchingCubes (positions_a[x][y], positions_a[x + 1][y], positions_a[x + 1][y + 1], positions_a[x][y + 1],
                         states_a[i][x][y].state,
                         states_a[i][x][y].n1 + 1.0f, states_a[i][x][y].n2 + 1.0f, states_a[i][x][y].n3 + 1.0f, states_a[i][x][y].n4 + 1.0f,
                         palette_[i + 1]);
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_MarchingCubes_Filter::handleSessionMessage (Test_U_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraFilter_MarchingCubes_Filter::handleSessionMessage (Test_U_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
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

      imgWidth_ =
        (resolution_.cx < resolution_.cy) ? resolution_.cx : resolution_.cy;
      minPixX_ = resolution_.cx / 2 - imgWidth_ / 2;
      maxPixX_ = resolution_.cx / 2 + imgWidth_ / 2;
      minPixY_ = resolution_.cy / 2 - imgWidth_ / 2;
      maxPixY_ = resolution_.cy / 2 + imgWidth_ / 2;
  
      areaWidth_ = static_cast<float> (resolution_.cx);
#else
      const struct Stream_MediaFramework_V4L_MediaType& media_type_r =
        session_data_r.formats.back ();
      ACE_ASSERT (Stream_MediaFramework_Tools::isRGB (media_type_r.format.pixelformat));

      bytesPerPixel_ =
        Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_r.format.pixelformat) / 8;
      resolution_.width = media_type_r.format.width;
      resolution_.height = media_type_r.format.height;

      imgWidth_ =
        (resolution_.width < resolution_.height) ? resolution_.width : resolution_.height;
      minPixX_ = resolution_.width / 2 - imgWidth_ / 2;
      maxPixX_ = resolution_.width / 2 + imgWidth_ / 2;
      minPixY_ = resolution_.height / 2 - imgWidth_ / 2;
      maxPixY_ = resolution_.height / 2 + imgWidth_ / 2;

      areaWidth_ = static_cast<float> (resolution_.width);
#endif // ACE_WIN32 || ACE_WIN64
      stepW_ = areaWidth_ / static_cast<float> (ACESTREAM_MC_FILTER_DEFAULT_TILE_NUM - 1);
      initX_ = -areaWidth_ / 2.0f;
      initY_ = initX_;

      float initH = Common_Tools::getRandomNumber  (0.0f, 360.0f);
      float colDirection = Common_Tools::testRandomProbability (0.5f) ? -1.0f : 1.0f;
      float r, g, b;
      Common_GL_Color_t color;
      for (int i = 0; i < ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD + 1; i++)
      {
        Common_Image_Tools::HSVToRGB (std::fmod (360.0f / static_cast<float> (ACESTREAM_MC_FILTER_DEFAULT_NUM_THRESHOLD) * i * colDirection + initH + 360.0f, 360.0f), 80.0f / 100.0f, 1.0f, r, g, b);
        color.r = static_cast<uint8_t> (r * 255.0f);
        color.g = static_cast<uint8_t> (g * 255.0f);
        color.b = static_cast<uint8_t> (b * 255.0f);
        palette_.push_back (color);
      } // end FOR

      olc::rcode result =
        inherited3::Construct (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               resolution_.cx, resolution_.cy,
#else
                               resolution_.width, resolution_.height,
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

uint8_t
Test_U_CameraFilter_MarchingCubes_Filter::getState (int n1, int n2, int n3, int n4)
{
  //STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::getState"));

  return (n1 * 8 + n2 * 4 + n3 * 2 + n4 * 1);
}

void
Test_U_CameraFilter_MarchingCubes_Filter::marchingCubes (glm::vec2& xyz1, glm::vec2& xyz2, glm::vec2& xyz3, glm::vec2& xyz4,
                                                         uint8_t state,
                                                         float n1, float n2, float n3, float n4,
                                                         Common_GL_Color_t& color)
{
  //STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::marchingCubes"));

  float amt = (1.0f - n1) / (n2 - n1);
  glm::vec2 a (
    xyz1.x + (xyz2.x - xyz1.x) * amt,
    xyz1.y + (xyz2.y - xyz1.y) * amt//,
//    xyz1[2]+(xyz2[2]-xyz1[2])*amt
  );
  amt = (1.0f - n2) / (n3 - n2);
  glm::vec2 b (
    xyz2.x + (xyz3.x - xyz2.x) * amt,
    xyz2.y + (xyz3.y - xyz2.y) * amt//,
//    xyz2[2]+(xyz3[2]-xyz2[2])*amt
  );
  amt = (1.0f - n3) / (n4 - n3);
  glm::vec2 c (
    xyz3.x + (xyz4.x - xyz3.x) * amt,
    xyz3.y + (xyz4.y - xyz3.y) * amt//,
//    xyz3[2]+(xyz4[2]-xyz3[2])*amt
  );
  amt = (1.0f - n4) / (n1 - n4);
  glm::vec2 d (
    xyz4.x + (xyz1.x - xyz4.x) * amt,
    xyz4.y + (xyz1.y - xyz4.y) * amt//,
//    xyz4[2]+(xyz1[2]-xyz4[2])*amt
  );
  //let v1 = createVector(xyz1[0], xyz1[1], xyz1[2]);
  //let v2 = createVector(xyz2[0], xyz2[1], xyz2[2]);
  //let v3 = createVector(xyz3[0], xyz3[1], xyz3[2]);
  //let v4 = createVector(xyz4[0], xyz4[1], xyz4[2]);

  switch (state)
  {
    case 0:
      break;
    case 1: //v4
      drawLine (c, d, color);
      break;
    case 2: //v3
      drawLine (b, c, color);
      break;
    case 3: //v3 v4
      drawLine (b, d, color);
      break;
    case 4: //v2
      drawLine (a, b, color);
      break;
    case 5: //v2 v4
      drawLine (a, d, color);
      drawLine (c, b, color);
      break;
    case 6: //v2 v3
      drawLine (a, c, color);
      break;
    case 7: //v2 v3 v4
      drawLine (a, d, color);
      break;
    case 8: //v1
      drawLine (a, d, color);
      break;
    case 9: //v1 v4
      drawLine (a, c, color);
      break;
    case 10: //v1 v3
      drawLine (a, b, color);
      drawLine (c, d, color);
      break;
    case 11: //v1 v3 v4
      drawLine (a, b, color);
      break;
    case 12: //v1 v2
      drawLine (b, d, color);
      break;
    case 13: //v1 v2 v4
      drawLine (b, c, color);
      break;
    case 14: //v1 v2 v3
      drawLine (c, d, color);
      break;
    case 15: //v1 v2 v3 v4
      break;
    default:
      ACE_ASSERT (false);
      break;
  } // end SWITCH
}

void
Test_U_CameraFilter_MarchingCubes_Filter::drawLine (glm::vec2& pos1_in, glm::vec2& pos2_in,
                                                    Common_GL_Color_t& color_in)
{
  //STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::drawLine"));
  static olc::vf2d center_s (inherited3::ScreenWidth () / 2.0f,
                             inherited3::ScreenHeight () / 2.0f);

  olc::vf2d pos1 (pos1_in.x, pos1_in.y), pos2 (pos2_in.x, pos2_in.y);
  olc::Pixel color (color_in.r, color_in.g, color_in.b, 255);
  inherited3::DrawLine (center_s + pos1, center_s + pos2, color, 0xFFFFFFFF);
}

bool
Test_U_CameraFilter_MarchingCubes_Filter::OnUserCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::OnUserCreate"));

  return true;
}

bool
Test_U_CameraFilter_MarchingCubes_Filter::OnUserUpdate (float fElapsedTime_in)
{
  //STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::OnUserUpdate"));

  inherited3::Clear (olc::BLACK);

  // process next message
  if (processNextMessage ())
    return false; // done

  return !inherited3::GetKey (olc::Key::ESCAPE).bPressed;
}

bool
Test_U_CameraFilter_MarchingCubes_Filter::OnUserDestroy ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::OnUserDestroy"));

  return true;
}

int
Test_U_CameraFilter_MarchingCubes_Filter::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::svc"));

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        Test_U_DirectShow_SessionMessage_t* session_message_p =
          static_cast<Test_U_DirectShow_SessionMessage_t*> (message_block_p);
#else
        Test_U_SessionMessage_t* session_message_p =
          static_cast<Test_U_SessionMessage_t*> (message_block_p);
#endif // ACE_WIN32 || ACE_WIN64
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

bool
Test_U_CameraFilter_MarchingCubes_Filter::processNextMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_MarchingCubes_Filter::processNextMessage"));

  ACE_Message_Block* message_block_p = NULL;
  static ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int result = inherited::getq (message_block_p, &no_wait);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    //if (likely (error == EWOULDBLOCK))
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
