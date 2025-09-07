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

#include "test_u_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "test_u_common_modules.h"
#if defined (LIBREOFFICE_SUPPORT)
#include "test_u_module_libreoffice_spreadsheetwriter.h"
#endif // LIBREOFFICE_SUPPORT

Test_U_Stream::Test_U_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Stream::Test_u_Stream"));

}

bool
Test_U_Stream::load (Stream_ILayout* layout_inout,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Stream::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Source_Module (this,
                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SOURCE_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
#if defined (LIBREOFFICE_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_U_LibreOffice_SpreadsheetWriter_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_DOCUMENT_LIBREOFFICE_WRITER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
#endif // LIBREOFFICE_SUPPORT

  delete_out = true;

  return true;
}

bool
Test_U_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
//  struct Test_U_SessionData* session_data_p = NULL;
//  Stream_Module_t* module_p = NULL;
//  Test_U_Source* source_impl_p = NULL;

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (default_stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  //ACE_ASSERT (inherited::sessionData_);
//  session_data_p =
//      &const_cast<struct Test_U_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Source ************************
  //module_p =
  //  const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SOURCE_DEFAULT_NAME_STRING)));
  //if (!module_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
  //              ACE_TEXT (default_stream_name_string_),
  //              ACE_TEXT (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)));
  //  goto failed;
  //} // end IF

  //source_impl_p = dynamic_cast<Test_U_Source*> (module_p->writer ());
  //if (!source_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: dynamic_cast<Test_U_Source*> failed, aborting\n"),
  //              ACE_TEXT (default_stream_name_string_)));
  //  goto failed;
  //} // end IF
  //source_impl_p->setP (&(inherited::state_));

  //// *NOTE*: push()ing the module will open() it
  ////         --> set the argument that is passed along (head module expects a
  ////             handle to the session data)
  //module_p->arg (inherited::sessionData_);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (default_stream_name_string_)));
      goto failed;
    } // end IF

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (default_stream_name_string_)));

  return false;
}
