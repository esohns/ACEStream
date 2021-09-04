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

#ifndef TEST_U_IMAGESCREEN_STREAM_H
#define TEST_U_IMAGESCREEN_STREAM_H

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
//#include <minwindef.h>
//#else
//#include <windef.h>
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
//#include <winnt.h>
//#include <guiddef.h>
//#include <mfidl.h>
//#include <mfobjects.h>
//#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_imagescreen_common.h"
#include "test_u_imagescreen_common_modules.h"
#include "test_u_imagescreen_message.h"
#include "test_u_imagescreen_session_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

class Stream_ImageScreen_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_ImageScreen_StreamState,
                        struct Stream_ImageScreen_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Stream_ImageScreen_ModuleHandlerConfiguration,
                        Stream_ImageScreen_SessionData,
                        Stream_ImageScreen_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_ImageScreen_Message_t,
                        Stream_ImageScreen_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_ImageScreen_StreamState,
                        struct Stream_ImageScreen_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Stream_ImageScreen_ModuleHandlerConfiguration,
                        Stream_ImageScreen_SessionData,
                        Stream_ImageScreen_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_ImageScreen_Message_t,
                        Stream_ImageScreen_SessionMessage_t> inherited;

 public:
  Stream_ImageScreen_Stream ();
  virtual ~Stream_ImageScreen_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_Stream (const Stream_ImageScreen_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_Stream& operator= (const Stream_ImageScreen_Stream&))

  // modules
  Stream_ImageScreen_FFMPEG_Source_Module       ffmpeg_source_;
#if defined (FFMPEG_SUPPORT)
  Stream_ImageScreen_FFMPEG_Decode_Module       ffmpeg_decode_;
  Stream_ImageScreen_FFMPEG_Resize_Module       ffmpeg_resize_; // --> window size/fullscreen
  Stream_ImageScreen_FFMPEG_Convert_Module      ffmpeg_convert_; // RGB32 --> BGR32
#endif // FFMPEG_SUPPORT
#if defined (IMAGEMAGICK_SUPPORT)
  Stream_ImageScreen_ImageMagick_Source_Module  imagemagick_source_;
  Stream_ImageScreen_ImageMagick_Resize_Module  imagemagick_resize_; // --> window size/fullscreen
  //Stream_ImageScreen_ImageMagick_Convert_Module imagemagick_convert_; // RGB32 --> BGR32
#endif // FFMPEG_SUPPORT || IMAGEMAGICK_SUPPORT
  Stream_ImageScreen_Delay_Module   delay_;
  Stream_ImageScreen_Display_Module display_;
};

#endif
