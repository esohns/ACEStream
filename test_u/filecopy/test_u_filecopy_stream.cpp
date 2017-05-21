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

#include <ace/Synch.h>
#include "test_u_filecopy_stream.h"

#include <ace/Log_Msg.h>

#include "stream_macros.h"

// initialize statics
ACE_Atomic_Op<ACE_Thread_Mutex,
              unsigned long> Stream_Filecopy_Stream::currentSessionID = 0;

Stream_Filecopy_Stream::Stream_Filecopy_Stream ()
 : inherited (ACE_TEXT_ALWAYS_CHAR ("FileCopyStream"))
 , fileReader_ (ACE_TEXT_ALWAYS_CHAR ("FileReader"),
                NULL,
                false)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL,
                      false)
 , fileWriter_ (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                NULL,
                false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::Stream_Filecopy_Stream"));

}

Stream_Filecopy_Stream::~Stream_Filecopy_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::~Stream_Filecopy_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Stream_Filecopy_Stream::load (Stream_ModuleList_t& modules_out,
                              bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::load"));

  // initialize return value(s)
  modules_out.clear ();
  delete_out = false;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Stream_Filecopy_FileReader_Module (ACE_TEXT_ALWAYS_CHAR ("FileReader"),
                                                     NULL,
                                                     false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Stream_Filecopy_StatisticReport_Module (ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                          NULL,
                                                          false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Stream_Filecopy_FileWriter_Module (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                                                     NULL,
                                                     false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Stream_Filecopy_Stream::initialize (const struct Stream_Filecopy_StreamConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.setupPipeline;
  bool reset_setup_pipeline = false;
  struct Stream_Filecopy_SessionData* session_data_p = NULL;
  Stream_Filecopy_ModuleHandlerConfigurationsIterator_t iterator;
  Stream_Filecopy_FileReader* fileReader_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<struct Stream_Filecopy_StreamConfiguration&> (configuration_in).setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  const_cast<struct Stream_Filecopy_StreamConfiguration&> (configuration_in).setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<struct Stream_Filecopy_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_p->sessionID = ++Stream_Filecopy_Stream::currentSessionID;
  iterator =
      const_cast<struct Stream_Filecopy_StreamConfiguration&> (configuration_in).moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.moduleHandlerConfigurations.end ());
  session_data_p->fileName = (*iterator).second->fileName;
  session_data_p->size = Common_File_Tools::size ((*iterator).second->fileName);

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  // ******************* File Reader ************************
  fileReader_impl_p =
    dynamic_cast<Stream_Filecopy_FileReader*> (fileReader_.writer ());
  if (!fileReader_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_Filecopy_FileReader> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  fileReader_impl_p->set (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  fileReader_.arg (inherited::sessionData_);

  if (configuration_in.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<struct Stream_Filecopy_StreamConfiguration&> (configuration_in).setupPipeline =
      setup_pipeline;

  return false;
}

bool
Stream_Filecopy_Stream::collect (Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_Filecopy_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
    dynamic_cast<Stream_Filecopy_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Filecopy_Module_Statistic_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  Stream_Filecopy_SessionData& session_data_r =
    const_cast<Stream_Filecopy_SessionData&> (inherited::sessionData_->get ());
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = runtimeStatistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic_T::collect(), continuing\n")));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IStatistic_T::collect(), aborting\n")));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result_2;
}

void
Stream_Filecopy_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_Stream::report"));

//   Net_Module_Statistic_ReaderTask_t* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<Net_Module_Statistic_ReaderTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Net_Module_Statistic_ReaderTask_t> failed, returning\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module
//   return (runtimeStatistic_impl->report ());

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
