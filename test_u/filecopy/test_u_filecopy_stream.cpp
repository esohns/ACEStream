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

#include "ace/Synch.h"
#include "test_u_filecopy_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Stream_Filecopy_Stream::Stream_Filecopy_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::Stream_Filecopy_Stream"));

}

Stream_Filecopy_Stream::~Stream_Filecopy_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::~Stream_Filecopy_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Stream_Filecopy_Stream::load (Stream_ILayout* layout_inout,
                              bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Stream_Filecopy_FileReader_Module (this,
                                                     ACE_TEXT_ALWAYS_CHAR ("FileReader")),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Stream_Filecopy_StatisticReport_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Stream_Filecopy_FileWriter_Module (this,
                                                     ACE_TEXT_ALWAYS_CHAR ("FileWriter")),
                  false);
  layout_inout->append (module_p, NULL, 0);

  delete_out = true;

  return true;
}

bool
Stream_Filecopy_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
  struct Stream_Filecopy_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator;
  Stream_Filecopy_FileReader* fileReader_impl_p = NULL;
  Stream_Module_t* module_p = NULL;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
    &const_cast<struct Stream_Filecopy_SessionData&> (inherited::sessionData_->getR ());
  iterator = inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  // *TODO*: remove type inferences
  session_data_p->fileName = (*iterator).second.second.fileIdentifier.identifier;
  session_data_p->size =
    Common_File_Tools::size ((*iterator).second.second.fileIdentifier.identifier);
  session_data_p->targetFileName = (*iterator).second.second.targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* File Reader ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("FileReader")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("FileReader")));
    return false;
  } // end IF
  fileReader_impl_p =
    dynamic_cast<Stream_Filecopy_FileReader*> (module_p->writer ());
  if (!fileReader_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_Filecopy_FileReader> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  fileReader_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (inherited::configuration_->configuration_.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;

  return false;
}
