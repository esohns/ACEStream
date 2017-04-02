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

#include <ace/Log_Msg.h>

#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}
#endif

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_tools.h"

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType>::Stream_Module_Vis_GTK_Pixbuf_T ()
 : inherited ()
 , buffer_ (NULL)
 , bufferHeight_ (0)
 , bufferWidth_ (0)
 , lock_ (NULL)
// , pixelBuffer_ (NULL)
 , isFirst_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::Stream_Module_Vis_GTK_Pixbuf_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType>::~Stream_Module_Vis_GTK_Pixbuf_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::~Stream_Module_Vis_GTK_Pixbuf_T"));

  if (buffer_)
    delete [] buffer_;

//  if (pixelBuffer_)
//    g_object_unref (pixelBuffer_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (!inherited::configuration_->window)
    return; // done
  ACE_ASSERT (inherited::configuration_->pixelBuffer);
  ACE_ASSERT (inherited::sessionData_);

  const typename SessionDataContainerType::DATA_T& session_data_r =
      inherited::sessionData_->get ();

  // *NOTE*: 'crunching' the message data simplifies the data transformation
  //         algorithms, at the cost of (several) memory copies. This is a
  //         tradeoff that may warrant further optimization efforts
  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    return;
  }

  unsigned int width, height = 0;
  unsigned int image_size = 0;
  enum AVPixelFormat pixel_format = AV_PIX_FMT_NONE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (session_data_r.format);

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header_2 = NULL;
  if (session_data_r.format->formattype == FORMAT_VideoInfo)
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (session_data_r.format->pbFormat);
  else if (session_data_r.format->formattype == FORMAT_VideoInfo2)
    video_info_header_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (session_data_r.format->pbFormat);
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown format type (was: %s), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (session_data_r.format->formattype).c_str ())));
    goto error;
  } // end ELSE
  height =
    static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biHeight
                                                   : video_info_header_2->bmiHeader.biHeight);
  width =
    static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biWidth
                                                   : video_info_header_2->bmiHeader.biWidth);
  image_size =
    (video_info_header_p ? video_info_header_p->bmiHeader.biSizeImage
                         : video_info_header_2->bmiHeader.biSizeImage);
  pixel_format =
    Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (session_data_r.format->subtype);
//  struct _GUID sub_type = GUID_NULL;
//  HRESULT result_3 = session_data_r.format->GetGUID (MF_MT_SUBTYPE,
//                                                     &sub_type);
//  if (FAILED (result_3))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
//                ACE_TEXT (Common_Tools::error2String (result_3).c_str ())));
//    return;
//  } // end IF
//  result_3 = MFGetAttributeSize (session_data_r.format,
//                                 MF_MT_FRAME_SIZE,
//                                 &width, &height);
//  if (FAILED (result_3))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", returning\n"),
//                ACE_TEXT (Common_Tools::error2String (result_3).c_str ())));
//    return;
//  } // end IF
//  result_3 = MFCalculateImageSize (sub_type,
//                                   width, height,
//                                   &image_size);
//  if (FAILED (result_3))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCalculateImageSize(\"%s\", %u,%u): \"%s\", returning\n"),
//                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ()),
//                width, height,
//                ACE_TEXT (Common_Tools::error2String (result_3).c_str ())));
//    return;
//  } // end IF
#else
  width  = session_data_r.width;
  height = session_data_r.height;
  image_size =
      av_image_get_buffer_size (session_data_r.format,
                                width,
                                height,
                                1); // *TODO*: linesize alignment
  pixel_format = session_data_r.format;
#endif

  bool leave_gdk = false;
  bool release_lock = false;
  int result = -1;
  ACE_Message_Block* message_block_p = message_inout;
  unsigned int offset = 0;//, length = message_block_p->length ();
  unsigned char* data_p =
      reinterpret_cast<unsigned char*> (message_inout->rd_ptr ());
  ACE_ASSERT (data_p);
  unsigned char* pixel_p = data_p;
  guchar* data_2 = NULL;
  unsigned int* pixel_2 = NULL;
  guchar* data_3 = NULL;
  unsigned int* pixel_3 = NULL;
//  int bytes_per_pixel = -1;
//  int bits_per_sample = 8;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  LONG row_stride = -1;
#else
  int row_stride = -1;
#endif
  int row_stride_2 = -1;

  if (lock_)
  {
    result = lock_->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", returning\n")));
      return;
    } // end IF
    release_lock = true;
  } // end IF

  //gdk_threads_enter ();
  //leave_gdk = true;

//  unsigned char* data_2 = cairo_image_surface_get_data (cairoSurface_);
  data_2 = gdk_pixbuf_get_pixels (inherited::configuration_->pixelBuffer);
  ACE_ASSERT (data_2);
  pixel_2 = reinterpret_cast<unsigned int*> (data_2);
//  bytes_per_pixel =
//      ((gdk_pixbuf_get_bits_per_sample (inherited::configuration_->pixelBuffer) / 8) * 4);
//  ACE_ASSERT (bytes_per_pixel == 4);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  row_stride =
    ((((width * (video_info_header_p ? video_info_header_p->bmiHeader.biBitCount
                                     : video_info_header_2->bmiHeader.biBitCount)) + 31) & ~31) >> 3);
//  result_3 = MFGetStrideForBitmapInfoHeader (sub_type.Data1,
//                                             width,
//                                             &row_stride);
//  if (FAILED (result_3))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetStrideForBitmapInfoHeader(): \"%s\", returning\n"),
//                ACE_TEXT (Common_Tools::error2String (result_3).c_str ())));
//    goto error;
//  } // end IF
#else
  row_stride = av_image_get_linesize (session_data_r.format,
                                      width,
                                      0);
#endif

  int pixbuf_height =
      gdk_pixbuf_get_height (inherited::configuration_->pixelBuffer);
  int pixbuf_width =
      gdk_pixbuf_get_width (inherited::configuration_->pixelBuffer);
  bool transform_image =
      ((pixel_format != AV_PIX_FMT_RGB24) ||
       ((width != pixbuf_width) || (height != pixbuf_height)));
  uint8_t* in_data[4] = { NULL, NULL, NULL, NULL };
  uint8_t* out_data[4] = { NULL, NULL, NULL, NULL };

  if (transform_image &&
      (pixbuf_height != bufferHeight_) || (pixbuf_width != bufferWidth_))
  {
    if (buffer_)
    {
      delete [] buffer_;
      buffer_ = NULL;
    } // end IF

    unsigned int buffer_size =
        av_image_get_buffer_size (AV_PIX_FMT_RGB24,
                                  width,
                                  height,
                                  1); // *TODO*: linesize alignment
    ACE_NEW_NORETURN (buffer_,
                      uint8_t[buffer_size]);
    if (!buffer_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory(%u): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  buffer_size));
      goto unlock;
    } // end IF
    bufferHeight_ = pixbuf_height;
    bufferWidth_ = pixbuf_width;

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: scaling frame(s) (size: %ux%u) to %ux%u...\n"),
                inherited::mod_->name (),
                width, height,
                pixbuf_width, pixbuf_height));
  } // end IF

  data_3 = (transform_image ? buffer_ : data_2);
  ACE_ASSERT (data_3);
  pixel_3 = (transform_image ? reinterpret_cast<unsigned int*> (data_3)
                             : reinterpret_cast<unsigned int*> (data_2));
  ACE_ASSERT (pixel_3);
  row_stride_2 =
      (transform_image ? av_image_get_linesize (AV_PIX_FMT_RGB24,
                                                width,
                                                0)
                       : gdk_pixbuf_get_rowstride (inherited::configuration_->pixelBuffer));

  result = 0;

  // step3: transform image?
  if (!transform_image)
    goto unlock; // done

  in_data[0] = buffer_;
  out_data[0] = static_cast<uint8_t*> (data_2);
  if (!Stream_Module_Decoder_Tools::convert (NULL,
                                             width, height, AV_PIX_FMT_RGB24,
                                             in_data,
                                             pixbuf_width, pixbuf_height, AV_PIX_FMT_RGB24,
                                             out_data))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::scale(), returning\n")));

    result = -1;

    goto unlock;
  } // end IF

unlock:
  if (release_lock)
  {
    ACE_ASSERT (lock_);
    result = lock_->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  if (result == -1)
    goto error;

//  gdk_threads_enter ();

//  pixelBuffer_ =
//      gdk_pixbuf_new_from_data (data_p,                               // data
//                                GDK_COLORSPACE_RGB,                   // color space
//                                false,                                // has alpha channel ?
//                                bits_per_sample,                      // bits per sample
//                                session_data_r.format->fmt.pix.width,  // width
//                                session_data_r.format->fmt.pix.height, // height
//                                row_stride,                           // row stride
//                                NULL,                                 // destroy function
//                                NULL);                                // destroy function act
//  if (!pixelBuffer_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_pixbuf_new_from_data(), returning\n")));

//    // clean up
//    gdk_threads_leave ();

//    return;
//  } // end IF

  // step3: draw pixbuf to widget
//  gint width, height;
//  gdk_drawable_get_size (GDK_DRAWABLE (configuration_->window),
//                         &width, &height);
  // *IMPORTANT NOTE*: potentially, this involves tranfer of image data to an X
  //                   server running on a different host. Also, X servers don't
  //                   react kindly to multithreaded access
  //                   --> move this into the gtk context and simply schedule a
  //                       refresh, which takes care of that
//  gdk_draw_pixbuf (GDK_DRAWABLE (configuration_->window), NULL,
//                   pixelBuffer_,
//                   0, 0, 0, 0, width, height,
//                   GDK_RGB_DITHER_NONE, 0, 0);

  // step5: schedule an 'expose' event
  // *NOTE*: gdk_window_invalidate_rect() is not thread-safe. It will race with
  //         the UI refresh and eventually crash (even though gdk_threads_enter/
  //         gdk_threads_leave is in effect)
  //         --> schedule a refresh with gtk_widget_queue_draw_area() instead
  // *NOTE*: this does not work either... :-(
  //         --> let the downstream event handler queue an idle request
//  gdk_window_invalidate_rect (inherited::configuration_->gdkWindow,
//                              NULL,
//                              false);
//  GtkWidget* widget_p = NULL;
//  gdk_window_get_user_data (configuration_->window,
//                            reinterpret_cast<gpointer*> (&widget_p));
//  ACE_ASSERT (widget_p);
//  gtk_widget_queue_draw (widget_p);
//  GtkAllocation allocation;
//  gtk_widget_get_allocation (widget_p,
//                             &allocation);
//  gtk_widget_queue_draw_area (widget_p,
//                              allocation.x, allocation.y,
//                              allocation.width, allocation.height);

error:
  if (leave_gdk)
    gdk_threads_leave ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleSessionMessage"));

//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      break;

//error:
//      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        delete [] buffer_;
        buffer_ = NULL;
      } // end IF

//      if (pixelBuffer_)
//      {
//        g_object_unref (pixelBuffer_);
//        pixelBuffer_ = NULL;
//      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      delete [] buffer_;
      buffer_ = NULL;
    } // end IF
    bufferHeight_ = 0;
    bufferWidth_ = 0;

//    if (pixelBuffer_)
//    {
//      g_object_unref (pixelBuffer_);
//      pixelBuffer_ = NULL;
//    } // end IF

    isFirst_ = true;

    inherited::isInitialized_ = false;
  } // end IF

  lock_ = configuration_in.pixelBufferLock;
  if (!configuration_in.pixelBuffer)
  {
    // *TODO*: remove type inference
    if (configuration_in.window)
    {
      gdk_threads_enter ();

      // *TODO*: remove type inference
//      pixelBuffer_ =
//#if GTK_CHECK_VERSION (3,0,0)
//          gdk_pixbuf_get_from_window (configuration_in.window,
//                                      0, 0,
//                                      configuration_in.area.width, configuration_in.area.height);
//#else
//          gdk_pixbuf_get_from_drawable (NULL,
//                                        GDK_DRAWABLE (configuration_in.window),
//                                        NULL,
//                                        0, 0,
//                                        0, 0, configuration_in.area.width, configuration_in.area.height);
//#endif
//      if (!pixelBuffer_)
//      { // *NOTE*: most probable reason: window is not mapped
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));

//        // clean up
//        gdk_threads_leave ();

//        return false;
//      } // end IF
      gdk_threads_leave ();
    } // end IF
  } // end IF
  else
  {
//    g_object_ref (configuration_in.pixelBuffer);
//    pixelBuffer_ = configuration_in.pixelBuffer;
  } // end ELSE
//  if (!pixelBuffer_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to obtain pixel buffer, aborting\n")));
//    return false;
//  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
AM_MEDIA_TYPE&
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*format_in,
                                                             result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
                inherited::mod_->name ()));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
AM_MEDIA_TYPE&
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;

  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (format_in),
                                        GUID_NULL,
                                        &result_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}
#endif
