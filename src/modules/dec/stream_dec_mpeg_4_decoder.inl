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
 , buffer_ (NULL)
 , needBytes_ (0)
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
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF
    needBytes_ = 0;
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
  ACE_UINT64 total_bytes_to_skip = 0, total_bytes_to_skip_2 = 0, bytes_to_skip = 0;

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
  if ((total_length < needBytes_) ||
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

  while (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_BoxHeader))
  {
    box_header_p =
      reinterpret_cast<struct Stream_Decoder_MPEG_4_BoxHeader*> (buffer_->rd_ptr ());

    // step1: gather complete box data
    needBytes_ =
      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->length)
                                             : box_header_p->length);
    if (needBytes_ == 1) // 'large' box ?
    {
      large_box_b = true;
      if (buffer_->length () < sizeof (struct Stream_Decoder_MPEG_4_LargeBoxHeader))
        break; // --> need more data
      struct Stream_Decoder_MPEG_4_LargeBoxHeader* box_header_2 =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_LargeBoxHeader*> (buffer_->rd_ptr ());
      needBytes_ =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_header_2->largesize)
                                               : box_header_2->largesize);
    } // end IF
    if (total_length < needBytes_)
      break; // --> need more data

    // step2: process box data
    boxes_.push_back (std::make_pair (box_header_p->type, skipped_bytes));
    buffer_->rd_ptr (large_box_b ? sizeof (struct Stream_Decoder_MPEG_4_LargeBoxHeader)
                                 : sizeof (struct Stream_Decoder_MPEG_4_BoxHeader));
    total_bytes_to_skip = needBytes_; total_bytes_to_skip_2 = needBytes_;
    if (!processBox (*box_header_p,
                     needBytes_ - (large_box_b ? sizeof (struct Stream_Decoder_MPEG_4_LargeBoxHeader)
                                               : sizeof (struct Stream_Decoder_MPEG_4_BoxHeader))))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_MPEG_4_Decoder_T::processBox(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    message_block_p = buffer_;
    while (total_bytes_to_skip)
    {
      bytes_to_skip =
        std::min (static_cast<ACE_UINT64> (message_block_p->length ()), total_bytes_to_skip);
      message_block_p->rd_ptr (bytes_to_skip);
      if (!message_block_p->length ())
        message_block_p = message_block_p->cont ();
      total_bytes_to_skip -= bytes_to_skip;
    } // end WHILE
    if (!buffer_->length ())
    {
      message_block_p = buffer_;
      while (message_block_p &&
             !message_block_p->length ())
        message_block_p = message_block_p->cont ();
      if (!message_block_p)
      {
        buffer_->release (); buffer_ = NULL;
        needBytes_ = 0;
        break;
      } // end IF
      else
      {
        ACE_Message_Block* message_block_2 = buffer_;
        while (message_block_2->cont () != message_block_p)
          message_block_2 = message_block_2->cont ();
        message_block_2->cont (NULL);
        buffer_->release ();
        buffer_ = message_block_p;
      } // end ELSE
    } // end IF

    skipped_bytes += total_bytes_to_skip_2;
  } // end WHILE

  return;

error:
  if (message_inout)
    message_inout->release ();
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
bool
Stream_Decoder_MPEG_4_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::processBox (const struct Stream_Decoder_MPEG_4_BoxHeader& header_in,
                                                                        ACE_UINT64 length_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_4_Decoder_T::processBox"));

  std::string value_string;

  switch (header_in.type)
  {
    case 0x66747970: // ftyp
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FileTypeBox));
      struct Stream_Decoder_MPEG_4_FileTypeBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FileTypeBox*> (buffer_->rd_ptr ());
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
      ACE_UINT32 num_compatible_brands = (length_in - 8 / 4);
      for (int i = 0;
           i < num_compatible_brands;
           ++i)
      {
        value_string.assign (reinterpret_cast<char*> (&box_p->compatible_brands[i]),
                             4);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: ftyp compatible_brands[%d]: \"%s\"\n"),
                    inherited::mod_->name (),
                    i, ACE_TEXT (value_string.c_str ())));
      } // end FOR
      break;
    }
    case 0x6D6F6F76: // moov
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov: %Q bytes\n"),
                  inherited::mod_->name (),
                  length_in));
      break;
    }
    case 0x6D766864: // moov_mvhd
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT32 version_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->version)
                                               : box_header_p->version);
      switch (version_i)
      {
        case 0:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_MovieHeaderBox0));
          struct Stream_Decoder_MPEG_4_MovieHeaderBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MovieHeaderBox0*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: duration: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->rate)
                                                   : box_p->rate);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: rate: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          ACE_UINT16 value_3 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
                                                   : box_p->volume);
          value_hi = (value_3 & 0xFF00) >> 8;
          value_lo = value_3 & 0x00FF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: volume: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        case 1:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_MovieHeaderBox1));
          struct Stream_Decoder_MPEG_4_MovieHeaderBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MovieHeaderBox1*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: duration: %Q\n"),
                      inherited::mod_->name (),
                      value_2));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->rate)
                                                   : box_p->rate);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: rate: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          ACE_UINT16 value_3 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
                                                   : box_p->volume);
          value_hi = (value_3 & 0xFF00) >> 8;
          value_lo = value_3 & 0x00FF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_mvhd: volume: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown moov/mvhd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      version_i));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 0x696F6473: // moov_iods
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_InitialObjectDescriptorBox));
      struct Stream_Decoder_MPEG_4_InitialObjectDescriptorBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_InitialObjectDescriptorBox*> (buffer_->rd_ptr ());
      break;
    }
    case 0x7472616B: // moov_trak
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak: %Q bytes\n"),
                  inherited::mod_->name (),
                  length_in));
      break;
    }
    case 0x746B6864: // moov_trak_tkhd
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT32 version_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->version)
                                               : box_header_p->version);
      switch (version_i)
      {
        case 0:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_TrackHeaderBox0));
          struct Stream_Decoder_MPEG_4_TrackHeaderBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_TrackHeaderBox0*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->track_ID)
                                                   : box_p->track_ID);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: track_ID: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: duration: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          //ACE_UINT16 value_3 =
          //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
          //                                         : box_p->volume);
          //value_hi = (value_3 & 0xFF00) >> 8;
          //value_lo = value_3 & 0x00FF;
          //float value_f = value_hi + (0.1F * value_lo);
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("%s: moov_trak_tkhd: volume: %f\n"),
          //            inherited::mod_->name (),
          //            value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->width)
                                                   : box_p->width);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: width: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->height)
                                                   : box_p->height);
          value_hi = (value_i & 0xFFFF0000) >> 16;
          value_lo = value_i & 0x0000FFFF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: height: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        case 1:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_TrackHeaderBox1));
          struct Stream_Decoder_MPEG_4_TrackHeaderBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_TrackHeaderBox1*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->track_ID)
                                                   : box_p->track_ID);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: track_ID: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: duration: %Q\n"),
                      inherited::mod_->name (),
                      value_2));
          //ACE_UINT16 value_3 =
          //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->volume)
          //                                         : box_p->volume);
          //value_hi = (value_3 & 0xFF00) >> 8;
          //value_lo = value_3 & 0x00FF;
          //float value_f = value_hi + (0.1F * value_lo);
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("%s: moov_trak_tkhd: volume: %f\n"),
          //            inherited::mod_->name (),
          //            value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->width)
                                                   : box_p->width);
          ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
          ACE_UINT32 value_lo = value_i & 0x0000FFFF;
          float value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: width: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->height)
                                                   : box_p->height);
          value_hi = (value_i & 0xFFFF0000) >> 16;
          value_lo = value_i & 0x0000FFFF;
          value_f = value_hi + (0.1F * value_lo);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_tkhd: height: %f\n"),
                      inherited::mod_->name (),
                      value_f));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown moov/trak/tkhd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      version_i));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 0x65647473: // edts
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_edts: %Q bytes\n"),
                  inherited::mod_->name (),
                  length_in));
      break;
    }
    case 0x65637374: // elst
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT32 version_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->version)
                                               : box_header_p->version);
      switch (version_i)
      {
        case 0:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_EditListBox0));
          struct Stream_Decoder_MPEG_4_EditListBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListBox0*> (buffer_->rd_ptr ());
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_edts_elst: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= (sizeof (struct Stream_Decoder_MPEG_4_EditListBox0) + sizeof (ACE_UINT32) + (i * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry0))));
            struct Stream_Decoder_MPEG_4_EditListEntry0* box_entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListEntry0*> (buffer_->rd_ptr () + sizeof (ACE_UINT32) + (i * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry0)));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->segment_duration)
                                                     : box_entry_p->segment_duration);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_edts_elst: segment_duration: %u\n"),
                        inherited::mod_->name (),
                        value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_time)
                                                     : box_entry_p->media_time);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_edts_elst: media_time: %u\n"),
                        inherited::mod_->name (),
                        value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_rate_integer)
                                                     : box_entry_p->media_rate_integer);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_edts_elst: media_rate_integer: %u\n"),
                        inherited::mod_->name (),
                        value_i));
            //value_i =
            //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_rate_fraction)
            //                                         : box_entry_p->media_rate_fraction);
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("%s: moov_trak_edts_elst: media_rate_fraction: %u\n"),
            //            inherited::mod_->name (),
            //            value_i));
          } // end FOR
          break;
        }
        case 1:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_EditListBox1));
          struct Stream_Decoder_MPEG_4_EditListBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListBox1*> (buffer_->rd_ptr ());
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_edts_elst: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 = 0;
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= (sizeof (struct Stream_Decoder_MPEG_4_EditListBox1) + sizeof (ACE_UINT32) + (i * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry1))));
            struct Stream_Decoder_MPEG_4_EditListEntry1* box_entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_EditListEntry1*> (buffer_->rd_ptr () + sizeof (ACE_UINT32) + (i * sizeof (struct Stream_Decoder_MPEG_4_EditListEntry1)));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_entry_p->segment_duration)
                                                     : box_entry_p->segment_duration);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_edts_elst: segment_duration: %Q\n"),
                        inherited::mod_->name (),
                        value_2));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG_LONG (box_entry_p->media_time)
                                                     : box_entry_p->media_time);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_edts_elst: media_time: %Q\n"),
                        inherited::mod_->name (),
                        value_2));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_rate_integer)
                                                     : box_entry_p->media_rate_integer);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_edts_elst: media_rate_integer: %u\n"),
                        inherited::mod_->name (),
                        value_i));
            //value_i =
            //  ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_entry_p->media_rate_fraction)
            //                                         : box_entry_p->media_rate_fraction);
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("%s: moov_trak_edts_elst: media_rate_fraction: %u\n"),
            //            inherited::mod_->name (),
            //            value_i));
          } // end FOR
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown moov/trak/edts/elst version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      version_i));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 0x6D646961: // mdia
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia: %Q bytes\n"),
                  inherited::mod_->name (),
                  length_in));
      break;
    }
    case 0x6D646864: // mdhd
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT32 version_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->version)
                                               : box_header_p->version);
      switch (version_i)
      {
        case 0:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_MediaHeaderBox0));
          struct Stream_Decoder_MPEG_4_MediaHeaderBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MediaHeaderBox0*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: duration: %u\n"),
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
                      ACE_TEXT ("%s: moov_trak_mdhd: language: \"%c%c%c\"\n"),
                      inherited::mod_->name (),
                      value_2, value_3, value_4));
          break;
        }
        case 1:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_MediaHeaderBox1));
          struct Stream_Decoder_MPEG_4_MediaHeaderBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_MediaHeaderBox1*> (buffer_->rd_ptr ());
          ACE_Date_Time date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->creation_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: creation_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          date_time =
            Stream_Module_Decoder_Tools::mpeg4ToDateTime (box_p->modification_time);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: modification_time: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Timer_Tools::dateTimeToString (date_time).c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->timescale)
                                                   : box_p->timescale);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: timescale: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT64 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->duration)
                                                   : box_p->duration);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdhd: duration: %Q\n"),
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
                      ACE_TEXT ("%s: moov_trak_mdhd: language: \"%c%c%c\"\n"),
                      inherited::mod_->name (),
                      value_3, value_4, value_5));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown moov/trak/tkhd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      version_i));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 0x68646C72: // hdlr
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_HandlerReferenceBox));
      struct Stream_Decoder_MPEG_4_HandlerReferenceBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_HandlerReferenceBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->handler_type)
                                                : box_p->handler_type);
      ACE_UINT8* char_p = reinterpret_cast<ACE_UINT8*> (&value_i);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_hdlr: handler_type: %c%c%c%c\n"),
                  inherited::mod_->name (),
                  *char_p, *(char_p + 1), *(char_p + 2), *(char_p + 3)));
      char_p = &box_p->name[0];
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_hdlr: name: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (char_p)));
      break;
    }
    case 0x6D696E66: // minf
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_minf: %Q bytes\n"),
                  inherited::mod_->name (),
                  length_in));
      break;
    }
    case 0x766D6864: // vmhd
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_VideoMediaHeaderBox));
      struct Stream_Decoder_MPEG_4_VideoMediaHeaderBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_VideoMediaHeaderBox*> (buffer_->rd_ptr ());
      break;
    }
    case 0x64696E66: // dinf
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_minf_dinf: %Q bytes\n"),
                  inherited::mod_->name (),
                  length_in));
      break;
    }
    case 0x64726566: // dref
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_DataReferenceBox));
      struct Stream_Decoder_MPEG_4_DataReferenceBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_DataReferenceBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_dinf_dref: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
      ACE_UINT32 entries_size_i = 0;
      for (int i = 0;
           i < value_i;
           ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_DataReferenceBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
        struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
          reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (data_p);
        switch (box_header_p->type)
        {
          case 0x75726C20: // url
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_DataReferenceBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_DataReferenceEntryUrlBox));
            struct Stream_Decoder_MPEG_4_DataReferenceEntryUrlBox* box_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_DataReferenceEntryUrlBox*> (data_p);
            value_i = (box_p->flags1 << 16) | (box_p->flags2 << 8) | box_p->flags3;
            if (value_i & 0x00000001) // stream is self-contained
              break; // --> there is no url
            value_string = reinterpret_cast<char*> (&box_p->location[0]);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_dinf_dref [#%d] location: \"%s\"\n"),
                        inherited::mod_->name (),
                        i, ACE_TEXT (value_string.c_str ())));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                                     : box_p->length);
            entries_size_i += value_i;
            data_p += value_i;
            break;
          }
          default:
          {
            value_string.assign (reinterpret_cast<char*> (&box_header_p->type),
                                 4);
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: invalid/unknown moov/trak/mdia/minf/dinf/dref type (was: %u), aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (value_string.c_str ())));
            return false;
          }
        } // end SWITCH
      } // end FOR
      break;
    }
    case 0x7374626C: // stbl
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_minf_stbl: %Q bytes\n"),
                  inherited::mod_->name (),
                  length_in));
      break;
    }
    case 0x73747364: // stsd
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox));
      struct Stream_Decoder_MPEG_4_SampleDescriptionBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDescriptionBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
      ACE_UINT32 entries_size_i = 0;
      for (int i = 0;
           i < value_i;
           ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_BoxHeader));
        struct Stream_Decoder_MPEG_4_BoxHeader* box_header_p =
          reinterpret_cast<struct Stream_Decoder_MPEG_4_BoxHeader*> (data_p);
        switch (box_header_p->type)
        {
          case 0x61766331: // avc1
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox));
            struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox* box_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox*> (data_p);
            ACE_UINT16 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->data_reference_index)
                                                     : box_p->data_reference_index);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: data_reference_index: %u\n"),
                        inherited::mod_->name (),
                        value_2));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->width)
                                                     : box_p->width);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: width: %u\n"),
                        inherited::mod_->name (),
                        value_2));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->height)
                                                     : box_p->height);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: height: %u\n"),
                        inherited::mod_->name (),
                        value_2));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->horizontal_resolution)
                                                     : box_p->horizontal_resolution);
            ACE_UINT32 value_hi = (value_i & 0xFFFF0000) >> 16;
            ACE_UINT32 value_lo = value_i & 0x0000FFFF;
            float value_f = value_hi + (0.1F * value_lo);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: horizontal_resolution: %f\n"),
                        inherited::mod_->name (),
                        value_f));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->vertical_resolution)
                                                     : box_p->vertical_resolution);
            value_hi = (value_i & 0xFFFF0000) >> 16;
            value_lo = value_i & 0x0000FFFF;
            value_f = value_hi + (0.1F * value_lo);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: vertical_resolution: %f\n"),
                        inherited::mod_->name (),
                        value_f));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->frame_count)
                                                     : box_p->frame_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: frame_count: %u\n"),
                        inherited::mod_->name (),
                        value_2));
            value_string.assign (reinterpret_cast<char*> (&box_p->compressor_name[1]),
                                 static_cast<size_t> (box_p->compressor_name[0]));
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: compressor_name: \"%s\"\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (value_string.c_str ())));
            value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->depth)
                                                     : box_p->depth);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1: depth: 0x%x\n"),
                        inherited::mod_->name (),
                        value_2));
            // parse additional extension boxes
            ACE_UINT32 remaining_bytes_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_p->length)
                                                     : box_p->length) -
              sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox);
            ACE_UINT32 entries_size_2 = 0;
            char* data_2 = data_p + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox);
            while (remaining_bytes_i)
            { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_BoxHeader));
              struct Stream_Decoder_MPEG_4_BoxHeader* box_header_2 =
                reinterpret_cast<struct Stream_Decoder_MPEG_4_BoxHeader*> (data_2);
              switch (box_header_2->type)
              {
                case 0x61766343: // avcC
                { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1));
                  struct Stream_Decoder_MPEG_4_AVCCBox_Part1* box_2 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBox_Part1*> (data_2);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC: configuration_version: %u\n"),
                              inherited::mod_->name (),
                              box_2->configuration_version));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC: avc_profile_indication: %u\n"),
                              inherited::mod_->name (),
                              box_2->avc_profile_indication));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC: profile_compatibility: %u\n"),
                              inherited::mod_->name (),
                              box_2->profile_compatibility));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC: avc_level_indication: %u\n"),
                              inherited::mod_->name (),
                              box_2->avc_level_indication));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC: length_size_minus_one: %u\n"),
                              inherited::mod_->name (),
                              box_2->length_size_minus_one));
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC: num_of_sequence_parameter_sets: %u\n"),
                              inherited::mod_->name (),
                              box_2->num_of_sequence_parameter_sets));
                  char* data_3 = reinterpret_cast<char*> (&box_2->entries[0]);
                  ACE_UINT32 entries_size_3 = 0;
                  for (int i = 0;
                       i < box_2->num_of_sequence_parameter_sets;
                       ++i)
                  { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry));
                    struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry* entry_p =
                      reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry*> (data_3);
                    value_i =
                      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sequence_parameter_set_length)
                                                             : entry_p->sequence_parameter_set_length);
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC_SPS[%d]: %u byte(s)\n"),
                                inherited::mod_->name (),
                                i, value_i));
                    entries_size_3 += value_i + sizeof (ACE_UINT16);
                    data_3 += value_i + sizeof (ACE_UINT16);
                  } // end FOR
                  ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part2));
                  struct Stream_Decoder_MPEG_4_AVCCBox_Part2* box_3 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBox_Part2*> (data_3);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC: num_of_picture_parameter_sets: %u\n"),
                              inherited::mod_->name (),
                              box_3->num_of_picture_parameter_sets));
                  data_3 = reinterpret_cast<char*> (&box_3->entries[0]);
                  ACE_UINT32 entries_size_4 = 0;
                  for (int i = 0;
                       i < box_3->num_of_picture_parameter_sets;
                       ++i)
                  { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part1) + entries_size_3 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBox_Part2) + entries_size_4 + sizeof (struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry));
                    struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry* entry_p =
                      reinterpret_cast<struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry*> (data_3);
                    value_i =
                      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->picture_parameter_set_length)
                                                             : entry_p->picture_parameter_set_length);
                    ACE_DEBUG ((LM_DEBUG,
                                ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_avcC_PPS[%d]: %u byte(s)\n"),
                                inherited::mod_->name (),
                                i, value_i));
                    entries_size_4 += value_i + sizeof (ACE_UINT16);
                    data_3 += value_i + sizeof (ACE_UINT16);
                  } // end FOR
                  break;
                }
                case 0x636F6C72: // colr
                { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_ColorInformationBoxBase));
                  struct Stream_Decoder_MPEG_4_ColorInformationBoxBase* box_2 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_ColorInformationBoxBase*> (data_2);
                  switch (box_2->colour_type)
                  {
                    case 0x6E636C78: // nclx
                    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_ColorInformationBoxNCLX));
                      struct Stream_Decoder_MPEG_4_ColorInformationBoxNCLX* box_3 =
                        reinterpret_cast<struct Stream_Decoder_MPEG_4_ColorInformationBoxNCLX*> (data_2);
                      value_2 =
                        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_3->colour_primaries)
                                                               : box_3->colour_primaries);
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_colr_clnx: colour_primaries: %u\n"),
                                  inherited::mod_->name (),
                                  value_2));
                      value_2 =
                        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_3->transfer_characteristics)
                                                               : box_3->transfer_characteristics);
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_colr_clnx: transfer_characteristics: %u\n"),
                                  inherited::mod_->name (),
                                  value_2));
                      value_2 =
                        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_3->matrix_coefficients)
                                                               : box_3->matrix_coefficients);
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_colr_clnx: matrix_coefficients: %u\n"),
                                  inherited::mod_->name (),
                                  value_2));
                      ACE_DEBUG ((LM_DEBUG,
                                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_colr_clnx: full_range_flag: %s\n"),
                                  inherited::mod_->name (),
                                  (box_3->full_range_flag ? ACE_TEXT ("set") : ACE_TEXT ("not set"))));
                      break;
                    }
                    default:
                    {
                      value_string.assign (reinterpret_cast<char*> (&box_2->colour_type),
                                           4);
                      ACE_DEBUG ((LM_ERROR,
                                  ACE_TEXT ("%s: invalid/unknown moov/trak/mdia/minf/stbl/stsd/avc1/colr colour_type (was: %u), aborting\n"),
                                  inherited::mod_->name (),
                                  ACE_TEXT (value_string.c_str ())));
                      return false;
                    }
                  } // end SWITCH
                  break;
                }
                case 0x70617370: // pasp
                { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox) + entries_size_2 + sizeof (struct Stream_Decoder_MPEG_4_PixelAspectRatioBox));
                  struct Stream_Decoder_MPEG_4_PixelAspectRatioBox* box_2 =
                    reinterpret_cast<struct Stream_Decoder_MPEG_4_PixelAspectRatioBox*> (data_2);
                  value_i =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->hSpacing)
                                                            : box_2->hSpacing);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_pasp: hSpacing: %u\n"),
                              inherited::mod_->name (),
                              value_i));
                  value_i =
                    ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (box_2->vSpacing)
                                                            : box_2->vSpacing);
                  ACE_DEBUG ((LM_DEBUG,
                              ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_avc1_pasp: vSpacing: %u\n"),
                              inherited::mod_->name (),
                              value_i));
                  break;
                }
                default:
                {
                  value_string.assign (reinterpret_cast<char*> (&box_header_2->type),
                                       4);
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: invalid/unknown moov/trak/mdia/minf/stbl/stsd/avc1 type (was: %u), aborting\n"),
                              inherited::mod_->name (),
                              ACE_TEXT (value_string.c_str ())));
                  return false;
                }
              } // end SWITCH
              value_i =
                ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_2->length)
                                                       : box_header_2->length);
              remaining_bytes_i -= value_i;
              data_2 += value_i;
            } // end WHILE
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                                     : box_p->length);
            entries_size_i += value_i;
            data_p += value_i;
            break;
          }
          case 0x62747274: // btrt
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + entries_size_i + sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBitRateBox));
            struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBitRateBox* box_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBitRateBox*> (data_p);
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->bufferSizeDB)
                                                     : box_p->bufferSizeDB);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_btrt: bufferSizeDB: %u\n"),
                        inherited::mod_->name (),
                        value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->maxBitrate)
                                                     : box_p->maxBitrate);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_btrt: maxBitrate: %u\n"),
                        inherited::mod_->name (),
                        value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->avgBitrate)
                                                     : box_p->avgBitrate);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsd_btrt: avgBitrate: %u\n"),
                        inherited::mod_->name (),
                        value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->length)
                                                     : box_p->length);
            entries_size_i += value_i;
            data_p += value_i;
            break;
          }
          default:
          {
            value_string.assign (reinterpret_cast<char*> (&box_header_p->type),
                                 4);
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: invalid/unknown moov/trak/mdia/minf/stbl/stsd type (was: %u), aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (value_string.c_str ())));
            return false;
          }
        } // end SWITCH
      } // end FOR
      break;
    }
    case 0x73747473: // stts
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_TimeToSampleBox));
      struct Stream_Decoder_MPEG_4_TimeToSampleBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_TimeToSampleBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                               : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stts: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
      for (int i = 0;
           i < value_i;
           ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDescriptionBox) + sizeof (ACE_UINT32) + (i * sizeof (struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry)));
        struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry* entry_p =
          reinterpret_cast<struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry*> (data_p);
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_count)
                                                 : entry_p->sample_count);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stts [#%d]: sample_count: %u\n"),
                    inherited::mod_->name (),
                    value_i));
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_delta)
                                                 : entry_p->sample_delta);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stts [#%d]: sample_delta: %u\n"),
                    inherited::mod_->name (),
                    value_i));
        data_p += sizeof (struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry);
      } // end FOR
      break;
    }
    case 0x63747473: // ctts
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT32 version_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->version)
                                               : box_header_p->version);
      switch (version_i)
      {
        case 0:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox0));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBox0* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBox0*> (buffer_->rd_ptr ());
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_ctts: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox0) + (i * sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0)));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0*> (data_p);
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_count)
                                                     : entry_p->sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_ctts [#%d]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_offset)
                                                     : entry_p->sample_offset);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_ctts [#%d]: sample_offset: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
            data_p += sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0);
          } // end FOR
          break;
        }
        case 1:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox1));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBox1*> (buffer_->rd_ptr ());
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_ctts: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBox1) + (i * sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1)));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1*> (data_p);
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_count)
                                                     : entry_p->sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_ctts [#%d]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
            ACE_INT32 value_2 =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->sample_offset)
                                                     : entry_p->sample_offset);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_ctts [#%d]: sample_offset: %d\n"),
                        inherited::mod_->name (),
                        i, value_2));
            data_p += sizeof (struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1);
          } // end FOR
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown moov/trak/minf_stbl_ctts version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      version_i));
          return false;
        }
      } // end SWITCH
      break;
    }
    case 0x73747373: // stss
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SyncSampleBox));
      struct Stream_Decoder_MPEG_4_SyncSampleBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SyncSampleBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stss: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      for (int i = 0;
            i < value_i;
            ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SyncSampleBox) + (i * sizeof (uint32_t)));
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i])
                                                  : box_p->entries[i]);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stss [#%d]: sample#: %u\n"),
                    inherited::mod_->name (),
                    i, value_i));
      } // end FOR
      break;
    }
    case 0x73647470: // sdtp
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox));
      struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i = length_in - sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sdtp: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i / sizeof (ACE_UINT8)));
      for (int i = 0;
            i < value_i;
            ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox) + (i * sizeof (struct Stream_Decoder_MPEG_4_SampleDependencyType)));
        value_i = *(ACE_UINT8*)&box_p->entries[i];
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sdtp [#%d]: sample#: %u\n"),
                    inherited::mod_->name (),
                    i, value_i));
      } // end FOR
      break;
    }
    case 0x73747363: // stsc
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleToChunkBox));
      struct Stream_Decoder_MPEG_4_SampleToChunkBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleToChunkBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsc: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      for (int i = 0;
            i < value_i;
            ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleToChunkBox) + (i * sizeof (struct Stream_Decoder_MPEG_4_SampleToChunkBoxEntry)));
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].first_chunk)
                                                 : box_p->entries[i].first_chunk);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsc [#%d]: first_chunk: %u\n"),
                    inherited::mod_->name (),
                    i, value_i));
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].samples_per_chunk)
                                                 : box_p->entries[i].samples_per_chunk);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsc [#%d]: samples_per_chunk: %u\n"),
                    inherited::mod_->name (),
                    i, value_i));
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].samples_description_index)
                                                 : box_p->entries[i].samples_description_index);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsc [#%d]: samples_description_index: %u\n"),
                    inherited::mod_->name (),
                    i, value_i));
      } // end FOR
      break;
    }
    case 0x7374737A: // stsz
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleSizeBox));
      struct Stream_Decoder_MPEG_4_SampleSizeBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleSizeBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->sample_size)
                                                : box_p->sample_size);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsz: sample_size: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->sample_count)
                                               : box_p->sample_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsz: sample_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      if (box_p->sample_size)
        break; // --> samples are all the same size
      for (int i = 0;
            i < value_i;
            ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleSizeBox) + (i * sizeof (uint32_t)));
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_size[i])
                                                 : box_p->entry_size[i]);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stsc [#%d]: entry_size: %u\n"),
                    inherited::mod_->name (),
                    i, value_i));
      } // end FOR
      break;
    }
    case 0x7374636F: // stco
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_ChunkOffsetBox));
      struct Stream_Decoder_MPEG_4_ChunkOffsetBox* box_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_ChunkOffsetBox*> (buffer_->rd_ptr ());
      ACE_UINT32 value_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                : box_p->entry_count);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stco: entry_count: %u\n"),
                  inherited::mod_->name (),
                  value_i));
      for (int i = 0;
            i < value_i;
            ++i)
      { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_ChunkOffsetBox) + (i * sizeof (uint32_t)));
        value_i =
          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->chunk_offset[i])
                                                 : box_p->chunk_offset[i]);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_stco [#%d]: chunk_offset: %u\n"),
                    inherited::mod_->name (),
                    i, value_i));
      } // end FOR
      break;
    }
    case 0x73677064: // sgpd
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT32 version_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->version)
                                               : box_header_p->version);
      switch (version_i)
      {
        case 0:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown moov/trak/minf_stbl_sgpd version (was: %u), aborting\n"),
                      inherited::mod_->name (),
                      version_i));
          return false;
        }
        case 1:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sgpd: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->default_length_or_sample_description_index)
                                                   : box_p->default_length_or_sample_description_index);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sgpd: default_length: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          if (!value_i)
            goto version_geq_2;
          ACE_UINT32 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sgpd: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10) + (i * sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10)));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10*> (data_p);
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (entry_p->description_length)
                                                     : entry_p->description_length);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sgpd [#%d]: description_length: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
            struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase* entry_2 =
              &entry_p->sample_group_entry;
            data_p += sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10);
          } // end FOR
          break;
        }
        default:
        {
version_geq_2:
          ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                                        4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sgpd: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->default_length_or_sample_description_index)
                                                   : box_p->default_length_or_sample_description_index);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sgpd: default_length_or_sample_description_index: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          ACE_UINT32 value_2 =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sgpd: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase* entry_p = NULL;
          char* data_p = reinterpret_cast<char*> (&box_p->entries[0]);
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox) + (i * sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase)));
            entry_p =
              reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase*> (data_p);
            data_p += sizeof (struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase);
          } // end FOR
          break;
        }
      } // end SWITCH
      break;
    }
   case 0x73626770: // sbgp
    { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_FullBoxHeader));
      struct Stream_Decoder_MPEG_4_FullBoxHeader* box_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_4_FullBoxHeader*> (buffer_->rd_ptr ());
      ACE_UINT32 version_i =
        ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_header_p->version)
                                               : box_header_p->version);
      switch (version_i)
      {
        case 1:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox1));
          struct Stream_Decoder_MPEG_4_SampleToGroupBox1* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleToGroupBox1*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->grouping_type_parameter)
                                                   : box_p->grouping_type_parameter);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp: grouping_type_parameter: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox1) + (i * sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBoxEntry)));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].sample_count)
                                                     : box_p->entries[i].sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp [#%d]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].group_description_index)
                                                     : box_p->entries[i].group_description_index);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp [#%d]: group_description_index: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
          } // end FOR
          break;
        }
        default:
        { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox));
          struct Stream_Decoder_MPEG_4_SampleToGroupBox* box_p =
            reinterpret_cast<struct Stream_Decoder_MPEG_4_SampleToGroupBox*> (buffer_->rd_ptr ());
          value_string.assign (reinterpret_cast<char*> (&box_p->grouping_type),
                                4);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp: grouping_type: \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (value_string.c_str ())));
          ACE_UINT32 value_i =
            ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entry_count)
                                                   : box_p->entry_count);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp: entry_count: %u\n"),
                      inherited::mod_->name (),
                      value_i));
          for (int i = 0;
               i < value_i;
               ++i)
          { ACE_ASSERT (buffer_->length () >= sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBox) + (i * sizeof (struct Stream_Decoder_MPEG_4_SampleToGroupBoxEntry)));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].sample_count)
                                                     : box_p->entries[i].sample_count);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp [#%d]: sample_count: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
            value_i =
              ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (box_p->entries[i].group_description_index)
                                                     : box_p->entries[i].group_description_index);
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: moov_trak_mdia_minf_stbl_sbgp [#%d]: group_description_index: %u\n"),
                        inherited::mod_->name (),
                        i, value_i));
          } // end FOR
          break;
        }
      } // end SWITCH
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
      return false;
    }
  } // end SWITCH

  return true;
}