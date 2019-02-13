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

#ifndef STREAM_VIS_X11_WINDOW_H
#define STREAM_VIS_X11_WINDOW_H

#include "X11/Xlib.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_ilock.h"

#include "common_ui_ifullscreen.h"

#include "stream_task_base_synch.h"

#include "stream_dev_tools.h"

int libacestream_vis_x11_error_handler_cb (Display*, XErrorEvent*);
int libacestream_vis_x11_io_error_handler_cb (Display*);

extern const char libacestream_default_vis_x11_window_module_name_string[];

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
          typename MediaType>
class Stream_Module_Vis_X11_Window_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Common_UI_IFullscreen
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Stream_Module_Vis_X11_Window_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Vis_X11_Window_T ();

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
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_X11_Window_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_X11_Window_T (const Stream_Module_Vis_X11_Window_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_X11_Window_T& operator= (const Stream_Module_Vis_X11_Window_T&))

  // helper methods
  inline enum AVPixelFormat getFormat (const MediaType& mediaType_in) { return getFormat_impl (mediaType_in); }
  inline Common_Image_Resolution_t getResolution (const MediaType& mediaType_in) { return getResolution_impl (mediaType_in); }
  inline enum AVPixelFormat getFormat_impl (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in) { return Stream_Device_Tools::v4l2FormatToffmpegFormat (mediaType_in.format.pixelformat); }
  inline Common_Image_Resolution_t getResolution_impl (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in) { Common_Image_Resolution_t return_value; return_value.width = mediaType_in.format.width; return_value.height = mediaType_in.format.height; return return_value; }

//  uint8_t*           buffer_;
  bool               closeDisplay_;
  bool               closeWindow_;
//  GC                 context_;
  struct _XDisplay*  display_;
  bool               isFirst_;
  Pixmap             pixmap_;
  Window             window_;
};

// include template definition
#include "stream_vis_x11_window.inl"

#endif
