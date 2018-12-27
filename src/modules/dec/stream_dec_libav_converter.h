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

#ifndef STREAM_DECODER_LIBAV_CONVERTER_T_H
#define STREAM_DECODER_LIBAV_CONVERTER_T_H

#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "common_ui_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_lib_mediatype_converter.h"

// forward declaration(s)
struct AVCodecContext;
struct AVFrame;
struct SwsContext;
class Stream_IAllocator;

extern const char libacestream_default_dec_libav_converter_module_name_string[];

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
          typename MediaType> // session data-
class Stream_Decoder_LibAVConverter_T
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
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                    >
#else
                                                     ,typename SessionDataContainerType::DATA_T>
#endif // ACE_WIN32 || ACE_WIN64
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
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                    > inherited2;
#else
                                                     ,typename SessionDataContainerType::DATA_T> inherited2;
#endif // ACE_WIN32 || ACE_WIN64

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Decoder_LibAVConverter_T (ISTREAM_T*); // stream handle
#else
  Stream_Decoder_LibAVConverter_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Decoder_LibAVConverter_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // helper methods
  inline void setFormat (enum AVPixelFormat format_in, MediaType& mediaType_inout) { setFormat_impl (format_in, mediaType_inout); }
  inline void setFormat_impl (enum AVPixelFormat format_in, enum AVPixelFormat& mediaType_inout) { mediaType_inout = format_in; }
  inline void setResolution (const Common_UI_Resolution_t& resolution_in, MediaType& mediaType_inout) { setResolution_impl (resolution_in, mediaType_inout); }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  void setFormat_impl (enum AVPixelFormat, struct _AMMediaType*&);
  void setFormat_impl (enum AVPixelFormat, IMFMediaType*&);
#else
//  inline void setFormat_impl (enum AVPixelFormat format_in, struct v4l2_pix_format& mediaType_inout) { mediaType_inout.pixelformat = Stream_Device_Tools::ffmpegFormatToV4L2Format (format_in); }
  inline void setFormat_impl (enum AVPixelFormat format_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_inout) { mediaType_inout.format.pixelformat = Stream_Device_Tools::ffmpegFormatToV4L2Format (format_in); }
#endif // ACE_WIN32 || ACE_WIN64

  DataMessageType*   buffer_;
  struct SwsContext* context_;
  struct AVFrame*    frame_;
  unsigned int       frameSize_; // output-
  enum AVPixelFormat inputFormat_;
  enum AVPixelFormat outputFormat_;

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  static char        paddingBuffer[AV_INPUT_BUFFER_PADDING_SIZE];
//#else
//  static char        paddingBuffer[FF_INPUT_BUFFER_PADDING_SIZE];
//#endif

 private:
  // convenient types
  typedef Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataContainerType,
                                          MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter_T (const Stream_Decoder_LibAVConverter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter_T& operator= (const Stream_Decoder_LibAVConverter_T&))
};

// include template definition
#include "stream_dec_libav_converter.inl"

#endif
