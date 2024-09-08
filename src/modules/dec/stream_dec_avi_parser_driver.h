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

#ifndef STREAM_DECODER_AVI_PARSER_DRIVER_H
#define STREAM_DECODER_AVI_PARSER_DRIVER_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "common_parser_defines.h"

#include "stream_common.h"
#include "stream_imessage.h"

#include "stream_dec_defines.h"
#include "stream_dec_riff_common.h"

// forward declaration(s)
class ACE_Message_Queue_Base;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct AVI_LTYPE;

class Stream_Decoder_IAVIParser
{
 public:
  // *NOTE*: to be invoked by the parser (ONLY !)
  virtual bool frame (const struct RIFF_chunk_meta&) = 0; // frame chunk
  virtual bool betweenFrameChunk (const struct RIFF_chunk_meta&) = 0; // in-between frames chunk
};

class Stream_Decoder_AVIParserDriver
 : public Stream_Decoder_IAVIParser
{
//  friend class Stream_Decoder_RIFF_Scanner;

 public:
  Stream_Decoder_AVIParserDriver (bool,  // debug scanning ?
                                  bool); // debug parsing ?
  virtual ~Stream_Decoder_AVIParserDriver ();

  // target data, needs to be set before invoking parse() !
  void initialize (ACE_UINT32&,                                           // target data (bitmap frame size)
                   bool,                                                  // parse header only ? : parse the whole (file) stream
                   bool = COMMON_PARSER_DEFAULT_LEX_TRACE,                // debug scanner ?
                   bool = COMMON_PARSER_DEFAULT_YACC_TRACE,               // debug parser ?
                   ACE_Message_Queue_Base* = NULL,                        // data buffer queue (yywrap)
                   bool = COMMON_PARSER_DEFAULT_FLEX_USE_YY_SCAN_BUFFER); // yy_scan_buffer() ? : yy_scan_bytes()

  bool parse (ACE_Message_Block*); // data

  // error handling
  //void error (const yy::location&, // location
  //            const std::string&); // message
  void error (const std::string&); // message
  void error (const AVI_LTYPE&,    // location
              const std::string&); // message

  // *NOTE*: to be invoked by the scanner (ONLY !)
  bool switchBuffer (ACE_Message_Block* = NULL); // fragment to switch to directly : fragment_->cont ()
  bool getDebugScanner () const;
  void wait ();

  // *NOTE*: current (unscanned) data fragment
  Stream_Decoder_RIFFChunks_t chunks_;
  bool                        inFrames_;
  bool                        finished_; // done ?
  ACE_Message_Block*          fragment_;
  unsigned int                fragmentCount_;
  ACE_UINT64                  fragmentOffset_; // parsed fragment bytes
  ACE_UINT64                  offset_; // parsed (file/stream) bytes
  bool                        parseHeaderOnly_;

  // target
  bool                        isVids_; // currently in 'vids' strh
  ACE_UINT32*                 frameSize_; // bitmap-

 protected:
  yyscan_t                    scannerState_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIParserDriver (const Stream_Decoder_AVIParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIParserDriver& operator= (const Stream_Decoder_AVIParserDriver&))

  // convenient typedefs
  typedef Stream_IMessage_T<enum Stream_SessionMessageType> ISESSIONMESSAGE_T;

  // helper methods
  bool scan_begin ();
  void scan_end ();

  // scanner
  YY_BUFFER_STATE             bufferState_;
  ACE_Message_Queue_Base*     messageQueue_;
  bool                        useYYScanBuffer_;

  bool                        initialized_;
};

#endif
