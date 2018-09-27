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

#ifndef STREAM_MODULE_VIS_TARGET_DIRECTSHOW_H
#define STREAM_MODULE_VIS_TARGET_DIRECTSHOW_H

#include <evr.h>
#include <strmif.h>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include <minwindef.h>
#else
#include <windef.h>
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "common_ui_ifullscreen.h"

#include "stream_lib_directshow_target.h"

#include "stream_vis_common.h"

// forward declarations
struct IVideoWindow;
class Stream_IAllocator;

extern const char libacestream_default_vis_directshow_module_name_string[];

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
          typename SessionDataType,
          ////////////////////////////////
          typename FilterConfigurationType, // DirectShow-
          typename PinConfigurationType,    // DirectShow Filter (Output) Pin-
          typename FilterType>              // DirectShow-
class Stream_Vis_Target_DirectShow_T
 : public Stream_MediaFramework_DirectShow_Target_T<ACE_SYNCH_USE,
                                                    TimePolicyType,
                                                    ConfigurationType,
                                                    ControlMessageType,
                                                    DataMessageType,
                                                    SessionMessageType,
                                                    SessionDataType,
                                                    FilterConfigurationType,
                                                    PinConfigurationType,
                                                    struct _AMMediaType,
                                                    FilterType>
 , public Common_UI_IFullscreen
{
  typedef Stream_MediaFramework_DirectShow_Target_T<ACE_SYNCH_USE,
                                                    TimePolicyType,
                                                    ConfigurationType,
                                                    ControlMessageType,
                                                    DataMessageType,
                                                    SessionMessageType,
                                                    SessionDataType,
                                                    FilterConfigurationType,
                                                    PinConfigurationType,
                                                    struct _AMMediaType,
                                                    FilterType> inherited;

 public:
  Stream_Vis_Target_DirectShow_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_DirectShow_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);
  using FilterType::initialize;

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_UI_IFullscreen
  virtual void toggle ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_DirectShow_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_DirectShow_T (const Stream_Vis_Target_DirectShow_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_DirectShow_T& operator= (const Stream_Vis_Target_DirectShow_T&))

  // helper methods
  bool initialize_DirectShow (IGraphBuilder*,             // graph handle
                              const struct _AMMediaType&, // media type
                              HWND&,                      // in/out (target) window handle
                              bool,                       // fullscreen ?
                              struct tagRECT&,            // in/out (target) window area
                              IVideoWindow*&,             // return value: window control handle
                              IMFVideoDisplayControl*&);  // return value: window control handle (EVR)

  bool                    closeWindow_;
  IMFVideoDisplayControl* IMFVideoDisplayControl_;
  IVideoWindow*           IVideoWindow_;
  HWND                    window_;
};

// include template definition
#include "stream_vis_target_directshow.inl"

#endif
