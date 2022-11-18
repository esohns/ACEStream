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
 , frameSize_ (0)
 , shellSurface_ (NULL)
 , topLevel_ (NULL)
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

  if (topLevel_)
    xdg_toplevel_destroy (topLevel_);
  if (shellSurface_)
    xdg_surface_destroy (shellSurface_);
  if (cbData_.surface)
    wl_surface_destroy (cbData_.surface);
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
  ACE_ASSERT (cbData_.display);
  ACE_ASSERT (cbData_.shm_data);
  ACE_ASSERT (cbData_.surface);

  ACE_OS::memcpy (cbData_.shm_data, message_inout->rd_ptr (), message_inout->length ());
//  wl_surface_commit (cbData_.surface);
//  wl_surface_damage (cbData_.surface,
//                     0, 0,
//                     cbData_.resolution.width, cbData_.resolution.height);
//  wl_display_flush (cbData_.display);
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
      const typename SessionDataContainerType::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
      const MediaType& media_type_r = session_data_r.formats.back ();
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited2::getMediaType (media_type_r,
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_2);
      cbData_.resolution = inherited2::getResolution (media_type_r);
      frameSize_ =
        av_image_get_buffer_size (media_type_2.format,
                                  cbData_.resolution.width, cbData_.resolution.height,
                                  1); // *TODO*: linesize alignment
      ACE_ASSERT (frameSize_ >= 0);
//      ACE_ASSERT (media_type_2.format == AV_PIX_FMT_RGB32);

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      inherited::stop (false,  // wait for completion ?
                       false); // high priority ?

      // sanity check(s)
      ACE_ASSERT (cbData_.display);

      if (cbData_.surface || shellSurface_)
      { ACE_ASSERT (cbData_.display);
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
    if (topLevel_)
    {
      xdg_toplevel_destroy (topLevel_); topLevel_ = NULL;
    } // end IF
    if (shellSurface_)
    {
      xdg_surface_destroy (shellSurface_); shellSurface_ = NULL;
    }
    if (cbData_.surface)
    {
      wl_surface_destroy (cbData_.surface); cbData_.surface = NULL;
    } // end IF
    if (closeDisplay_)
    { ACE_ASSERT (cbData_.display);
      wl_display_disconnect (cbData_.display);
      cbData_.display = NULL;
    } // end IF
    closeDisplay_ = false;
    cbData_.display = NULL;
  } // end IF

#if defined (_DEBUG)
//  wl_log_set_handler_client (libacestream_vis_wayland_global_log_cb);
#endif // _DEBUG

  ACE_ASSERT (!cbData_.display);
  // *TODO*: remove type inferences
  if (configuration_in.waylandDisplay)
  {
    cbData_.display = configuration_in.waylandDisplay;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode (display: %@)\n"),
                inherited::mod_->name (),
                cbData_.display));
  } // end IF
  else
  {
    cbData_.display = wl_display_connect (NULL);
    if (unlikely (!cbData_.display))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to wl_display_connect(NULL): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: connected to Wayland display (%@)\n"),
                inherited::mod_->name (),
                cbData_.display));
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
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: retrieved Wayland compositor handle (%@)\n"),
                inherited::mod_->name (),
                cbData_.compositor));

  if (configuration_in.surface)
  {
//    surface_ = configuration_in.surface;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode (display: %@, surface: %u)\n"),
                inherited::mod_->name (),
                cbData_.display, cbData_.surface));
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

    cbData_.surface = wl_compositor_create_surface (cbData_.compositor);
    if (!cbData_.surface)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: cannot create Wayland surface (display was: %@): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  cbData_.display));
      return false;
    } // end IF
    struct wl_callback* callback_p = wl_surface_frame (cbData_.surface);
    ACE_ASSERT (callback_p);
    wl_callback_add_listener (callback_p,
                              &libacestream_vis_wayland_wl_surface_frame_listener,
                              &cbData_);

//    ACE_ASSERT (cbData_.shell);
//    shellSurface_ = wl_shell_get_shell_surface (cbData_.shell, surface_);
    ACE_ASSERT (cbData_.wm_base);
    shellSurface_ =
        xdg_wm_base_get_xdg_surface (cbData_.wm_base, cbData_.surface);
    if (!shellSurface_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: cannot create Wayland shell surface (display was: %@): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  cbData_.display));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: created Wayland shell surface (display: %@)\n"),
                inherited::mod_->name (),
                cbData_.display));
  } // end ELSE
  ACE_ASSERT (shellSurface_);
  xdg_surface_add_listener (shellSurface_,
                            &libacestream_vis_wayland_xdg_surface_listener,
                            &cbData_);

//  wl_shell_surface_set_toplevel (shellSurface_);
  topLevel_ = xdg_surface_get_toplevel (shellSurface_);
  ACE_ASSERT (topLevel_);
  xdg_toplevel_set_title (topLevel_,
                          ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING));
  ACE_ASSERT (topLevel_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: display %@ (shell surface: %@, toplevel surface: %@)\n"),
              inherited::mod_->name (),
              cbData_.display, shellSurface_, topLevel_));

  // *TODO*: move this into handleSessionMessage
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

  struct Common_UI_DisplayDevice display_device =
    Common_UI_Tools::getDisplay (configuration_in.display.device); // device identifier

//  int x =
//      (inherited::configuration_->fullScreen ? inherited::configuration_->display.clippingArea.x
//                                             : 0);
//  int y =
//      (inherited::configuration_->fullScreen ? inherited::configuration_->display.clippingArea.y
//                                             : 0);
  Common_Image_Resolution_t resolution_s =
      inherited2::getResolution (configuration_in.outputFormat);
  unsigned int width_i =
      (configuration_in.fullScreen ? display_device.clippingArea.width
                                   : resolution_s.width);
  unsigned int height_i =
      (configuration_in.fullScreen ? display_device.clippingArea.height
                                   : resolution_s.height);

  struct wl_shm_pool* pool_p = NULL;
  int fd, size, stride;
  void* data_p = NULL;

  stride = width_i * 4;
  size = stride * height_i;

  static const char name_template[] = "/libacestream-shared-XXXXXX";
  const char* path_p = NULL;
  char* name_p = NULL;
  path_p = ACE_OS::getenv ("XDG_RUNTIME_DIR");
  if (!path_p)
  {
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
  if (fd < 0)
  {
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
  if (data_p == MAP_FAILED)
  {
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
  wl_shm_pool_destroy (pool_p); pool_p = NULL;
  ACE_OS::close (fd); fd = ACE_INVALID_HANDLE;

  ACE_ASSERT (cbData_.surface);
  wl_surface_attach (cbData_.surface, cbData_.buffer, 0, 0);
  wl_surface_commit (cbData_.surface);
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
int
Stream_Module_Vis_Wayland_Window_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Wayland_Window_T::svc"));

  int result = -1;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  bool stop_processing = false;

  // step1: start processing data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result == 0)
    {
      ACE_ASSERT (message_block_p);
      ACE_Message_Block::ACE_Message_Type message_type =
        message_block_p->msg_type ();
      if (message_type == ACE_Message_Block::MB_STOP)
      {
        // clean up
        message_block_p->release (); message_block_p = NULL;

        result_2 = 0; // OK

        break; // aborted
      } // end IF

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
    } // end IF
    else if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
        break;
      } // end IF
    } // end ELSE IF

    wl_display_dispatch (cbData_.display);
  } while (true);

//done:
  return result_2;
}
