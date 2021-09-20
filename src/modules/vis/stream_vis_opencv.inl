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
Stream_Visualization_OpenCV_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              MediaType>::Stream_Visualization_OpenCV_T (ISTREAM_T* stream_in)
#else
                              MediaType>::Stream_Visualization_OpenCV_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , mediaType_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCV_T::Stream_Visualization_OpenCV_T"));

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
Stream_Visualization_OpenCV_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCV_T::initialize"));

  if (inherited::isInitialized_)
  {
    ACE_OS::memset (&mediaType_, 0, sizeof (MediaType)); // *TODO*: free any allocated resources !
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
Stream_Visualization_OpenCV_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCV_T::handleDataMessage"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  inherited2::getMediaType (mediaType_,
                            media_type_s);
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
  cv::Mat frame_mat (resolution_s.cy,
                     resolution_s.cx,
                     Stream_Module_Decoder_Tools::mediaSubTypeToOpenCVFormat (media_type_s.subtype),
#else
#if defined (FFMPEG_SUPPORT)
  cv::Mat frame_mat (mediaType_.resolution.height,
                     mediaType_.resolution.width,
                     Stream_Module_Decoder_Tools::AVPixelFormatToOpenCVFormat (mediaType_.format),
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
                     message_inout->rd_ptr (),
                     cv::Mat::AUTO_STEP);
//  cv::Mat image_BGR;
//  cv::cvtColor (image_mat, image_BGR, cv::COLOR_BGRA2BGR);
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
Stream_Visualization_OpenCV_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_OpenCV_T::handleSessionMessage"));

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

      inherited2::getMediaType (session_data_r.formats.back (),
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
