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
#include "stdafx.h"

#include "stream_vis_wayland_window.h"

#include "stream_vis_defines.h"

const char libacestream_default_vis_wayland_window_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_WAYLAND_WINDOW_DEFAULT_NAME_STRING);

void
libacestream_vis_wayland_global_log_cb (const char* format_in,
                                        va_list arguments_in)
{
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT (format_in),
              arguments_in));
}

static void
libacestream_default_vis_xdg_wm_base_ping (void* data_in, struct xdg_wm_base* xdg_wm_base_in, uint32_t serial_in)
{
  xdg_wm_base_pong (xdg_wm_base_in, serial_in);
}

static const struct xdg_wm_base_listener libacestream_default_vis_xdg_wm_base_listener = {
  .ping = libacestream_default_vis_xdg_wm_base_ping,
};

void
libacestream_vis_wayland_global_registry_handler (void* data_in,
                                                  struct wl_registry* registry_in,
                                                  uint32_t id_in,
                                                  const char* interface_in,
                                                  uint32_t version_in)
{
  ACE_UNUSED_ARG (version_in);

  struct libacestream_vis_wayland_cb_data* data_p =
      static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);

//  printf ("Got a registry event for %s id %d\n", interface_in, id_in);

  if (!ACE_OS::strcmp (interface_in, ACE_TEXT_ALWAYS_CHAR ("wl_compositor")))
    data_p->compositor =
      static_cast<struct wl_compositor*> (wl_registry_bind (registry_in,
                                                            id_in,
                                                            &wl_compositor_interface,
                                                            1));
//  else if (!ACE_OS::strcmp (interface_in, ACE_TEXT_ALWAYS_CHAR ("wl_shell")))
//    data_p->shell =
//      static_cast<struct wl_shell*> (wl_registry_bind (registry_in,
//                                                       id_in,
//                                                       &wl_shell_interface,
//                                                       1));
  else if (!ACE_OS::strcmp (interface_in, ACE_TEXT_ALWAYS_CHAR ("wl_shm")))
    data_p->shm =
      static_cast<struct wl_shm*> (wl_registry_bind (registry_in,
                                                     id_in,
                                                     &wl_shm_interface,
                                                     1));
  else if (!ACE_OS::strcmp (interface_in, ACE_TEXT_ALWAYS_CHAR ("xdg_wm_base")))
  {
    data_p->wm_base =
      static_cast<struct xdg_wm_base*> (wl_registry_bind (registry_in,
                                                          id_in,
                                                          &xdg_wm_base_interface,
                                                          1));
    xdg_wm_base_add_listener (data_p->wm_base,
                              &libacestream_default_vis_xdg_wm_base_listener,
                              data_in);
  } // end ELSE IF
}

void
libacestream_vis_wayland_global_registry_remover (void* data_in,
                                                  struct wl_registry* registry_in,
                                                  uint32_t id_in)
{
  ACE_UNUSED_ARG (registry_in);

  struct libacestream_vis_wayland_cb_data* data_p =
      static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p); ACE_UNUSED_ARG (data_p);

//  printf ("Got a registry losing event for %d\n", id_in);
}

struct wl_registry_listener libacestream_vis_wayland_registry_listener = {
    libacestream_vis_wayland_global_registry_handler,
    libacestream_vis_wayland_global_registry_remover
};

//////////////////////////////////////////

void
libacestream_vis_wayland_buffer_release (void *data_in,
                                         struct wl_buffer* buffer_in)
{
  ACE_UNUSED_ARG (buffer_in);

  struct libacestream_vis_wayland_cb_data* data_p =
      static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p); ACE_UNUSED_ARG (data_p);

//  data_p->buffer_busy = false;
}

struct wl_buffer_listener libacestream_vis_wayland_buffer_listener = {
  libacestream_vis_wayland_buffer_release
};

void
libacestream_vis_wayland_xdg_surface_configure (void* data_in,
                                                struct xdg_surface* surface_in,
                                                uint32_t serial_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
      static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->display);
//  ACE_ASSERT (data_p->surface);

  xdg_surface_ack_configure (surface_in, serial_in);

//  wl_surface_attach (data_p->surface, data_p->buffer, 0, 0);
//  wl_surface_damage_buffer (data_p->surface,
//                            0, 0,
//                            data_p->resolution.width, data_p->resolution.height);
//  wl_surface_commit (data_p->surface);
}

struct xdg_surface_listener libacestream_vis_wayland_xdg_surface_listener = {
  .configure = libacestream_vis_wayland_xdg_surface_configure,
};

void
libacestream_vis_wayland_wl_surface_frame_done (void* data_in,
                                                struct wl_callback* callback_in,
                                                uint32_t time_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
      static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->surface);

  /* Destroy this callback */
  wl_callback_destroy (callback_in);

  /* Request another frame */
  struct wl_callback* callback_p = wl_surface_frame (data_p->surface);
  ACE_ASSERT (callback_p);
  wl_callback_add_listener (callback_p,
                            &libacestream_vis_wayland_wl_surface_frame_listener,
                            data_in);

  /* Submit a frame for this event */
  wl_surface_attach (data_p->surface, data_p->buffer, 0, 0);
//  wl_surface_damage_buffer (data_p->surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_damage (data_p->surface, 0, 0, data_p->resolution.width, data_p->resolution.height);
  wl_surface_commit (data_p->surface);
}

struct wl_callback_listener libacestream_vis_wayland_wl_surface_frame_listener = {
  .done = libacestream_vis_wayland_wl_surface_frame_done,
};
