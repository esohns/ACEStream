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

#include "common_file_tools.h"

#include "common_image_tools.h"

#include "common_ui_defines.h"
#include "common_ui_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

//#include "stream_lib_tools.h"

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::Stream_Module_Vis_Wayland_Window_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , cbData_ ()
 , closeDisplay_ (false)
 , shellSurface_ (NULL)
 , surface_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::Stream_Module_Vis_Wayland_Window_T"));

  ACE_OS::memset (&cbData_, 0, sizeof (struct libacestream_vis_wayland_cb_data));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::~Stream_Module_Vis_Wayland_Window_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::~Stream_Module_Vis_Wayland_Window_T"));

  if (surface_ || shellSurface_)
  { ACE_ASSERT (cbData_.display);
//    result = XDestroyWindow (display_, window_);
//    if (unlikely (!result))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to XDestroyWindow(0x%@,%u): \"%m\", continuing\n"),
//                  inherited::mod_->name (),
//                  display_, window_));
  } // end IF
  if (closeDisplay_)
  { ACE_ASSERT (cbData_.display);
    wl_display_disconnect (cbData_.display);
  } // end IF
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
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (cbData_.display);
  ACE_ASSERT (surface_);
  ACE_ASSERT (cbData_.shm_data);

  // sanity check(s)
  const typename SessionDataContainerType::DATA_T& session_data_r =
      inherited::sessionData_->getR ();
  ACE_ASSERT (!session_data_r.formats.empty ());
  const MediaType& media_type_r = session_data_r.formats.back ();
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
  inherited2::getMediaType (media_type_r,
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_2);
//  Common_Image_Resolution_t resolution_s =
//      Stream_MediaFramework_Tools::toResolution (cbData_.display,
//                                                 window_);
//  int row_size_i =
//      av_image_get_linesize (media_type_2.format,
//                             media_type_2.resolution.width,
//                             0);
  ACE_ASSERT (media_type_2.format == AV_PIX_FMT_RGB32);
//  ACE_ASSERT ((media_type_2.resolution.width == resolution_s.width) && (media_type_2.resolution.height == resolution_s.height));

  unsigned int image_size_i =
      media_type_2.resolution.width  *
      media_type_2.resolution.height *
      4;

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

  ACE_OS::memcpy (cbData_.shm_data, message_inout->rd_ptr (), image_size_i);
  wl_surface_commit (surface_);
  wl_surface_damage (surface_,
                     0, 0,
                     media_type_2.resolution.width, media_type_2.resolution.height);
  wl_display_flush (cbData_.display);
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
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (cbData_.display);
      ACE_ASSERT (shellSurface_);
//      const typename SessionDataContainerType::DATA_T& session_data_r =
//          inherited::sessionData_->getR ();
//      ACE_ASSERT (!session_data_r.formats.empty ());
//      const MediaType& media_type_r = session_data_r.formats.back ();
//      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
//      inherited2::getMediaType (media_type_r,
//                                media_type_2);
//      Common_Image_Resolution_t resolution_s =
//          Stream_MediaFramework_Tools::toResolution (*cbData_.display,
//                                                     window_);
//      int row_size_i =
//          av_image_get_linesize (media_type_2.format,
//                                 media_type_2.resolution.width,
//                                 0);
//      int depth_i =
//          (row_size_i / media_type_2.resolution.width) * 8;
//      XWindowAttributes attributes_s = Common_UI_Tools::get (*cbData_.display,
//                                                             window_);
      // *NOTE*: otherwise there will be 'BadMatch' errors
//      ACE_ASSERT ((media_type_2.resolution.width == resolution_s.width) && (media_type_2.resolution.height == resolution_s.height));
//      ACE_ASSERT (depth_i == attributes_s.depth);

//      pixmap_ = XCreatePixmap (display_,
//                               window_,
//                               resolution_s.width, resolution_s.height,
//                               attributes_s.depth);
//      if (unlikely (!pixmap_))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to XCreatePixmap(0x%@,%u,%ux%u,%d): \"%m\", aborting\n"),
//                    inherited::mod_->name (),
//                    display_, window_,
//                    resolution_s.width, resolution_s.height,
//                    attributes_s.depth));
//        goto error;
//      } // end IF
#if defined (_DEBUG)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: allocated %ux%u pixmap (depth: %d bits)\n"),
//                  inherited::mod_->name (),
//                  resolution_s.width, resolution_s.height,
//                  attributes_s.depth));
#endif // _DEBUG

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      ACE_ASSERT (cbData_.display);

      if (surface_ || shellSurface_)
      { ACE_ASSERT (cbData_.display);
//        result = XDestroyWindow (display_, window_);
//        if (unlikely (!result))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%s: failed to XDestroyWindow(0x%@,%u): \"%m\", continuing\n"),
//                      inherited::mod_->name (),
//                      display_, window_));
//        window_ = None;
//        closeWindow_ = false;
      } // end IF
      if (closeDisplay_)
      { ACE_ASSERT (cbData_.display);
        wl_display_disconnect (cbData_.display);
        cbData_.display = NULL;
        closeDisplay_ = false;
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
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::toggle"));

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
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (surface_ || shellSurface_)
    { ACE_ASSERT (cbData_.display);
//      int result = XUnmapWindow (display_, window_);
//      if (unlikely (result))
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to XUnmapWindow(0x%@,%u): \"%m\", continuing\n"),
//                    inherited::mod_->name (),
//                    display_, window_));
      shellSurface_ = NULL;
      surface_ = NULL;
    } // end IF
    if (closeDisplay_)
    { ACE_ASSERT (cbData_.display);
      wl_display_disconnect (cbData_.display);
      cbData_.display = NULL;
    } // end IF
    closeDisplay_ = false;
    cbData_.display = NULL;
  } // end IF

  ACE_ASSERT (!cbData_.display);
  // *TODO*: remove type inferences
  if (configuration_in.waylandDisplay)
  {
    cbData_.display = configuration_in.waylandDisplay;
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode (display: %@)\n"),
                inherited::mod_->name (),
                cbData_.display));
#endif // _DEBUG
  } // end IF
  else
  {
//    std::string x11_display_name =
//        Common_UI_Tools::getX11DisplayName (configuration_in.display.device);
//    if (unlikely (x11_display_name.empty ()))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to Common_UI_Tools::getX11DisplayName(\"%s\"), aborting\n"),
//                  inherited::mod_->name (),
//                  ACE_TEXT (configuration_in.display.device.c_str ())));
//    const char* display_name_p =
//        (x11_display_name.empty () ? NULL
//                                   : x11_display_name.c_str ());
    cbData_.display = wl_display_connect (NULL);
    if (unlikely (!cbData_.display))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to wl_display_connect(NULL): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: connected to Wayland display (%@)\n"),
                inherited::mod_->name (),
                cbData_.display));
#endif // _DEBUG
    closeDisplay_ = true;
  } // end ELSE
  ACE_ASSERT (cbData_.display);

  struct wl_registry* registry_p = wl_display_get_registry (cbData_.display);
  ACE_ASSERT (registry_p);
  wl_registry_add_listener (registry_p,
                            &libacestream_vis_wayland_registry_listener,
                            &cbData_);
  wl_display_dispatch (cbData_.display);
  wl_display_roundtrip (cbData_.display);
  if (!cbData_.compositor)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: cannot retrieve Wayland compositor handle (display was: %@): \"%m\", aborting\n"),
                inherited::mod_->name (),
                cbData_.display));
    return false;
  } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: retrieved Wayland compositor handle (%@)\n"),
                inherited::mod_->name (),
                cbData_.compositor));
#endif // _DEBUG

  if (configuration_in.surface)
  {
    shellSurface_ = configuration_in.surface;
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode (display: %@, surface: %u)\n"),
                inherited::mod_->name (),
                cbData_.display, surface_));
#endif // _DEBUG
  } // end IF
  else
  {
//    int x =
//        (configuration_in.fullScreen ? configuration_in.display.clippingArea.x
//                                     : 0);
//    int y =
//        (configuration_in.fullScreen ? configuration_in.display.clippingArea.y
//                                     : 0);
//    unsigned int width_i =
//        (configuration_in.fullScreen ? configuration_in.display.clippingArea.width
//                                     : configuration_in.outputFormat.resolution.width);
//    unsigned int height_i =
//        (configuration_in.fullScreen ? configuration_in.display.clippingArea.height
//                                     : configuration_in.outputFormat.resolution.height);

    surface_ = wl_compositor_create_surface (cbData_.compositor);
    if (!surface_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: cannot create Wayland surface (display was: %@): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  cbData_.display));
      return false;
    } // end IF
    ACE_ASSERT (cbData_.shell);
    shellSurface_ = wl_shell_get_shell_surface (cbData_.shell, surface_);
    if (!shellSurface_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: cannot create Wayland shell surface (display was: %@): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  cbData_.display));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: created Wayland shell surface (display: %@)\n"),
                inherited::mod_->name (),
                cbData_.display));
#endif // _DEBUG
  } // end ELSE
  ACE_ASSERT (shellSurface_);
  wl_shell_surface_set_toplevel (shellSurface_);
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: display %@ (shell surface: %@)\n"),
              inherited::mod_->name (),
              cbData_.display, shellSurface_));
#endif // _DEBUG

  init_shm_pool (configuration_in);

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
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::init_shm_pool (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::init_shm_pool"));

  ACE_ASSERT (cbData_.shm);

//  int x =
//      (inherited::configuration_->fullScreen ? inherited::configuration_->display.clippingArea.x
//                                             : 0);
//  int y =
//      (inherited::configuration_->fullScreen ? inherited::configuration_->display.clippingArea.y
//                                             : 0);
  Common_Image_Resolution_t resolution_s =
      inherited2::getResolution (configuration_in.outputFormat);
  unsigned int width_i =
      (configuration_in.fullScreen ? configuration_in.display.clippingArea.width
                                   : resolution_s.width);
  unsigned int height_i =
      (configuration_in.fullScreen ? configuration_in.display.clippingArea.height
                                   : resolution_s.height);

  struct wl_shm_pool* pool_p = NULL;
  int fd, size, stride;
  void* data_p = NULL;

  stride = width_i * 4;
  size = stride * height_i;

  static const char name_template[] = "/libacestream-shared-XXXXXX";
  const char* path_p = NULL;
  char* name_p = NULL;
  path_p = ACE_OS::getenv("XDG_RUNTIME_DIR");
  if (!path_p) {
    errno = ENOENT;
    return;
  }
  name_p =
      static_cast<char*> (malloc (ACE_OS::strlen (path_p) + sizeof (name_template)));
  if (!name_p)
    return;
  ACE_OS::strcpy (name_p, path_p);
  ACE_OS::strcat (name_p, name_template);
  fd = ACE_OS::mkstemp (name_p);
  if (fd < 0) {
    fprintf (stderr, "creating a buffer file for %d B failed: %s\n",
             size, strerror(errno));
    return;
  }
  if (ACE_OS::ftruncate (fd, size) < 0) {
    ACE_OS::close(fd);
    return;
  }

  data_p = ACE_OS::mmap (NULL,
                         size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED,
                         fd,
                         0);
  if (data_p == MAP_FAILED) {
    fprintf (stderr, "mmap failed: %s\n", strerror(errno));
    ACE_OS::close (fd);
    return;
  }
  cbData_.shm_data = data_p;
  ACE_OS::memset (cbData_.shm_data, 0, size);

  pool_p = wl_shm_create_pool (cbData_.shm, fd, size);
  ACE_ASSERT (pool_p);
  cbData_.buffer = wl_shm_pool_create_buffer (pool_p,
                                              0,
                                              width_i, height_i,
                                              stride, WL_SHM_FORMAT_XRGB8888);
  ACE_ASSERT (cbData_.buffer);
//  wl_buffer_add_listener (cbData_.buffer,
//                          &libacestream_vis_wayland_buffer_listener,
//                          &cbData_);
  wl_shm_pool_destroy (pool_p);
  ACE_OS::close (fd);

  ACE_ASSERT (surface_);
  wl_surface_attach (surface_,
                     cbData_.buffer,
                     0,
                     0);
  wl_surface_commit (surface_);
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataContainerType,
//          typename MediaType>
//void
//Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
//                               TimePolicyType,
//                               ConfigurationType,
//                               ControlMessageType,
//                               DataMessageType,
//                               SessionMessageType,
//                               SessionDataContainerType,
//                               MediaType>::init_window ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::init_window"));

//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (cbData_.surface);


//}
