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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename ConnectorType>
Test_I_Source_Stream_T<ConnectorType>::Test_I_Source_Stream_T (const std::string& name_in)
 : inherited (name_in)
 , source_ (ACE_TEXT_ALWAYS_CHAR ("CamSource"),
            NULL,
            false)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL,
                      false)
 , netTarget_ (ACE_TEXT_ALWAYS_CHAR ("NetTarget"),
               NULL,
               false)
 , display_ (ACE_TEXT_ALWAYS_CHAR ("Display"),
             NULL,
             false)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::Test_I_Source_Stream_T"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that all modules which have NOT enqueued onto the
  //         stream (e.g. because initialize() failed...) need to be explicitly
  //         close()d
  inherited::modules_.push_front (&source_);
  inherited::modules_.push_front (&runtimeStatistic_);
  inherited::modules_.push_front (&netTarget_);
  inherited::modules_.push_front (&display_);

  // *TODO* fix ACE bug: modules should initialize their "next" member to NULL
  //inherited::MODULE_T* module_p = NULL;
  //for (ACE_DLList_Iterator<inherited::MODULE_T> iterator (inherited::availableModules_);
  //     iterator.next (module_p);
  //     iterator.advance ())
  //  module_p->next (NULL);
  for (inherited::MODULE_CONTAINER_ITERATOR_T iterator = inherited::modules_.begin ();
       iterator != inherited::modules_.end ();
       iterator++)
     (*iterator)->next (NULL);
}

template <typename ConnectorType>
Test_I_Source_Stream_T<ConnectorType>::~Test_I_Source_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::~Test_I_Source_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

template <typename ConnectorType>
void
Test_I_Source_Stream_T<ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectorType>::initialize (const Test_I_Source_StreamConfiguration& configuration_in,
                                                   bool setupPipeline_in,
                                                   bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  if (inherited::isInitialized_)
  {
    // *TODO*: move this to stream_base.inl ?
    const inherited::MODULE_T* module_p = NULL;
    inherited::IMODULE_T* imodule_p = NULL;
    for (inherited::ITERATOR_T iterator (*this);
         (iterator.next (module_p) != 0);
         iterator.advance ())
    {
      if ((module_p == inherited::head ()) ||
          (module_p == inherited::tail ()))
        continue;

      // need a downcast...
      imodule_p =
        dynamic_cast<inherited::IMODULE_T*> (const_cast<inherited::MODULE_T*> (module_p));
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                    module_p->name ()));
        return false;
      } // end IF
      if (imodule_p->isFinal ())
      {
        //ACE_ASSERT (module_p == configuration_in.module);
        result = inherited::remove (module_p->name (),
                                    ACE_Module_Base::M_DELETE_NONE);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", aborting\n"),
                      module_p->name ()));
          return false;
        } // end IF
        imodule_p->reset ();

        break; // done
      } // end IF
    } // end FOR

    if (!inherited::finalize ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Base_T::finalize(): \"%m\", continuing\n")));
  } // end IF

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
  ACE_ASSERT (inherited::sessionData_);

  // things to be done here:
  // [- initialize base class]
  // ------------------------------------
  // - initialize notification strategy (if any)
  // ------------------------------------
  // - push the final module onto the stream (if any)
  // ------------------------------------
  // - initialize modules
  // - push them onto the stream (tail-first) !
  // ------------------------------------

  //inherited::MODULE_T* module_p = NULL;
  //if (configuration_in.notificationStrategy)
  //{
  //  module_p = inherited::head ();
  //  if (!module_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("no head module found, aborting\n")));
  //    return false;
  //  } // end IF
  //  inherited::TASK_T* task_p = module_p->reader ();
  //  if (!task_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("no head module reader task found, aborting\n")));
  //    return false;
  //  } // end IF
  //  inherited::QUEUE_T* queue_p = task_p->msg_queue ();
  //  if (!queue_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("no head module reader task queue found, aborting\n")));
  //    return false;
  //  } // end IF
  //  queue_p->notification_strategy (configuration_in.notificationStrategy);
  //} // end IF
//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------
  ACE_ASSERT (configuration_in.moduleConfiguration);
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

//  if (configuration_in.module)
//  {
//    // *TODO*: (at least part of) this procedure belongs in libACEStream
//    //         --> remove type inferences
//    typename inherited::IMODULE_T* imodule_p = NULL;
//    try
//    {
//      imodule_p =
//          dynamic_cast<typename inherited::IMODULE_T*> (configuration_in.module);
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: caught exception in dynamic_cast<Stream_IModule_T>(%@), aborting\n"),
//                  configuration_in.module->name (),
//                  configuration_in.module));
//      imodule_p = NULL;
//    }
//    if (!imodule_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
//                  configuration_in.module->name ()));
//      return false;
//    } // end IF
//    if (!imodule_p->initialize (*configuration_in.moduleConfiguration))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to initialize module, aborting\n"),
//                  configuration_in.module->name ()));
//      return false;
//    } // end IF
//    imodule_p->reset ();
//    inherited::IMODULEHANDLER_T* module_handler_p =
//      dynamic_cast<inherited::IMODULEHANDLER_T*> (configuration_in.module->writer ());
//    if (!module_handler_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: dynamic_cast<Common_IInitialize_T<HandlerConfigurationType>> failed, aborting\n"),
//                  configuration_in.module->name ()));
//      return false;
//    } // end IF
//    if (!module_handler_p->initialize (*configuration_in.moduleHandlerConfiguration))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to initialize module writer, aborting\n"),
//                  configuration_in.module->name ()));
//      return false;
//    } // end IF
//    inherited::modules_.push_front (configuration_in.module);
//  } // end IF

  // ---------------------------------------------------------------------------

  Test_I_Source_Stream_Module_Display* display_impl_p = NULL;
  WRITER_T* netTarget_impl_p = NULL;
  Test_I_Source_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p = NULL;
  Test_I_Stream_Module_CamSource* source_impl_p = NULL;
  //Test_I_Source_Stream_SessionData* session_data_p = NULL;

  // ******************* Display ************************
  display_.initialize (*configuration_in.moduleConfiguration);
  display_impl_p =
    dynamic_cast<Test_I_Source_Stream_Module_Display*> (display_.writer ());
  if (!display_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_Stream_Module_Display> failed, aborting\n")));
    return false;
  } // end IF
  if (!display_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                display_.name ()));
    return false;
  } // end IF

  // ******************* Net Target ************************
  netTarget_.initialize (*configuration_in.moduleConfiguration);
  netTarget_impl_p = dynamic_cast<WRITER_T*> (netTarget_.writer ());
  if (!netTarget_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_Net_Target_T> failed, aborting\n")));
    goto failed;
  } // end IF
  if (!netTarget_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                netTarget_.name ()));
    goto failed;
  } // end IF

  // ******************* Runtime Statistic ************************
  runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  runtimeStatistic_impl_p =
      dynamic_cast<Test_I_Source_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_Stream_Module_RuntimeStatistic> failed, aborting\n")));
    goto failed;
  } // end IF
  if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval, // reporting interval (seconds)
                                            true,                                        // push 1-second interval statistic messages downstream ?
                                            configuration_in.printFinalReport,           // print final report ?
                                            configuration_in.messageAllocator))          // message allocator handle
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                runtimeStatistic_.name ()));
    goto failed;
  } // end IF

  // ******************* Camera Source ************************
  source_.initialize (*configuration_in.moduleConfiguration);
  source_impl_p =
    dynamic_cast<Test_I_Stream_Module_CamSource*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_CamSource> failed, aborting\n")));
    goto failed;
  } // end IF
  if (!source_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module writer, aborting\n"),
                source_.name ()));
    goto failed;
  } // end IF
  if (!source_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module writer, aborting\n"),
                source_.name ()));
    goto failed;
  } // end IF
  //fileReader_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  source_.arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_p =
  //    &const_cast<Test_I_Source_Stream_SessionData&> (inherited::sessionData_->get ());
  //session_data_p->fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_p->size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // set (session) message allocator
  inherited::allocator_ = configuration_in.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

failed:
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::reset(): \"%m\", continuing\n")));

  return false;
}

template <typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectorType>::collect (Test_I_RuntimeStatistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Source_Stream_SessionData& session_data_r =
      const_cast<Test_I_Source_Stream_SessionData&> (inherited::sessionData_->get ());

  Test_I_Source_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_I_Source_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_Stream_Module_Statistic_WriterTask_t> failed, aborting\n")));
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
  try
  {
    result_2 = runtimeStatistic_impl->collect (data_out);
  }
  catch (...)
  {
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

template <typename ConnectorType>
void
Test_I_Source_Stream_T<ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::report"));

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
