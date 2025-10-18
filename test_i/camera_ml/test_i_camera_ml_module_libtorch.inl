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
#include <iomanip>
#include <regex>
#include <sstream>

#if defined (OPENCV_SUPPORT)
#include "opencv2/core/mat.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#endif // OPENCV_SUPPORT

#include "torch/torch.h"

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
Test_I_CameraML_Module_Libtorch_T<ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  MediaType>::Test_I_CameraML_Module_Libtorch_T (ISTREAM_T* stream_in)
#else
                                  MediaType>::Test_I_CameraML_Module_Libtorch_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , labels_ ()
 , resolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Libtorch_T::Test_I_CameraML_Module_Libtorch_T"));

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_I_CameraML_Module_Libtorch_T<ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Libtorch_T::initialize"));

  if (inherited::isInitialized_)
  {
    labels_.clear ();
  } // end IF

  if (!loadImageNetLabels (configuration_in.labelFile,
                           labels_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to loadImageNetLabels(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.labelFile.c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: loaded %Q label(s)\n"),
              inherited::mod_->name (),
              labels_.size ()));

  if (!inherited::initialize (configuration_in,
                              allocator_in))
    return false;
  //ACE_ASSERT (inherited::module_);

  return true;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_Libtorch_T<ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Libtorch_T::handleDataMessage"));

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

  // step1: preprocess image matrix
  cv::Mat frame_matrix_normalized;
  //cv::resize (frame_matrix, frame_matrix_normalized, cv::Size (224, 224), 0, 0, cv::INTER_LINEAR);
  cv::cvtColor (frame_matrix, frame_matrix_normalized, cv::COLOR_BGR2RGB);
  frame_matrix_normalized.convertTo (frame_matrix_normalized, CV_32FC3, 1.0f / 255.0f);

  torch::NoGradGuard no_grad;

  // step2: create input tensor from matrix
  torch::Tensor tensor_image =
    torch::from_blob (frame_matrix_normalized.data, {1, frame_matrix_normalized.rows, frame_matrix_normalized.cols, 3}, torch::kFloat);
  tensor_image = tensor_image.permute ({0, 3, 1, 2});
  tensor_image[0][0] = tensor_image[0][0].sub_ (0.485f).div_ (0.229f);
  tensor_image[0][1] = tensor_image[0][1].sub_ (0.456f).div_ (0.224f);
  tensor_image[0][2] = tensor_image[0][2].sub_ (0.406f).div_ (0.225f);
  // to GPU ?
  tensor_image = tensor_image.to (inherited::device_);

  // step3: run inference
  torch::Tensor outputs = inherited::module_.forward ({tensor_image}).toTensor ();

  // step4: process results
  std::tuple<torch::Tensor, torch::Tensor> results = outputs.sort (-1, true);
  torch::Tensor softmaxs = std::get<0> (results)[0].softmax (0);
  torch::Tensor indexs = std::get<1> (results)[0];

  // step5: print results
  for (int i = 0; i < 3; ++i)
  {
    int idx = indexs[i].item<int> ();
    std::cout << ACE_TEXT_ALWAYS_CHAR ("    ============= Top-") << i + 1
              << ACE_TEXT_ALWAYS_CHAR (" =============") << std::endl;
    std::cout << ACE_TEXT_ALWAYS_CHAR ("    Label:  ") << labels_[idx] << std::endl;
    std::cout << ACE_TEXT_ALWAYS_CHAR ("    With Probability:  ")
              << softmaxs[i].item<float> () * 100.0f << ACE_TEXT_ALWAYS_CHAR ("%") << std::endl;
  } // end FOR

  // step6: draw fps
  std::ostringstream converter;
  converter << fps;
  cv::putText (frame_matrix,
               ACE_TEXT_ALWAYS_CHAR ("fps: ") + converter.str ().substr (0, 5),
               cv::Point (12, frame_matrix.rows - 25),
               cv::FONT_HERSHEY_SIMPLEX,
               0.4,
               cv::Scalar (255, 255, 255),
               1, 
               cv::LineTypes::LINE_8,
               false);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_I_CameraML_Module_Libtorch_T<ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Libtorch_T::handleSessionMessage"));

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
bool
Test_I_CameraML_Module_Libtorch_T<ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType>::loadImageNetLabels (const std::string& filePath_in,
                                                                  std::vector<std::string>& labels_inout)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraML_Module_Libtorch_T::loadImageNetLabels"));

  std::ifstream ifs (filePath_in);
  if (!ifs)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to load file (was: \"%s\", aborting\n"),
                ACE_TEXT (filePath_in.c_str ())));
    return false;
  } // end IF

  std::string line;
  while (std::getline (ifs, line))
    labels_inout.push_back (line);

  return true;
}
