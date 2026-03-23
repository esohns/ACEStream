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
  ACE_TCHAR buffer_a[BUFSIZ];
  int result = ACE_OS::vsprintf (buffer_a, ACE_TEXT (format_in), arguments_in);
  ACE_ASSERT (result >= 0);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s\n"),
              buffer_a));
}

void
libacestream_default_vis_xdg_wm_base_ping (void* data_in,
                                           struct xdg_wm_base* xdg_wm_base_in,
                                           uint32_t serial_in)
{
  xdg_wm_base_pong (xdg_wm_base_in, serial_in);
}

struct xdg_wm_base_listener libacestream_default_vis_xdg_wm_base_listener = {
  .ping = libacestream_default_vis_xdg_wm_base_ping
};

void
libacestream_default_vis_wl_keyboard_keymap (void* data_in,
                                             struct wl_keyboard* keyboard_in,
                                             uint32_t format_in,
                                             int32_t fd_in,
                                             uint32_t size_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
    static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (format_in == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

  char* map_shm_p =
    static_cast<char*> (ACE_OS::mmap (NULL, size_in, PROT_READ, MAP_SHARED, fd_in, 0));
  ACE_ASSERT (map_shm_p != MAP_FAILED);

  struct xkb_keymap* xkb_keymap_p =
    xkb_keymap_new_from_string (data_p->xkb_context,
                                map_shm_p,
                                XKB_KEYMAP_FORMAT_TEXT_V1,
                                XKB_KEYMAP_COMPILE_NO_FLAGS);
  ACE_ASSERT (xkb_keymap_p);
  ACE_OS::munmap (map_shm_p, size_in);
  ACE_OS::close (fd_in);

  struct xkb_state* xkb_state_p = xkb_state_new (xkb_keymap_p);
  ACE_ASSERT (xkb_state_p);
  xkb_keymap_unref (data_p->xkb_keymap);
  xkb_state_unref (data_p->xkb_state);
  data_p->xkb_keymap = xkb_keymap_p;
  data_p->xkb_state = xkb_state_p;
}

void
libacestream_default_vis_wl_keyboard_enter (void* data_in,
                                            struct wl_keyboard* keyboard_in,
                                            uint32_t serial_in,
                                            struct wl_surface* surface_in,
                                            struct wl_array* keys_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
    static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);

  // fprintf (stderr, "keyboard enter; keys pressed are:\n");
  void* key_p;
  char buf[128];
  xkb_keysym_t sym;
  wl_array_for_each (key_p, keys_in)
  {
    sym = xkb_state_key_get_one_sym (data_p->xkb_state,
                                     *static_cast<uint32_t*> (key_p) + 8);
    xkb_keysym_get_name (sym, buf, sizeof (char[128]));
    // fprintf (stderr, "sym: %-12s (%d), ", buf, sym);
    xkb_state_key_get_utf8 (data_p->xkb_state,
                            *static_cast<uint32_t*> (key_p) + 8,
                            buf, sizeof (char[128]));
    // fprintf (stderr, "utf8: '%s'\n", buf);
  } // end wl_array_for_each
}

void
libacestream_default_vis_wl_keyboard_leave (void* data_in,
                                            struct wl_keyboard* keyboard_in,
                                            uint32_t serial_in,
                                            struct wl_surface* surface_in)
{
  // fprintf (stderr, "keyboard leave\n");
}

void
libacestream_default_vis_wl_keyboard_key (void* data_in,
                                          struct wl_keyboard* keyboard_in,
                                          uint32_t serial_in,
                                          uint32_t time_in,
                                          uint32_t key_in,
                                          uint32_t state_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
    static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);

  char buf[128];
  uint32_t keycode = key_in + 8;
  xkb_keysym_t sym = xkb_state_key_get_one_sym (data_p->xkb_state,
                                                keycode);
  if (sym == XKB_KEY_Escape &&
      state_in == WL_KEYBOARD_KEY_STATE_PRESSED)
    data_p->escapeKeyWasPressed = true;
  xkb_keysym_get_name (sym, buf, sizeof (char[128]));
  const char* action_p =
    state_in == WL_KEYBOARD_KEY_STATE_PRESSED ? ACE_TEXT_ALWAYS_CHAR ("press")
                                              : ACE_TEXT_ALWAYS_CHAR ("release");
  // fprintf (stderr, "key %s: sym: %-12s (%d), ", action_p, buf, sym);
  xkb_state_key_get_utf8 (data_p->xkb_state,
                          keycode,
                          buf, sizeof (char[128]));
  // fprintf (stderr, "utf8: '%s'\n", buf);
}

void
libacestream_default_vis_wl_keyboard_modifiers (void* data_in,
                                                struct wl_keyboard* keyboard_in,
                                                uint32_t serial_in,
                                                uint32_t mods_depressed_in,
                                                uint32_t mods_latched_in,
                                                uint32_t mods_locked_in,
                                                uint32_t group_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
    static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);

  xkb_state_update_mask (data_p->xkb_state,
                         mods_depressed_in, mods_latched_in, mods_locked_in,
                         0, 0, group_in);
}

void
libacestream_default_vis_wl_keyboard_repeat_info (void* data_in,
                                                  struct wl_keyboard* keyboard_in,
                                                  int32_t rate_in,
                                                  int32_t delay_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
    static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);
}

struct wl_keyboard_listener libacestream_default_vis_wl_keyboard_listener = {
  .keymap = libacestream_default_vis_wl_keyboard_keymap,
  .enter = libacestream_default_vis_wl_keyboard_enter,
  .leave = libacestream_default_vis_wl_keyboard_leave,
  .key = libacestream_default_vis_wl_keyboard_key,
  .modifiers = libacestream_default_vis_wl_keyboard_modifiers,
  .repeat_info = libacestream_default_vis_wl_keyboard_repeat_info
};

void
libacestream_default_vis_wl_seat_capabilities (void* data_in,
                                               struct wl_seat* seat_in,
                                               uint32_t capabilities_in)
{
  struct libacestream_vis_wayland_cb_data* data_p =
    static_cast<struct libacestream_vis_wayland_cb_data*> (data_in);
  ACE_ASSERT (data_p);

  bool have_keyboard = capabilities_in & WL_SEAT_CAPABILITY_KEYBOARD;
  if (have_keyboard && data_p->keyboard == NULL)
  {
    data_p->keyboard = wl_seat_get_keyboard (seat_in);
    ACE_ASSERT (data_p->keyboard);
    wl_keyboard_add_listener (data_p->keyboard,
                              &libacestream_default_vis_wl_keyboard_listener,
                              data_in);
  } // end IF
  else if (!have_keyboard && data_p->keyboard != NULL)
  {
    wl_keyboard_release (data_p->keyboard); data_p->keyboard = NULL;
  } // end ELSE IF
}

void
libacestream_default_vis_wl_seat_name (void* data_in,
                                       struct wl_seat* seat_in,
                                       const char* name_in)
{
  // sanity check(s)
  ACE_ASSERT (name_in);

  fprintf (stderr, "seat: \"%s\"\n", name_in);
}

struct wl_seat_listener libacestream_default_vis_wl_seat_listener = {
  .capabilities = libacestream_default_vis_wl_seat_capabilities,
  .name = libacestream_default_vis_wl_seat_name
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

  if (!ACE_OS::strcmp (interface_in,
                       ACE_TEXT_ALWAYS_CHAR (wl_compositor_interface.name)))
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
  else if (!ACE_OS::strcmp (interface_in,
                            ACE_TEXT_ALWAYS_CHAR (wl_seat_interface.name)))
  {
    data_p->seat =
      static_cast<struct wl_seat*> (wl_registry_bind (registry_in,
                                                      id_in,
                                                      &wl_seat_interface,
                                                      1));
    wl_seat_add_listener (data_p->seat,
                          &libacestream_default_vis_wl_seat_listener,
                          data_in);
  } // end ELSE IF
  else if (!ACE_OS::strcmp (interface_in,
                            ACE_TEXT_ALWAYS_CHAR (wl_shm_interface.name)))
    data_p->shm =
      static_cast<struct wl_shm*> (wl_registry_bind (registry_in,
                                                     id_in,
                                                     &wl_shm_interface,
                                                     1));
  else if (!ACE_OS::strcmp (interface_in,
                            ACE_TEXT_ALWAYS_CHAR (xdg_wm_base_interface.name)))
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
  .global = libacestream_vis_wayland_global_registry_handler,
  .global_remove = libacestream_vis_wayland_global_registry_remover
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
  .release = libacestream_vis_wayland_buffer_release
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
  .configure = libacestream_vis_wayland_xdg_surface_configure
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
  .done = libacestream_vis_wayland_wl_surface_frame_done
};
