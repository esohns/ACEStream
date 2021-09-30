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

#include "ace/Log_Msg.h"

#include "common_image_defines.h"

#include "common_timer_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_mpeg_4_common.h"
#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_MPEG_4_Decoder_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                SessionDataContainerType>::Stream_Decoder_MPEG_4_Decoder_T (ISTREAM_T* stream_in)
#else
                                SessionDataContainerType>::Stream_Decoder_MPEG_4_Decoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , boxes_ ()
 , boxSize_ (0)
 , buffer_ (NULL)
 , missingBoxBytes_ (0)
 , offset_ (0)
 , PPSNalUnits_ ()
 , SPSNalUnits_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_4_Decoder_T::Stream_Decoder_MPEG_4_Decoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_MPEG_4_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::~Stream_Decoder_MPEG_4_Decoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_4_Decoder_T::~Stream_Decoder_MPEG_4_Decoder_T"));

  if (buffer_)
    buffer_->release ();

  for (NALUNITSITERATOR_T iterator = PPSNalUnits_.begin ();
       iterator != PPSNalUnits_.end ();
       ++iterator)
    delete [] (*iterator).second;
  for (NALUNITSITERATOR_T iterator = SPSNalUnits_.begin ();
       iterator != SPSNalUnits_.end ();
       ++iterator)
    delete [] (*iterator).second;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_MPEG_4_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                        Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_4_Decoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    boxes_.clear ();
    boxSize_ = 0;
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF
    missingBoxBytes_ = 0;
    offset_ = 0;
    for (NALUNITSITERATOR_T iterator = PPSNalUnits_.begin ();
         iterator != PPSNalUnits_.end ();
         ++iterator)
      delete [] (*iterator).second;
    PPSNalUnits_.clear ();
    for (NALUNITSITERATOR_T iterator = SPSNalUnits_.begin ();
         iterator != SPSNalUnits_.end ();
         ++iterator)
      delete [] (*iterator).second;
    SPSNalUnits_.clear ();
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_MPEG_4_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_4_Decoder_T::handleDataMessage"));

  int result = -1;
  struct Stream_Decoder_MPEG_4_BoxHeader* box_header_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;
  ACE_UINT64 skipped_bytes = 0;
  bool large_box_b = false;
  bool need_more_data_b = false;
  ACE_UINT64 processed_bytes_i = 0, total_bytes_to_skip_i = 0, bytes_to_skip_i = 0;

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  if (unlikely (!buffer_))
  {
    buffer_ = message_inout;
    message_block_p = buffer_;
  } // end IF
  else
  {
    message_block_p = buffer_;
    while (message_block_p->cont ())
      message_block_p = message_block_p->cont ();
    message_block_p->cont (message_inout);
    message_block_p = message_inout;
  } // end ELSE
  ACE_ASSERT (message_block_p);
  // message_block_p points at the trailing fragment

  unsigned int total_length = buffer_->total_length ();
  if ((total_length < missingBoxBytes_) ||
      (total_length < sizeof (struct Stream_Decoder_MPEG_4_BoxHeader)))
    return; // done

  // *TODO*: this step is unnecessary --> implement a proper parser
  typename DataMessageType::IDATA_MESSAGE_T* idata_message_p =
    dynamic_cast<typename DataMessageType::IDATA_MESSAGE_T*> (buffer_);
  ACE_ASSERT (idata_message_p);
  try {
    idata_message_p->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  while (buffer_ &&
         (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_BoxHeader)))
  {
    box_header_p =
      reinterpret_cast<struct Stream_Decoder_MPEG_4_BoxHeader*> (buffer_->rd_ptr ());

    // step1: retrieve box size
    boxSize_ =
      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->length)
                                             : box_header_p->length);
    if (boxSize_ == 1) // 'large' box ?
    {
      large_box_b = true;
      if (buffer_->length () < sizeof (struct Stream_Decoder_MPEG_4_LargeBoxHeader))
        break; // --> need more data
      struct Stream_Decoder_MPEG_4_LargeBoxHeader* box_header_2 =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_LargeBoxHeader*> (buffer_->rd_ptr ());
      boxSize_ =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_header_2->largesize)
                                               : box_header_2->largesize);
    } // end IF
    missingBoxBytes_ =
      ((total_length < boxSize_) ? boxSize_ - total_length : 0);

    // step2: process box data
    boxes_.push_back (std::make_pair (box_header_p->type, offset_));
    processed_bytes_i = processBox (*box_header_p,
                                    need_more_data_b);
    if (unlikely (processed_bytes_i == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_MPEG_4_Decoder_T::processBox(), aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    offset_ += processed_bytes_i;

    // step3: account for any processed data
    message_block_p = buffer_;
    total_bytes_to_skip_i = processed_bytes_i;
    while (total_bytes_to_skip_i)
    {
      bytes_to_skip_i =
        std::min (static_cast<ACE_UINT64> (message_block_p->length ()), total_bytes_to_skip_i);
      message_block_p->rd_ptr (static_cast<size_t> (bytes_to_skip_i));
      if (!message_block_p->length ())
        message_block_p = message_block_p->cont ();
      total_bytes_to_skip_i -= bytes_to_skip_i;
    } // end WHILE
    while (!buffer_->length ())
    {
      message_block_p = buffer_->cont ();
      if (!message_block_p)
      {
        buffer_->release (); buffer_ = NULL;
        break;
      } // end IF

      buffer_->cont (NULL);
      buffer_->release (); buffer_ = message_block_p;
    } // end WHILE

    skipped_bytes += processed_bytes_i;

    if (need_more_data_b)
      break;
  } // end WHILE

  return;

error:
  if (buffer_ == message_inout)
    buffer_ = NULL;
  if (message_inout)
    message_inout->release ();

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_MPEG_4_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_4_Decoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  //const SessionDataContainerType& session_data_container_r =
  //  message_inout->getR ();
//  typename SessionDataContainerType::DATA_T& session_data_r =
//    const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      goto continue_;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        buffer_->release (); buffer_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
ACE_UINT64
Stream_Decoder_MPEG_4_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::processBox (const struct Stream_Decoder_MPEG_4_BoxHeader& header_in,
                                                                        bool& needMoreData_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_4_Decoder_T::processBox"));

#define ASSERT_CONTIGUOUS_BYTES(bytes) \
  if (buffer_->length () < bytes) { needMoreData_out = true; return result; }

  // initialize return value
  ACE_UINT64 result = 0;

  std::string value_string;

  ACE_UINT32 header_type_i =
    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (header_in.type)
                                           : header_in.type);
  switch (header_type_i)
  {
    case 0x66747970: // ftyp
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FileTypeBox));
      struct Stream_Decoder_MPEG_4_FileTypeBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FileTypeBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: ftyp: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      value_string.assign (reinterpret_cast<char*> (&box_p->major_brand),
                           4);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: ftyp major_brand: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (value_string.c_str ())));
      ACE_UINT32 minor_version =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->minor_version)
                                               : box_p->minor_version);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: ftyp minor_version: %u\n"),
                  inherited::mod_->name (),
                  minor_version));
      int num_compatible_brands_i =
        static_cast<int> ((boxSize_ - sizeof (struct Stream_Decoder_MPEG_4_FileTypeBox)) / 4);
      ACE_ASSERT (sizeof (struct Stream_Decoder_MPEG_4_FileTypeBox) + (num_compatible_brands_i * 4) == boxSize_);
      for (int i = 0;
           i < num_compatible_brands_i;
           ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FileTypeBox) + ((i + 1) * 4));
        value_string.assign (reinterpret_cast<char*> (&box_p->compatible_brands[i]),
                             4);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: ftyp compatible_brands[%d]: \"%s\"\n"),
                    inherited::mod_->name (),
                    i, ACE_TEXT (value_string.c_str ())));
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x6D6F6F76: // moov
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = sizeof (struct Stream_Decoder_MPEG_4_BoxHeader);
      break;
    }
    case 0x6D766864: // mvhd
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: mvhd: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      switch (box_header_p->version)
      {
        case 0:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_MovieHeaderBox0));
          struct Stream_Decoder_MPEG_4_MovieHeaderBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MovieHeaderBox0*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: duration: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->rate)
                                                   : box_p->rate);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: rate: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          ACE_UINT16 value_3 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
                                                   : box_p->volume);
          value_hi = (value_3 & 0xFF00) >> 8;
          value_lo = value_3 & 0x00FF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: volume: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        case 1:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_MovieHeaderBox1));
          struct Stream_Decoder_MPEG_4_MovieHeaderBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MovieHeaderBox1*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: duration: %Q\n"),
                      inherited::mod_->name (),
                      value_2));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->rate)
                                                   : box_p->rate);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: rate: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          ACE_UINT16 value_3 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
                                                   : box_p->volume);
          value_hi = (value_3 & 0xFF00) >> 8;
          value_lo = value_3 & 0x00FF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mvhd: volume: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown mvhd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      box_header_p->version));
          return -1;
        }
      } // end SWITCH
      result = boxSize_;
      break;
    }
    case 0x696F6473: // iods
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_InitialObjectDescriptorBox));
      struct Stream_Decoder_MPEG_4_InitialObjectDescriptorBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_InitialObjectDescriptorBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: iods: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = boxSize_;
      break;
    }
    case 0x7472616B: // trak
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: trak: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = sizeof (struct Stream_Decoder_MPEG_4_BoxHeader);
      break;
    }
    case 0x746B6864: // tkhd
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: tkhd: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      switch (box_header_p->version)
      {
        case 0:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_TrackHeaderBox0));
          struct Stream_Decoder_MPEG_4_TrackHeaderBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_TrackHeaderBox0*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->track_ID)
                                                   : box_p->track_ID);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: track_ID: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: duration: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          //ACE_UINT16 value_3 =
          //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
          //                                         : box_p->volume);
          //value_hi = (value_3 & 0xFF00) >> 8;
          //value_lo = value_3 & 0x00FF;
          //float value_f = value_hi + (0.1F * value_lo);
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("%s: tkhd: volume: %f\n"),
          //            inherited::mod_->name (),
          //            value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->width)
                                                   : box_p->width);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: width: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->height)
                                                   : box_p->height);
          value_hi = (value_i & 0xFFFF0000) >> 16;
          value_lo = value_i & 0x0000FFFF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: height: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        case 1:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_TrackHeaderBox1));
          struct Stream_Decoder_MPEG_4_TrackHeaderBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_TrackHeaderBox1*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->track_ID)
                                                   : box_p->track_ID);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: track_ID: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: duration: %Q\n"),
                      inherited::mod_->name (),
                      value_2));
          //ACE_UINT16 value_3 =
          //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
          //                                         : box_p->volume);
          //value_hi = (value_3 & 0xFF00) >> 8;
          //value_lo = value_3 & 0x00FF;
          //float value_f = value_hi + (0.1F * value_lo);
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("%s: tkhd: volume: %f\n"),
          //            inherited::mod_->name (),
          //            value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->width)
                                                   : box_p->width);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: width: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->height)
                                                   : box_p->height);
          value_hi = (value_i & 0xFFFF0000) >> 16;
          value_lo = value_i & 0x0000FFFF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: tkhd: height: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown tkhd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      box_header_p->version));
          return -1;
        }
      } // end SWITCH
      result = boxSize_;
      break;
    }
    case 0x65647473: // edts
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: edts: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = sizeof (struct Stream_Decoder_MPEG_4_BoxHeader);
      break;
    }
    case 0x656C7374: // elst
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: elst: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      switch (box_header_p->version)
      {
        case 0:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_EditListBox0));
          struct Stream_Decoder_MPEG_4_EditListBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListBox0*> (buffer_->rd_ptr ());
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: elst: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          for (ACE_UINT32 i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= (sizeof (struct Stream_Decoder_MPEG_4_EditListBox0) + 4 + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry0))));
            struct Stream_Decoder_MPEG_4_EditListEntry0* box_entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListEntry0*> (buffer_->rd_ptr () + 4 + (i * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry0)));
            ACE_UINT32 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->segment_duration)
                                                     : box_entry_p->segment_duration);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: elst [%u]: segment_duration: %u\n"),
                        inherited::mod_->name (),
                        i, value_2));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_time)
                                                     : box_entry_p->media_time);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: elst [%u]: media_time: %u\n"),
                        inherited::mod_->name (),
                        i, value_2));
            ACE_UINT16 value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_entry_p->media_rate_integer)
                                                     : box_entry_p->media_rate_integer);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: elst [%u]: media_rate_integer: %u\n"),
                        inherited::mod_->name (),
                        i, value_3));
            //value_i =
            //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_rate_fraction)
            //                                         : box_entry_p->media_rate_fraction);
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("%s: elst: media_rate_fraction: %u\n"),
            //            inherited::mod_->name (),
            //            value_i));
          } // end FOR
          break;
        }
        case 1:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_EditListBox1));
          struct Stream_Decoder_MPEG_4_EditListBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListBox1*> (buffer_->rd_ptr ());
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: elst: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 = 0;
          for (ACE_UINT32 i = 0;
               i < value_i;
               ++i)
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_EditListBox1) + 4 + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry1)));
            struct Stream_Decoder_MPEG_4_EditListEntry1* box_entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListEntry1*> (buffer_->rd_ptr () + 4 + (i * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry1)));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_entry_p->segment_duration)
                                                     : box_entry_p->segment_duration);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: elst [%u]: segment_duration: %Q\n"),
                        inherited::mod_->name (),
                        i, value_2));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_entry_p->media_time)
                                                     : box_entry_p->media_time);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: elst [%u]: media_time: %Q\n"),
                        inherited::mod_->name (),
                        i, value_2));
            ACE_UINT16 value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_entry_p->media_rate_integer)
                                                     : box_entry_p->media_rate_integer);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: elst [%u]: media_rate_integer: %u\n"),
                        inherited::mod_->name (),
                        i, value_3));
            //value_i =
            //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_rate_fraction)
            //                                         : box_entry_p->media_rate_fraction);
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("%s: elst: media_rate_fraction: %u\n"),
            //            inherited::mod_->name (),
            //            value_i));
          } // end FOR
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown elst version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      box_header_p->version));
          return -1;
        }
      } // end SWITCH
      result = boxSize_;
      break;
    }
    case 0x6D646961: // mdia
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: mdia: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = sizeof (struct Stream_Decoder_MPEG_4_BoxHeader);
      break;
    }
    case 0x6D646864: // mdhd
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: mdhd: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      switch (box_header_p->version)
      {
        case 0:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_MediaHeaderBox0));
          struct Stream_Decoder_MPEG_4_MediaHeaderBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MediaHeaderBox0*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: duration: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT8 value_2 = box_p->language1;
#if defined (ACE_LITTLE_ENDIAN)
          ACE_UINT8 value_3 = box_p->language2_lo | (box_p->language2_hi << 2);
#else
          ACE_UINT8 value_3 = box_p->language2;
#endif // ACE_LITTLE_ENDIAN
          ACE_UINT8 value_4 = box_p->language3;
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: language: \"%c%c%c\"\n"),
                      inherited::mod_->name (),
                      value_2, value_3, value_4));
          break;
        }
        case 1:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_MediaHeaderBox1));
          struct Stream_Decoder_MPEG_4_MediaHeaderBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MediaHeaderBox1*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: duration: %Q\n"),
                      inherited::mod_->name (),
                      value_2));
          ACE_UINT8 value_3 = box_p->language1;
#if defined (ACE_LITTLE_ENDIAN)
          ACE_UINT8 value_4 = box_p->language2_lo | (box_p->language2_hi << 2);
#else
          ACE_UINT8 value_4 = box_p->language2;
#endif // ACE_LITTLE_ENDIAN
          ACE_UINT8 value_5 = box_p->language3;
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: mdhd: language: \"%c%c%c\"\n"),
                      inherited::mod_->name (),
                      value_3, value_4, value_5));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown mdhd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      box_header_p->version));
          return -1;
        }
      } // end SWITCH
      result = boxSize_;
      break;
    }
    case 0x68646C72: // hdlr
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_HandlerReferenceBox));
      struct Stream_Decoder_MPEG_4_HandlerReferenceBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_HandlerReferenceBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: hdlr: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->handler_type)
                                                : box_p->handler_type);
      ACE_UINT8* char_p = reinterpret_cast<ACE_UINT8*> (&value_i);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: hdlr: handler_type: %c%c%c%c\n"),
                  inherited::mod_->name (),
                  *char_p, *(char_p + 1), *(char_p + 2), *(char_p + 3)));
      char_p = &box_p->name[0];
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: hdlr: name: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (char_p)));
      result = boxSize_;
      break;
    }
    case 0x6D696E66: // minf
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: minf: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = sizeof (struct Stream_Decoder_MPEG_4_BoxHeader);
      break;
    }
    case 0x766D6864: // vmhd
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_VideoMediaHeaderBox));
      struct Stream_Decoder_MPEG_4_VideoMediaHeaderBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_VideoMediaHeaderBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: vmhd: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = boxSize_;
      break;
    }
    case 0x736D6864: // smhd
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SoundMediaHeaderBox));
      struct Stream_Decoder_MPEG_4_SoundMediaHeaderBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SoundMediaHeaderBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: smhd: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = boxSize_;
      break;
    }
    case 0x64696E66: // dinf
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: dinf: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = sizeof (struct Stream_Decoder_MPEG_4_BoxHeader);
      break;
    }
    case 0x64726566: // dref
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_DataReferenceBox));
      struct Stream_Decoder_MPEG_4_DataReferenceBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_DataReferenceBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: dref: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: dref: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
      ACE_UINT32 entries_size_i = 0;
      for (ACE_UINT32 i = 0;
           i < value_i;
           ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_DataReferenceBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
        struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
          reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (data_p);
        header_type_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->type)
                                                   : box_header_p->type);
        switch (header_type_i)
        {
          case 0x75726C20: // url
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_DataReferenceBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_DataReferenceEntryUrlBox));
            struct Stream_Decoder_MPEG_4_DataReferenceEntryUrlBox* box_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_DataReferenceEntryUrlBox*> (data_p);
            ACE_UINT64 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                                     : box_p->length);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: url : %Q bytes\n"),
                        inherited::mod_->name (),
                        value_2));
            ACE_UINT32 value_3 =
              (box_p->flags[0] << 16) | (box_p->flags[1] << 8) | box_p->flags[2];
            if (value_3 & 0x00000001) // stream is self-contained
              break; // --> there is no url
            value_string = reinterpret_cast<char*> (&box_p->location[0]);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: dref [#%u] location: \"%s\"\n"),
                        inherited::mod_->name (),
                        i, ACE_TEXT (value_string.c_str ())));
            entries_size_i += static_cast<ACE_UINT32> (value_2);
            data_p += value_2;
            break;
          }
          default:
          {
            value_string.assign (reinterpret_cast<char*> (&box_header_p->type),
                                 4);
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: invalid/unknown dref type (was: \"%s\"), aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (value_string.c_str ())));
            return -1;
          }
        } // end SWITCH
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x7374626C: // stbl
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stbl: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = sizeof (struct Stream_Decoder_MPEG_4_BoxHeader);
      break;
    }
    case 0x73747364: // stsd
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox));
      struct Stream_Decoder_MPEG_4_SampleDescriptionBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDescriptionBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stsd: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stsd: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
      ACE_UINT32 entries_size_i = 0;
      for (ACE_UINT32 i = 0;
           i < value_i;
           ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_BoxHeader));
        struct Stream_Decoder_MPEG_4_BoxHeader* box_header_p =
          reinterpret_cast<struct Stream_Decoder_MPEG_4_BoxHeader*> (data_p);
        header_type_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->type)
                                                   : box_header_p->type);
        switch (header_type_i)
        {
          case 0x61766331: // avc1
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox));
            struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox* box_2 =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox*> (data_p);
            ACE_UINT64 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->length)
                                                     : box_2->length);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: %Q bytes\n"),
                        inherited::mod_->name (),
                        value_2));
            ACE_UINT16 value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->data_reference_index)
                                                     : box_2->data_reference_index);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: data_reference_index: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->width)
                                                     : box_2->width);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: width: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->height)
                                                     : box_2->height);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: height: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            ACE_UINT32 value_4 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->horizontal_resolution)
                                                     : box_2->horizontal_resolution);
            ACE_UINT32 value_hi = (value_4 & 0xFFFF0000) >> 16;
            ACE_UINT32 value_lo = value_4 & 0x0000FFFF;
            float value_f = value_hi + (0.1F * value_lo);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: horizontal_resolution: %f\n"),
                        inherited::mod_->name (),
                        value_f));
            value_4 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->vertical_resolution)
                                                     : box_2->vertical_resolution);
            value_hi = (value_4 & 0xFFFF0000) >> 16;
            value_lo = value_4 & 0x0000FFFF;
            value_f = value_hi + (0.1F * value_lo);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: vertical_resolution: %f\n"),
                        inherited::mod_->name (),
                        value_f));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->frame_count)
                                                     : box_2->frame_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: frame_count: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            value_string.assign (reinterpret_cast<char*> (&box_2->compressor_name[1]),
                                 static_cast<size_t> (box_2->compressor_name[0]));
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: compressor_name: \"%s\"\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (value_string.c_str ())));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->depth)
                                                     : box_2->depth);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: avc1: depth: 0x%x\n"),
                        inherited::mod_->name (),
                        value_3));
            // parse additional extension boxes
            ACE_UINT32 remaining_bytes_i =
              static_cast<ACE_UINT32> (value_2 -
                                       sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox));
            ACE_UINT32 entries_size_2 = 0;
            char* data_2 = data_p + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox);
            while (remaining_bytes_i)
            { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_BoxHeader));
              struct Stream_Decoder_MPEG_4_BoxHeader* box_header_2 =
                reinterpret_cast<struct Stream_Decoder_MPEG_4_BoxHeader*> (data_2);
              header_type_i =
                  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_2->type)
                                                         : box_header_2->type);
              switch (header_type_i)
              {
                case 0x61766343: // avcC
                { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1));
                  struct Stream_Decoder_MPEG_4_AVCCBox_Part1* box_3 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBox_Part1*> (data_2);
                  ACE_UINT64 value_5 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->length)
                                                           : box_3->length);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: %Q bytes\n"),
                              inherited::mod_->name (),
                              value_5));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: configuration_version: %u\n"),
                              inherited::mod_->name (),
                              box_3->configuration_version));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: avc_profile_indication: %u\n"),
                              inherited::mod_->name (),
                              box_3->avc_profile_indication));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: profile_compatibility: %u\n"),
                              inherited::mod_->name (),
                              box_3->profile_compatibility));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: avc_level_indication: %u\n"),
                              inherited::mod_->name (),
                              box_3->avc_level_indication));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: length_size_minus_one: %u\n"),
                              inherited::mod_->name (),
                              box_3->length_size_minus_one));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: num_of_sequence_parameter_sets: %u\n"),
                              inherited::mod_->name (),
                              box_3->num_of_sequence_parameter_sets));
                  char* data_3 = reinterpret_cast<char*> (&box_3->entries[0]);
                  ACE_UINT32 entries_size_3 = 0;
                  for (int j = 0;
                       j < box_3->num_of_sequence_parameter_sets;
                       ++j)
                  { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry));
                    struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry* entry_p =
                      reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry*> (data_3);
                    ACE_UINT16 value_6 =
                      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (entry_p->sequence_parameter_set_length)
                                                             : entry_p->sequence_parameter_set_length);
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: avcC SPS[%d]: %u byte(s)\n"),
                                inherited::mod_->name (),
                                j, value_6));
                    entries_size_3 += (value_6 + 2);
                    data_3 += (value_6 + 2);
                  } // end FOR
                  ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part2));
                  struct Stream_Decoder_MPEG_4_AVCCBox_Part2* box_4 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBox_Part2*> (data_3);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: avcC: num_of_picture_parameter_sets: %u\n"),
                              inherited::mod_->name (),
                              box_4->num_of_picture_parameter_sets));
                  data_3 = reinterpret_cast<char*> (&box_4->entries[0]);
                  ACE_UINT32 entries_size_4 = 0;
                  for (int k = 0;
                       k < box_4->num_of_picture_parameter_sets;
                       ++k)
                  { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part2) + entries_size_4 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry));
                    struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry* entry_p =
                      reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry*> (data_3);
                    ACE_UINT16 value_6 =
                      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (entry_p->picture_parameter_set_length)
                                                             : entry_p->picture_parameter_set_length);
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: avcC PPS[%d]: %u byte(s)\n"),
                                inherited::mod_->name (),
                                k, value_6));
                    entries_size_4 += (value_6 + 2);
                    data_3 += (value_6 + 2);
                  } // end FOR
                  break;
                }
                case 0x636F6C72: // colr
                { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_ColorInformationBoxBase));
                  struct Stream_Decoder_MPEG_4_ColorInformationBoxBase* box_3 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_ColorInformationBoxBase*> (data_2);
                  ACE_UINT64 value_5 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->length)
                                                           : box_3->length);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: colr: %Q bytes\n"),
                              inherited::mod_->name (),
                              value_5));
                  header_type_i =
                      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->colour_type)
                                                             : box_3->colour_type);
                  switch (header_type_i)
                  {
                    case 0x6E636C78: // nclx
                    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_ColorInformationBoxNCLX));
                      struct Stream_Decoder_MPEG_4_ColorInformationBoxNCLX* box_4 =
                        reinterpret_cast<struct Stream_Decoder_MPEG_4_ColorInformationBoxNCLX*> (data_2);
                      ACE_UINT16 value_6 =
                        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_4->colour_primaries)
                                                               : box_4->colour_primaries);
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: clnx: colour_primaries: %u\n"),
                                  inherited::mod_->name (),
                                  value_6));
                      value_6 =
                        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_4->transfer_characteristics)
                                                               : box_4->transfer_characteristics);
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: clnx: transfer_characteristics: %u\n"),
                                  inherited::mod_->name (),
                                  value_6));
                      value_6 =
                        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_4->matrix_coefficients)
                                                               : box_4->matrix_coefficients);
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: clnx: matrix_coefficients: %u\n"),
                                  inherited::mod_->name (),
                                  value_6));
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: clnx: full_range_flag: %s\n"),
                                  inherited::mod_->name (),
                                  (box_4->full_range_flag ? ACE_TEXT ("set") : ACE_TEXT ("not set"))));
                      break;
                    }
                    default:
                    {
                      value_string.assign (reinterpret_cast<char*> (&box_3->colour_type),
                                           4);
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("%s: invalid/unknown colr colour_type (was: \"%s\"), aborting\n"),
                                  inherited::mod_->name (),
                                  ACE_TEXT (value_string.c_str ())));
                      return -1;
                    }
                  } // end SWITCH
                  break;
                }
                case 0x70617370: // pasp
                { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_PixelAspectRatioBox));
                  struct Stream_Decoder_MPEG_4_PixelAspectRatioBox* box_3 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_PixelAspectRatioBox*> (data_2);
                  ACE_UINT64 value_5 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->length)
                                                           : box_3->length);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: pasp: %Q bytes\n"),
                              inherited::mod_->name (),
                              value_5));
                  ACE_UINT32 value_6 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->hSpacing)
                                                            : box_3->hSpacing);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: pasp: hSpacing: %u\n"),
                              inherited::mod_->name (),
                              value_6));
                  value_6 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->vSpacing)
                                                            : box_3->vSpacing);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: pasp: vSpacing: %u\n"),
                              inherited::mod_->name (),
                              value_6));
                  break;
                }
                default:
                {
                  value_string.assign (reinterpret_cast<char*> (&box_header_2->type),
                                       4);
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: invalid/unknown avc1 type (was: \"%s\"), aborting\n"),
                              inherited::mod_->name (),
                              ACE_TEXT (value_string.c_str ())));
                  return -1;
                }
              } // end SWITCH
              ACE_UINT64 value_5 =
                ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_2->length)
                                                       : box_header_2->length);
              remaining_bytes_i -= static_cast<ACE_UINT32> (value_5);
              data_2 += value_5;
            } // end WHILE
            entries_size_i += static_cast<ACE_UINT32> (value_2);
            data_p += value_2;
            break;
          }
          case 0x6D703461: // mp4a
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox));
            struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox* box_2 =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox*> (data_p);
            ACE_UINT64 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->length)
                                                      : box_2->length);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: mp4a: %Q bytes\n"),
                        inherited::mod_->name (),
                        value_2));
            ACE_UINT16 value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->data_reference_index)
                                                     : box_2->data_reference_index);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: mp4a: data_reference_index: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->channel_count)
                                                     : box_2->channel_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: mp4a: channel_count: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->sample_size)
                                                     : box_2->sample_size);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: mp4a: sample_size: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            ACE_UINT32 value_4 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->sample_rate)
                                                     : box_2->sample_rate);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: mp4a: sample_rate: %u\n"),
                        inherited::mod_->name (),
                        value_4));
            // parse additional extension boxes
            ACE_UINT32 remaining_bytes_i =
              static_cast<ACE_UINT32> (value_2 -
                                       sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox));
            ACE_UINT32 entries_size_2 = 0;
            char* data_2 = data_p + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox);
            while (remaining_bytes_i)
            { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_BoxHeader));
              struct Stream_Decoder_MPEG_4_BoxHeader* box_header_2 =
                reinterpret_cast<struct Stream_Decoder_MPEG_4_BoxHeader*> (data_2);
              header_type_i =
                  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_2->type)
                                                         : box_header_2->type);
              switch (header_type_i)
              {
                case 0x65736473: // esds
                { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_ElementarySampleDescriptionBox));
                  struct Stream_Decoder_MPEG_4_ElementarySampleDescriptionBox* box_3 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_ElementarySampleDescriptionBox*> (data_2);
                  ACE_UINT64 value_5 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->length)
                                                            : box_3->length);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: %Q bytes\n"),
                              inherited::mod_->name (),
                              value_5));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: tag: 0x%x\n"),
                              inherited::mod_->name (),
                              box_3->es_descriptor.tag));
                  int j = 0;
                  ACE_UINT32 es_descriptor_size = 0;
                  while (box_3->es_descriptor.size_of_instance[j] & 0x80)
                  {
                    es_descriptor_size =
                      ((es_descriptor_size << 7) | (box_3->es_descriptor.size_of_instance[j] & 0x7F));
                    ++j;
                  } // end WHILE
                  es_descriptor_size =
                    ((es_descriptor_size << 7) | (box_3->es_descriptor.size_of_instance[j] & 0x7F));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: size_of_instance: %u\n"),
                              inherited::mod_->name (),
                              es_descriptor_size));
                  ACE_UINT16 value_6 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_3->es_descriptor.es_id)
                                                           : box_3->es_descriptor.es_id);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: es_id: %u\n"),
                              inherited::mod_->name (),
                              value_6));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: stream_dependence_flag: %s\n"),
                              inherited::mod_->name (),
                              (box_3->es_descriptor.stream_dependence_flag ? ACE_TEXT ("set") : ACE_TEXT ("not set"))));
                  ACE_ASSERT (!box_3->es_descriptor.stream_dependence_flag);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: url_flag: %s\n"),
                              inherited::mod_->name (),
                              (box_3->es_descriptor.url_flag ? ACE_TEXT ("set") : ACE_TEXT ("not set"))));
                  ACE_ASSERT (!box_3->es_descriptor.url_flag);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: ocr_stream_flag: %s\n"),
                              inherited::mod_->name (),
                              (box_3->es_descriptor.ocr_stream_flag ? ACE_TEXT ("set") : ACE_TEXT ("not set"))));
                  ACE_ASSERT (!box_3->es_descriptor.ocr_stream_flag);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: stream_priority: %s\n"),
                              inherited::mod_->name (),
                              box_3->es_descriptor.stream_priority));
                  // parse decoder-configuration descriptor
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: tag2: 0x%x\n"),
                              inherited::mod_->name (),
                              box_3->es_descriptor.decoder_configuration_descriptor.tag));
                  j = 0;
                  ACE_UINT32 decoder_configuration_descriptor_size = 0;
                  while (box_3->es_descriptor.decoder_configuration_descriptor.size_of_instance[j] & 0x80)
                  {
                    decoder_configuration_descriptor_size =
                      ((decoder_configuration_descriptor_size << 7) | (box_3->es_descriptor.decoder_configuration_descriptor.size_of_instance[j] & 0x7F));
                    ++j;
                  } // end WHILE
                  decoder_configuration_descriptor_size =
                    ((decoder_configuration_descriptor_size << 7) | (box_3->es_descriptor.decoder_configuration_descriptor.size_of_instance[j] & 0x7F));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: size_of_instance2: %u\n"),
                              inherited::mod_->name (),
                              decoder_configuration_descriptor_size));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: object_type_indication: 0x%x\n"),
                              inherited::mod_->name (),
                              box_3->es_descriptor.decoder_configuration_descriptor.object_type_indication));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: stream_type: %u\n"),
                              inherited::mod_->name (),
                              box_3->es_descriptor.decoder_configuration_descriptor.stream_type));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: up_stream: %s\n"),
                              inherited::mod_->name (),
                              (box_3->es_descriptor.decoder_configuration_descriptor.up_stream ? ACE_TEXT ("set") : ACE_TEXT ("not set"))));
                  ACE_UINT32 value_7 =
                    (box_3->es_descriptor.decoder_configuration_descriptor.buffer_size_db[0] << 16) |
                    (box_3->es_descriptor.decoder_configuration_descriptor.buffer_size_db[1] << 8)  |
                    box_3->es_descriptor.decoder_configuration_descriptor.buffer_size_db[2];
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: buffer_size_db: %u\n"),
                              inherited::mod_->name (),
                              value_7));
                  value_7 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->es_descriptor.decoder_configuration_descriptor.max_bitrate)
                                                           : box_3->es_descriptor.decoder_configuration_descriptor.max_bitrate);

                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: max_bitrate: %u\n"),
                              inherited::mod_->name (),
                              value_7));
                  value_7 =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_3->es_descriptor.decoder_configuration_descriptor.avg_bitrate)
                                                           : box_3->es_descriptor.decoder_configuration_descriptor.avg_bitrate);

                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: esds: avg_bitrate: %u\n"),
                              inherited::mod_->name (),
                              value_7));
                  // parse decoder-specific-information descriptor(s)
                  ACE_UINT32 remaining_bytes_2 =
                    (decoder_configuration_descriptor_size -
                     (sizeof (struct Stream_Decoder_MPEG_4_DecoderConfigurationDescriptor) -
                      sizeof (struct Stream_Decoder_MPEG_4_BaseDescriptor)));
                  ACE_UINT32 entries_size_3 = 0;
                  char* data_3 =
                    reinterpret_cast<char*> (&box_3->es_descriptor.decoder_configuration_descriptor.decoder_specific_information[0]);
                  j = 0;
                  while (remaining_bytes_2)
                  { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_ElementarySampleDescriptionBox) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase));
                    struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase* descriptor_p =
                      reinterpret_cast<struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase*> (data_3);
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: esds: tag[%d]: 0x%x\n"),
                                inherited::mod_->name (),
                                j, descriptor_p->tag));
                    int k = 0;
                    value_7 = 0;
                    while (descriptor_p->size_of_instance[k] & 0x80)
                    {
                      value_7 =
                        ((value_7 << 7) | (descriptor_p->size_of_instance[k] & 0x7F));
                      ++k;
                    } // end WHILE
                    value_7 =
                      ((value_7 << 7) | (descriptor_p->size_of_instance[k] & 0x7F));
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: esds: size_of_instance[%d]: %u\n"),
                                inherited::mod_->name (),
                                j, value_7));
                    entries_size_3 +=
                      (value_7 + sizeof (struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase));
                    data_3 +=
                      (value_7 + sizeof (struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase));
                    ++j;
                    remaining_bytes_2 -=
                      (value_7 + sizeof (struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase));
                  } // end WHILE
                  // parse remaining descriptor(s)
                  remaining_bytes_2 =
                    es_descriptor_size -
                    (decoder_configuration_descriptor_size +
                     sizeof (struct Stream_Decoder_MPEG_4_BaseDescriptor) +
                     (sizeof (struct Stream_Decoder_MPEG_4_ElementaryStreamDescriptor) -
                      sizeof (struct Stream_Decoder_MPEG_4_BaseDescriptor) -
                      sizeof (struct Stream_Decoder_MPEG_4_DecoderConfigurationDescriptor)));
                  j = 0;
                  while (remaining_bytes_2)
                  { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_ElementarySampleDescriptionBox) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_BaseDescriptor));
                    struct Stream_Decoder_MPEG_4_BaseDescriptor* descriptor_p =
                      reinterpret_cast<struct Stream_Decoder_MPEG_4_BaseDescriptor*> (data_3);
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: esds: tag2[%d]: 0x%x\n"),
                                inherited::mod_->name (),
                                j, descriptor_p->tag));
                    int k = 0;
                    value_7 = 0;
                    while (descriptor_p->size_of_instance[k] & 0x80)
                    {
                      value_7 =
                        ((value_7 << 7) | (descriptor_p->size_of_instance[k] & 0x7F));
                      ++k;
                    } // end WHILE
                    value_7 =
                      ((value_7 << 7) | (descriptor_p->size_of_instance[k] & 0x7F));
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: esds: size_of_instance2[%d]: %u\n"),
                                inherited::mod_->name (),
                                value_7));
                    entries_size_3 +=
                      (value_7 + sizeof (struct Stream_Decoder_MPEG_4_BaseDescriptor));
                    data_3 +=
                      (value_7 + sizeof (struct Stream_Decoder_MPEG_4_BaseDescriptor));
                    ++j;
                    remaining_bytes_2 -=
                      (value_7 + sizeof (struct Stream_Decoder_MPEG_4_BaseDescriptor));
                  } // end WHILE
                  break;
                }
                default:
                {
                  value_string.assign (reinterpret_cast<char*> (&box_header_2->type),
                                       4);
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: invalid/unknown mp4a type (was: \"%s\"), aborting\n"),
                              inherited::mod_->name (),
                              ACE_TEXT (value_string.c_str ())));
                  return -1;
                }
              } // end SWITCH
              ACE_UINT32 value_5 =
                ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_2->length)
                                                       : box_header_2->length);
              remaining_bytes_i -= value_5;
              data_2 += value_5;
            } // end WHILE
            entries_size_i += static_cast<ACE_UINT32> (value_2);
            data_p += value_2;
            break;
          }
          case 0x62747274: // btrt
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + 4 + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBitRateBox));
            struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBitRateBox* box_2 =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBitRateBox*> (data_p);
            ACE_UINT64 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->length)
                                                      : box_2->length);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: btrt: %Q bytes\n"),
                        inherited::mod_->name (),
                        value_2));
            ACE_UINT32 value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->bufferSizeDB)
                                                     : box_2->bufferSizeDB);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: btrt: bufferSizeDB: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->maxBitrate)
                                                     : box_2->maxBitrate);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: btrt: maxBitrate: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_2->avgBitrate)
                                                     : box_2->avgBitrate);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: btrt: avgBitrate: %u\n"),
                        inherited::mod_->name (),
                        value_3));
            entries_size_i += static_cast<ACE_UINT32> (value_2);
            data_p += value_2;
            break;
          }
          default:
          {
            value_string.assign (reinterpret_cast<char*> (&box_header_p->type),
                                 4);
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: invalid/unknown stsd type (was: \"%s\"), aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (value_string.c_str ())));
            return -1;
          }
        } // end SWITCH
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x73747473: // stts
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_TimeToSampleBox));
      struct Stream_Decoder_MPEG_4_TimeToSampleBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_TimeToSampleBox*> (buffer_->rd_ptr ());
      ACE_UINT64 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                               : box_p->length);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stts: %Q bytes\n"),
                  inherited::mod_->name (),
                  value_i));
      ACE_UINT32 value_2 =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stts: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_2));
      char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
      for (ACE_UINT32 i = 0;
           i < value_2;
           ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_TimeToSampleBox) + 4 + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry)));
        struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry* entry_p =
          reinterpret_cast<struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry*> (data_p);
        ACE_UINT32 value_3 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_count)
                                                 : entry_p->sample_count);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stts [#%u]: sample_count: %u\n"),
                    inherited::mod_->name (),
                    i, value_3));
        value_3 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_delta)
                                                 : entry_p->sample_delta);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stts [#%u]: sample_delta: %u\n"),
                    inherited::mod_->name (),
                    i, value_3));
        data_p += sizeof (struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry);
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x63747473: // ctts
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT64 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->length)
                                               : box_header_p->length);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: ctts: %Q bytes\n"),
                  inherited::mod_->name (),
                  value_i));
      switch (box_header_p->version)
      {
        case 0:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox0));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBox0*> (buffer_->rd_ptr ());
          ACE_UINT32 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: ctts: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_2));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          for (ACE_UINT32 i = 0;
               i < value_2;
               ++i)
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox0) + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0)));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0*> (data_p);
            ACE_UINT32 value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_count)
                                                     : entry_p->sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: ctts [#%u]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_3));
            value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_offset)
                                                     : entry_p->sample_offset);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: ctts [#%u]: sample_offset: %u\n"),
                        inherited::mod_->name (),
                        i, value_3));
            data_p += sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0);
          } // end FOR
          break;
        }
        case 1:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox1));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBox1*> (buffer_->rd_ptr ());
          ACE_UINT32 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: ctts: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_2));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          for (ACE_UINT32 i = 0;
               i < value_2;
               ++i)
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox1) + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1)));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1*> (data_p);
            ACE_UINT32 value_3 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_count)
                                                     : entry_p->sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: ctts [#%u]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_3));
            ACE_INT32 value_4 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_offset)
                                                     : entry_p->sample_offset);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: ctts [#%u]: sample_offset: %d\n"),
                        inherited::mod_->name (),
                        i, value_4));
            data_p += sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1);
          } // end FOR
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown ctts version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      box_header_p->version));
          return -1;
        }
      } // end SWITCH
      result = boxSize_;
      break;
    }
    case 0x73747373: // stss
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SyncSampleBox));
      struct Stream_Decoder_MPEG_4_SyncSampleBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SyncSampleBox*> (buffer_->rd_ptr ());
      ACE_UINT64 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                               : box_p->length);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stss: %Q bytes\n"),
                  inherited::mod_->name (),
                  value_i));
      ACE_UINT32 value_2 =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stss: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_2));
      for (ACE_UINT32 i = 0;
            i < value_2;
            ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SyncSampleBox) + ((i + 1) * sizeof (uint32_t)));
        ACE_UINT32 value_3 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i])
                                                 : box_p->entries[i]);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stss [#%u]: sample#: %u\n"),
                    inherited::mod_->name (),
                    i, value_3));
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x73647470: // sdtp
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox));
      struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox*> (buffer_->rd_ptr ());
      ACE_UINT64 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                               : box_p->length);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: sdtp: %Q bytes\n"),
                  inherited::mod_->name (),
                  value_i));
      ACE_UINT32 value_2 =
        static_cast<ACE_UINT32> (value_i - sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: sdtp: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_2));
      for (ACE_UINT32 i = 0;
            i < value_2;
            ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox) + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyType)));
        ACE_UINT8 value_3 = *(ACE_UINT8*)&box_p->entries[i];
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: sdtp [#%u]: 0x%x\n"),
                    inherited::mod_->name (),
                    i, value_3));
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x73747363: // stsc
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleToChunkBox));
      struct Stream_Decoder_MPEG_4_SampleToChunkBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleToChunkBox*> (buffer_->rd_ptr ());
      ACE_UINT64 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                               : box_p->length);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stsc: %Q bytes\n"),
                  inherited::mod_->name (),
                  value_i));
      ACE_UINT32 value_2 =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stsc: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_2));
      for (ACE_UINT32 i = 0;
           i < value_2;
           ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleToChunkBox) + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_SampleToChunkBoxEntry)));
        ACE_UINT32 value_3 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].first_chunk)
                                                 : box_p->entries[i].first_chunk);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stsc [#%u]: first_chunk: %u\n"),
                    inherited::mod_->name (),
                    i, value_3));
        value_3 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].samples_per_chunk)
                                                 : box_p->entries[i].samples_per_chunk);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stsc [#%u]: samples_per_chunk: %u\n"),
                    inherited::mod_->name (),
                    i, value_3));
        value_3 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].samples_description_index)
                                                 : box_p->entries[i].samples_description_index);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stsc [#%u]: samples_description_index: %u\n"),
                    inherited::mod_->name (),
                    i, value_3));
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x7374737A: // stsz
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleSizeBox));
      struct Stream_Decoder_MPEG_4_SampleSizeBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleSizeBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stsz: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->sample_size)
                                               : box_p->sample_size);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stsz: sample_size: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->sample_count)
                                               : box_p->sample_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stsz: sample_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      if (box_p->sample_size)
        break; // --> samples are all the same size
      for (ACE_UINT32 i = 0;
           i < value_i;
           ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleSizeBox) + ((i + 1) * sizeof (uint32_t)));
        ACE_UINT32 value_2 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_size[i])
                                                 : box_p->entry_size[i]);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stsz [#%u]: entry_size: %u\n"),
                    inherited::mod_->name (),
                    i, value_2));
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x7374636F: // stco
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_ChunkOffsetBox));
      struct Stream_Decoder_MPEG_4_ChunkOffsetBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_ChunkOffsetBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stco: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stco: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      for (ACE_UINT32 i = 0;
           i < value_i;
           ++i)
      { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_ChunkOffsetBox) + ((i + 1) * sizeof (uint32_t)));
        ACE_UINT32 value_2 =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->chunk_offset[i])
                                                 : box_p->chunk_offset[i]);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stco [#%u]: chunk_offset: %u\n"),
                    inherited::mod_->name (),
                    i, value_2));
      } // end FOR
      result = boxSize_;
      break;
    }
    case 0x73677064: // sgpd
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: sgpd: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      switch (box_header_p->version)
      {
        case 0:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown sgpd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      box_header_p->version));
          return false;
        }
        case 1:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sgpd: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->default_length_or_sample_description_index)
                                                   : box_p->default_length_or_sample_description_index);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sgpd: default_length: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          if (!value_i)
            goto version_geq_2;
          ACE_UINT32 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sgpd: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          ACE_UINT32 entries_size_i = 0;
          for (ACE_UINT32 i = 0;
               i < value_2;
               ++i)
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10*> (data_p);
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->description_length)
                                                     : entry_p->description_length);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: sgpd [#%u]: description_length: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
            struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase* entry_2 =
              &entry_p->sample_group_entry;
            entries_size_i +=
              (box_p->default_length_or_sample_description_index ? box_p->default_length_or_sample_description_index
                                                                 : value_i);
            data_p +=
              (sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10) +
               (box_p->default_length_or_sample_description_index ? box_p->default_length_or_sample_description_index
                                                                  : value_i));
          } // end FOR
          break;
        }
        default:
        {
version_geq_2:
          ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                                        4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sgpd: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->default_length_or_sample_description_index)
                                                   : box_p->default_length_or_sample_description_index);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sgpd: default_length_or_sample_description_index: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT32 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sgpd: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_2));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          // *TODO*: this is incomplete !
          for (ACE_UINT32 i = 0;
               i < value_2;
               ++i)
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox) + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase)));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase*> (data_p);
            data_p += sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase);
          } // end FOR
          break;
        }
      } // end SWITCH
      result = boxSize_;
      break;
    }
   case 0x73626770: // sbgp
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: sbgp: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      switch (box_header_p->version)
      {
        case 1:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox1));
          struct Stream_Decoder_MPEG_4_SampleToGroupBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleToGroupBox1*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sbgp: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->grouping_type_parameter)
                                                   : box_p->grouping_type_parameter);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sbgp: grouping_type_parameter: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sbgp: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          for (ACE_UINT32 i = 0;
               i < value_i;
               ++i)
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox1) + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBoxEntry)));
            ACE_UINT32 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].sample_count)
                                                     : box_p->entries[i].sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: sbgp [#%u]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_2));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].group_description_index)
                                                     : box_p->entries[i].group_description_index);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: sbgp [#%u]: group_description_index: %u\n"),
                        inherited::mod_->name (),
                        i, value_2));
          } // end FOR
          break;
        }
        default:
        { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox));
          struct Stream_Decoder_MPEG_4_SampleToGroupBox* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleToGroupBox*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sbgp: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: sbgp: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          for (ACE_UINT32 i = 0;
               i < value_i;
               ++i)
          { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox) + ((i + 1) * sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBoxEntry)));
            ACE_UINT32 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].sample_count)
                                                     : box_p->entries[i].sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: sbgp [#%u]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_2));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].group_description_index)
                                                     : box_p->entries[i].group_description_index);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: sbgp [#%u]: group_description_index: %u\n"),
                        inherited::mod_->name (),
                        i, value_2));
          } // end FOR
          break;
        }
      } // end SWITCH
      result = boxSize_;
      break;
    }
    case 0x66726565: // free
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_FreeSpaceBox));
      struct Stream_Decoder_MPEG_4_FreeSpaceBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FreeSpaceBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: free: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      result = boxSize_;
      break;
    }
    case 0x6D646174: // mdat
    { ASSERT_CONTIGUOUS_BYTES (sizeof (struct Stream_Decoder_MPEG_4_MediaDataBox));
      struct Stream_Decoder_MPEG_4_MediaDataBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_MediaDataBox*> (buffer_->rd_ptr ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: mdat: %Q bytes\n"),
                  inherited::mod_->name (),
                  boxSize_));
      ACE_ASSERT (false); // *TODO*
      result = boxSize_;
      break;
    }
    default:
    {
      value_string.assign (reinterpret_cast<char*> (&const_cast<struct Stream_Decoder_MPEG_4_BoxHeader&> (header_in).type),
                           4);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown box type (was: %u [%s]), aborting\n"),
                  inherited::mod_->name (),
                  header_in.type, ACE_TEXT (value_string.c_str ())));
      return -1;
    }
  } // end SWITCH

  return result;
}