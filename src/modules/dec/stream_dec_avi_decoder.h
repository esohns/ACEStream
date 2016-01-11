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

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "stream_dec_common.h"
#include "stream_dec_avi_parser_driver.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
class Stream_Decoder_AVIDecoder_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ConfigurationType>
{
 public:
  Stream_Decoder_AVIDecoder_T ();
  virtual ~Stream_Decoder_AVIDecoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);
  virtual const ConfigurationType& get () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  Stream_Decoder_AVIParserDriver driver_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIDecoder_T (const Stream_Decoder_AVIDecoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIDecoder_T& operator= (const Stream_Decoder_AVIDecoder_T&))

  // helper methods
  MessageType* allocateMessage (unsigned int); // requested size

  Stream_IAllocator*             allocator_;
  ACE_Message_Block*             buffer_; // <-- continuation chain
  bool                           crunchMessages_;
  bool                           isInitialized_;
  SessionDataType*               sessionData_;

  // driver
  bool                           debugScanner_;
  bool                           debugParser_;
  bool                           isDriverInitialized_;
};

// include template implementation
#include "stream_dec_avi_decoder.inl"

#endif
