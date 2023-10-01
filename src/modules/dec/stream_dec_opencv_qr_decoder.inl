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

#include <iostream>

#include "opencv2/core/cvstd.hpp"
#include "opencv2/core/mat.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

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
Stream_Decoder_OpenCVQRDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                 MediaType>::Stream_Decoder_OpenCVQRDecoder_T (ISTREAM_T* stream_in)
#else
                                 MediaType>::Stream_Decoder_OpenCVQRDecoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , detector_ ()
 , format_ (0)
 , resolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_OpenCVQRDecoder_T::Stream_Decoder_OpenCVQRDecoder_T"));

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
Stream_Decoder_OpenCVQRDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_OpenCVQRDecoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    format_ = 0;
    resolution_ = {0, 0};
  } // end IF

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
Stream_Decoder_OpenCVQRDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::frame (cv::Mat& frame_inout,
                                                    const cv::Mat& boundingBox_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_OpenCVQRDecoder_T::frame"));

  for (int i = 0; i < boundingBox_in.rows; i++)
    cv::line (frame_inout,
              cv::Point2i (boundingBox_in.at<int> (i, 0),
                           boundingBox_in.at<int> (i, 1)),
              cv::Point2i (boundingBox_in.at<int> ((i + 1) % boundingBox_in.rows, 0),
                           boundingBox_in.at<int> ((i + 1) % boundingBox_in.rows, 1)),
              cv::Scalar (255, 0, 0),
              3);
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
Stream_Decoder_OpenCVQRDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_OpenCVQRDecoder_T::handleDataMessage"));

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

  // step1: convert to BGR (why ?)
  //  cv::Mat image_BGR;
  //  cv::cvtColor (image_mat, image_BGR, cv::COLOR_BGRA2BGR);

  // step2: detect QR code(s)
  cv::Mat bbox, rectified_image;
  std::string data = detector_.detectAndDecode (frame_matrix,
                                                bbox,
                                                rectified_image);
  if (data.size () > 0)
  {
    std::cout << "Decoded Data : " << data << std::endl;
    frame (frame_matrix, bbox);
    //rectified_image.convertTo (rectified_image, CV_8UC3);
    //cv::imshow ("Rectified QRCode", rectified_image);
  } // end IF
  //else
  //  std::cout << "QR Code not detected" << std::endl;

  cv::imshow (cv::String (ACE_TEXT_ALWAYS_CHAR ("ACEStream OpenCV display")),
//              image_BGR);
              frame_matrix);
  int key_i = cv::waitKey (1);
  if (unlikely (key_i == 27)) // ASCII Escape
    this->notify (STREAM_SESSION_MESSAGE_ABORT);
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
Stream_Decoder_OpenCVQRDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_OpenCVQRDecoder_T::handleSessionMessage"));

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

      cv::namedWindow (cv::String (ACE_TEXT_ALWAYS_CHAR ("ACEStream OpenCV display")),
                       cv::WINDOW_AUTOSIZE);
//      cv::startWindowThread ();

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
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
