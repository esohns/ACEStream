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

extern "C"
{
#include "libavutil/frame.h"
}

#include "ace/Log_Msg.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "common_tools.h"

#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              SessionDataContainerType>::Stream_Module_Vis_GTK_Cairo_T (ISTREAM_T* stream_in)
#else
                              SessionDataContainerType>::Stream_Module_Vis_GTK_Cairo_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , isFirst_ (true)
 , lock_ (NULL)
 , scaleContext_ (NULL)
 , scaleContextHeight_ (0)
 , scaleContextWidth_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::Stream_Module_Vis_GTK_Cairo_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::~Stream_Module_Vis_GTK_Cairo_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::~Stream_Module_Vis_GTK_Cairo_T"));

//  if (cairoSurface_)
//    cairo_surface_destroy (cairoSurface_);
//  if (cairoContext_)
//    cairo_destroy (cairoContext_);

  if (scaleContext_)
    sws_freeContext (scaleContext_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (!inherited::configuration_->window)
    return; // done
  ACE_ASSERT (inherited::configuration_->pixelBuffer);
  ACE_ASSERT (inherited::sessionData_);

  const typename SessionDataContainerType::DATA_T& session_data_r =
    inherited::sessionData_->getR ();

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
  enum AVPixelFormat pixel_format = AV_PIX_FMT_NONE;
  unsigned int row_stride = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (session_data_r.inputFormat);

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header_2 = NULL;
  if (InlineIsEqualGUID (session_data_r.inputFormat->formattype, FORMAT_VideoInfo))
    video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (session_data_r.inputFormat->pbFormat);
  else if (InlineIsEqualGUID (session_data_r.inputFormat->formattype, FORMAT_VideoInfo2))
    video_info_header_2 =
    reinterpret_cast<struct tagVIDEOINFOHEADER2*> (session_data_r.inputFormat->pbFormat);
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown format type (was: %s), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::GUIDToString (session_data_r.inputFormat->formattype).c_str ())));
    goto error;
  } // end ELSE
  height =
    static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biHeight
                               : video_info_header_2->bmiHeader.biHeight);
  width =
    static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biWidth
                               : video_info_header_2->bmiHeader.biWidth);
//  image_size =
//    (video_info_header_p ? video_info_header_p->bmiHeader.biSizeImage
//     : video_info_header_2->bmiHeader.biSizeImage);
  pixel_format =
    Stream_Module_Decoder_Tools::mediaSubTypeToAVPixelFormat (session_data_r.inputFormat->subtype);
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
  width = inherited::configuration_->sourceFormat.width;
  height = inherited::configuration_->sourceFormat.height;
//  image_size =
//    av_image_get_buffer_size (session_data_r.format,
//                              width,
//                              height,
//                              1); // *TODO*: linesize alignment
  pixel_format = session_data_r.inputFormat;
  row_stride = av_image_get_linesize (session_data_r.inputFormat,
                                      width,
                                      0);
  ACE_UNUSED_ARG (row_stride);
#endif

  //  bool leave_gdk = false;
  bool release_lock = false;
  int result = -1;
  guchar* data_2 = NULL;
  enum AVPixelFormat pixel_format_2 = AV_PIX_FMT_RGB24;

  if (likely (lock_))
  {
    result = lock_->acquire ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    release_lock = true;
  } // end IF

    //gdk_threads_enter ();
    //leave_gdk = true;

  // sanity check(s)
  ACE_ASSERT (GDK_IS_PIXBUF (inherited::configuration_->pixelBuffer));
  ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (inherited::configuration_->pixelBuffer) == 8);
  ACE_ASSERT (gdk_pixbuf_get_colorspace (inherited::configuration_->pixelBuffer) == GDK_COLORSPACE_RGB);
  if (unlikely (gdk_pixbuf_get_has_alpha (inherited::configuration_->pixelBuffer) ||
                (gdk_pixbuf_get_n_channels (inherited::configuration_->pixelBuffer) == 4)))
    pixel_format_2 = AV_PIX_FMT_RGBA;
  data_2 = gdk_pixbuf_get_pixels (inherited::configuration_->pixelBuffer);
  ACE_ASSERT (data_2);

  int pixbuf_height =
    gdk_pixbuf_get_height (inherited::configuration_->pixelBuffer);
  int pixbuf_width =
    gdk_pixbuf_get_width (inherited::configuration_->pixelBuffer);
//  int pixbuf_row_stride =
//    gdk_pixbuf_get_rowstride (inherited::configuration_->pixelBuffer);
  bool transform_image =
    ((pixel_format != pixel_format_2) ||
     ((static_cast<int> (width) != pixbuf_width) || (static_cast<int> (height) != pixbuf_height)));
  uint8_t* in_data[AV_NUM_DATA_POINTERS];
  uint8_t* out_data[AV_NUM_DATA_POINTERS];

  if (likely (!transform_image))
    goto next;

  if ((pixbuf_height != static_cast<int> (scaleContextHeight_)) ||
      (pixbuf_width  != static_cast<int> (scaleContextWidth_)))
  {
    if (scaleContext_)
    {
      sws_freeContext (scaleContext_);
      scaleContext_ = NULL;
    } // end IF
    int flags = (SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
    //                 SWS_LANCZOS | SWS_ACCURATE_RND);
    scaleContext_ =
      sws_getCachedContext (NULL,
                            width, height, pixel_format,
                            pixbuf_width, pixbuf_height, pixel_format_2,
                            flags,                             // flags
                            NULL, NULL,
                            0);                                // parameters
    if (!scaleContext_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));

      result = -1;

      goto unlock;
    } // end IF
    scaleContextHeight_ = pixbuf_height;
    scaleContextWidth_ = pixbuf_width;

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: scaling frame(s) (resolution: %ux%u) to %ux%u\n"),
                inherited::mod_->name (),
                width, height,
                pixbuf_width, pixbuf_height));
  } // end IF

  // step3: transform image?
  ACE_OS::memset (in_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (out_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
  in_data[0] = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  out_data[0] = static_cast<uint8_t*> (data_2);
  if (!Stream_Module_Decoder_Tools::convert (scaleContext_,
                                             width, height, pixel_format,
                                             in_data,
                                             pixbuf_width, pixbuf_height, pixel_format_2,
                                             out_data))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                inherited::mod_->name ()));

    result = -1;

    goto unlock;
  } // end IF

next:
  result = 0;

unlock:
  if (likely (release_lock))
  {
    ACE_ASSERT (lock_);
    result = lock_->release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF

  if (unlikely (result == -1))
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

  return;

error:
  //  if (leave_gdk)
  //    gdk_threads_leave ();
  return;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

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
      break;
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
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::initialize"));

  if (inherited::isInitialized_)
  {
    isFirst_ = true;
    lock_ = NULL;
    if (scaleContext_)
    {
      sws_freeContext (scaleContext_);
      scaleContext_ = NULL;
    } // end IF
    scaleContextHeight_ = 0;
    scaleContextWidth_ = 0;
  } // end IF

  lock_ = configuration_in.pixelBufferLock;
  if (!configuration_in.pixelBuffer)
  {
    // *TODO*: remove type inference
    if (configuration_in.window)
    {
//      gdk_threads_enter ();

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
//      gdk_threads_leave ();
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
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::getFormat_impl"));

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
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::getFormat_impl"));

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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}
#endif
