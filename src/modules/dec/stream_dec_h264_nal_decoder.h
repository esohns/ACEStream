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

#ifndef STREAM_DEC_H264_NAL_DECODER_T_H
#define STREAM_DEC_H264_NAL_DECODER_T_H

#include "ace/Global_Macros.h"

#include "common_iscanner.h"
#include "common_time_common.h"

#include "stream_task_base_asynch.h"

#include "stream_dec_h264_nal_bisector.h"

// forward declaration(s)
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
          typename SessionDataContainerType>
class Stream_Decoder_H264_NAL_Decoder_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Common_IScannerBase
{
 public:
  Stream_Decoder_H264_NAL_Decoder_T ();
  virtual ~Stream_Decoder_H264_NAL_Decoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IScannerBase
  inline virtual ACE_Message_Block* buffer () { return buffer_; };
//  inline virtual bool debugScanner () const { return Stream_Decoder_H264_NAL_Bisector_get_debug (scannerState_); };
  inline virtual bool isBlocking () const { return true; };
  inline virtual void offset (unsigned int offset_in) { Stream_Decoder_H264_NAL_Bisector_set_column (offset_in, scannerState_); };
  inline virtual unsigned int offset () const { return Stream_Decoder_H264_NAL_Bisector_get_column (scannerState_); };
  // *IMPORTANT NOTE*: when the parser detects a frame end, it inserts a new
  //                   buffer to the continuation and passes 'true'
  //                   --> separate the current frame from the next
  virtual bool switchBuffer (bool = false); // unlink current buffer ?
  virtual void waitBuffer ();
  virtual void error (const std::string&);

 private:
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_H264_NAL_Decoder_T (const Stream_Decoder_H264_NAL_Decoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_H264_NAL_Decoder_T& operator= (const Stream_Decoder_H264_NAL_Decoder_T&))

  // implement Common_IScannerBase
  virtual bool begin (const char*,   // buffer handle
                      unsigned int); // buffer size
  virtual void end ();

  ACE_Message_Block*      buffer_;
  bool                    isFirst_;
  yyscan_t                scannerState_;
  //std::string             scannerTables_;
  struct yy_buffer_state* bufferState_;
  bool                    useYYScanBuffer_;
};

// include template definition
#include "stream_dec_h264_nal_decoder.inl"

#endif
