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

#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#endif /* __cplusplus */

#include "gtk/gtk.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               MediaType>::Stream_Module_Vis_GTK_Pixbuf_T (ISTREAM_T* stream_in)
#else
                               MediaType>::Stream_Module_Vis_GTK_Pixbuf_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , isFirst_ (true)
 , lock_ (NULL)
 , scaleContext_ (NULL)
 , scaleContextHeight_ (0)
 , scaleContextWidth_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::Stream_Module_Vis_GTK_Pixbuf_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::~Stream_Module_Vis_GTK_Pixbuf_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::~Stream_Module_Vis_GTK_Pixbuf_T"));

  if (scaleContext_)
    sws_freeContext (scaleContext_);
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
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
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

//  const typename SessionDataContainerType::DATA_T& session_data_r =
//      inherited::sessionData_->getR ();

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

//  const MediaType& media_type_r = session_data_r.formats.front ();
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;
  inherited2::getMediaType (inherited::configuration_->outputFormat,
                            media_type_s);
  unsigned int image_size = 0;
  unsigned int row_stride = 0;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  struct _AMMediaType media_type_s =
//      inherited2::getMediaType (inherited::configuration_->outputFormat);
//  image_size =
//      Stream_MediaFramework_DirectShow_Tools::toFramesize (media_type_s);
////  struct _GUID sub_type = GUID_NULL;
////  HRESULT result_3 = session_data_r.inputFormat->GetGUID (MF_MT_SUBTYPE,
////                                                          &sub_type);
////  if (FAILED (result_3))
////  {
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
////                ACE_TEXT (Common_Tools::error2String (result_3).c_str ())));
////    return;
////  } // end IF
////  result_3 = MFCalculateImageSize (sub_type,
////                                   resolution.width, resolution.height,
////                                   &image_size);
////  if (FAILED (result_3))
////  {
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to MFCalculateImageSize(\"%s\", %u,%u): \"%s\", returning\n"),
////                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (sub_type).c_str ()),
////                resolution.width, resolution.height,
////                ACE_TEXT (Common_Tools::error2String (result_3).c_str ())));
////    return;
////  } // end IF
//  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
//#else
  image_size =
      av_image_get_buffer_size (media_type_s.format,
                                media_type_s.resolution.width,
                                media_type_s.resolution.height,
                                1); // *TODO*: linesize alignment
  row_stride = av_image_get_linesize (media_type_s.format,
                                      media_type_s.resolution.width,
                                      0);
//#endif // ACE_WIN32 || ACE_WIN64

//  bool leave_gdk = false;
  bool release_lock = false;
  int result = -1;
  guchar* data_2 = NULL;

  if (lock_)
  {
    result = lock_->acquire ();
    if (result == -1)
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
  ACE_ASSERT (gdk_pixbuf_get_colorspace (inherited::configuration_->pixelBuffer) == GDK_COLORSPACE_RGB);
  ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (inherited::configuration_->pixelBuffer) == 8);
  ACE_ASSERT (gdk_pixbuf_get_has_alpha (inherited::configuration_->pixelBuffer));
  ACE_ASSERT (gdk_pixbuf_get_n_channels (inherited::configuration_->pixelBuffer) == 4);
  data_2 = gdk_pixbuf_get_pixels (inherited::configuration_->pixelBuffer);
  ACE_ASSERT (data_2);

  int pixbuf_height =
      gdk_pixbuf_get_height (inherited::configuration_->pixelBuffer);
  int pixbuf_width =
      gdk_pixbuf_get_width (inherited::configuration_->pixelBuffer);
  int pixbuf_row_stride =
      gdk_pixbuf_get_rowstride (inherited::configuration_->pixelBuffer);
  bool transform_image =
      ((media_type_s.format != AV_PIX_FMT_RGBA) ||
       ((static_cast<int> (media_type_s.resolution.width) != pixbuf_width) || (static_cast<int> (media_type_s.resolution.height) != pixbuf_height)));
  uint8_t* in_data[AV_NUM_DATA_POINTERS];
  uint8_t* out_data[AV_NUM_DATA_POINTERS];

  if (transform_image &&
      ((pixbuf_height != static_cast<int> (scaleContextHeight_)) || (pixbuf_width != static_cast<int> (scaleContextWidth_))))
  {
    if (scaleContext_)
    {
      sws_freeContext (scaleContext_); scaleContext_ = NULL;
    } // end IF
    int flags = (SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
//                 SWS_LANCZOS | SWS_ACCURATE_RND);
    scaleContext_ =
        sws_getCachedContext (NULL,
                              media_type_s.resolution.width, media_type_s.resolution.height, media_type_s.format,
                              pixbuf_width, pixbuf_height, AV_PIX_FMT_RGBA,
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
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: scaling frame(s) (resolution: %ux%u) to %ux%u\n"),
                inherited::mod_->name (),
                media_type_s.resolution.width, media_type_s.resolution.height,
                pixbuf_width, pixbuf_height));
#endif // _DEBUG
  } // end IF

  result = 0;

  // step3: transform image?

  // *TODO*: this looks wrong
  if (!transform_image)
  { ACE_ASSERT (image_size == message_inout->length ());
    // *TODO*: GTK requires RGB, not RGBA --> drop transparency
    ACE_ASSERT (static_cast<unsigned int> (pixbuf_row_stride) == row_stride);
    for (unsigned int i = 0;
         i < media_type_s.resolution.height;
         ++i)
      ACE_OS::memcpy (data_2 + (i * pixbuf_row_stride),
                      message_inout->rd_ptr () + (i * row_stride),
                      row_stride);
    goto unlock; // done
  } // end IF

  ACE_OS::memset (in_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (out_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
  in_data[0] = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  out_data[0] = static_cast<uint8_t*> (data_2);
  if (!Stream_Module_Decoder_Tools::convert (scaleContext_,
                                             media_type_s.resolution.width, media_type_s.resolution.height, media_type_s.format,
                                             in_data,
                                             pixbuf_width, pixbuf_height, AV_PIX_FMT_RGBA,
                                             out_data))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                inherited::mod_->name ()));
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
                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
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
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleSessionMessage"));

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
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::initialize"));

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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Pixbuf_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
