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

#include "test_i_imagesave_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_tools.h"

#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_file_defines.h"

#include "stream_stat_defines.h"

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

Test_I_Stream::Test_I_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING))
 , MP4Decoder_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_4_DEFAULT_NAME_STRING))
 , MPEGTSDecoder_ (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_TS_DEFAULT_NAME_STRING))
 , defragment_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING))
 , splitter_ (this,
              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING))
#if defined (FFMPEG_SUPPORT)
 , decoder2_ (this,
              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING))
#endif // FFMPEG_SUPPORT
  //, report_ (this,
 //            ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 , resize_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING))
#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , Direct3D_ (this,
              ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING))
#else
 , X11_ (this,
         ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING))
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_SUPPORT)
 , GTKCairo_ (this,
              ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING))
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
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

  ACE_ASSERT (inherited::configuration_);
  inherited::CONFIGURATION_T::ITERATOR_T iterator/*, iterator_2*/;
  iterator = inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  //iterator_2 =
  //  const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
  //// sanity check(s)
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  //ACE_ASSERT (iterator_2 != configuration_in.end ());
  std::string file_extension =
    Common_File_Tools::fileExtension ((*iterator).second.second->fileIdentifier.identifier,
                                      false);
  bool is_mpeg_4 = !ACE_OS::strcmp (file_extension.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR ("mp4"));
  bool is_mpeg_ts = !ACE_OS::strcmp (file_extension.c_str (),
                                     ACE_TEXT_ALWAYS_CHAR ("ts"));


  layout_in->append (&source_, NULL, 0);
  if (is_mpeg_4)
    layout_in->append (&MP4Decoder_, NULL, 0);
  else if (is_mpeg_ts)
    layout_in->append (&MPEGTSDecoder_, NULL, 0);

  layout_in->append (&defragment_, NULL, 0);

#if defined(FFMPEG_SUPPORT)
  layout_in->append (&decoder2_, NULL, 0);
#endif // FFMPEG_SUPPORT

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  Stream_Branches_t branches_a;

  layout_in->append (&splitter_, NULL, 0);
  branch_p = &splitter_;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
  //  configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (splitter_.writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (branches_a);

  //layout_in->append (&report_, NULL, 0);
  layout_in->append (&resize_, branch_p, index_i);
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  layout_in->append (&GTKCairo_, branch_p, index_i);
#else
  layout_in->append (&X11_, branch_p, index_i);
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

  ++index_i;

  return true;
}

bool
Test_I_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
//  bool reset_setup_pipeline = false;
  Test_I_ImageSave_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator, iterator_2;
  std::string log_file_name;

  iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  // sanity check(s)
  ACE_ASSERT (iterator != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  iterator_2 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING));
  // sanity check(s)
  ACE_ASSERT (iterator_2 != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());
#endif // ACE_WIN32 || ACE_WIN64

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  // step2: update stream module configuration(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*iterator_2).second.second = (*iterator).second.second;
#endif // ACE_WIN32 || ACE_WIN64

  // ---------------------------------------------------------------------------
  // step3: allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
//  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
//  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration);

  session_data_p =
    &const_cast<Test_I_ImageSave_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  //if ((*iterator).second.second->direct3DConfiguration->handle)
  //{
  //  (*iterator).second.second->direct3DConfiguration->handle->AddRef ();
  //  session_data_p->direct3DDevice =
  //    (*iterator).second.second->direct3DConfiguration->handle;
  //} // end IF
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------
  // step5: update session data
  session_data_p->formats.push_back (configuration_in.configuration_->format);

  // ---------------------------------------------------------------------------

  // step7: assemble stream
  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  return false;
}
