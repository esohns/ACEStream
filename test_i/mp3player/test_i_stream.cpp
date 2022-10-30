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
#include "stdafx.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mtype.h"
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_string_tools.h"

#include "stream_macros.h"

#include "stream_file_defines.h"

#include "test_i_stream.h"

Test_I_Stream::Test_I_Stream ()
 : inherited ()
 , FileSource_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING))
#if defined (MPG123_SUPPORT)
 , Mp3Source_ (this,
               ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_1LAYER3_DEFAULT_NAME_STRING))
#endif // MPG123_SUPPORT
#if defined (FFMPEG_SUPPORT)
 , FfmpegDecoder_ (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING))
#endif // FFMPEG_SUPPORT
#if defined (FAAD_SUPPORT)
 , FaadDecoder_ (this,
                 ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_FAAD_DEFAULT_NAME_STRING))
#endif // FAAD_SUPPORT
 , statisticReport_ (this,
                     ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 //, WAVEncoder_ (this,
 //               ACE_TEXT_ALWAYS_CHAR ("WAVEncoder"))
 //, FileSink_ (this,
 //             ACE_TEXT_ALWAYS_CHAR ("FileSink"))
 , player_ (this,
            ACE_TEXT_ALWAYS_CHAR ("Player"))
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::Test_I_Stream"));

}

Test_I_Stream::~Test_I_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::~Test_I_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_I_Stream::load (Stream_ILayout* layout_in,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  ACE_ASSERT (!(*iterator).second.second->fileIdentifier.identifier.empty ());

  std::string extension_string =
    Common_String_Tools::tolower (Common_File_Tools::fileExtension ((*iterator).second.second->fileIdentifier.identifier,
                                                                    false));
  bool is_mp3_file_b =
    !ACE_OS::strcmp (extension_string.c_str (),
                     ACE_TEXT_ALWAYS_CHAR ("mp3"));
  if (is_mp3_file_b)
    layout_in->append (&Mp3Source_, NULL, 0);
  else
  {
    layout_in->append (&FileSource_, NULL, 0);
#if defined (FFMPEG_SUPPORT)
    layout_in->append (&FfmpegDecoder_, NULL, 0);
#endif // FFMPEG_SUPPORT
  } // end ELSE
  layout_in->append (&statisticReport_, NULL, 0);
  //layout_in->append (&WAVEncoder_, NULL, 0);
  //layout_in->append (&FileSink_, NULL, 0);
  layout_in->append (&player_, NULL, 0);

  return true;
}

bool
Test_I_Stream::initialize (const Test_I_StreamConfiguration_t& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_MP3Player_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
  Test_I_MP3Decoder* MP3Decoder_impl_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
#else
  struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64

  // allocate a new session state, reset stream
  const_cast<Test_I_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<Test_I_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
      &const_cast<struct Test_I_MP3Player_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  iterator =
      const_cast<Test_I_StreamConfiguration_t&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
//  Stream_MediaFramework_DirectShow_Tools::copy ((*iterator).second.second->outputFormat,
//                                                media_type_s);
//  session_data_p->formats.push_back (media_type_s);
#else
//  media_type_s = (*iterator).second.second->outputFormat;
#endif // ACE_WIN32 || ACE_WIN64
  session_data_p->targetFileName =
    configuration_in.configuration_->fileIdentifier.identifier;

  // ---------------------------------------------------------------------------
  // *TODO*: remove type inferences

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<Test_I_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}

bool
Test_I_Stream::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  struct Test_I_MP3Player_SessionData& session_data_r =
      const_cast<struct Test_I_MP3Player_SessionData&> (inherited::sessionData_->getR ());

  //Test_I_Statistic_WriterTask_t* statistic_impl =
  //  dynamic_cast<Test_I_Statistic_WriterTask_t*> (statisticReport_.writer ());
  //if (!statistic_impl)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: dynamic_cast<Test_I_Statistic_WriterTask_t> failed, aborting\n"),
  //              ACE_TEXT (stream_name_string_)));
  //  return false;
  //} // end IF

  // synch access
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      return false;
    } // end IF
  } // end IF

  session_data_r.statistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistics module...
  bool result_2 = false;
  try {
    //result_2 = statistic_impl->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (stream_name_string_)));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
  else
    session_data_r.statistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (stream_name_string_)));
  } // end IF

  return result_2;
}

void
Test_I_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::report"));

//   Test_I_Stream_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<Test_I_Stream_Statistic_WriterTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Test_I_Stream_Statistic_WriterTask_t*> failed, returning\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module...
//   return (runtimeStatistic_impl->report ());

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_Stream::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
