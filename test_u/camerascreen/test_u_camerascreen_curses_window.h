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

#ifndef TEST_U_CAMERASCREEN_CURSES_WINDOW_H
#define TEST_U_CAMERASCREEN_CURSES_WINDOW_H

#include "common_time_common.h"

#include "stream_control_message.h"
#include "stream_streammodule_base.h"

#include "stream_vis_curses_window.h"

#include "test_u_camerascreen_common.h"
#include "test_u_camerascreen_message.h"
#include "test_u_camerascreen_session_message.h"

class Test_U_CameraScreen_Curses_Window
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 : public Stream_Module_Vis_Curses_Window_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Stream_CameraScreen_DirectShow_Message_t,
                                            Stream_CameraScreen_DirectShow_SessionMessage_t,
                                            Stream_CameraScreen_DirectShow_SessionData_t,
                                            struct _AMMediaType>
#else
#endif // ACE_WIN32 || ACE_WIN64
{
#if defined(ACE_WIN32) || defined(ACE_WIN64)
 typedef Stream_Module_Vis_Curses_Window_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Stream_CameraScreen_DirectShow_Message_t,
                                           Stream_CameraScreen_DirectShow_SessionMessage_t,
                                           Stream_CameraScreen_DirectShow_SessionData_t,
                                           struct _AMMediaType> inherited;
#else
#endif // ACE_WIN32 || ACE_WIN64

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_CameraScreen_Curses_Window (ISTREAM_T*); // stream handle
#else
  Test_U_CameraScreen_Curses_Window (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_U_CameraScreen_Curses_Window () {}

  // implement (part of) Stream_ITaskBase_T
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void handleDataMessage (Stream_CameraScreen_DirectShow_Message_t*&, // data message handle
                                  bool&);                                     // return value: pass message downstream ?
#else
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraScreen_Curses_Window ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraScreen_Curses_Window (const Test_U_CameraScreen_Curses_Window&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraScreen_Curses_Window& operator= (const Test_U_CameraScreen_Curses_Window&))

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
};

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_curses_window_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_CameraScreen_Curses_Window);                               // writer type

#endif
