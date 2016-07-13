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

#include "test_i_target_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "test_i_source_stream.h"

Test_I_Target_Stream::Test_I_Target_Stream (const std::string& name_in)
 : inherited (name_in)
 , netIO_ (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
           NULL,
           false)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL,
                      false)
 , fileWriter_ (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                NULL,
                false)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::Test_I_Target_Stream"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that all modules which have NOT enqueued onto the
  //         stream (e.g. because initialize() failed...) need to be explicitly
  //         close()d
  //inherited::modules_.push_front (&netIO_);
  //inherited::modules_.push_front (&runtimeStatistic_);
  //inherited::modules_.push_front (&fileWriter_);

  //// *TODO* fix ACE bug: modules should initialize their "next" member to NULL
  ////inherited::MODULE_T* module_p = NULL;
  ////for (ACE_DLList_Iterator<inherited::MODULE_T> iterator (inherited::availableModules_);
  ////     iterator.next (module_p);
  ////     iterator.advance ())
  ////  module_p->next (NULL);
  //for (Stream_ModuleListIterator_t iterator = inherited::modules_.begin ();
  //     iterator != inherited::modules_.end ();
  //     iterator++)
  //   (*iterator)->next (NULL);

  // *TODO*: these really shouldn't be necessary
  inherited::head ()->next (inherited::tail ());
  ACE_ASSERT (inherited::head ()->next () == inherited::tail ());
  inherited::tail ()->next (NULL);
  ACE_ASSERT (inherited::tail ()->next () == NULL);
}

Test_I_Target_Stream::~Test_I_Target_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::~Test_I_Target_Stream"));

  inherited::shutdown ();
}

void
Test_I_Target_Stream::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

bool
Test_I_Target_Stream::load (Stream_ModuleList_t& modules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::load"));

  //  // initialize return value(s)
  //  for (Stream_ModuleListIterator_t iterator = modules_out.begin ();
  //       iterator != modules_out.end ();
  //       iterator++)
  //    delete *iterator;
  //  modules_out.clear ();

  modules_out.push_back (&fileWriter_);
  modules_out.push_back (&runtimeStatistic_);
  modules_out.push_back (&netIO_);

  return true;
}

bool
Test_I_Target_Stream::initialize (const Test_I_Stream_Configuration& configuration_in,
                                  bool setupPipeline_in,
                                  bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in,
                              false,
                              resetSessionData_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name ().c_str ())));
    return false;
  } // end IF
  if (!inherited::sessionData_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocate session data, aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  // *TODO*: remove type inferences
  Test_I_Stream_SessionData& session_data_r =
      const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());
//  session_data_r.fileName =
//    configuration_in.moduleHandlerConfiguration->fileName;
  session_data_r.sessionID = configuration_in.sessionID;
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->fileName;

  // things to be done here:
  // [- initialize base class]
  // ------------------------------------
  // - initialize modules
  // ------------------------------------
  // - push them onto the stream (tail-first) !
  // ------------------------------------

//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------
  ACE_ASSERT (configuration_in.moduleConfiguration);
  //ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

  //if (configuration_in.module)
  //{
  //  // *TODO*: (at least part of) this procedure belongs in libACEStream
  //  //         --> remove type inferences
  //  inherited::IMODULE_T* imodule_p =
  //      dynamic_cast<inherited::IMODULE_T*> (configuration_in.module);
  //  if (!imodule_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
  //                configuration_in.module->name ()));
  //    return false;
  //  } // end IF
  //  if (!imodule_p->initialize (*configuration_in.moduleConfiguration))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to initialize module, aborting\n"),
  //                configuration_in.module->name ()));
  //    return false;
  //  } // end IF
  //  imodule_p->reset ();
  //  Stream_Task_t* task_p = configuration_in.module->writer ();
  //  ACE_ASSERT (task_p);
  //  inherited::MODULEHANDLER_IINITIALIZE_T* iinitialize_p =
  //    dynamic_cast<inherited::MODULEHANDLER_IINITIALIZE_T*> (task_p);
  //  if (!iinitialize_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: dynamic_cast<Common_IInitialize_T<HandlerConfigurationType>> failed, aborting\n"),
  //                configuration_in.module->name ()));
  //    return false;
  //  } // end IF
  //  //if (!iinitialize_p->initialize (*configuration_in.moduleHandlerConfiguration))
  //  //{
  //  //  ACE_DEBUG ((LM_ERROR,
  //  //              ACE_TEXT ("%s: failed to initialize module handler, aborting\n"),
  //  //              configuration_in.module->name ()));
  //  //  return false;
  //  //} // end IF
  //} // end IF

  // ---------------------------------------------------------------------------

  // ******************* File Writer ************************
  //fileWriter_.initialize (*configuration_in.moduleConfiguration);
  //Test_I_Module_FileWriter* fileWriter_impl_p =
  //  dynamic_cast<Test_I_Module_FileWriter*> (fileWriter_.writer ());
  //if (!fileWriter_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Test_I_Module_FileWriter> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!fileWriter_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              fileWriter_.name ()));
  //  return false;
  //} // end IF

  // ******************* Runtime Statistics ************************
  //runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  //Test_I_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
  //    dynamic_cast<Test_I_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  //if (!runtimeStatistic_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Test_I_Module_RuntimeStatistic> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval, // reporting interval
  //                                          true,                                        // push statistic messages
  //                                          configuration_in.printFinalReport,           // print final report ?
  //                                          configuration_in.messageAllocator))          // message allocator handle
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              runtimeStatistic_.name ()));
  //  return false;
  //} // end IF

  // ******************* Net IO ***********************
  //netIO_.initialize (*configuration_in.moduleConfiguration);
  Test_I_Stream_Module_Net_Writer_t* netIO_impl_p =
    dynamic_cast<Test_I_Stream_Module_Net_Writer_t*> (netIO_.writer ());
  if (!netIO_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_Writer_T> failed, aborting\n")));
    return false;
  } // end IF
  if (!netIO_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                netIO_.name ()));
    return false;
  } // end IF
  if (!netIO_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                netIO_.name ()));
    return false;
  } // end IF
//  netIO_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  netIO_.arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  // OK: all went well
  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;
}

bool
Test_I_Target_Stream::collect (Test_I_RuntimeStatistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Stream_SessionData& session_data_r =
        const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());

  Test_I_Target_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_I_Target_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
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
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistics module...
  bool result_2 = false;
  try {
    result_2 = runtimeStatistic_impl->collect (data_out);
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
