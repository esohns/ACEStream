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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

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
                               MediaType>::Stream_Module_Vis_GTK_Pixbuf_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , inherited4 ()
#if GTK_CHECK_VERSION (3,0,0)
 , context_ (NULL)
#endif // GTK_CHECK_VERSION (3,0,0)
 , sourceHasAlphaChannel_ (false)
 , sourceResolution_ ()
 , targetResolution_ ()
 , window_ (NULL)
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

#if GTK_CHECK_VERSION (4,0,0)
  if (context_)
    g_object_unref (context_);
#elif GTK_CHECK_VERSION (3,0,0)
 if (context_)
   cairo_destroy (context_);
#endif // GTK_CHECK_VERSION
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
#if GTK_CHECK_VERSION (4,0,0)
    if (context_)
    {
      g_object_unref (context_); context_ = NULL;
    } // end IF
#elif GTK_CHECK_VERSION (3,0,0)
    if (context_)
    {
      cairo_destroy (context_); context_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION
    window_ = NULL;
  } // end IF

  // sanity check(s)
  window_ = inherited4::convert (configuration_in.window);
  ACE_ASSERT (window_);
#if GTK_CHECK_VERSION (4,0,0)
  context_ = gdk_surface_create_cairo_context (window_);
  ACE_ASSERT (context_);
#elif GTK_CHECK_VERSION(3, 0, 0)
#if GTK_CHECK_VERSION (3,22,0)
#else
  context_ = gdk_cairo_create (window_);
  ACE_ASSERT (context_);
#endif // GTK_CHECK_VERSION (3,22,0)
#endif // GTK_CHECK_VERSION

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if GTK_CHECK_VERSION (4,0,0)
  targetResolution_.cx = gdk_surface_get_width (window_);
  targetResolution_.cy = gdk_surface_get_height (window_);
#else
  targetResolution_.cx = gdk_window_get_width (window_);
  targetResolution_.cy = gdk_window_get_height (window_);
#endif // GTK_CHECK_VERSION (4,0,0)
#else
#if GTK_CHECK_VERSION (4,0,0)
  targetResolution_.width = gdk_surface_get_width (window_);
  targetResolution_.height = gdk_surface_get_height (window_);
#else
  targetResolution_.width = gdk_window_get_width (window_);
  targetResolution_.height = gdk_window_get_height (window_);
#endif // GTK_CHECK_VERSION (4,0,0)
#endif // ACE_WIN32 || ACE_WIN64

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
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (window_);
  if (unlikely (inherited2::resizing_))
    return; // done

#if GTK_CHECK_VERSION (3,6,0)
#else
  bool leave_gdk = false;
  gdk_threads_enter ();
  leave_gdk = true;
#endif // GTK_CHECK_VERSION (3,6,0)

#if GTK_CHECK_VERSION (4,0,0)
  GdkCairoContext* context_p = context_;
  cairo_region_t* cairo_region_p = NULL;
#elif GTK_CHECK_VERSION(3, 0, 0)
  cairo_t* context_p = context_;
#if GTK_CHECK_VERSION (3,22,0)
  cairo_region_t* cairo_region_p = NULL;
  GdkDrawingContext* drawing_context_p = NULL;
#endif // GTK_CHECK_VERSION (3,22,0)
#endif // GTK_CHECK_VERSION

  GdkPixbuf* pixbuf_p =
    gdk_pixbuf_new_from_data (reinterpret_cast<guchar*> (message_inout->rd_ptr ()),
                              GDK_COLORSPACE_RGB,
                              (sourceHasAlphaChannel_ ? TRUE : FALSE),
                              8,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              targetResolution_.cx, targetResolution_.cy,
                              targetResolution_.cx * (sourceHasAlphaChannel_ ? 4 : 3),
#else
                              targetResolution_.width, targetResolution_.height,
                              targetResolution_.width * (sourceHasAlphaChannel_ ? 4 : 3),
#endif // ACE_WIN32 || ACE_WIN64
                              NULL, NULL);
  if (!pixbuf_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_pixbuf_new_from_data(), continuing\n"),
                inherited::mod_->name ()));
    goto continue_;
  } // end IF
  //ACE_ASSERT (GDK_IS_PIXBUF (pixbuf_p));
  //ACE_ASSERT (gdk_pixbuf_get_colorspace (pixbuf_p) == GDK_COLORSPACE_RGB);
  //ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (pixbuf_p) == 8);
  //ACE_ASSERT (gdk_pixbuf_get_n_channels (pixbuf_p) == (sourceHasAlphaChannel_ ? 4 : 3));
  //ACE_ASSERT (gdk_pixbuf_get_has_alpha (pixbuf_p) == (sourceHasAlphaChannel_ ? TRUE : FALSE));

#if GTK_CHECK_VERSION (4,0,0)
  cairo_region_p = cairo_region_create ();
  ACE_ASSERT (cairo_region_p);
  gdk_draw_context_begin_frame (GDK_DRAW_CONTEXT (context_p), cairo_region_p);
  cairo_t* context_2 = gdk_cairo_context_cairo_create (context_p);
  ACE_ASSERT (context_2);
#elif GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,22,0)
  cairo_region_p = cairo_region_create ();
  ACE_ASSERT (cairo_region_p);
  drawing_context_p = gdk_window_begin_draw_frame (window_, cairo_region_p);
  ACE_ASSERT (drawing_context_p);
  context_p =
    gdk_drawing_context_get_cairo_context (drawing_context_p);
#endif // GTK_CHECK_VERSION (3,22,0)
#endif // GTK_CHECK_VERSION

#if GTK_CHECK_VERSION (4,0,0)
  gdk_cairo_set_source_pixbuf (context_2, pixbuf_p, 0.0, 0.0);
  cairo_paint (context_2);
#elif GTK_CHECK_VERSION (3,0,0)
  gdk_cairo_set_source_pixbuf (context_p, pixbuf_p, 0.0, 0.0);
  cairo_paint (context_p);
#else
  gdk_draw_pixbuf (GDK_DRAWABLE (window_),
                   NULL,
                   pixbuf_p,
                   0, 0, 0, 0, -1, -1,
                   GDK_RGB_DITHER_NONE, 0, 0);
#endif // GTK_CHECK_VERSION

#if GTK_CHECK_VERSION (4,0,0)
  gdk_draw_context_end_frame (GDK_DRAW_CONTEXT (context_p));
  cairo_region_destroy (cairo_region_p);
#elif GTK_CHECK_VERSION (3,22,0)
  gdk_window_end_draw_frame (window_, drawing_context_p);
  cairo_region_destroy (cairo_region_p);
#endif // GTK_CHECK_VERSION
  g_object_unref (pixbuf_p); pixbuf_p = NULL;

continue_:
#if GTK_CHECK_VERSION (3,6,0)
 ;
#else
  if (likely (leave_gdk))
  {
    gdk_threads_leave ();
    leave_gdk = false;
  } // end IF
#endif // GTK_CHECK_VERSION (3,6,0)
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
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (session_data_r.lock);
      unsigned int frame_channels_i = 0;
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
      frame_channels_i =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_s) / 8;
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      frame_channels_i =
        Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_s.format.pixelformat) / 8;
      resolution_s.width = media_type_s.format.width;
      resolution_s.height = media_type_s.format.height;
#endif // ACE_WIN32 || ACE_WIN64
      sourceHasAlphaChannel_ = (frame_channels_i >= 4); // *TODO*: this isn't really accurate
      sourceResolution_ = resolution_s;

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      // *TODO*: remove type inferences
      window_ = inherited4::convert (inherited::configuration_->window);
      ACE_ASSERT (window_);

      gint width_i = 0, height_i = 0;
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

#if GTK_CHECK_VERSION (4,0,0)
      if (context_)
      {
        g_object_unref (context_); context_ = NULL;
      } // end IF
#elif GTK_CHECK_VERSION (3,0,0)
      if (context_)
      {
        cairo_destroy (context_); context_ = NULL;
      } // end IF
#endif  // GTK_CHECK_VERSION (3,0,0)

#if GTK_CHECK_VERSION (4,0,0)
      width_i = gdk_surface_get_width (window_);
      height_i = gdk_surface_get_height (window_);
#elif GTK_CHECK_VERSION (3,0,0)
      width_i = gdk_window_get_width (window_);
      height_i = gdk_window_get_height (window_);
#elif GTK_CHECK_VERSION (2,0,0)
      gdk_drawable_get_size (GDK_DRAWABLE (window_),
                             &width_i, &height_i);
#endif // GTK_CHECK_VERSION (3,0,0)

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      targetResolution_.cx = width_i;
      targetResolution_.cy = height_i;
#else
      targetResolution_.width = width_i;
      targetResolution_.height = height_i;
#endif // ACE_WIN32 || ACE_WIN64


#if GTK_CHECK_VERSION (4,0,0)
      context_ = gdk_surface_create_cairo_context (window_);
      ACE_ASSERT (context_);
#elif GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,22,0)
#else
      context_ = gdk_cairo_create (window_);
      ACE_ASSERT (context_);
#endif // GTK_CHECK_VERSION (3,22,0)
#endif // GTK_CHECK_VERSION

#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (session_data_r.lock);
      unsigned int frame_channels_i = 0;
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
      frame_channels_i =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_s) / 8;
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      frame_channels_i =
        Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_s.format.pixelformat) / 8;
      resolution_s.width = media_type_s.format.width;
      resolution_s.height = media_type_s.format.height;
#endif // ACE_WIN32 || ACE_WIN64
      sourceHasAlphaChannel_ = (frame_channels_i >= 4); // *TODO*: this isn't really accurate
      sourceResolution_ = resolution_s;

      inherited2::resizing_ = false;

      break;

//error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

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
