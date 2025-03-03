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

#ifndef STREAM_DEC_DEEPSPEECH_DECODER_T_H
#define STREAM_DEC_DEEPSPEECH_DECODER_T_H

#include "deepspeech.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dec_common.h"

// forward declaration(s)
class Stream_IAllocator;

extern const char libacestream_default_dec_deepspeech_decoder_module_name_string[];

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
class Stream_Decoder_DeepSpeechDecoder_T
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

 public:
  Stream_Decoder_DeepSpeechDecoder_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_DeepSpeechDecoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_DeepSpeechDecoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_DeepSpeechDecoder_T (const Stream_Decoder_DeepSpeechDecoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_DeepSpeechDecoder_T& operator= (const Stream_Decoder_DeepSpeechDecoder_T&))

  // helper methods
  unsigned int processWords (const char*,                   // input string
                             Stream_Decoder_STT_Result_t&); // return value: result

  unsigned int           bufferedMs_;
  struct ModelState*     context_;
  struct StreamingState* context2_;
  unsigned int           decodedWords_;
  unsigned int           sampleSize_; // mono-
};

// include template definition
#include "stream_dec_deepspeech_decoder.inl"

#endif
