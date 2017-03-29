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

#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>

#include "stream_common.h"
#include "stream_imessage.h"

#include "stream_dec_common.h"
#include "stream_dec_defines.h"
#include "stream_dec_exports.h"
//#include "stream_dec_riff_scanner.h"

// forward declaration(s)
class ACE_Message_Queue_Base;
//class Stream_Dec_RIFF_Scanner;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
struct YYLTYPE;

class Stream_Dec_Export Stream_Decoder_AVIParserDriver
{
//  friend class Stream_Decoder_RIFF_Scanner;

 public:
  Stream_Decoder_AVIParserDriver (bool,  // debug scanning ?
                                  bool); // debug parsing ?
  virtual ~Stream_Decoder_AVIParserDriver ();

  // target data, needs to be set before invoking parse() !
  void initialize (unsigned int&,                                          // target data (frame size)
                   bool,                                                   // parse header only ? : parse the whole (file) stream
                   bool,                                                   // extract frames ? (see below)
                   bool = STREAM_DECODER_DEFAULT_LEX_TRACE,                // debug scanner ?
                   bool = STREAM_DECODER_DEFAULT_YACC_TRACE,               // debug parser ?
                   ACE_Message_Queue_Base* = NULL,                         // data buffer queue (yywrap)
                   bool = STREAM_DECODER_DEFAULT_FLEX_USE_YY_SCAN_BUFFER); // yy_scan_buffer() ? : yy_scan_bytes()

  bool parse (ACE_Message_Block*); // data

  // error handling
  //void error (const yy::location&, // location
  //            const std::string&); // message
  void error (const std::string&); // message
  void error (const YYLTYPE&,      // location
              const std::string&); // message

  // *NOTE*: to be invoked by the scanner (ONLY !)
  bool switchBuffer ();
  bool getDebugScanner () const;
  void wait ();

  // *NOTE*: current (unscanned) data fragment
  Stream_Decoder_RIFFChunks_t chunks_;
  // *NOTE*: the scanner automatically inserts buffers that 'point' to the
  //         (chunk) data. This setting additionally 'discards' all (chunk) meta
  //         data
  bool                        extractFrames_;
  bool                        finished_; // done ?
  ACE_Message_Block*          fragment_;
  unsigned int                fragmentCount_;
  unsigned int                fragmentOffset_; // parsed fragment bytes
  unsigned int                offset_; // parsed (file/stream) bytes
  bool                        parseHeaderOnly_;

  // target
  unsigned int*               frameSize_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIParserDriver ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIParserDriver (const Stream_Decoder_AVIParserDriver&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIParserDriver& operator= (const Stream_Decoder_AVIParserDriver&))

  // convenient typedefs
  // *TODO*: to be templatized
  typedef Stream_IDataMessage_T<Stream_MessageType,
                                int> IMESSAGE_T;
  typedef Stream_IMessage_T<Stream_SessionMessageType> ISESSIONMESSAGE_T;

  // helper methods
  bool scan_begin ();
  void scan_end ();

  // context
  //bool                        trace_;

  // scanner
  yyscan_t                    scannerState_;
  YY_BUFFER_STATE             bufferState_;
  ACE_Message_Queue_Base*     messageQueue_;
  bool                        useYYScanBuffer_;

  bool                        initialized_;
};

#endif
