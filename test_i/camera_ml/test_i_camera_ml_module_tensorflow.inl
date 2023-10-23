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

#include <fstream>
#include <regex>
#include <set>
#include <sstream>

#if defined (OPENCV_SUPPORT)
#include "opencv2/imgproc/imgproc.hpp"
#endif // OPENCV_SUPPORT

#if defined (TENSORFLOW_CC_SUPPORT)
#include "tensorflow/core/framework/types.pb.h"
#endif // TENSORFLOW_CC_SUPPORT

#include "ace/Log_Msg.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#else
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_macros.h"

#if defined (TENSORFLOW_CC_SUPPORT)
template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Test_I_CameraML_Module_Tensorflow_2<ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                    MediaType>::Test_I_CameraML_Module_Tensorflow_2 (ISTREAM_T* stream_in)
#else
                                    MediaType>::Test_I_CameraML_Module_Tensorflow_2 (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , labelMap_ ()
 , resolution_ ()
 , shape_ ()
 , stride_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Tensorflow_2::Test_I_CameraML_Module_Tensorflow_2"));

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_Tensorflow_2<ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    MediaType>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Tensorflow_2::initialize"));

  if (inherited::isInitialized_)
  {
  } // end IF

  if (!loadLabels (configuration_in.labelFile))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to loadLabels(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.labelFile.c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: loaded %Q label(s)\n"),
              inherited::mod_->name (),
              labelMap_.size ()));

  return inherited::initialize (configuration_in,
                                allocator_in);
}


template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_Tensorflow_2<ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Tensorflow_2::handleDataMessage"));

  // set input & output nodes names
  static std::string inputLayer = "image_tensor:0";
  static std::vector<std::string> outputLayer =
    {"detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0"};

  static int nFrames = 30;
  static int iFrame = 0;
  static double fps = 0.0;
  static time_t start = time (NULL);
  static time_t end;

  if (nFrames % (iFrame + 1) == 0)
  {
    time (&end);
    fps = nFrames / difftime (end, start);
    time (&start);
  } // end IF
  iFrame++;

  std::vector<tensorflow::Tensor> outputs;
  std::vector<size_t> good_indices_a;

//  tensorflow::Tensor tensor = tensorflow::Tensor (tensorflow::DT_FLOAT, shape_);
  tensorflow::Tensor tensor = tensorflow::Tensor (tensorflow::DT_UINT8, shape_);

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

//  float* data_p = tensor.flat<float> ().data ();
  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
//  cv::Mat float_matrix (frame_matrix.rows, frame_matrix.cols, CV_32FC3, data_p);
//  frame_matrix.convertTo (float_matrix, CV_32FC3);

//  auto input_tensor_mapped = tensor.tensor<float, 4> ();
  auto input_tensor_mapped = tensor.tensor<uint8_t, 4> ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  for (int y = 0; y < static_cast<int> (resolution_.cy); ++y)
#else
  for (int y = 0; y < static_cast<int> (resolution_.height); ++y)
#endif // ACE_WIN32 || ACE_WIN64
  {
    const uchar* source_row = (uchar*)data_p + (y * stride_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    for (int x = 0; x < static_cast<int> (resolution_.cx); ++x)
#else
    for (int x = 0; x < static_cast<int> (resolution_.width); ++x)
#endif // ACE_WIN32 || ACE_WIN64
    {
      const uchar* source_pixel = source_row + (x * 3);
      for (int c = 0; c < 3; ++c)
      {
        const uchar* source_value = source_pixel + c;
//        input_tensor_mapped (0, y, x, c) = (float)*source_value;
        input_tensor_mapped (0, y, x, c) = (uint8_t)*source_value;
      } // end FOR
    } // end FOR
  } // end FOR

  // run the graph on tensor
  tensorflow::Status status = inherited::session_->Run ({{inputLayer, tensor}},
                                                        outputLayer, {},
                                                        &outputs);
  if (unlikely (!status.ok ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Session::Run(), aborting\n"),
                inherited::mod_->name ()));
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
    return;
  } // end IF

  // extract results from the outputs vector
  tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float> ();
  tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float> ();
  tensorflow::TTypes<float>::Flat number_of_detections = outputs[3].flat<float> ();
  tensorflow::TTypes<float, 3>::Tensor boxes = outputs[0].flat_outer_dims<float, 3> ();
  ACE_UNUSED_ARG (number_of_detections);

  good_indices_a = filterBoxes (scores, boxes, 0.8, 0.5);
//  for (size_t i = 0; i < goodIdxs.size(); i++)
//      LOG(INFO) << "score:" << scores(goodIdxs.at(i)) << ",class:" << labelsMap[classes(goodIdxs.at(i))]
//                << " (" << classes(goodIdxs.at(i)) << "), box:" << "," << boxes(0, goodIdxs.at(i), 0) << ","
//                << boxes(0, goodIdxs.at(i), 1) << "," << boxes(0, goodIdxs.at(i), 2) << ","
//                << boxes(0, goodIdxs.at(i), 3);

  // draw bboxes and captions
  drawBoundingBoxes (frame_matrix, scores, classes, boxes, good_indices_a);

  // draw fps
  cv::putText (frame_matrix,
               std::to_string (fps).substr (0, 5),
               cv::Point (0, frame_matrix.rows),
               cv::FONT_HERSHEY_SIMPLEX,
               0.7,
               cv::Scalar (255, 255, 255));
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_Tensorflow_2<ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Tensorflow_2::handleSessionMessage"));

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
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      stride_ = resolution_.cx * 3;
#else
      ACE_ASSERT (Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_s.format.pixelformat) == 24);
      resolution_.height = media_type_s.format.height;
      resolution_.width = media_type_s.format.width;
      stride_ = resolution_.width * 3;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      stride_ = resolution_.cx * 3;
#else
      stride_ = resolution_.width * 3;
#endif // ACE_WIN32 || ACE_WIN64

      shape_ = tensorflow::TensorShape ();
      shape_.AddDim (1);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      shape_.AddDim (resolution_.cy);
      shape_.AddDim (resolution_.cx);
#else
      shape_.AddDim (resolution_.height);
      shape_.AddDim (resolution_.width);
#endif // ACE_WIN32 || ACE_WIN64
      shape_.AddDim (3);

      break;

//error:
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
bool
Test_I_CameraML_Module_Tensorflow_2<ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    MediaType>::loadLabels (const std::string& fileName_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Tensorflow_2::loadLabels"));

  labelMap_.clear ();

  // read file into a string
  std::ifstream file_stream (fileName_in);
  if (file_stream.bad ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to open label map file (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (fileName_in.c_str ())));
    return false;
  } // end IF
  std::stringstream string_stream;
  string_stream << file_stream.rdbuf ();
  std::string file_string = string_stream.str ();

  // search entry patterns of type 'item { ... }' and parse each of them
  std::smatch matcherEntry;
  std::smatch matcherId;
  std::smatch matcherName;
  std::regex reEntry ("item \\{([\\S\\s]*?)\\}");
  std::regex reId ("[0-9]+");
  std::regex reName ("\'.+\'");
  std::string entry;

  std::sregex_iterator stringBegin (file_string.begin (), file_string.end (), reEntry);
  std::sregex_iterator stringEnd;

  int id;
  std::string name;
  for (std::sregex_iterator i = stringBegin; i != stringEnd; i++)
  {
    matcherEntry = *i;
    entry = matcherEntry.str ();
    std::regex_search (entry, matcherId, reId);
    if (!matcherId.empty ())
      id = stoi (matcherId[0].str ());
    else
      continue;
    std::regex_search (entry, matcherName, reName);
    if (!matcherName.empty ())
      name = matcherName[0].str ().substr (1, matcherName[0].str ().length () - 2);
    else
      continue;
    labelMap_.insert (std::pair<int, std::string> (id, name));
  } // end FOR

  return true;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
std::vector<size_t>
Test_I_CameraML_Module_Tensorflow_2<ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    MediaType>::filterBoxes (tensorflow::TTypes<float>::Flat& scores_in,
                                                             tensorflow::TTypes<float, 3>::Tensor& boxes_in,
                                                             double thresholdIOU_in,
                                                             double thresholdScore_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Tensorflow_2::filterBoxes"));

  std::vector<size_t> sortIdxs (scores_in.size ());
  std::iota (sortIdxs.begin (), sortIdxs.end (), 0);

  // Create set of "bad" idxs
  std::set<size_t> badIdxs;
  size_t i = 0;
  while (i < sortIdxs.size ())
  {
    if (scores_in (sortIdxs.at (i)) < thresholdScore_in)
      badIdxs.insert (sortIdxs[i]);
    if (badIdxs.find (sortIdxs.at (i)) != badIdxs.end ())
    {
      i++;
      continue;
    } // end IF

//    Rect2f box1 = Rect2f(Point2f(boxes(0, sortIdxs.at(i), 1), boxes(0, sortIdxs.at(i), 0)),
//                         Point2f(boxes(0, sortIdxs.at(i), 3), boxes(0, sortIdxs.at(i), 2)));
//    for (size_t j = i + 1; j < sortIdxs.size(); j++) {
//        if (scores(sortIdxs.at(j)) < thresholdScore) {
//            badIdxs.insert(sortIdxs[j]);
//            continue;
//        }
//        Rect2f box2 = Rect2f(Point2f(boxes(0, sortIdxs.at(j), 1), boxes(0, sortIdxs.at(j), 0)),
//                             Point2f(boxes(0, sortIdxs.at(j), 3), boxes(0, sortIdxs.at(j), 2)));
//        if (IOU(box1, box2) > thresholdIOU)
//            badIdxs.insert(sortIdxs[j]);
//    }
    i++;
  } // end WHILE

  // Prepare "good" idxs for return
  std::vector<size_t> goodIdxs;
  for (std::vector<size_t>::iterator iterator = sortIdxs.begin ();
       iterator != sortIdxs.end ();
       ++iterator)
    if (badIdxs.find (sortIdxs.at (*iterator)) == badIdxs.end ())
      goodIdxs.push_back (*iterator);

  return goodIdxs;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_Tensorflow_2<ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    MediaType>::drawBoundingBoxes (cv::Mat& image_in,
                                                                   tensorflow::TTypes<float>::Flat& scores_in,
                                                                   tensorflow::TTypes<float>::Flat& classes_in,
                                                                   tensorflow::TTypes<float,3>::Tensor& boxes_in,
                                                                   std::vector<size_t>& indices_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Tensorflow_2::drawBoundingBoxes"));

  for (size_t j = 0;
       j < indices_in.size ();
       j++)
  {
    double xMin = boxes_in (0, indices_in.at (j), 1);
    double xMax = boxes_in (0, indices_in.at (j), 3);
    double yMin = boxes_in (0, indices_in.at (j), 0);
    double yMax = boxes_in (0, indices_in.at (j), 2);

    cv::Point tl, br;
    tl = cv::Point((int) (xMin * image_in.cols), (int) (yMin * image_in.rows));
    br = cv::Point((int) (xMax * image_in.cols), (int) (yMax * image_in.rows));
    cv::rectangle (image_in, tl, br, cv::Scalar (0, 255, 255), 1);

    // Ceiling the score down to 3 decimals (weird!)
    float scoreRounded = std::floor (scores_in (indices_in.at (j)) * 1000.0f) / 1000.0f;
    std::string score_string = std::to_string (scoreRounded).substr (0, 5);
    std::string caption = labelMap_[classes_in (indices_in.at(j))] + " (" + score_string + ")";

    // Adding caption of type "LABEL (X.XXX)" to the top-left corner of the bounding box
    int fontCoeff = 12;
    cv::Point brRect = cv::Point (tl.x + caption.length () * fontCoeff / 1.6, tl.y + fontCoeff);
    cv::rectangle (image_in, tl, brRect, cv::Scalar (0, 255, 255), -1);
    cv::Point textCorner = cv::Point (tl.x, tl.y + fontCoeff * 0.9);
    cv::putText (image_in, caption, textCorner, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar (255, 0, 0));
  } // end FOR
}
#endif // TENSORFLOW_CC_SUPPORT
