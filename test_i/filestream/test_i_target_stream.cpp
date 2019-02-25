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
#include "test_i_target_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "test_i_source_stream.h"

Test_I_Target_Stream::Test_I_Target_Stream ()
 : inherited ()
 , netIO_ (this,
           ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING))
 , statisticReport_ (this,
                     ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 , fileWriter_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::Test_I_Target_Stream"));

}

Test_I_Target_Stream::~Test_I_Target_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::~Test_I_Target_Stream"));

  inherited::shutdown ();
}

bool
Test_I_Target_Stream::load (Stream_ModuleList_t& modules_out,
                            bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::load"));

  // initialize return value(s)
//  modules_out.clear ();
  delete_out = false;

  modules_out.push_back (&fileWriter_);
  modules_out.push_back (&statisticReport_);
  modules_out.push_back (&netIO_);

  return true;
}

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_Target_Stream::initialize (const CONFIGURATION_T& configuration_in,
#else
Test_I_Target_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
#endif
                                  ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  inherited::CONFIGURATION_T::CONST_ITERATOR_T iterator;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  struct Test_I_Target_SessionData* session_data_p = NULL;
  bool reset_setup_pipeline = false;
  Test_I_Target_Module_Net_Writer_t* netIO_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
      &const_cast<struct Test_I_Target_SessionData&> (inherited::sessionData_->getR ());
  iterator = configuration_in.find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());

  // *TODO*: remove type inferences
  //  session_data_r.fileName =
  //    configuration_in.moduleHandlerConfiguration->fileName;
  session_data_p->targetFileName = (*iterator).second.second.targetFileName;

  // things to be done here:
  // [- initialize base class]
  // ------------------------------------
  // - initialize modules
  // ------------------------------------
  // - push them onto the stream (tail-first) !
  // ------------------------------------

//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  // ******************* Net IO ***********************
  netIO_impl_p =
    dynamic_cast<Test_I_Target_Module_Net_Writer_t*> (netIO_.writer ());
  if (!netIO_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_Writer_T> failed, aborting\n")));
    goto error;
  } // end IF
  netIO_impl_p->setP (&(inherited::state_));
//  netIO_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  netIO_.arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------

  inherited::isInitialized_ = true;

  result = true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;

  return result;
}

bool
Test_I_Target_Stream::collect (Test_I_Statistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  bool release_lock = false;

  struct Test_I_Target_SessionData& session_data_r =
        const_cast<struct Test_I_Target_SessionData&> (inherited::sessionData_->getR ());

  Test_I_Target_Module_Statistic_WriterTask_t* statistic_impl_p =
    dynamic_cast<Test_I_Target_Module_Statistic_WriterTask_t*> (statisticReport_.writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
    release_lock = true;
  } // end IF

  session_data_r.statistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic_T::collect(), continuing\n")));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IStatistic_T::collect(), aborting\n")));
  else
    session_data_r.statistic = data_out;

  if (release_lock)
  {
    ACE_ASSERT (session_data_r.lock);
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result_2;
}

void
Test_I_Target_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::report"));

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
//   // delegate to this module...
//   return (runtimeStatistic_impl->report ());

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
