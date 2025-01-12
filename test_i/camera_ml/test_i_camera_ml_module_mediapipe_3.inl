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

#include <chrono>
#include <vector>

#if defined (OPENCV_SUPPORT)
#include "opencv2/core/mat.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif // OPENCV_SUPPORT

#include "mediapipe/framework/formats/image_format.pb.h"
#include "mediapipe/framework/formats/landmark.pb.h"

#include "ace/Log_Msg.h"

#include "common_gl_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#else
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_macros.h"

#include "test_i_camera_ml_defines.h"

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   MediaType>::Test_I_CameraML_Module_MediaPipe_3 (ISTREAM_T* stream_in)
#else
                                   MediaType>::Test_I_CameraML_Module_MediaPipe_3 (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , resolution_ ()
 , graph_ (NULL)
 , world_ (NULL)
 , halfDimension_ (0.0f)
 , bridge_ ()
 , bridgeRope_ (NULL)
 , balls_ ()
 , positionThumb_ (b2Vec2_zero)
 , positionIndex_ (b2Vec2_zero)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::Test_I_CameraML_Module_MediaPipe_3"));

  uint32 flags = 0;
  flags += b2Draw::e_shapeBit;
  //flags += b2Draw::e_aabbBit;
  //flags += b2Draw::e_pairBit;
  //flags += b2Draw::e_centerOfMassBit;
  flags += b2Draw::e_jointBit;
  // flags += b2Draw::e_particleBit;
  b2Draw::SetFlags (flags);

  b2Vec2 gravity (0.0f, TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_GRAVITY);
  world_ = new b2World (gravity);
  world_->SetAllowSleeping (true);
  world_->SetDebugDraw (this);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::~Test_I_CameraML_Module_MediaPipe_3 ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::~Test_I_CameraML_Module_MediaPipe_3"));

  delete graph_;
  delete world_;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::initialize"));

  if (inherited::isInitialized_)
  {
    delete graph_; graph_ = NULL;
    delete world_; world_ = NULL;
  } // end IF

  graph_ = mediapipe::LibMP::Create (configuration_in.model.c_str (),
                                     ACE_TEXT_ALWAYS_CHAR ("input_video"));
  if (unlikely (!graph_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::Create(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  bool result = graph_->AddOutputStream (configuration_in.outputStream.c_str ());
  if (unlikely (!result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::AddOutputStream(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.outputStream.c_str ())));
    return false;
  } // end IF

  result = graph_->Start ();
  if (unlikely (!result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::Start(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::DrawPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
  float x1, x2, y1, y2;

  for (int32 i = 0; i < vertexCount; ++i)
  {
    if (i == vertexCount - 1)
      break;

    x1 =
      Common_GL_Tools::map (vertices[i].x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth () - 1));
    y1 =
      Common_GL_Tools::map (vertices[i].y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight () - 1));
    x2 =
      Common_GL_Tools::map (vertices[i + 1].x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth () - 1));
    y2 =
      Common_GL_Tools::map (vertices[i + 1].y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight () - 1));
    olc::PixelGameEngine::DrawLine (static_cast<int32_t> (x1), static_cast<int32_t> (y1),
                                    static_cast<int32_t> (x2), static_cast<int32_t> (y2),
                                    {static_cast<uint8_t> (color.r * 255.0f), static_cast<uint8_t> (color.g * 255.0f), static_cast<uint8_t> (color.b * 255.0f), 255},
                                    0xFFFFFFFF);
  } // end FOR
  x1 =
    Common_GL_Tools::map (vertices[vertexCount - 1].x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth () - 1));
  y1 =
    Common_GL_Tools::map (vertices[vertexCount - 1].y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight () - 1));
  x2 =
    Common_GL_Tools::map (vertices[0].x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth () - 1));
  y2 =
    Common_GL_Tools::map (vertices[0].y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight () - 1));
  olc::PixelGameEngine::DrawLine (static_cast<int32_t> (x1), static_cast<int32_t> (y1),
                                  static_cast<int32_t> (x2), static_cast<int32_t> (y2),
                                  {static_cast<uint8_t> (color.r * 255.0f), static_cast<uint8_t> (color.g * 255.0f), static_cast<uint8_t> (color.b * 255.0f), 255},
                                  0xFFFFFFFF);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::DrawSolidPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
  DrawPolygon (vertices, vertexCount, color);
  // *TODO*: flood-fill ?
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::DrawCircle (const b2Vec2& center, float32 radius, const b2Color& color)
{
  float x, y;
  x =
    Common_GL_Tools::map (center.x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth() - 1));
  y =
    Common_GL_Tools::map (center.y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight() - 1));
  olc::PixelGameEngine::DrawCircle (static_cast<int32_t> (x), static_cast<int32_t> (y),
                                    static_cast<int32_t> (radius),
                                    {static_cast<uint8_t> (color.r * 255.0f), static_cast<uint8_t> (color.g * 255.0f), static_cast<uint8_t> (color.b * 255.0f), 255},
                                    0xFF);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
  float x, y;
  x =
    Common_GL_Tools::map (center.x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth() - 1));
  y =
    Common_GL_Tools::map (center.y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight() - 1));
  olc::PixelGameEngine::FillCircle (static_cast<int32_t> (x), static_cast<int32_t> (y),
                                    static_cast<int32_t> (radius),
                                    {static_cast<uint8_t> (color.r * 255.0f), static_cast<uint8_t> (color.g * 255.0f), static_cast<uint8_t> (color.b * 255.0f), 255});

  // draw line to see angular velocity
  //float a_deg_f = std::atan2 (axis.y, axis.x) * 180.0f / static_cast<float> (M_PI);
  olc::vf2d pos1 = {x, y};
  olc::vf2d pos2 = {x + (axis.x * radius), y + (axis.y * radius)};
  olc::PixelGameEngine::DrawLine (pos1, pos2,
                                  olc::BLACK,
                                  0xFFFFFFFF);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::DrawParticles (const b2Vec2* centers, float32 radius, const b2ParticleColor* colors, int32 count)
{
  float x, y;

  for (int32 i = 0; i < count; ++i)
  {
    x =
      Common_GL_Tools::map (centers[i].x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth() - 1));
    y =
      Common_GL_Tools::map (centers[i].y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight() - 1));
    olc::PixelGameEngine::DrawCircle (static_cast<int32_t> (x), static_cast<int32_t> (y),
                                      static_cast<int32_t> (radius),
                                      colors ? olc::Pixel (static_cast<uint8_t> (colors[i].r * 255.0f), static_cast<uint8_t> (colors[i].g * 255.0f), static_cast<uint8_t> (colors[i].b * 255.0f), 255) : olc::WHITE,
                                      0xFF);
  } // end FOR
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
  float x1, y1, x2, y2;
  x1 =
    Common_GL_Tools::map (p1.x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth() - 1));
  y1 =
    Common_GL_Tools::map (p1.y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight() - 1));
  x2 =
    Common_GL_Tools::map (p2.x, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth() - 1));
  y2 =
    Common_GL_Tools::map (p2.y, -halfDimension_, halfDimension_, 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight() - 1));
  olc::PixelGameEngine::DrawLine (static_cast<int32_t> (x1), static_cast<int32_t> (y1),
                                  static_cast<int32_t> (x2), static_cast<int32_t> (y2),
                                  {static_cast<uint8_t> (color.r * 255.0f), static_cast<uint8_t> (color.g * 255.0f), static_cast<uint8_t> (color.b * 255.0f), 255},
                                  0xFFFFFFFF);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::DrawTransform (const b2Transform& xf)
{
  b2Vec2 p1 = xf.p, p2;
  static float32 k_axisScale_x = 0.4f;
  p2 = p1 + k_axisScale_x * xf.q.GetXAxis ();
  DrawSegment (p1, p2, b2Color (1, 0, 0));

  static float32 k_axisScale_y = 0.4f;
  p2 = p1 + k_axisScale_y * xf.q.GetYAxis ();
  DrawSegment (p1, p2, b2Color (0, 1, 0));
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::handleDataMessage"));

  static int nFrames = 30;
  static int iFrame = 0;
  static float fps = 0.0f;
  static time_t start = time (NULL);
  static time_t end;
  if (nFrames % (iFrame + 1) == 0)
  {
    time (&end);
    fps = nFrames / (float)difftime (end, start);
    time (&start);
  } // end IF
  iFrame++;

  // step0: convert image frame to matrix
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  cv::Mat frame_matrix (resolution_.cy,
                        resolution_.cx,
#else
  cv::Mat frame_matrix (resolution_.height,
                        resolution_.width,
#endif // ACE_WIN32 || ACE_WIN64
                        CV_8UC3,
                        message_inout->rd_ptr (),
                        cv::Mat::AUTO_STEP);

  // step1: run the graph on the image frame
  // start inference clock
  auto t0 = std::chrono::high_resolution_clock::now ();

  // feed RGB frame into MP graph (image data is COPIED internally by LibMP)
  if (unlikely (!graph_->Process (frame_matrix.data,
                                  frame_matrix.cols, frame_matrix.rows,
                                  mediapipe::ImageFormat::SRGB)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::Process(), aborting\n"),
                inherited::mod_->name ()));
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
    return;
  } // end IF
  if (unlikely (!graph_->WaitUntilIdle ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::WaitUntilIdle(), aborting\n"),
                inherited::mod_->name ()));
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
    return;
  } // end IF

  // stop inference clock
  auto t1 = std::chrono::high_resolution_clock::now ();
  int inference_time_ms =
    std::chrono::duration_cast<std::chrono::milliseconds> (t1 - t0).count ();

  // get landmark coordinates in custom data structure using helper function
  std::vector<std::vector<std::array<float, 3> > > normalized_landmarks =
    getLandmarks (graph_);

  size_t num_objs = normalized_landmarks.size ();
  for (int i = 0; i < num_objs; i++)
  {
    float x_translated_f, y_translated_f;
    int j = 0;
    for (const std::array<float, 3>& norm_xyz : normalized_landmarks[i])
    {
      int x = static_cast<int> (norm_xyz[0] * frame_matrix.cols);
      int y = static_cast<int> (norm_xyz[1] * frame_matrix.rows);
      //cv::circle (frame_matrix, cv::Point (x, y), 1, cv::Scalar (0, 255, 0));

      if (j == 4 || j == 8) // thumb 'n index finger
      {
        x_translated_f =
          Common_GL_Tools::map (static_cast<float> (x), 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenWidth () - 1), -halfDimension_, halfDimension_);
        y_translated_f =
          Common_GL_Tools::map (static_cast<float> (y), 0.0f, static_cast<float> (olc::PixelGameEngine::ScreenHeight() - 1), -halfDimension_, halfDimension_);
        if (j == 4)
          positionThumb_.Set (x_translated_f, y_translated_f);
        else
          positionIndex_.Set (x_translated_f, y_translated_f);
      } // end IF

      j++;
    } // end FOR
  } // end FOR

  //cv::putText (frame_matrix, ACE_TEXT_ALWAYS_CHAR ("# Objects Detected: ") + std::to_string (num_objs),
  //             cv::Point (10, 40), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar (0, 255, 0));
  //cv::putText (frame_matrix, ACE_TEXT_ALWAYS_CHAR ("Inference time: ") + std::to_string (inference_time_ms) + ACE_TEXT_ALWAYS_CHAR (" ms"),
  //             cv::Point (10, 60), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar (0, 255, 0));

  // step3b: draw fps
  //std::ostringstream converter;
  //converter << fps;
  //cv::putText (frame_matrix, converter.str ().substr (0, 5) + ACE_TEXT_ALWAYS_CHAR (" fps"),
  //             cv::Point (10, frame_matrix.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar (255, 255, 255));

  // update frame data
  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  static int screen_width_i = inherited3::ScreenWidth (), screen_height_i = inherited3::ScreenHeight ();

  for (int y = 0; y < screen_height_i; y++)
    for (int x = 0; x < screen_width_i; x++)
    {
      // draw image
      inherited3::Draw (x, y, olc::Pixel (data_p[0], data_p[1], data_p[2], 255U));

      data_p += 3;
    } // end FOR
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_V4L_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_s) == 24);
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
#else
      ACE_ASSERT (Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_s.format.pixelformat) == 24);
      resolution_.height = media_type_s.format.height;
      resolution_.width = media_type_s.format.width;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

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

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
std::vector<std::vector<std::array<float, 3>>>
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::getLandmarks (mediapipe::LibMP* graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::getLandmarks"));

  std::vector<std::vector<std::array<float, 3> > > normalized_landmarks;

  // I use a unique_ptr for convenience, so that DeletePacket is called automatically
  // You could also manage deletion yourself, manually:
  // const void* packet = face_mesh->GetOutputPacket("multi_face_landmarks");
  // mediapipe::LibMP::DeletePacket(packet);
  std::unique_ptr<const void, decltype (&mediapipe::LibMP::DeletePacket)> lm_packet_ptr (nullptr, mediapipe::LibMP::DeletePacket);

  // Keep getting packets from queue until empty
  while (graph_in->GetOutputQueueSize (inherited::configuration_->outputStream.c_str ()) > 0)
    lm_packet_ptr.reset (graph_in->GetOutputPacket (inherited::configuration_->outputStream.c_str ()));
  if (lm_packet_ptr.get () == nullptr || mediapipe::LibMP::PacketIsEmpty (lm_packet_ptr.get ()))
    return normalized_landmarks; // return empty vector if no output packets or packet is invalid

  // Create landmarks from packet's protobuf data
  size_t num_objs = mediapipe::LibMP::GetPacketProtoMsgVecSize (lm_packet_ptr.get ());
  for (size_t i = 0; i < num_objs; i++)
  {
    // Get reference to protobuf message for face
    const void* lm_list_proto = mediapipe::LibMP::GetPacketProtoMsgAt (lm_packet_ptr.get (), i);
    // Get byte size of protobuf message
    size_t lm_list_proto_size = mediapipe::LibMP::GetProtoMsgByteSize (lm_list_proto);

    // Create buffer to hold protobuf message data; copy data to buffer
    std::shared_ptr<uint8_t[]> proto_data (new uint8_t[lm_list_proto_size]);
    mediapipe::LibMP::WriteProtoMsgData (proto_data.get (), lm_list_proto, static_cast<int> (lm_list_proto_size));

    // Initialize a mediapipe::NormalizedLandmarkList object from the buffer
    mediapipe::NormalizedLandmarkList landmarks;
    landmarks.ParseFromArray (proto_data.get (), static_cast<int> (lm_list_proto_size));

    // Copy the landmark data to our custom data structure
    normalized_landmarks.emplace_back ();
    for (const mediapipe::NormalizedLandmark& lm : landmarks.landmark ())
      normalized_landmarks[i].push_back ({ lm.x (), lm.y (), lm.z () });
  } // end FOR

  return normalized_landmarks;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnUserCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::OnUserCreate"));

  halfDimension_ =
    std::min (olc::PixelGameEngine::ScreenWidth (), olc::PixelGameEngine::ScreenHeight ()) / 2.0f;
  initializeBox2d ();

  return true;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnUserUpdate (float fElapsedTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::OnUserUpdate"));

  // process next message
  if (!processNextMessage ())
    return false; // done

  // handle bridge
  b2Body* body_p = *bridge_.begin ();
  b2Transform transform = body_p->GetTransform ();
  body_p->SetTransform (positionThumb_, transform.q.GetAngle ());
  body_p = *(bridge_.end () - 1);
  transform = body_p->GetTransform ();
  body_p->SetTransform (positionIndex_, transform.q.GetAngle ());

  // handle balls
  if (Common_Tools::testRandomProbability (0.1f))
    balls_.push_back (new ball (world_, halfDimension_));
  for (std::vector<ball*>::iterator iterator = balls_.begin ();
       iterator != balls_.end ();
       )
  {
    if ((*iterator)->isGone (halfDimension_))
    {
      delete *iterator;
      iterator = balls_.erase (iterator);
      continue;
    } // end IF

    ++iterator;
  } // end FOR

  static float32 time_step_f =
    1.0f / TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_STEP_FPS;
  world_->Step (time_step_f,
                TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_ITER_VEL_PER_STEP,
                TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_ITER_POS_PER_STEP);
  world_->DrawDebugData ();

  return !inherited3::GetKey (olc::Key::ESCAPE).bPressed;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnUserDestroy ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_PGE_T::OnUserDestroy"));

  ACE_ASSERT (world_);
  world_->DestroyJoint (bridgeRope_); bridgeRope_ = NULL;
  for (std::vector<b2Body*>::iterator iterator = bridge_.begin ();
        iterator != bridge_.end ();
        ++iterator)
    (*iterator)->GetWorld ()->DestroyBody (*iterator);
  bridge_.clear ();
  for (std::vector<ball*>::iterator iterator = balls_.begin ();
       iterator != balls_.end ();
       ++iterator)
    delete *iterator;
  balls_.clear ();

  return true;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
int
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
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
        typename inherited::SESSION_MESSAGE_T* session_message_p =
          static_cast<typename inherited::SESSION_MESSAGE_T*> (message_block_p);
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

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::processNextMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::processNextMessage"));

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

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_3<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::initializeBox2d ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_3::initializeBox2d"));

  b2BodyDef bodyDef;
  //bodyDef.allowSleep = false;
  bodyDef.type = b2_dynamicBody;
  //bodyDef.type = b2_staticBody;
  b2FixtureDef fixtureDef;
  fixtureDef.density =
    TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_DENSITY;
  fixtureDef.friction =
    TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_FRICTION;
  fixtureDef.restitution =
    TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_RESTITUTION;
  b2CircleShape circleShape;
  circleShape.m_radius = TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS;
  fixtureDef.shape = &circleShape;

#if defined (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_USE_REVOLUTE_JOINTS)
  b2RevoluteJointDef jointDef;
  //jointDef.collideConnected = false;
  jointDef.localAnchorA.Set (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS, 0.0f);
  jointDef.localAnchorB.Set (-TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS, 0.0f);
#elif defined (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_USE_DISTANCE_JOINTS)
   b2DistanceJointDef jointDef;
   jointDef.collideConnected = false;
  //jointDef.localAnchorA.Set (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS, 0.0f);
  //jointDef.localAnchorB.Set (-TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS, 0.0f);
   jointDef.length =
     TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_JOINT_LENGTH;
   jointDef.dampingRatio =
     TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_JOINT_DAMP_RATIO;
   jointDef.frequencyHz =
     TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_JOINT_FREQ_HZ;
#else
  b2RopeJointDef jointDef;
  jointDef.localAnchorA.SetZero ();
  jointDef.localAnchorB.SetZero ();
  //jointDef.collideConnected = false;
  jointDef.maxLength = 0.1f;
#endif

  b2Body* link = world_->CreateBody (&bodyDef);
  ACE_ASSERT (link);
  //bodyDef.type = b2_dynamicBody;

  link->CreateFixture (&fixtureDef);
  bridge_.push_back (link);
  for (int i = 0; i < TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_NUMBER_OF_BRIDGE_LINKS - 1; i++)
  {
    //if (i == TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_NUMBER_OF_BRIDGE_LINKS - 2)
    //  bodyDef.type = b2_staticBody;
    bodyDef.position.Set (static_cast<float> (i + 1) * TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_JOINT_LENGTH, 0.0f);
    b2Body* newLink = world_->CreateBody (&bodyDef);
    ACE_ASSERT (newLink);
    newLink->CreateFixture (&fixtureDef);

    jointDef.bodyA = link;
    jointDef.bodyB = newLink;
    world_->CreateJoint (&jointDef);

    link = newLink; // prepare for next iteration
    bridge_.push_back (link);
  } // end FOR

  b2RopeJointDef jointDef_2;
  jointDef_2.bodyA = *bridge_.begin ();
  jointDef_2.bodyB = link;
  jointDef_2.localAnchorA.SetZero ();
  jointDef_2.localAnchorB.SetZero ();
  //jointDef_2.collideConnected = false;
  jointDef_2.maxLength = (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_NUMBER_OF_BRIDGE_LINKS - 1) * (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS * 2.0f);
  bridgeRope_ = world_->CreateJoint (&jointDef_2);
  ACE_ASSERT (bridgeRope_);
}
