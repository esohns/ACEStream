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

#ifndef STREAM_DEC_SOX_RESAMPLER_H
#define STREAM_DEC_SOX_RESAMPLER_H

#include "sox.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_dec_sox_resampler_module_name_string[];

// forward declarations
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
          typename SessionDataType,
          ////////////////////////////////
          typename MediaType>
class Stream_Decoder_SoXResampler_T
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
  Stream_Decoder_SoXResampler_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_SoXResampler_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_SoXResampler_T (const Stream_Decoder_SoXResampler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_SoXResampler_T& operator= (const Stream_Decoder_SoXResampler_T&))

#if defined(ACE_WIN32) || defined(ACE_WIN64)
  // helper methods
  void extractBuffer (sox_format_t*); // output buffer
#endif // ACE_WIN32 || ACE_WIN64

  ACE_Message_Block*          buffer_;
  struct sox_effects_chain_t* chain_;
  struct sox_encodinginfo_t   encodingInfo_;
  struct sox_encodinginfo_t   encodingInfoOut_;
  struct sox_effect_t*        input_;
  struct sox_effect_t*        output_;
  struct sox_signalinfo_t     signalInfo_;
  struct sox_signalinfo_t     signalInfoOut_;
};

// include template definition
#include "stream_dec_sox_resampler.inl"

#endif
