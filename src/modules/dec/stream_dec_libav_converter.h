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

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

// forward declaration(s)
struct AVCodecContext;
struct AVFrame;
struct SwsContext;
class Stream_IAllocator;

extern const char libacestream_default_dec_libav_converter_module_name_string[];

template <typename TaskType,  // Stream_TaskBaseSynch_T || Stream_TaskBaseAsynch_T
          ////////////////////////////////
          typename MediaType> // session data-
class Stream_Decoder_LibAVConverter_T
 : public TaskType
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef TaskType inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Decoder_LibAVConverter_T (typename TaskType::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_LibAVConverter_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const typename TaskType::CONFIGURATION_T&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (typename TaskType::DATA_MESSAGE_T*&, // data message handle
                                  bool&);                              // return value: pass message downstream ?
  virtual void handleSessionMessage (typename TaskType::SESSION_MESSAGE_T*&, // session message handle
                                     bool&);                                 // return value: pass message downstream ?

 protected:
  typename TaskType::DATA_MESSAGE_T* buffer_;
  struct SwsContext*                 context_;
  struct AVFrame*                    frame_;
  unsigned int                       frameSize_; // output-
  enum AVPixelFormat                 inputFormat_;

 private:
  // convenient types
  typedef Stream_Decoder_LibAVConverter_T<TaskType,
                                          MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter_T (const Stream_Decoder_LibAVConverter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter_T& operator= (const Stream_Decoder_LibAVConverter_T&))
};

//////////////////////////////////////////

template <typename TaskType,  // Stream_TaskBaseSynch_T || Stream_TaskBaseAsynch_T,
          ////////////////////////////////
          typename MediaType> // session data-
class Stream_Decoder_LibAVConverter1_T
 : public TaskType
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef TaskType inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Decoder_LibAVConverter1_T (typename TaskType::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Decoder_LibAVConverter1_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const typename TaskType::CONFIGURATION_T&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (typename TaskType::DATA_MESSAGE_T*&, // data message handle
                                  bool&);                              // return value: pass message downstream ?
  virtual void handleSessionMessage (typename TaskType::SESSION_MESSAGE_T*&, // session message handle
                                     bool&);                                 // return value: pass message downstream ?

 private:
  // convenient types
  typedef Stream_Decoder_LibAVConverter1_T<TaskType,
                                           MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter1_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter1_T (const Stream_Decoder_LibAVConverter1_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVConverter1_T& operator= (const Stream_Decoder_LibAVConverter1_T&))
};

// include template definition
#include "stream_dec_libav_converter.inl"

#endif
