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

#ifndef STREAM_VIS_CURSES_WINDOW_H
#define STREAM_VIS_CURSES_WINDOW_H

#if defined (ACE_WIN32) || defined (ACE_WIN32)
#undef MOUSE_MOVED
#include "curses.h"
#else
#include "ncurses.h"
// *NOTE*: the ncurses "timeout" macros conflicts with
//         ACE_Synch_Options::timeout. Since not currently used, it's safe to
//         undefine
#undef timeout
#endif // ACE_WIN32 || ACE_WIN32
#include "panel.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_ui_windowtype_converter.h"
#include "common_ui_ifullscreen.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_vis_curses_window_module_name_string[];

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
class Stream_Module_Vis_Curses_Window_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_UI_WindowTypeConverter_T<void>
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
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
  typedef Common_UI_WindowTypeConverter_T<void> inherited3;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Vis_Curses_Window_T (ISTREAM_T*); // stream handle
#else
  Stream_Module_Vis_Curses_Window_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Module_Vis_Curses_Window_T ();

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
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_Curses_Window_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_Curses_Window_T (const Stream_Module_Vis_Curses_Window_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_Curses_Window_T& operator= (const Stream_Module_Vis_Curses_Window_T&))

  void classifyPixelGrey (float, float, float, // r, g, b - normalized
                          chtype&,             // return value: character symbol
                          ACE_INT32&,          // return value: foreground color
                          ACE_INT32&);         // return value: background color
  void classifyPixelHSV (float, float, float, // r, g, b - normalized
                         chtype&,             // return value: character symbol
                         ACE_INT32&,          // return value: foreground color
                         ACE_INT32&);         // return value: background color
  void classifyPixelOLC (float, float, float,  // r, g, b - normalized
                         chtype&,              // return value: character symbol
                         ACE_INT32&,           // return value: foreground color
                         ACE_INT32&);          // return value: background color

  bool closeWindow_;
};

// include template definition
#include "stream_vis_curses_window.inl"

#endif
