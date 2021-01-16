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

#include "test_u_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "test_u_common_modules.h"
#include "test_u_module_spreadsheetwriter.h"

Test_U_Stream::Test_U_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Stream::Test_u_Stream"));

}

Test_U_Stream::~Test_U_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Stream::~Test_u_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_U_Stream::load (Stream_ILayout* layout_inout,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Stream::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Source_Module (this,
                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_SpreadsheetWriter_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (MODULE_DOCUMENT_LIBREOFFICE_WRITER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

bool
Test_U_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

  Test_U_StreamConfiguration_t::ITERATOR_T iterator;
  struct Test_U_SessionData* session_data_p = NULL;
  struct Test_U_ModuleHandlerConfiguration* configuration_p = NULL;
  Stream_Module_t* module_p = NULL;
  Test_U_Source* source_impl_p = NULL;

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration->setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (default_stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
      &const_cast<struct Test_U_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  configuration_p =
    dynamic_cast<struct Test_U_ModuleHandlerConfiguration*> (&(*iterator).second.second);
  ACE_ASSERT (configuration_p);
  //session_data_p->targetFileName = configuration_p->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Source ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (default_stream_name_string_),
                ACE_TEXT (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)));
    goto failed;
  } // end IF

  source_impl_p = dynamic_cast<Test_U_Source*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_U_Source*> failed, aborting\n"),
                ACE_TEXT (default_stream_name_string_)));
    goto failed;
  } // end IF
  source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (default_stream_name_string_)));
      goto failed;
    } // end IF

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration->setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (default_stream_name_string_)));

  return false;
}
