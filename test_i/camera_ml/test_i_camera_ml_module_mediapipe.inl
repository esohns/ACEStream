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
Test_I_CameraML_Module_MediaPipe_T<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   MediaType>::Test_I_CameraML_Module_MediaPipe_T (ISTREAM_T* stream_in)
#else
                                   MediaType>::Test_I_CameraML_Module_MediaPipe_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , resolution_ ()
 , stride_ (0)
 , graph_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_T::Test_I_CameraML_Module_MediaPipe_T"));

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Test_I_CameraML_Module_MediaPipe_T<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::~Test_I_CameraML_Module_MediaPipe_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_T::~Test_I_CameraML_Module_MediaPipe_T"));

  delete graph_;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_MediaPipe_T<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_T::initialize"));

  if (inherited::isInitialized_)
  {
    delete graph_; graph_ = NULL;
  } // end IF

  // adapted from https://github.com/google/mediapipe/blob/master/mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt
  // runs face mesh for up to 1 face with both attention and previous landmark usage enabled
  const char* graph_string = R"(
    # MediaPipe graph that performs face mesh with TensorFlow Lite on CPU.

    # Input image. (ImageFrame)
    input_stream: "input_video"

    # Output image with rendered results. (ImageFrame)
    output_stream: "output_video"
    # Collection of detected/processed faces, each represented as a list of
    # landmarks. (std::vector<NormalizedLandmarkList>)
    output_stream: "multi_face_landmarks"

    # Throttles the images flowing downstream for flow control. It passes through
    # the very first incoming image unaltered, and waits for downstream nodes
    # (calculators and subgraphs) in the graph to finish their tasks before it
    # passes through another image. All images that come in while waiting are
    # dropped, limiting the number of in-flight images in most part of the graph to
    # 1. This prevents the downstream nodes from queuing up incoming images and data
    # excessively, which leads to increased latency and memory usage, unwanted in
    # real-time mobile applications. It also eliminates unnecessarily computation,
    # e.g., the output produced by a node may get dropped downstream if the
    # subsequent nodes are still busy processing previous inputs.
    node {
      calculator: "FlowLimiterCalculator"
      input_stream: "input_video"
      input_stream: "FINISHED:output_video"
      input_stream_info: {
        tag_index: "FINISHED"
        back_edge: true
      }
      output_stream: "throttled_input_video"
    }

    # Defines side packets for further use in the graph.
    node {
      calculator: "ConstantSidePacketCalculator"
      output_side_packet: "PACKET:0:num_faces"
      output_side_packet: "PACKET:1:use_prev_landmarks"
      output_side_packet: "PACKET:2:with_attention"
      node_options: {
        [type.googleapis.com/mediapipe.ConstantSidePacketCalculatorOptions]: {
          packet { int_value: 1 }
          packet { bool_value: true }
          packet { bool_value: true }
        }
      }
    }
    # Subgraph that detects faces and corresponding landmarks.
    node {
      calculator: "FaceLandmarkFrontCpu"
      input_stream: "IMAGE:throttled_input_video"
      input_side_packet: "NUM_FACES:num_faces"
      input_side_packet: "USE_PREV_LANDMARKS:use_prev_landmarks"
      input_side_packet: "WITH_ATTENTION:with_attention"
      output_stream: "LANDMARKS:multi_face_landmarks"
      output_stream: "ROIS_FROM_LANDMARKS:face_rects_from_landmarks"
      output_stream: "DETECTIONS:face_detections"
      output_stream: "ROIS_FROM_DETECTIONS:face_rects_from_detections"
    }
    # Subgraph that renders face-landmark annotation onto the input image.
    node {
      calculator: "FaceRendererCpu"
      input_stream: "IMAGE:throttled_input_video"
      input_stream: "LANDMARKS:multi_face_landmarks"
      input_stream: "NORM_RECTS:face_rects_from_landmarks"
      input_stream: "DETECTIONS:face_detections"
      output_stream: "IMAGE:output_video"
    }
  )";
  graph_ = mediapipe::LibMP::Create (graph_string,
                                     ACE_TEXT_ALWAYS_CHAR ("input_video"));
  if (unlikely (!graph_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::Create(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  bool result = graph_->AddOutputStream (ACE_TEXT_ALWAYS_CHAR ("multi_face_landmarks"));
  if (unlikely (!result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::AddOutputStream(), aborting\n"),
                inherited::mod_->name ()));
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
Test_I_CameraML_Module_MediaPipe_T<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_T::handleDataMessage"));

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

  // Feed RGB frame into MP face mesh graph (image data is COPIED internally by
  // LibMP)
  if (!graph_->Process (frame_matrix.data,
                        frame_matrix.cols, frame_matrix.rows,
                        mediapipe::ImageFormat::SRGB))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mediapipe::LibMP::Process(), aborting\n"),
                inherited::mod_->name ()));
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
    return;
  } // end IF
  graph_->WaitUntilIdle ();

  // stop inference clock
  auto t1 = std::chrono::high_resolution_clock::now ();
  int inference_time_ms =
    std::chrono::duration_cast<std::chrono::milliseconds> (t1 - t0).count ();

  // get landmark coordinates in custom data structure using helper function
  std::vector<std::vector<std::array<float, 3>>> normalized_landmarks =
    getLandmarks (graph_);

  // For each face, draw a circle at each landmark's position
  size_t num_faces = normalized_landmarks.size ();
  for (int face_num = 0; face_num < num_faces; face_num++)
    for (const std::array<float, 3>& norm_xyz: normalized_landmarks[face_num])
    {
      int x = static_cast<int> (norm_xyz[0] * frame_matrix.cols);
      int y = static_cast<int> (norm_xyz[1] * frame_matrix.rows);
      cv::circle (frame_matrix, cv::Point (x, y), 1, cv::Scalar (0, 255, 0), -1);
    } // end FOR

  cv::putText (frame_matrix, "# Faces Detected: " + std::to_string (num_faces),
               cv::Point (10, 40), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar (0, 255, 0));
  cv::putText (frame_matrix, "Inference time: " + std::to_string (inference_time_ms) + " ms",
               cv::Point (10, 60), cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar (0, 255, 0));

  // step3b: draw fps
  std::ostringstream converter;
  converter << fps;
  cv::putText (frame_matrix, converter.str ().substr (0, 5) + ACE_TEXT_ALWAYS_CHAR (" fps"),
               cv::Point (10, frame_matrix.rows - 3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar (255, 255, 255));
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_MediaPipe_T<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_T::handleSessionMessage"));

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
      stride_ = resolution_.cx * 3;
#else
      stride_ = resolution_.width * 3;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      break;

//error:
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
Test_I_CameraML_Module_MediaPipe_T<ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::getLandmarks (mediapipe::LibMP* graph_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_MediaPipe_T::getLandmarks"));

  std::vector<std::vector<std::array<float, 3>>> normalized_landmarks;

  // I use a unique_ptr for convenience, so that DeletePacket is called automatically
  // You could also manage deletion yourself, manually:
  // const void* packet = face_mesh->GetOutputPacket("multi_face_landmarks");
  // mediapipe::LibMP::DeletePacket(packet);
  std::unique_ptr<const void, decltype(&mediapipe::LibMP::DeletePacket)> lm_packet_ptr (nullptr, mediapipe::LibMP::DeletePacket);

  // Keep getting packets from queue until empty
  while (graph_in->GetOutputQueueSize ("multi_face_landmarks") > 0)
    lm_packet_ptr.reset (graph_in->GetOutputPacket ("multi_face_landmarks"));
  if (lm_packet_ptr.get () == nullptr || mediapipe::LibMP::PacketIsEmpty (lm_packet_ptr.get ()))
    return normalized_landmarks; // return empty vector if no output packets or packet is invalid

  // Create multi_face_landmarks from packet's protobuf data
  size_t num_faces = mediapipe::LibMP::GetPacketProtoMsgVecSize (lm_packet_ptr.get ());
  for (int face_num = 0; face_num < num_faces; face_num++)
  {
    // Get reference to protobuf message for face
    const void* lm_list_proto = mediapipe::LibMP::GetPacketProtoMsgAt (lm_packet_ptr.get (), face_num);
    // Get byte size of protobuf message
    size_t lm_list_proto_size = mediapipe::LibMP::GetProtoMsgByteSize (lm_list_proto);

    // Create buffer to hold protobuf message data; copy data to buffer
    std::shared_ptr<uint8_t[]> proto_data (new uint8_t[lm_list_proto_size]);
    mediapipe::LibMP::WriteProtoMsgData (proto_data.get (), lm_list_proto, static_cast<int> (lm_list_proto_size));

    // Initialize a mediapipe::NormalizedLandmarkList object from the buffer
    mediapipe::NormalizedLandmarkList face_landmarks;
    face_landmarks.ParseFromArray (proto_data.get (), static_cast<int> (lm_list_proto_size));

    // Copy the landmark data to our custom data structure
    normalized_landmarks.emplace_back ();
    for (const mediapipe::NormalizedLandmark& lm : face_landmarks.landmark ())
      normalized_landmarks[face_num].push_back ({ lm.x (), lm.y (), lm.z () });
  } // end FOR

  return normalized_landmarks;
}