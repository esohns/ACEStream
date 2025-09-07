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

#ifndef STREAM_MODULE_WINDOW_SOURCE_H
#define STREAM_MODULE_WINDOW_SOURCE_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "common_ui_windowtype_converter.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"

// forward declarations
class ACE_Message_Queue_Base;

extern const char libacestream_default_misc_window_source_module_name_string[];

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          ////////////////////////////////
          typename MediaType>
class Stream_Module_Window_Source_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public Common_UI_WindowTypeConverter_T<HWND>
#else
#if defined (GTK_SUPPORT)
#if GTK_CHECK_VERSION (4,0,0)
 , public Common_UI_WindowTypeConverter_T<GdkSurface*>
#else
 , public Common_UI_WindowTypeConverter_T<GdkWindow*>
#endif // GTK_CHECK_VERSION (4,0,0)
#else
 , public Common_UI_WindowTypeConverter_T<Window>
#endif // GTK_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
 , public Common_ITimerHandler
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef Common_UI_WindowTypeConverter_T<HWND> inherited3;
#else
#if defined (GTK_SUPPORT)
#if GTK_CHECK_VERSION (4,0,0)
  typedef Common_UI_WindowTypeConverter_T<GdkSurface*> inherited3;
#else
  typedef Common_UI_WindowTypeConverter_T<GdkWindow*> inherited3;
#endif // GTK_CHECK_VERSION (4,0,0)
#else
  typedef Common_UI_WindowTypeConverter_T<Window> inherited3;
#endif // GTK_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

 public:
   Stream_Module_Window_Source_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Window_Source_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                    Common_TimePolicy_t,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    StatisticContainerType,
                                    SessionManagerType,
                                    TimerManagerType,
                                    struct Stream_UserData>::initialize;
#endif // __GNUG__ || _MSC_VER

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ITimerHandler
  inline virtual const long get_2 () const { return handler_.get_2 (); }
  virtual void handle (const void*); // asynchronous completion token handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Window_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Window_Source_T (const Stream_Module_Window_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Window_Source_T& operator= (const Stream_Module_Window_Source_T&))

  Common_Timer_Handler      handler_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HDC                       captureContext_;
  HBITMAP                   captureBitmap_;
  HDC                       sourceContext_;
  Common_Image_Resolution_t resolution_;
  BITMAPINFO                bitmapInfo_;
#else
  unsigned int              frameSize_;
#if defined (GTK_SUPPORT)
#if GTK_CHECK_VERSION (4,0,0)
  GdkSurface*               window_;
#else
  GdkWindow*                window_;
#endif // GTK_CHECK_VERSION (4,0,0)
#else
  Window                    window_;
#endif // GTK_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
};

// include template definition
#include "stream_misc_window_source.inl"

#endif
