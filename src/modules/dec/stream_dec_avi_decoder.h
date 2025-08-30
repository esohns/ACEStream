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

#ifndef STREAM_DEC_AVI_DECODER_H
#define STREAM_DEC_AVI_DECODER_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_task_base_asynch.h"

#include "stream_dec_avi_parser_driver.h"

extern const char libacestream_default_dec_avi_decoder_module_name_string[];

// forward declaration(s)
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
          typename SessionDataContainerType>
class Stream_Decoder_AVIDecoder_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Stream_Decoder_AVIParserDriver
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData> inherited;
  typedef Stream_Decoder_AVIParserDriver inherited2;

 public:
  Stream_Decoder_AVIDecoder_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_AVIDecoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIDecoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIDecoder_T (const Stream_Decoder_AVIDecoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIDecoder_T& operator= (const Stream_Decoder_AVIDecoder_T&))

  virtual void header (const Stream_Decoder_RIFFChunks_t&); // header data
  virtual bool frame (const struct RIFF_chunk_meta&); // frame chunk
  virtual bool betweenFrameChunk (const struct RIFF_chunk_meta&); // in-between frames chunk
  virtual ACE_UINT64 fileSize () const { return fileSize_; }

  ACE_Message_Block* buffer_; // <-- continuation chain
  ACE_UINT64         fileSize_;
  bool               headerParsed_;
};

// include template definition
#include "stream_dec_avi_decoder.inl"

#endif
