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

#ifndef STREAM_DEC_AVI_ENCODER_H
#define STREAM_DEC_AVI_ENCODER_H

#include <string>

#include "ace/Global_Macros.h"

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "stream_dec_common.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType>
class Stream_Decoder_AVIEncoder_ReaderTask_T
 : public ACE_Thru_Task<TaskSynchType,
                        TimePolicyType>
{
 public:
  Stream_Decoder_AVIEncoder_ReaderTask_T ();
  virtual ~Stream_Decoder_AVIEncoder_ReaderTask_T ();

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  typedef ACE_Thru_Task<TaskSynchType,
                        TimePolicyType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_ReaderTask_T (const Stream_Decoder_AVIEncoder_ReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_ReaderTask_T& operator= (const Stream_Decoder_AVIEncoder_ReaderTask_T&))

  // helper function(s)
  bool postProcessHeader (const std::string&); // file name
};

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
class Stream_Decoder_AVIEncoder_WriterTask_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ConfigurationType>
{
 public:
  Stream_Decoder_AVIEncoder_WriterTask_T ();
  virtual ~Stream_Decoder_AVIEncoder_WriterTask_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);
  virtual const ConfigurationType& get () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  ConfigurationType* configuration_;
  SessionDataType*   sessionData_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))

  // helper methods
  MessageType* allocateMessage (unsigned int); // requested size

  // *NOTE*: the RIFF-AVI (storage) format (like many others) foresees a
  //         header that contains size fields with information about
  //         the length of the consecutively linear, structured bulk data.
  //         Note that in a (streaming) scenario generating data this
  //         information often is not available prior to the event, and may have
  //         to be filled in after the stream ends (i.e. in a post-processing
  //         step potentially requiring reparsing of the written data).
  //         Here, this means that, unless preconfiguration data (duration,
  //         format) is supplied initially, either through session data/and or
  //         module configuration, the encoding process must be split into two
  //         separate phases (or modules, probably more adequate for modular,
  //         pipelined processing), to comply with the standard. This
  //         implementation contains the post-processing step by reacting to the
  //         completion event message sent upstream by final module(s) of the
  //         processing stream
  bool               isFirst_;
  bool               isInitialized_;
};

// include template implementation
#include "stream_dec_avi_encoder.inl"

#endif
