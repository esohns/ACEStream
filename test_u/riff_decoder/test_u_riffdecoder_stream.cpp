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

#include "test_u_riffdecoder_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_file_defines.h"
#include "stream_stat_defines.h"

Test_U_RIFFDecoder_Stream::Test_U_RIFFDecoder_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING))
 , decoder_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_AVI_DEFAULT_NAME_STRING))
 , statistic_ (this,
               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Test_U_RIFFDecoder_Stream::Test_U_RIFFDecoder_Stream"));

}

bool
Test_U_RIFFDecoder_Stream::load (Stream_ILayout* layout_inout,
                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_RIFFDecoder_Stream::load"));

  layout_inout->append (&source_, NULL, 0);
  layout_inout->append (&decoder_, NULL, 0);
//  layout_inout->append (&statistic_, NULL, 0);

  delete_out = false;

  return true;
}

bool
Test_U_RIFFDecoder_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_RIFFDecoder_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_U_RIFFDecoder_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  Test_U_SessionManager_t* session_manager_p =
    Test_U_SessionManager_t::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
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

  session_data_p =
    &const_cast<struct Test_U_RIFFDecoder_SessionData&> (session_manager_p->getR (inherited::id_));
  session_data_p->fileSize =
    Common_File_Tools::size (iterator->second.second->fileIdentifier.identifier);

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
