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

//#include "opencv2/opencv.hpp"
#include "opencv2/core/cvstd.hpp"
#include "opencv2/core/mat.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Visualization_OpenCVClassifier_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                        MediaType>::Stream_Visualization_OpenCVClassifier_T (ISTREAM_T* stream_in)
#else
                                        MediaType>::Stream_Visualization_OpenCVClassifier_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , cascadeClassifier_ ()
 , mediaType_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCVClassifier_T::Stream_Visualization_OpenCVClassifier_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Visualization_OpenCVClassifier_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
                                        MediaType>::initialize (const ConfigurationType& configuration_in,
                                                                Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCVClassifier_T::initialize"));

  if (inherited::isInitialized_)
  {
    ACE_OS::memset (&mediaType_, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));
  } // end IF

  bool result = false;
  try {
    result =
        cascadeClassifier_.load (cv::String (configuration_in.cascadeFile.c_str ()));
  } catch (cv::Exception& e) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in cv::BaseCascadeClassifier::load(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (e.what ())));
  }
  if (!result || cascadeClassifier_.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to load cascade file (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.cascadeFile.c_str ())));
    return false;
  } // end IF

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: loaded cascade file \"%s\"\n"),
              inherited::mod_->name (),
              ACE_TEXT (Common_File_Tools::basename (configuration_in.cascadeFile, false).c_str ())));
#endif // _DEBUG

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Visualization_OpenCVClassifier_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
                                        MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCVClassifier_T::handleDataMessage"));

  cv::Mat frame_mat (mediaType_.resolution.height,
                     mediaType_.resolution.width,
                     Stream_Module_Decoder_Tools::pixelFormatToOpenCVFormat (mediaType_.format),
                     message_inout->rd_ptr (),
                     cv::Mat::AUTO_STEP);

  // step1: convert to grayscale (why ?)
//  cv::Mat frame_gray;
//  cv::cvtColor (frame_mat, frame_gray, cv::COLOR_BGRA2GRAY);
//  cv::equalizeHist (frame_gray, frame_gray);

  // step2: detect feature(s)
  std::vector<cv::Rect> features_a;
  cascadeClassifier_.detectMultiScale (//frame_gray,   // image
                                       frame_mat,
                                       features_a,   // features
                                       1.1,          // scale factor
                                       3,            // min. neighbours
                                       0,            // flags
                                       cv::Size (),  // min. size
                                       cv::Size ()); // max. Size

  // step3: draw feature(s) into the frame
  for (unsigned int i = 0;
       i < features_a.size ();
       ++i)
    cv::rectangle (frame_mat,              // image
                   features_a[i],          // rectangle
                   cv::Scalar (255, 0, 0), // color (blue)
                   1,                      // thickness
                   cv::LINE_8,             // line type
                   0);                     // shift

  // step4: show result
  cv::imshow (cv::String (ACE_TEXT_ALWAYS_CHAR ("frame")),
//              image_BGR);
              frame_mat);
  cv::waitKey (1);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Visualization_OpenCVClassifier_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
                                        MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCVClassifier_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());

      inherited2::getMediaType (session_data_r.formats.front (),
                                mediaType_);

      cv::namedWindow (cv::String (ACE_TEXT_ALWAYS_CHAR ("frame")),
                       cv::WINDOW_AUTOSIZE);
//      cv::startWindowThread ();

      break;

//error:
//      this->notify (STREAM_SESSION_MESSAGE_ABORT)

//      break;
    }
//    case STREAM_SESSION_MESSAGE_RESIZE:
//    {
//      break;
//    }
    case STREAM_SESSION_MESSAGE_END:
    {
      cv::destroyAllWindows ();

      break;
    }
    default:
      break;
  } // end SWITCH
}
