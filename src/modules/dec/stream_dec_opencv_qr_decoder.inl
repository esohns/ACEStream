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
                                 MediaType>::Stream_Decoder_OpenCVQRDecoder_T (typename inherited::ISTREAM_T* stream_in)
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
                                 MediaType>::drawFrame (cv::Mat& frame_inout,
                                                        const std::vector<cv::Point>& boundingBox_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_OpenCVQRDecoder_T::drawFrame"));

  double show_radius_d =
    (frame_inout.rows > frame_inout.cols) ? (2.813 * frame_inout.rows) / frame_inout.cols
                                          : (2.813 * frame_inout.cols) / frame_inout.rows;
  double contour_radius_d = show_radius_d * 0.4;
  std::vector<std::vector<cv::Point> > contours_a;
  static cv::RNG rng (1000);

  for (size_t i = 0; i < boundingBox_in.size (); i += 4)
  {
    std::vector<cv::Point> contour_a (boundingBox_in.begin () + i,
                                      boundingBox_in.begin () + i + 4);
    contours_a.clear ();
    contours_a.push_back (contour_a);
    cv::drawContours (frame_inout, contours_a, 0, cv::Scalar (211, 0, 148),
                      cvRound (contour_radius_d), cv::LINE_8, cv::noArray (), INT_MAX, cv::Point ());
  
    cv::Scalar color (rng.uniform (0, 255), rng.uniform (0, 255),
                      rng.uniform (0, 255));
    for (size_t i = 0; i < 4; i++)
      cv::circle (frame_inout, contour_a[i], cvRound (show_radius_d), color, -1);
  } // end FOR
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

  // step1: detect QR code(s)
  //cv::Mat rectified_image;
  std::vector<cv::Point> bbox_a;
  std::string data = detector_.detectAndDecode (frame_matrix,
                                                bbox_a,
                                                //rectified_image
                                                cv::noArray ());
  if (data.size () > 0)
  {
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s: decoded data \"%s\"...\n"),
                inherited::mod_->name (),
                ACE_TEXT (data.c_str ())));

    drawFrame (frame_matrix, bbox_a);
    
    //cv::resize (rectified_image, rectified_image, cv::Size (320, 320), 0.0, 0.0, cv::INTER_NEAREST);
    //cv::imshow (ACE_TEXT_ALWAYS_CHAR ("rectified QRCode"),
    //            rectified_image);
  } // end IF

  // step2: display
  cv::imshow (ACE_TEXT_ALWAYS_CHAR ("ACEStream OpenCV display"),
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

      cv::namedWindow (ACE_TEXT_ALWAYS_CHAR ("ACEStream OpenCV display"),
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
