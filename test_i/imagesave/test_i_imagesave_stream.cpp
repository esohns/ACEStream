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
  , decoder_ (this,
              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING))
  //, report_ (this,
 //           ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , display_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING))
#else
 , display_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING))
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::Test_I_Stream"));

}

Test_I_Stream::~Test_I_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::~Test_I_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_I_Stream::load (Stream_ILayout* layout_in,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  ACE_ASSERT (inherited::configuration_);
  //inherited::CONFIGURATION_T::ITERATOR_T iterator, iterator_2;
  //iterator =
  //  const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  //iterator_2 =
  //  const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
  //// sanity check(s)
  //ACE_ASSERT (iterator != configuration_in.end ());
  //ACE_ASSERT (iterator_2 != configuration_in.end ());

  // *NOTE*: one problem is that any module that was NOT enqueued onto the
  //         stream (e.g. because initialize() failed) needs to be explicitly
  //         close()d

  layout_in->append (&source_, NULL, 0);
  layout_in->append (&decoder_, NULL, 0);
  //layout_in->append (&report_, NULL, 0);
  layout_in->append (&display_, NULL, 0);

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
  Test_I_Source* source_impl_p = NULL;
  std::string log_file_name;
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;

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
  //ACE_ASSERT ((*iterator).second.second.direct3DConfiguration);

  session_data_p =
    &const_cast<Test_I_ImageSave_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  //if ((*iterator).second.second.direct3DConfiguration->handle)
  //{
  //  (*iterator).second.second.direct3DConfiguration->handle->AddRef ();
  //  session_data_p->direct3DDevice =
  //    (*iterator).second.second.direct3DConfiguration->handle;
  //} // end IF
  session_data_p->targetFileName = (*iterator).second.second.targetFileName;

  // ---------------------------------------------------------------------------
  // step4: initialize module(s)

  // ******************* Camera Source ************************
  source_impl_p =
    dynamic_cast<Test_I_Source*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_I_Source> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  // ---------------------------------------------------------------------------
  // step5: update session data
  session_data_p->formats.push_front (configuration_in.configuration_->format);

  // ---------------------------------------------------------------------------
  // step6: initialize head module
  source_impl_p->setP (&(inherited::state_));
  //fileReader_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  source_.arg (inherited::sessionData_);

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

  //// *TODO*: remove type inferences
  //session_data_r.fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  return false;
}
