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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfobjects.h>
#include <strmif.h>
#endif

extern "C"
{
#include "libavcodec/avcodec.h"
}

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "stream_task_base_synch.h"

#include "stream_vis_exports.h"

extern Stream_Vis_Export const char libacestream_default_vis_gtk_cairo_module_name_string[];

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
          typename SessionDataContainerType>
class Stream_Module_Vis_GTK_Cairo_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Vis_GTK_Cairo_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_Vis_GTK_Cairo_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_Vis_GTK_Cairo_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: return values needs to be Stream_Module_Device_DirectShow_Tools::deleteMediaType()d !
  template <typename FormatType2> AM_MEDIA_TYPE& getFormat (const FormatType2* format_in) { return getFormat_impl (format_in); };
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T (const Stream_Module_Vis_GTK_Cairo_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_T& operator= (const Stream_Module_Vis_GTK_Cairo_T&))

  // helper methods
  inline unsigned char clamp (int value_in) { return ((value_in > 255) ? 255 : ((value_in < 0) ? 0 : static_cast<unsigned char> (value_in))); };
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: return values needs to be Stream_Module_Device_DirectShow_Tools::deleteMediaType()d !
  struct _AMMediaType& getFormat_impl (const struct _AMMediaType*);
  struct _AMMediaType& getFormat_impl (const IMFMediaType*);
#endif

  uint8_t*               buffer_;
  struct AVCodec*        codec_;
  struct AVCodecContext* codecContext_;
  struct AVFrame*        frame_;
//  cairo_t*                   cairoContext_;
//  cairo_surface_t*           cairoSurface_;
  ACE_SYNCH_MUTEX_T*     lock_;
  GdkPixbuf*             pixelBuffer_;

  bool                   isFirst_;
};

// include template definition
#include "stream_vis_gtk_cairo.inl"

#endif
