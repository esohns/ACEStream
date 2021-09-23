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

#ifndef STREAM_DEC_WAV_ENCODER_H
#define STREAM_DEC_WAV_ENCODER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//#include <sndfile.h>
#if defined (SOX_SUPPORT)
#include "sox.h"
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_avi_encoder.h"

extern const char libacestream_default_dec_wav_encoder_module_name_string[];

// *NOTE*: the WAV format is a (non standard-compliant) RIFF container
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
          typename MediaType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Decoder_WAVEncoder_T
 : public Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                                 TimePolicyType,
                                                 ConfigurationType,
                                                 ControlMessageType,
                                                 DataMessageType,
                                                 SessionMessageType,
                                                 SessionDataContainerType,
                                                 SessionDataType,
                                                 MediaType,
                                                 UserDataType>
{
  typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                                 TimePolicyType,
                                                 ConfigurationType,
                                                 ControlMessageType,
                                                 DataMessageType,
                                                 SessionMessageType,
                                                 SessionDataContainerType,
                                                 SessionDataType,
                                                 MediaType,
                                                 UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Decoder_WAVEncoder_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Decoder_WAVEncoder_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Decoder_WAVEncoder_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T (const Stream_Decoder_WAVEncoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T& operator= (const Stream_Decoder_WAVEncoder_T&))

  // helper methods
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle
#endif // ACE_WIN32 || ACE_WIN64

  // *NOTE*: the WAV (storage) format (like many others) foresees a
  //         header that contains size fields with information about
  //         the length of the consecutive, linearly structured bulk data.
  //         Note how in a (streaming) scenario continuously generating data,
  //         this information often is not available during initial processing
  //         and therefore has to be filled in in a post-processing step after
  //         the stream ends, potentially requiring reparsing of (written) data.
  //         This means that, unless configuration data (duration[, format]) is
  //         supplied externally - either through session data/and or module
  //         configuration, the encoding process has to be split into two
  //         separate phases (or distinct processing modules - more adequate for
  //         pipelined processing), in order to generate ('standard-')compliant
  //         files. This implementation fills in the size information upon
  //         reception of completion event messages sent upstream by trailing
  //         modules of the processing stream (i.e. reader-side processing)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (SOX_SUPPORT)
  struct sox_encodinginfo_t encodingInfo_;
  struct sox_signalinfo_t   signalInfo_;

  struct sox_format_t*      outputFile_;
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
};

// include template definition
#include "stream_dec_wav_encoder.inl"

#endif
