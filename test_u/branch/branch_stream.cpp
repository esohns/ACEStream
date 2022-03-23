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

#include "branch_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "branch_common_modules.h"

Branch_Stream::Branch_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Branch_Stream::Branch_Stream"));

}

bool
Branch_Stream::load (Stream_ILayout* layout_inout,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Branch_Stream::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Branch_Source_Module (this,
                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Branch_StatisticReport_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Branch_Distributor_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  branch_p = module_p;
  inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR ("1"));
  inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR ("2"));
  inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR ("3"));
  Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  idistributor_p->initialize (inherited::configuration_->configuration_->branches);
  module_p = NULL;
  ACE_ASSERT (inherited::configuration_->configuration_->module);
  Common_IClone_T<ACE_Module<ACE_MT_SYNCH,
                             Common_TimePolicy_t> >* iclone_p =
      dynamic_cast<Common_IClone_T<ACE_Module<ACE_MT_SYNCH,
                                              Common_TimePolicy_t> >*> (inherited::configuration_->configuration_->module);
  if (unlikely (!iclone_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Common_IClone_T> failed, aborting\n")));
    return false;
  } // end IF
  module_p = iclone_p->clone ();
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, branch_p, 1);
  module_p = NULL;
  module_p = iclone_p->clone ();
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, branch_p, 2);
  module_p = NULL;
  module_p = iclone_p->clone ();
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, branch_p, 3);
  module_p = NULL;

  delete_out = true;

  return true;
}

bool
Branch_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_Stream::initialize"));

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  //inherited::dump_state ();

  return inherited::initialize (configuration_in);
}

//////////////////////////////////////////

Branch_Stream_2::Branch_Stream_2 ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Branch_Stream_2::Branch_Stream_2"));

}

bool
Branch_Stream_2::load (Stream_ILayout* layout_inout,
                       bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Branch_Stream_2::load"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Branch_Source_Module (this,
                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);

  ACE_ASSERT (inherited::configuration_->configuration_->module_2);
  layout_inout->append (inherited::configuration_->configuration_->module_2, NULL, 0);

  delete_out = true;

  return true;
}

bool
Branch_Stream_2::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_Stream_2::initialize"));

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  Branch_SessionData* session_data_p = NULL;

   // sanity check(s)
  ACE_ASSERT (configuration_in.configuration_);
  ACE_ASSERT (configuration_in.configuration_->module_2);

  // ---------------------------------------------------------------------------
  // step1: allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    true;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p = &const_cast<Branch_SessionData&> (inherited::sessionData_->getR ());  
  session_data_p->stream = this;
  // session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  if (!inherited::setup (NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

  inherited::dump_state ();

  return true;

error:
  return false;
}

void
Branch_Stream_2::onSessionEnd (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_Stream_2::onSessionEnd"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: session end (id: %u)\n"),
              ACE_TEXT (stream_name_string_),
              sessionId_in));
}
