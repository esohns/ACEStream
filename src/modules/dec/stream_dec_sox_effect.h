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

#ifndef STREAM_DEC_SOX_EFFECT_H
#define STREAM_DEC_SOX_EFFECT_H

#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "sox.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "stream_dev_common.h"

extern const char libacestream_default_dec_sox_effect_module_name_string[];

// forward declarations
struct v4l2_format;
class ACE_Message_Block;
class Stream_IAllocator;

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
          typename SessionDataType>
class Stream_Decoder_SoXEffect_T
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
  Stream_Decoder_SoXEffect_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_SoXEffect_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // helper methods
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  // *IMPORTANT NOTE*: callers must Stream_MediaFramework_DirectShow_Tools::free_() the return value
//  template <typename MediaType2> AM_MEDIA_TYPE& getMediaType (const MediaType2 mediaType_in) { return getMediaType_impl (mediaType_in); }
#else
  template <typename MediaType> struct Stream_MediaFramework_ALSA_MediaType& getMediaType (const MediaType& mediaType_in) { return getMediaType_impl (mediaType_in); }
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_SoXEffect_T (const Stream_Decoder_SoXEffect_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_SoXEffect_T& operator= (const Stream_Decoder_SoXEffect_T&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  AM_MEDIA_TYPE& getMediaType_impl (const struct _AMMediaType*&);
//  AM_MEDIA_TYPE& getMediaType_impl (const IMFMediaType*);
#else
  inline struct Stream_MediaFramework_ALSA_MediaType& getMediaType_impl (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in) { return const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in); }
//  inline struct Stream_MediaFramework_ALSA_MediaType& getMediaType_impl (const struct v4l2_format& format_in) { return const_cast<struct v4l2_format*> (format_in); } // return value: media type handle
#endif // ACE_WIN32 || ACE_WIN64

  ACE_Message_Block*          buffer_;
  struct sox_effects_chain_t* chain_;
  struct sox_encodinginfo_t   encodingInfo_;
  struct sox_effect_t*        input_;
  bool                        manageSoX_;
  struct sox_effect_t*        output_;
  struct sox_signalinfo_t     signalInfo_;
};

// include template definition
#include "stream_dec_sox_effect.inl"

#endif
