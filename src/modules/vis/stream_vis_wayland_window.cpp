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
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING);

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

  printf ("Got a registry event for %s id %d\n", interface_in, id_in);

  if (!ACE_OS::strcmp (interface_in, ACE_TEXT_ALWAYS_CHAR ("wl_compositor")))
    data_p->compositor =
      static_cast<struct wl_compositor*> (wl_registry_bind (registry_in,
                                                            id_in,
                                                            &wl_compositor_interface,
                                                            1));
  else if (!ACE_OS::strcmp (interface_in, ACE_TEXT_ALWAYS_CHAR ("wl_shell")))
    data_p->shell =
      static_cast<struct wl_shell*> (wl_registry_bind (registry_in,
                                                       id_in,
                                                       &wl_shell_interface,
                                                       1));
  else if (!ACE_OS::strcmp (interface_in, ACE_TEXT_ALWAYS_CHAR ("wl_shm")))
    data_p->shm =
      static_cast<struct wl_shm*> (wl_registry_bind (registry_in,
                                                     id_in,
                                                     &wl_shm_interface,
                                                     1));
}

void
libacestream_vis_wayland_global_registry_remover (void* data_in,
                                                  struct wl_registry* registry_in,
                                                  uint32_t id_in)
{
  ACE_UNUSED_ARG (registry_in);

  struct libacestream_vis_wayland_cb_data* data_p =
      static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);

  printf ("Got a registry losing event for %d\n", id_in);
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
  ACE_ASSERT (data_p);

//  data_p->buffer_busy = false;
}

struct wl_buffer_listener libacestream_vis_wayland_buffer_listener = {
  libacestream_vis_wayland_buffer_release
};
