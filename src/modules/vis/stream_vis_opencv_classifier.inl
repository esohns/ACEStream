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
 , format_ (0)
 , resolution_ ()
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
    format_ = 0;
    resolution_ = {0, 0};
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
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: loaded cascade file \"%s\"\n"),
              inherited::mod_->name (),
              ACE_TEXT (Common_File_Tools::basename (configuration_in.cascadeFile, false).c_str ())));

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

  // step0: convert image frame to matrix
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  cv::Mat frame_matrix (resolution_.cy,
                        resolution_.cx,
#else
  cv::Mat frame_matrix (resolution_.height,
                        resolution_.width,
#endif // ACE_WIN32 || ACE_WIN64
                        format_,
                        message_inout->rd_ptr (),
                        cv::Mat::AUTO_STEP);

  // step1: convert to grayscale (why ?)
//  cv::Mat frame_gray;
//  cv::cvtColor (frame_mat, frame_gray, cv::COLOR_BGRA2GRAY);
//  cv::equalizeHist (frame_gray, frame_gray);

  // step2: detect feature(s)
  std::vector<cv::Rect> features_a;
  cascadeClassifier_.detectMultiScale (//frame_gray,   // image
                                       frame_matrix,
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
    cv::rectangle (frame_matrix,           // image
                   features_a[i],          // rectangle
                   cv::Scalar (255, 0, 0), // color (blue)
                   1,                      // thickness
                   cv::LINE_8,             // line type
                   0);                     // shift

  // step4: show result
  cv::imshow (cv::String (ACE_TEXT_ALWAYS_CHAR ("frame")),
//              image_BGR);
              frame_matrix);
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
#if defined (FFMPEG_SUPPORT)
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      format_ =
        Stream_Module_Decoder_Tools::mediaSubTypeToOpenCVFormat (media_type_s.subtype),
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
      format_ =
        Stream_Module_Decoder_Tools::AVPixelFormatToOpenCVFormat (media_type_s.format),
      resolution_ = media_type_s.resolution;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

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
