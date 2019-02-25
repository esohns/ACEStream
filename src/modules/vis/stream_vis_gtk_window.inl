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

#include "gtk/gtk.h"

#include "ace/Log_Msg.h"

#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               MediaType>::Stream_Module_Vis_GTK_Window_T (ISTREAM_T* stream_in)
#else
                               MediaType>::Stream_Module_Vis_GTK_Window_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , window_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::Stream_Module_Vis_GTK_Window_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::~Stream_Module_Vis_GTK_Window_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::~Stream_Module_Vis_GTK_Window_T"));

  if (window_)
    gdk_window_destroy (window_);
  if (mainLoop_)
    g_main_destroy (mainLoop_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (window_);

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
  bool leave_gdk = false;
  GdkPixbuf* buffer_p = NULL;
  gint width_i, height_i;

  gdk_threads_enter ();
  leave_gdk = true;

  gdk_drawable_get_size (GDK_DRAWABLE (window_),
                         &width_i, &height_i);

  buffer_p =
#if GTK_CHECK_VERSION (3,0,0)
      gdk_pixbuf_get_from_window (window_,
                                  0, 0,
                                  width_i, height_i);
#else
      gdk_pixbuf_get_from_drawable (NULL,
                                    GDK_DRAWABLE (window_),
                                    NULL,
                                    0, 0,
                                    0, 0, width_i, height_i);
#endif
  if (!buffer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));
    return;
  } // end IF
  ACE_ASSERT (gdk_pixbuf_get_colorspace (buffer_p) == GDK_COLORSPACE_RGB);
  ACE_ASSERT (gdk_pixbuf_get_bits_per_sample (buffer_p) == 8);
  ACE_ASSERT (gdk_pixbuf_get_n_channels (buffer_p) == 3);

  ACE_OS::memcpy (gdk_pixbuf_get_pixels (buffer_p),
                  message_inout->rd_ptr (),
                  message_inout->length ());

//    gint width_i, height_i;
//    gdk_drawable_get_size (GDK_DRAWABLE (inherited::configuration_->window),
//                           &width_i, &height_i);
  gdk_draw_pixbuf (GDK_DRAWABLE (window_),
                   NULL,
                   buffer_p,
                   0, 0, 0, 0, -1, -1,
                   GDK_RGB_DITHER_NONE, 0, 0);

    g_object_unref (buffer_p); buffer_p = NULL;

  if (likely (leave_gdk))
  {
    gdk_threads_leave ();
    leave_gdk = false;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (!window_ && !mainLoop_);
      ACE_ASSERT (Common_UI_GTK_Tools::GTKInitialized);

      gdk_threads_enter ();

      GdkWindowAttr attributes_a;
      gint attributes_mask;

      attributes_a.window_type = GDK_WINDOW_TOPLEVEL;
      attributes_a.width =
          inherited::configuration_->outputFormat.resolution.width;
      attributes_a.height =
          inherited::configuration_->outputFormat.resolution.height;
      attributes_a.wclass = GDK_INPUT_OUTPUT;
      attributes_a.colormap = gdk_rgb_get_cmap ();

      attributes_mask = GDK_WA_COLORMAP;

      window_ = gdk_window_new (NULL,
                                &attributes_a, attributes_mask);
      ACE_ASSERT (window_);

      gdk_window_show (window_);

      mainLoop_ = g_main_new (TRUE);
      ACE_ASSERT (mainLoop_);

      gdk_threads_leave ();

      ACE_thread_t thread_id;
      inherited::start (thread_id);
      ACE_UNUSED_ARG (thread_id);

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (window_)
      {
        gdk_window_destroy (window_); window_ = NULL;
      } // end IF

      ACE_ASSERT (g_main_is_running (mainLoop_));
      g_main_quit (mainLoop_);

      if (mainLoop_)
      {
        g_main_destroy (mainLoop_); mainLoop_ = NULL;
      } // end IF


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
          typename MediaType>
bool
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (window_)
    {
      gdk_window_destroy (window_); window_ = NULL;
    } // end IF
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
          typename MediaType>
void
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::toggle"));

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
          typename MediaType>
int
Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::svc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Window_T::svc"));

  // sanity check(s)
  ACE_ASSERT (Common_UI_GTK_Tools::GTKInitialized);
  ACE_ASSERT (mainLoop_);

  g_main_run (mainLoop_);
}
