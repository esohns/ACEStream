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

#define JC_VORONOI_IMPLEMENTATION
#undef OK
#include "test_u_module_weightedvoronoistipple_filter.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_gl_tools.h"

#include "common_math_tools.h"

#include "stream_macros.h"

const char libacestream_default_weighted_voronoi_stipple_filter_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR ("WeightedVoronoiStippleFilter");

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::Test_U_CameraFilter_WeightedVoronoiStipple_Filter (ISTREAM_T* stream_in)
#else
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::Test_U_CameraFilter_WeightedVoronoiStipple_Filter (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , bytesPerPixel_ (4)
 , points_ (NULL)
 , resolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::Test_U_CameraFilter_WeightedVoronoiStipple_Filter"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::handleDataMessage (Test_U_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::handleDataMessage (Test_U_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  
  static bool is_first_b = true;
  if (unlikely (is_first_b))
  {
    is_first_b = false;

    for (int i = 0; i < ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS; i++)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      int x =
        Common_Tools::getRandomNumber (0, static_cast<int> (resolution_.cx - 1));
      int y =
        Common_Tools::getRandomNumber (0, static_cast<int> (resolution_.cy - 1));
      uint8_t r = data_p[(y * resolution_.cx + x) * bytesPerPixel_ + 0];
      uint8_t g = data_p[(y * resolution_.cx + x) * bytesPerPixel_ + 1];
      uint8_t b = data_p[(y * resolution_.cx + x) * bytesPerPixel_ + 2];
#else
      int x =
        Common_Tools::getRandomNumber (0, static_cast<int> (resolution_.width - 1));
      int y =
        Common_Tools::getRandomNumber (0, static_cast<int> (resolution_.height - 1));
      uint8_t r = data_p[(y * resolution_.width + x) * bytesPerPixel_ + 0];
      uint8_t g = data_p[(y * resolution_.width + x) * bytesPerPixel_ + 1];
      uint8_t b = data_p[(y * resolution_.width + x) * bytesPerPixel_ + 2];
#endif // ACE_WIN32 || ACE_WIN64
      float brightness_f = (0.2126f * r + 0.7152f * g + 0.0722f * b);
      if (Common_Tools::getRandomNumber (0.0f, 100.0f) > brightness_f)
      {
        points_[i].x = static_cast<float> (x);
        points_[i].y = static_cast<float> (y);
      } // end IF
      else
        i--;
    } // end FOR
  } // end IF

  jcv_diagram diagram;
  ACE_OS::memset (&diagram, 0, sizeof (jcv_diagram));
  jcv_diagram_generate (ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS, (const jcv_point*)points_, NULL, NULL, &diagram);

  olc::vf2d centroids[ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS];
  float weights[ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS];
  int counts[ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS];
  float avgWeights[ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS];
  for (int i = 0; i < ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS; i++)
    centroids[i] = olc::vf2d (0.0f, 0.0f);
  ACE_OS::memset (weights, 0, sizeof (float) * ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS);
  ACE_OS::memset (counts, 0, sizeof (int) * ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS);
  ACE_OS::memset (avgWeights, 0, sizeof (float) * ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS);

  int delaunayIndex = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  for (int i = 0; i < resolution_.cx; i++)
    for (int j = 0; j < resolution_.cy; j++)
#else
  for (int i = 0; i < static_cast<int> (resolution_.width); i++)
    for (int j = 0; j < static_cast<int> (resolution_.height); j++)
#endif // ACE_WIN32 || ACE_WIN64
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      uint8_t r = data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 0];
      uint8_t g = data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 1];
      uint8_t b = data_p[(j * resolution_.cx + i) * bytesPerPixel_ + 2];
#else
      uint8_t r = data_p[(j * resolution_.width + i) * bytesPerPixel_ + 0];
      uint8_t g = data_p[(j * resolution_.width + i) * bytesPerPixel_ + 1];
      uint8_t b = data_p[(j * resolution_.width + i) * bytesPerPixel_ + 2];
#endif // ACE_WIN32 || ACE_WIN64
      float brightness_f = (0.2126f * r + 0.7152f * g + 0.0722f * b);
      float weight = 1.0f - brightness_f / 255.0f;

      olc::vf2d point_s (static_cast<float> (i), static_cast<float> (j));
      delaunayIndex = pointToSite (diagram, point_s);
      centroids[delaunayIndex].x += i * weight;
      centroids[delaunayIndex].y += j * weight;
      weights[delaunayIndex] += weight;
      counts[delaunayIndex]++;
    } // end FOR

  float maxWeight = 0.0f;
  for (int i = 0; i < ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS; i++)
  {
    if (weights[i] > 0.0f)
    {
      centroids[i] /= weights[i];
      avgWeights[i] = weights[i] / (counts[i] ? counts[i] : 1);
      if (avgWeights[i] > maxWeight)
        maxWeight = avgWeights[i];
    } // end IF
    else
      centroids[i] = {points_[i].x, points_[i].y};
  } // end FOR

  for (int i = 0; i < ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS; i++)
  {
    points_[i].x = Common_Math_Tools::lerp (points_[i].x, centroids[i].x, 1.0f);
    points_[i].y = Common_Math_Tools::lerp (points_[i].y, centroids[i].y, 1.0f);
  } // end FOR

  for (int i = 0; i < ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS; i++)
  {
    int index =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      static_cast<int> (std::floor (points_[i].x) + std::floor (points_[i].y) * resolution_.cx) * bytesPerPixel_;
#else
      static_cast<int> (std::floor (points_[i].x) + std::floor (points_[i].y) * resolution_.width) * bytesPerPixel_;
#endif // ACE_WIN32 || ACE_WIN64
    uint8_t r = data_p[index + 0];
    uint8_t g = data_p[index + 1];
    uint8_t b = data_p[index + 2];
    olc::Pixel color (r, g, b, 255);
    // let col = video.get(v.x, v.y);
    // stroke(col);
    // stroke(0);
    float sw = Common_GL_Tools::map (avgWeights[i], 0.0f, maxWeight, 0.0f, 12.0f);
    //strokeWeight (sw);
    inherited3::FillCircle (static_cast<int32_t> (points_[i].x), static_cast<int32_t> (points_[i].y),
                            static_cast<int32_t> (sw / 2.0f), color);
    //inherited3::Draw (static_cast<int32_t> (points_[i].x), static_cast<int32_t> (points_[i].y),
    //                  olc::BLACK);
  } // end FOR

  jcv_diagram_free (&diagram);
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::handleSessionMessage (Test_U_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::handleSessionMessage (Test_U_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      free (points_); points_ = NULL;

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

      points_ =
        (jcv_point*)malloc (sizeof (jcv_point) * ACESTREAM_WVS_FILTER_DEFAULT_NUMBER_OF_POINTS);

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
      free (points_); points_ = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}

bool
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::OnUserCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::OnUserCreate"));

  return true;
}

bool
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::OnUserUpdate (float fElapsedTime_in)
{
  //STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::OnUserUpdate"));

  inherited3::Clear (olc::WHITE);
  //int pixels =
  //  inherited3::GetDrawTargetWidth () * inherited3::GetDrawTargetHeight ();
  //olc::Pixel* p = inherited3::GetDrawTarget ()->GetData ();
  //for (int i = 0; i < pixels; i++)
  //  p[i].a = (p[i].a > ACESTREAM_MC_FILTER_DEFAULT_ALPHA_DECAY ? p[i].a - ACESTREAM_MC_FILTER_DEFAULT_ALPHA_DECAY : 0);

  // process next message
  if (!processNextMessage ())
    return false; // done

  return !inherited3::GetKey (olc::Key::ESCAPE).bPressed;
}

bool
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::OnUserDestroy ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::OnUserDestroy"));

  return true;
}

int
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::svc"));

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
        stop_processing = true;
        this->notify (STREAM_SESSION_MESSAGE_ABORT);
        return -1;
      } // end IF
      stop_processing = true;
      this->notify (STREAM_SESSION_MESSAGE_ABORT);
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
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::processNextMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::processNextMessage"));

  ACE_Message_Block* message_block_p = NULL;
  static ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int result = inherited::getq (message_block_p, &no_wait);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (likely (error == EWOULDBLOCK))
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
    return true; // stop PGE
  } // end IF

  return true; // continue PGE
}

int
Test_U_CameraFilter_WeightedVoronoiStipple_Filter::pointToSite (jcv_diagram& diagram, olc::vf2d& point)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraFilter_WeightedVoronoiStipple_Filter::pointToSite"));

  int result = 0;
  float min_distance_f = std::numeric_limits<float>::max ();

  const jcv_site* sites_p = jcv_diagram_get_sites (&diagram);
  for (int i = 0; i < diagram.numsites; i++)
  {
    olc::vf2d point_s (sites_p[i].p.x, sites_p[i].p.y);
    float distance_f = point.dist (point_s);
    if (distance_f < min_distance_f)
    {
      min_distance_f = distance_f;
      result = i;
    } // end IF
  } // end FOR

  return sites_p[result].index;
}
