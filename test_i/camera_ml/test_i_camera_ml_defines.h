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

#ifndef TEST_I_CAMERA_ML_DEFINES_H
#define TEST_I_CAMERA_ML_DEFINES_H

#define TEST_I_CAMERA_ML_DEFAULT_MODEL_FILE            "model.pb"
#define TEST_I_CAMERA_ML_DEFAULT_LABEL_FILE            "labels_map.pbtxt"

#define TEST_I_CAMERA_ML_DEFAULT_MAX_DETECTIONS_I      100
#define TEST_I_CAMERA_ML_DEFAULT_THRESHOLD_SCORE_F     0.5f

#if defined (MEDIAPIPE_SUPPORT)
#define TEST_I_CAMERA_ML_MEDIAPIPE_DEFAULT_NAME_STRING "MediaPipe"

// adapted from https://github.com/google/mediapipe/blob/master/mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt
// runs face mesh for up to 1 face with both attention and previous landmark usage enabled
#define TEST_I_CAMERA_ML_MEDIAPIPE_DEFAULT_FACE_GRAPH_STRING R"(
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
)"
#define TEST_I_CAMERA_ML_MEDIAPIPE_DEFAULT_FACE_OUTPUT_STREAM_STRING "multi_face_landmarks"

// ---------------------------------------
// adapted from https://github.com/google/mediapipe/blob/master/mediapipe/graphs/hand_tracking/hand_tracking_desktop_live.pbtxt
// runs for up to 2 hands with previous landmark usage enabled
#define TEST_I_CAMERA_ML_MEDIAPIPE_DEFAULT_HANDS_GRAPH_STRING R"(
  # MediaPipe graph that performs hands mesh with TensorFlow Lite on CPU.

  # Input image. (ImageFrame)
  input_stream: "input_video"

  # Collection of detected/processed hands, each represented as a list of
  # landmarks. (std::vector<NormalizedLandmarkList>)
  output_stream: "landmarks"

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
  #node {
  #  calculator: "FlowLimiterCalculator"
  #  input_stream: "input_video"
  #  input_stream: "FINISHED:output_video"
  #  input_stream_info: {
  #    tag_index: "FINISHED"
  #    back_edge: true
  #  }
  #  output_stream: "throttled_input_video"
  #}

  # Defines side packets for further use in the graph.
  node {
    calculator: "ConstantSidePacketCalculator"
    output_side_packet: "PACKET:0:num_hands"
    output_side_packet: "PACKET:1:use_prev_landmarks"
    node_options: {
      [type.googleapis.com/mediapipe.ConstantSidePacketCalculatorOptions]: {
        packet { int_value: 2 }
        packet { bool_value: true }
      }
    }
  }
  # Subgraph that detects hands and corresponding landmarks.
  node {
    calculator: "HandLandmarkTrackingCpu"
    input_stream: "IMAGE:input_video"
    input_side_packet: "NUM_HANDS:num_hands"
    input_side_packet: "USE_PREV_LANDMARKS:use_prev_landmarks"
    output_stream: "LANDMARKS:landmarks"
    output_stream: "HANDEDNESS:handedness"
    output_stream: "PALM_DETECTIONS:multi_palm_detections"
    output_stream: "HAND_ROIS_FROM_LANDMARKS:multi_hand_rects"
    output_stream: "HAND_ROIS_FROM_PALM_DETECTIONS:multi_palm_rects"
  }
)"
#define TEST_I_CAMERA_ML_MEDIAPIPE_DEFAULT_HANDS_OUTPUT_STREAM_STRING "landmarks"
#endif // MEDIAPIPE_SUPPORT

#endif
