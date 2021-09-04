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

#ifndef STREAM_VIS_WAYLAND_WINDOW_H
#define STREAM_VIS_WAYLAND_WINDOW_H

#include "wayland-client.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_ui_ifullscreen.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_vis_wayland_window_module_name_string[];

struct libacestream_vis_wayland_cb_data
{
  struct wl_buffer*       buffer;
//  bool                    buffer_busy;
  struct wl_compositor*   compositor;
  struct wl_display*      display;
//  ACE_Message_Queue_Base* queue;
  struct wl_shell*        shell;
  struct wl_shm*          shm;
  void*                   shm_data;
};

void
libacestream_vis_wayland_global_registry_handler (void*,
                                                  struct wl_registry*,
                                                  uint32_t,
                                                  const char*,
                                                  uint32_t);
void
libacestream_vis_wayland_global_registry_remover (void*,
                                                  struct wl_registry*,
                                                  uint32_t);
extern struct wl_registry_listener libacestream_vis_wayland_registry_listener;

//////////////////////////////////////////

void
libacestream_vis_wayland_buffer_release (void*,
                                         struct wl_buffer*);
extern struct wl_buffer_listener libacestream_vis_wayland_buffer_listener;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType> // *IMPORTANT NOTE*: must correspond to session data 'formats' member
class Stream_Module_Vis_Wayland_Window_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                    >
#else
                                                     ,typename SessionMessageType::DATA_T::DATA_T>
#endif // ACE_WIN32 || ACE_WIN64
 , public Common_UI_IFullscreen
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                    > inherited2;
#else
                                                     ,typename SessionMessageType::DATA_T::DATA_T> inherited2;
#endif // ACE_WIN32 || ACE_WIN64

 public:
  Stream_Module_Vis_Wayland_Window_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Vis_Wayland_Window_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_UI_IFullscreen
  virtual void toggle ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_Wayland_Window_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_Wayland_Window_T (const Stream_Module_Vis_Wayland_Window_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_Wayland_Window_T& operator= (const Stream_Module_Vis_Wayland_Window_T&))

  void init_shm_pool (const ConfigurationType&);
//  void init_window ();

  struct libacestream_vis_wayland_cb_data cbData_;
  bool                                    closeDisplay_;
  struct wl_shell_surface*                shellSurface_;
  struct wl_surface*                      surface_;
};

// include template definition
#include "stream_vis_wayland_window.inl"

#endif
