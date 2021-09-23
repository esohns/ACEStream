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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#if defined (__cplusplus)
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT

#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              MediaType>::Stream_Module_Vis_GTK_Cairo_T (ISTREAM_T* stream_in)
#else
                              MediaType>::Stream_Module_Vis_GTK_Cairo_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , inherited2 ()
 , buffer_ (NULL)
 , context_ (NULL)
 , isFirst_ (true)
 , lock_ (NULL)
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
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType,
                              MediaType>::~Stream_Module_Vis_GTK_Cairo_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::~Stream_Module_Vis_GTK_Cairo_T"));

  if (buffer_)
#if GTK_CHECK_VERSION(3,10,0)
    cairo_surface_destroy (buffer_);
#else
    g_object_unref (buffer_);
#endif // GTK_CHECK_VERSION
  if (context_)
    cairo_destroy (context_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (unlikely (!context_))
    return; // done
  ACE_ASSERT (buffer_);

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

  int result = -1;
  //  bool leave_gdk = false;
  bool release_lock = false;

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

#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_flush (buffer_);
  ACE_OS::memcpy (cairo_image_surface_get_data (buffer_),
#else
  ACE_OS::memcpy (gdk_pixbuf_get_pixels (buffer_),
#endif // GTK_CHECK_VERSION
                  message_inout->rd_ptr (),
                  message_inout->length ());

  // step3: draw pixbuf to widget
#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_mark_dirty (buffer_);
  cairo_paint (context_);
#else
  //  gint width, height;
  //  gdk_drawable_get_size (GDK_DRAWABLE (configuration_->window),
  //                         &width, &height);
  // *IMPORTANT NOTE*: potentially, this involves tranfer of image data to an X
  //                   server running on a different host. Also, X servers don't
  //                   react kindly to multithreaded access
  //                   --> move this into the gtk context and simply schedule a
  //                       refresh, which takes care of that
  //gdk_draw_pixbuf (GDK_DRAWABLE (configuration_->window), NULL,
  //                 buffer_,
  //                 0, 0, 0, 0, width, height,
  //                 GDK_RGB_DITHER_NONE, 0, 0);
#endif // GTK_CHECK_VERSION

  if (likely (release_lock))
  {
    ACE_ASSERT (lock_);
    result = lock_->release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF

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

  //  if (leave_gdk)
  //    gdk_threads_leave ();

  return;

//error:
  //  if (leave_gdk)
  //    gdk_threads_leave ();

  if (likely (release_lock))
  { ACE_ASSERT (lock_);
    result = lock_->release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const SessionDataType& session_data_r = inherited::sessionData_->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      Common_Image_Resolution_t resolution_s;
#else
#if defined (FFMPEG_SUPPORT)
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#else
      struct Stream_MediaFramework_V4L_MediaType media_type_s;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      unsigned int frame_size_i =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        Stream_MediaFramework_DirectShow_Tools::toFramesize (media_type_s);
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
        av_image_get_buffer_size (media_type_s.format,
                                  media_type_s.resolution.width,
                                  media_type_s.resolution.height,
                                  1); // *TODO*: linesize alignment
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      ACE_UNUSED_ARG (frame_size_i);
      unsigned int row_stride_i =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        Stream_MediaFramework_DirectShow_Tools::toRowStride (media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
        av_image_get_linesize (media_type_s.format,
                               media_type_s.resolution.width,
                               0);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (buffer_);
#if GTK_CHECK_VERSION(3,10,0)
      ACE_ASSERT (cairo_surface_status (buffer_) == CAIRO_STATUS_SUCCESS);
      ACE_ASSERT (cairo_surface_get_type (buffer_) == CAIRO_SURFACE_TYPE_IMAGE);
      int width_2 = 0, height_2 = 0, row_stride_2 = 0, n_channels_i = 0;
      cairo_format_t format_e = CAIRO_FORMAT_INVALID;
      width_2 = cairo_image_surface_get_width (buffer_);
      height_2 = cairo_image_surface_get_height (buffer_);
      row_stride_2 = cairo_image_surface_get_stride (buffer_);
      format_e = cairo_image_surface_get_format (buffer_);
#else
      gint width_2 = 0, height_2 = 0, row_stride_2 = 0, n_channels_i = 0;
      ACE_ASSERT (GDK_IS_PIXBUF (buffer_));
      g_object_get (G_OBJECT (buffer_),
                    ACE_TEXT_ALWAYS_CHAR ("width"),      &width_2,
                    ACE_TEXT_ALWAYS_CHAR ("height"),     &height_2,
                    ACE_TEXT_ALWAYS_CHAR ("rowstride"),  &row_stride_2,
                    ACE_TEXT_ALWAYS_CHAR ("n-channels"), &n_channels_i,
                    NULL);
#endif // GTK_CHECK_VERSION
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT ((resolution_s.cx == static_cast<unsigned int> (width_2)) && (resolution_s.cy == static_cast<unsigned int> (height_2)));
#else
#if defined (FFMPEG_SUPPORT)
      ACE_ASSERT ((media_type_s.resolution.width == static_cast<unsigned int> (width_2)) && (media_type_s.resolution.height == static_cast<unsigned int> (height_2)));
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (row_stride_i == static_cast<unsigned int> (row_stride_2));
#if GTK_CHECK_VERSION(3,10,0)
      ACE_ASSERT ((format_e == CAIRO_FORMAT_RGB24) || (format_e == CAIRO_FORMAT_ARGB32));
#else
      ACE_ASSERT (gdk_pixbuf_get_colorspace (buffer_) == GDK_COLORSPACE_RGB);
      ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (buffer_) == 8);
//      ACE_ASSERT (n_channels_i == 4);
//      ACE_ASSERT (!gdk_pixbuf_get_has_alpha (buffer_));
#endif // GTK_CHECK_VERSION
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (n_channels_i == 3)
        ACE_ASSERT (Stream_MediaFramework_Tools::toBitCount (media_type_s.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW) == 24); // CAIRO_FORMAT_RGB24
      else
        ACE_ASSERT (Stream_MediaFramework_Tools::toBitCount (media_type_s.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW) == 32); // CAIRO_FORMAT_RGB32
#else
#if defined (FFMPEG_SUPPORT)
      if (n_channels_i == 3)
        ACE_ASSERT (media_type_s.format == AV_PIX_FMT_RGB24); // CAIRO_FORMAT_RGB24
      else
        ACE_ASSERT (media_type_s.format == AV_PIX_FMT_RGB32); // CAIRO_FORMAT_ARGB32
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType,
                              MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::initialize"));

  int scale_i = 0, width_i = 0, height_i = 0;
  int width_2 = 0, height_2 = 0, row_stride_i = 0, n_channels_i = 0;
//  cairo_format_t format_e = CAIRO_FORMAT_INVALID;
  GdkRectangle clip_area_s;

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
#if GTK_CHECK_VERSION(3,10,0)
      cairo_surface_destroy (buffer_); buffer_ = NULL;
#else
      g_object_unref (buffer_); buffer_ = NULL;
#endif // GTK_CHECK_VERSION
    } // end IF
    if (context_)
    {
      cairo_destroy (context_); context_ = NULL;
    } // end IF
    isFirst_ = true;
    lock_ = NULL;
  } // end IF

  // sanity check(s)
  if (!configuration_in.window)
    return true; // nothing to do

#if GTK_CHECK_VERSION(3,10,0)
  scale_i = gdk_window_get_scale_factor (configuration_in.window);
#else
  scale_i = 1;
#endif // GTK_CHECK_VERSION(2,0,0)
  width_i = gdk_window_get_width (configuration_in.window) * scale_i;
  height_i = gdk_window_get_height (configuration_in.window) * scale_i;

#if GTK_CHECK_VERSION(2,8,0)
  context_ = gdk_cairo_create (configuration_in.window);
#else
    ACE_ASSERT (false);
#endif // GTK_CHECK_VERSION
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_create(%@), aborting\n"),
                configuration_in.window));
    return false;
  } // end IF

  ACE_OS::memset (&clip_area_s, 0, sizeof (GdkRectangle));
#if GTK_CHECK_VERSION(3,0,0)
  if (!gdk_cairo_get_clip_rectangle (context_,
                                     &clip_area_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_get_clip_rectangle(%@), aborting\n"),
                context_));
    goto error;
  } // end IF
#else
    ACE_ASSERT (false);
#endif // GTK_CHECK_VERSION

  // *TODO*: remove type inferences
  lock_ = configuration_in.lock;

  gdk_threads_enter ();
  buffer_ =
#if GTK_CHECK_VERSION(3,10,0)
    gdk_window_create_similar_image_surface (configuration_in.window,
                                             CAIRO_FORMAT_RGB24,
                                             width_i, height_i, scale_i);
//                                               clip_area.width, clip_area.height,
//                                               0); // scale {0: same as window}
  if (!buffer_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_window_create_similar_image_surface(%@), aborting\n"),
                configuration_in.window));
    gdk_threads_leave ();
    goto error;
  } // end IF
#elif GTK_CHECK_VERSION (3,0,0)
    gdk_pixbuf_get_from_window (configuration_in.window,
                                0, 0,
                                width_i, height_i);
#elif GTK_CHECK_VERSION(2,0,0)
    gdk_pixbuf_get_from_drawable (NULL,
                                  GDK_DRAWABLE (configuration_in.window),
                                  NULL,
                                  0, 0,
                                  0, 0, width_i, height_i);
#else
      NULL;
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif
  if (!buffer_)
  { // *NOTE*: most probable reason: window is not mapped
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
    gdk_threads_leave ();
    goto error;
  } // end IF
  gdk_threads_leave ();
  ACE_ASSERT (buffer_);

  // sanity check(s)
#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (cairo_surface_status (buffer_) == CAIRO_STATUS_SUCCESS);
  ACE_ASSERT (cairo_surface_get_type (buffer_) == CAIRO_SURFACE_TYPE_IMAGE);
  width_2 = cairo_image_surface_get_width (buffer_);
  height_2 = cairo_image_surface_get_height (buffer_);
  row_stride_i =
      cairo_image_surface_get_stride (buffer_);
  format_e = cairo_image_surface_get_format (buffer_);
//    ACE_ASSERT ((clip_area_s.width == width_2) && (clip_area_s.height == height_2));
  ACE_UNUSED_ARG (row_stride_i);
  ACE_UNUSED_ARG (n_channels_i);
  ACE_ASSERT (format_e == CAIRO_FORMAT_ARGB32);
#else
  ACE_ASSERT (GDK_IS_PIXBUF (buffer_));
  g_object_get (G_OBJECT (buffer_),
                ACE_TEXT_ALWAYS_CHAR ("width"),      &width_2,
                ACE_TEXT_ALWAYS_CHAR ("height"),     &height_2,
                ACE_TEXT_ALWAYS_CHAR ("rowstride"),  &row_stride_i,
                ACE_TEXT_ALWAYS_CHAR ("n-channels"), &n_channels_i,
                NULL);
//    ACE_ASSERT ((clip_area_s.width == width_2) && (clip_area_s.height == height_2));
  ACE_ASSERT (gdk_pixbuf_get_colorspace (buffer_) == GDK_COLORSPACE_RGB);
  ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (buffer_) == 8);
  ACE_ASSERT (n_channels_i == 4);
//    ACE_ASSERT (!gdk_pixbuf_get_has_alpha (configuration_in.pixelBuffer));
#endif // GTK_CHECK_VERSION

#if GTK_CHECK_VERSION(3,10,0)
  cairo_set_source_surface (context_,
                            buffer_,
                            0, 0);
#else
  gdk_cairo_set_source_pixbuf (context_,
                               buffer_,
                               0, 0);
#endif // GTK_CHECK_VERSION

  return inherited::initialize (configuration_in,
                                allocator_in);

error:
  if (buffer_)
  {
#if GTK_CHECK_VERSION(3,10,0)
    cairo_surface_destroy (buffer_); buffer_ = NULL;
#else
    g_object_unref (buffer_); buffer_ = NULL;
#endif // GTK_CHECK_VERSION
  } // end IF
  if (context_)
  {
    cairo_destroy (context_); context_ = NULL;
  } // end IF

  return false;
}
