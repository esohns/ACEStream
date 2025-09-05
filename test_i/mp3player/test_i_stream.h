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

#ifndef TEST_I_STREAM_H
#define TEST_I_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_i_modules.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;
typedef Stream_SessionData_T<struct Test_I_SessionData> Test_I_SessionData_t;
struct Test_I_MP3Player_StreamConfiguration;
struct Test_I_MP3Player_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_MP3Player_StreamConfiguration,
                               struct Test_I_MP3Player_ModuleHandlerConfiguration> Test_I_StreamConfiguration_t;

extern const char stream_name_string_[];

class Test_I_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_MP3Player_StreamState,
                        struct Test_I_MP3Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_MP3Player_ModuleHandlerConfiguration,
                        struct Test_I_MP3Player_SessionData,
                        Test_I_MP3Player_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_MP3Player_StreamState,
                        struct Test_I_MP3Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_MP3Player_ModuleHandlerConfiguration,
                        struct Test_I_MP3Player_SessionData,
                        Test_I_MP3Player_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage> inherited;

 public:
  Test_I_Stream ();
  inline virtual ~Test_I_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_StreamConfiguration_t&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (struct Stream_Statistic&); // return value: statistic data
  virtual void report () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream (const Test_I_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream& operator= (const Test_I_Stream&))

  // *TODO*: re-consider this API
  void ping ();

  // modules
  Test_I_FileSource_Module        FileSource_;
#if defined (MPG123_SUPPORT)
  Test_I_MP3Decoder_Module        Mp3Source_;
#endif // MPG123_SUPPORT
#if defined (FFMPEG_SUPPORT)
  Test_I_LibAVAudioDecoder_Module FfmpegDecoder_;
#endif // FFMPEG_SUPPORT
#if defined (FAAD_SUPPORT)
  Test_I_AACDecoder_Module        FaadDecoder_;
#endif // FAAD_SUPPORT
  Test_I_StatisticReport_Module   statisticReport_;
  //Test_I_WAVEncoder_Module      WAVEncoder_;
  //Test_I_FileWriter_Module      FileSink_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_WavOutPlayer_Module      waveOutPlayer_;
#else
  Test_I_ALSAPlayer_Module        ALSAPlayer_;
#if defined (LIBPIPEWIRE_SUPPORT)
  Test_I_PipewirePlayer_Module    PipewirePlayer_;
#endif // LIBPIPEWIRE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
};

#endif
