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

#include "test_i_stream.h"

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/codec_id.h"
}
#endif /* __cplusplus */
#endif // FFMPEG_SUPPORT

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_file_defines.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

#include "test_i_common_modules.h"
#include "test_i_decoder.h"

Test_I_Stream::Test_I_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::Test_I_Stream"));

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

  // *NOTE*: this processing stream may have branches, depending on:
  //         - whether the output(s) is/are displayed on a screen
  //         - whether the output is saved to file
  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;

  Stream_Module_t* module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_FileSource_Module (this,
  //                                          ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING)),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Decoder_Module (this,
                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  // layout_in->append (&statisticReport_, NULL, 0);

  if ((*iterator).second.second->codecId == AV_CODEC_ID_AAC)
    ACE_NEW_RETURN (module_p,
                    Test_I_AACDecoder_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_FAAD_DEFAULT_NAME_STRING)),
                    false);
  else
    ACE_NEW_RETURN (module_p,
                    Test_I_LibAVAudioDecoder_Module (this,
                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING)),
                    false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  if (inherited::configuration_->configuration_->slowDown != -1)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_AudioEffect_Module (this,
                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;

    (*iterator).second.second->effectOptions.clear ();
    (*iterator).second.second->effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-m")); // optimize for music
    std::ostringstream converter;
    converter << static_cast<float> (inherited::configuration_->configuration_->slowDown) / 100.0;
    (*iterator).second.second->effectOptions.push_back (converter.str ());
  } // end IF

  ACE_NEW_RETURN (module_p,
                  Test_I_AudioTagger_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_TAGGER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Distributor_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
  branch_p = module_p;
  //configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
  configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (configuration_->configuration_->branches);  
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

//      layout_in->append (&converter_, branch_p, index_i); // output is
//      uncompressed 24-bit RGB
//      layout_in->append (&resizer_, branch_p, index_i); // output is window
//      size/fullscreen
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
////      if (configuration_->configuration->renderer !=
///STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW) /        layout_in->append
///(&display_, branch_p, 0); /      else /        layout_in->append
///(&display_2_, branch_p, index_i);
//      layout_in->append (&GTKCairoDisplay_, branch_p, index_i);
//#elif defined (WXWIDGETS_USE)
//      layout_in->append (&display_, branch_p, index_i);
//#endif // GTK_USE || WXWIDGETS_USE
//#else
//      ACE_ASSERT ((*iterator).second.second->fullScreen &&
//      !(*iterator).second.second->display.identifier.empty ()); ACE_ASSERT
//      (false); // *TODO*
//#endif // GUI_SUPPORT
  //++index_i;

  ACE_NEW_RETURN (module_p,
                  Test_I_WAVEncoder_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_I_FileWriter_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}

bool
Test_I_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::initialize"));

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_I_ExtractStream_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator;
  bool remove_module_2 = false;

  // sanity check(s)
  iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());

  // ---------------------------------------------------------------------------
  // step1: allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  //if (inherited::isInitialized_)
  //  remove_module_2 = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  //if (remove_module_2 &&
  //    unlikely (!remove (inherited::configuration_->configuration_->module_2,
  //                       false,   // lock ?
  //                       false))) // reset ? (see above)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
  //              ACE_TEXT (stream_name_string_),
  //              inherited::configuration_->configuration_->module_2->name ()));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<Test_I_ExtractStream_SessionData&> (inherited::sessionData_->getR ());
  //Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format,
  //                                              media_type_s);
  //session_data_p->formats.push_back (media_type_s);
  //session_data_p->stream = this;
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // step2: assemble stream
  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  return true;

error:
  return false;
}
