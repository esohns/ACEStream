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
extern "C"
{
#include "libavutil/imgutils.h"
}
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
                              MediaType>::Stream_Module_Vis_GTK_Cairo_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , inherited4 ()
 //, context_ (NULL)
 , surface_ (NULL)
 , surfaceLock_ ()
#if GTK_CHECK_VERSION (4,0,0)
 , drawingContext_ (NULL)
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
 , cairoRegion_ (NULL)
#endif // GTK_CHECK_VERSION (3,22,0)
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

  //if (unlikely (context_))
  //  cairo_destroy (context_);
  if (unlikely (surface_))
#if GTK_CHECK_VERSION (3,10,0)
    cairo_surface_destroy (surface_);
#else
    g_object_unref (surface_);
#endif // GTK_CHECK_VERSION(3,10,0)
#if GTK_CHECK_VERSION (4,0,0)
  if (drawingContext_)
    g_object_unref (drawingContext_);
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
  if (cairoRegion_)
    cairo_region_destroy (cairoRegion_);
#endif // GTK_CHECK_VERSION (3,22,0)
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

  if (inherited::isInitialized_)
  {
    //if (unlikely (context_))
    //{
    //  cairo_destroy (context_); context_ = NULL;
    //} // end IF
    if (unlikely (surface_))
#if GTK_CHECK_VERSION (3,10,0)
      cairo_surface_destroy (surface_);
#else
      g_object_unref (surface_);
#endif // GTK_CHECK_VERSION(3,10,0)
    surface_ = NULL;

#if GTK_CHECK_VERSION (4,0,0)
    if (drawingContext_)
    {
      g_object_unref (drawingContext_); drawingContext_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
    if (cairoRegion_)
    {
      cairo_region_destroy (cairoRegion_); cairoRegion_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION (3,22,0)
  } // end IF

  // sanity check(s)
#if GTK_CHECK_VERSION (4,0,0)
  GdkSurface* window_h = inherited4::convert (configuration_in.window);
#else
  GdkWindow* window_h = inherited4::convert (configuration_in.window);
#endif // GTK_CHECK_VERSION (4,0,0)
  if (!window_h)
    return inherited::initialize (configuration_in,
                                  allocator_in); // nothing to do

#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
#if GTK_CHECK_VERSION (4,0,0)
  drawingContext_ = gdk_surface_create_cairo_context (window_h);
  if (unlikely (!drawingContext_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_surface_create_cairo_context(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
  cairoRegion_ = cairo_region_create ();
  ACE_ASSERT (cairoRegion_);
#elif GTK_CHECK_VERSION (2,8,0)
  //context_ = gdk_cairo_create (window_h);
  //if (unlikely (!context_))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to gdk_cairo_create(%@), aborting\n"),
  //              inherited::mod_->name (),
  //              window_h));
//#if GTK_CHECK_VERSION (3,6,0)
//#else
//    GDK_THREADS_LEAVE ();
//#endif // GTK_CHECK_VERSION (3,6,0)
//    return false;
//  } // end IF
#endif // GTK_CHECK_VERSION ()
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)

  return inherited::initialize (configuration_in,
                                allocator_in);

//error:
  //if (context_)
  //{
  //  cairo_destroy (context_); context_ = NULL;
  //} // end IF

  return false;
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
#if GTK_CHECK_VERSION (4,0,0)
                              MediaType>::setP (GdkSurface* window_in)
#else
                              MediaType>::setP (GdkWindow* window_in)
#endif // GTK_CHECK_VERSION (4,0,0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::setP"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (!surface_);
  ACE_ASSERT (window_in);

  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
  Common_Image_Resolution_t resolution_s;
#if GTK_CHECK_VERSION (3,10,0)
  int width_2 = 0, height_2 = 0, row_stride_2 = 0, n_channels_i = 0;
  cairo_format_t format_e = CAIRO_FORMAT_INVALID;
#else
  gint width_2 = 0, height_2 = 0, row_stride_2 = 0, n_channels_i = 0;
#endif // GTK_CHECK_VERSION

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#else
  struct Stream_MediaFramework_V4L_MediaType media_type_s;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  inherited3::getMediaType (session_data_r.formats.back (),
                            STREAM_MEDIATYPE_VIDEO,
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
  resolution_s = media_type_s.resolution;
#else
    0;
  ACE_ASSERT (false); // *TODO*
  resolution_s = media_type_s.resolution;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ACE_UNUSED_ARG (frame_size_i);
  unsigned int row_stride_i =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Stream_MediaFramework_DirectShow_Tools::toRowStride (media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
    av_image_get_linesize (media_type_s.format,
                           resolution_s.width,
                           0);
#else
    0;
  ACE_ASSERT (false); // *TODO*
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
  surface_ =
#if GTK_CHECK_VERSION (4,0,0)
    gdk_surface_create_similar_surface (window_in,
                                        CAIRO_CONTENT_COLOR, // CAIRO_CONTENT_COLOR_ALPHA
                                        gdk_surface_get_width (window_in),
                                        gdk_surface_get_height (window_in));
  if (unlikely (!surface_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_surface_create_similar_surface(%@), returning\n"),
                inherited::mod_->name (),
                window_in));
    return;
  } // end IF
#elif GTK_CHECK_VERSION (3,10,0)
    gdk_window_create_similar_image_surface (window_in,
                                             CAIRO_FORMAT_ARGB32,
                                             gdk_window_get_width (window_in),
                                             gdk_window_get_height (window_in),
                                             gdk_window_get_scale_factor (window_in));
  if (unlikely (!surface_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_window_create_similar_image_surface(%@), returning\n"),
                inherited::mod_->name (),
                window_in));
    return;
  } // end IF
#elif GTK_CHECK_VERSION (3,0,0)
    gdk_pixbuf_get_from_window (window_in,
                                0, 0,
                                gdk_window_get_width (window_in),
                                gdk_window_get_height (window_in));
  if (unlikely (!surface_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(%@), returning\n"),
                inherited::mod_->name (),
                window_in));
#if GTK_CHECK_VERSION (3,6,0)
#else
    GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)
    return;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("%s: created %ux%u surface buffer\n"),
  //            inherited::mod_->name (),
  //            gdk_window_get_width (window_in),
  //            gdk_window_get_height (window_in)));
#elif GTK_CHECK_VERSION (2,0,0)
    gdk_pixbuf_get_from_drawable (NULL,
                                  GDK_DRAWABLE (window_in),
                                  NULL,
                                  0, 0,
                                  0, 0,
                                  gdk_window_get_width (window_in),
                                  gdk_window_get_height (window_in));
  if (unlikely (!surface_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_drawable(%@), returning\n"),
                inherited::mod_->name (),
                window_in));
#if GTK_CHECK_VERSION (3,6,0)
#else
    GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)
    return;
  } // end IF
#else
    NULL;
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return;)
#endif // GTK_CHECK_VERSION

#if GTK_CHECK_VERSION (3,10,0)
  ACE_ASSERT (cairo_surface_status (surface_) == CAIRO_STATUS_SUCCESS);
  ACE_ASSERT (cairo_surface_get_type (surface_) == CAIRO_SURFACE_TYPE_IMAGE);
  width_2 = cairo_image_surface_get_width (surface_);
  height_2 = cairo_image_surface_get_height (surface_);
  row_stride_2 = cairo_image_surface_get_stride (surface_);
  ACE_UNUSED_ARG (n_channels_i);
  format_e = cairo_image_surface_get_format (surface_);
#else
  ACE_ASSERT (GDK_IS_PIXBUF (surface_));
  g_object_get (G_OBJECT (surface_),
                ACE_TEXT_ALWAYS_CHAR ("width"),      &width_2,
                ACE_TEXT_ALWAYS_CHAR ("height"),     &height_2,
                ACE_TEXT_ALWAYS_CHAR ("rowstride"),  &row_stride_2,
                ACE_TEXT_ALWAYS_CHAR ("n-channels"), &n_channels_i,
                NULL);
#endif // GTK_CHECK_VERSION
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT ((resolution_s.cx <= static_cast<LONG> (width_2)) && (resolution_s.cy <= static_cast<LONG> (height_2)));
#else
#if defined (FFMPEG_SUPPORT)
  ACE_ASSERT ((resolution_s.width <= static_cast<unsigned int> (width_2)) && (media_type_s.resolution.height <= static_cast<unsigned int> (height_2)));
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  //      ACE_ASSERT (row_stride_i <= static_cast<unsigned int> (row_stride_2));
  ACE_UNUSED_ARG (row_stride_i);
#if GTK_CHECK_VERSION (3,6,0)
#else
  GDK_THREADS_LEAVE ();
#endif // GTK_CHECK_VERSION (3,6,0)
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

  ACE_GUARD (ACE_Thread_Mutex, aGuard, surfaceLock_);

#if GTK_CHECK_VERSION (3,10,0)
  cairo_surface_flush (surface_);
  ACE_OS::memcpy (cairo_image_surface_get_data (surface_),
#else
  ACE_OS::memcpy (gdk_pixbuf_get_pixels (surface_),
#endif // GTK_CHECK_VERSION (3,10,0)
                  message_inout->rd_ptr (),
                  message_inout->length ());

#if GTK_CHECK_VERSION (3,10,0)
  cairo_surface_mark_dirty (surface_);
#else
  // *TODO*: do this somewhere else (original flip happens during capture)
  //         --> fix it there)
//  GdkPixbuf* pixbuf_p = gdk_pixbuf_flip (surface_, FALSE);
//  ACE_ASSERT (pixbuf_p);
//  g_object_unref (surface_);
//  surface_ = pixbuf_p;
#endif // GTK_CHECK_VERSION (3,10,0)
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *TODO*: remove type inferences
#if GTK_CHECK_VERSION (4,0,0)
      GdkSurface* window_h = inherited4::convert (inherited::configuration_->window);
#else
      GdkWindow* window_h = inherited4::convert (inherited::configuration_->window);
#endif // GTK_CHECK_VERSION (4,0,0)
      if (!window_h)
        break; // do it later, using setP()

      setP (window_h);
      if (unlikely (!surface_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Vis_GTK_Cairo_T::setP(%@), aborting\n"),
                    inherited::mod_->name (),
                    window_h));
        goto error;
      } // end IF

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (session_data_r.lock);
      Common_Image_Resolution_t resolution_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_V4L_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        inherited3::getMediaType (session_data_r.formats.back (),
                                  STREAM_MEDIATYPE_VIDEO,
                                  media_type_s);
      } // end lock scope
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      resolution_s.width = media_type_s.format.width;
      resolution_s.height = media_type_s.format.height;
#endif // ACE_WIN32 || ACE_WIN64

      ACE_GUARD (ACE_Thread_Mutex, aGuard, surfaceLock_);

#if GTK_CHECK_VERSION (3,6,0)
#else
      GDK_THREADS_ENTER ();
#endif // GTK_CHECK_VERSION (3,6,0)
      if (surface_)
#if GTK_CHECK_VERSION (3,10,0)
        cairo_surface_destroy (surface_);
#else
        g_object_unref (surface_);
#endif // GTK_CHECK_VERSION(3,10,0)
      surface_ = NULL;

#if GTK_CHECK_VERSION (4,0,0)
      GdkSurface* window_h = inherited4::convert (inherited::configuration_->window);
#else
      GdkWindow* window_h = inherited4::convert (inherited::configuration_->window);
#endif // GTK_CHECK_VERSION (4,0,0)
      ACE_ASSERT (window_h);

      setP (window_h);
      if (unlikely (!surface_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Vis_GTK_Cairo_T::setP(%@), aborting\n"),
                    inherited::mod_->name (),
                    window_h));
        goto error_2;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resized surface to %ux%u\n"),
                  inherited::mod_->name (),
                  resolution_s.cx,
                  resolution_s.cy));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resized surface to %ux%u\n"),
                  inherited::mod_->name (),
                  resolution_s.width,
                  resolution_s.height));
#endif // ACE_WIN32 || ACE_WIN64

      inherited2::resizing_ = false;

      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (likely (surface_))
#if GTK_CHECK_VERSION (3,10,0)
        cairo_surface_destroy (surface_);
#else
        g_object_unref (surface_);
#endif // GTK_CHECK_VERSION(3,10,0)
      surface_ = NULL;

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
                              MediaType>::dispatch (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::dispatch"));

  // sanity check(s)
  if (unlikely (!surface_ ||
                inherited2::resizing_))
    return;
  cairo_t* context_p = static_cast<cairo_t*> (arg_in);

  ACE_GUARD (ACE_Thread_Mutex, aGuard, surfaceLock_);

#if GTK_CHECK_VERSION (3,10,0)
  ACE_ASSERT (context_p);
  cairo_surface_flush (surface_);
  cairo_set_source_surface (context_p, surface_, 0.0, 0.0);
  cairo_paint (context_p);
#elif GTK_CHECK_VERSION (3,0,0)
  ACE_ASSERT (context_p);
  gdk_cairo_set_source_pixbuf (context_p, surface_, 0.0, 0.0);
  //cairo_pattern_t* pattern_p = cairo_get_source (context_p);
  //ACE_ASSERT (pattern_p);
  //cairo_surface_t* surface_p = NULL;
  //cairo_pattern_get_surface (pattern_p, &surface_p);
  //ACE_ASSERT (surface_p);
  //cairo_surface_flush (surface_p);
  cairo_paint (context_p);
#else
  ACE_UNUSED_ARG (context_p);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (surface_);
  GdkWindow* window_h = inherited4::convert (inherited::configuration_->window);
  ACE_ASSERT (window_h);

  gdk_draw_pixbuf (GDK_DRAWABLE (window_h),
                   NULL,
                   surface_,
                   0, 0, 0, 0,
                   gdk_pixbuf_get_width (surface_),
                   gdk_pixbuf_get_height (surface_),
                   GDK_RGB_DITHER_NONE, 0, 0);
#endif // GTK_CHECK_VERSION(3,10,0)
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

  //ACE_ASSERT (false); // *TODO*
  //ACE_NOTSUP;
  //ACE_NOTREACHED (return;)
}
