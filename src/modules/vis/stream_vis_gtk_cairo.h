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

#include "common_ui_ifullscreen.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

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
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                    >
#else
                                                     ,SessionDataType>
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
                                                     ,SessionDataType> inherited2;
#endif // ACE_WIN32 || ACE_WIN64

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Vis_GTK_Cairo_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_Vis_GTK_Cairo_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Module_Vis_GTK_Cairo_T ();

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
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T (const Stream_Module_Vis_GTK_Cairo_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T& operator= (const Stream_Module_Vis_GTK_Cairo_T&))

  // helper methods
  inline unsigned char clamp (int value_in) { return ((value_in > 255) ? 255 : ((value_in < 0) ? 0 : static_cast<unsigned char> (value_in))); }

#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_t*                         buffer_; // target-
#else
  GdkPixbuf*                               buffer_; // target-
#endif // GTK_CHECK_VERSION
  cairo_t*                                 context_;
  bool                                     isFirst_;
  typename ACE_SYNCH_USE::RECURSIVE_MUTEX* lock_; // surface-
};

// include template definition
#include "stream_vis_gtk_cairo.inl"

#endif
