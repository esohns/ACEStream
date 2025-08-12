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

#ifndef STREAM_MODULE_VIS_GTK_CAIRO_H
#define STREAM_MODULE_VIS_GTK_CAIRO_H

#include "gtk/gtk.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_inotify.h"

#include "common_ui_ifullscreen.h"
#include "common_ui_windowtype_converter.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_vis_base.h"

extern const char libacestream_default_vis_gtk_cairo_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType>
class Stream_Module_Vis_GTK_Cairo_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_Visualization_Base
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
#if GTK_CHECK_VERSION (4,0,0)
 , public Common_UI_WindowTypeConverter_T<GdkSurface*>
#else
 , public Common_UI_WindowTypeConverter_T<GdkWindow*>
#endif // GTK_CHECK_VERSION (4,0,0)
 , public Common_IDispatch
#if GTK_CHECK_VERSION (4,0,0)
 , public Common_ISetP_T<GdkSurface>
#else
 , public Common_ISetP_T<GdkWindow>
#endif // GTK_CHECK_VERSION (4,0,0)
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
  typedef Stream_Visualization_Base inherited2;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited3;
#if GTK_CHECK_VERSION (4,0,0)
  typedef Common_UI_WindowTypeConverter_T<GdkSurface*> inherited4;
#else
  typedef Common_UI_WindowTypeConverter_T<GdkWindow*> inherited4;
#endif // GTK_CHECK_VERSION (4,0,0)

 public:
  Stream_Module_Vis_GTK_Cairo_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Vis_GTK_Cairo_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IDispatch
  // *IMPORTANT NOTE*: argument MUST be cairo_t* !
  virtual void dispatch (void*);

  // implement Common_ISetP_T
#if GTK_CHECK_VERSION (4,0,0)
  virtual void setP (GdkSurface*); // target window
#else
  virtual void setP (GdkWindow*); // target window
#endif // GTK_CHECK_VERSION (4,0,0)

  // implement Common_UI_IFullscreen
  virtual void toggle ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T (const Stream_Module_Vis_GTK_Cairo_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T& operator= (const Stream_Module_Vis_GTK_Cairo_T&))

  cairo_t*         context_;
#if GTK_CHECK_VERSION (3,10,0)
  cairo_surface_t* surface_; // target-
#else
  GdkPixbuf*       surface_; // target-
#endif // GTK_CHECK_VERSION
  ACE_Thread_Mutex surfaceLock_;
#if GTK_CHECK_VERSION (4,0,0)
  GdkCairoContext* drawingContext_;
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
  cairo_region_t*  cairoRegion_;
#endif // GTK_CHECK_VERSION (3,22,0)
};

// include template definition
#include "stream_vis_gtk_cairo.inl"

#endif
