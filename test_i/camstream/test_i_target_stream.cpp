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

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "test_i_source_stream.h"

Test_I_Target_Stream::Test_I_Target_Stream (const std::string& name_in)
 : inherited (name_in)
// , source_ (ACE_TEXT_ALWAYS_CHAR ("NetSource"),
//            NULL,
//            false)
// , decoder_ (ACE_TEXT_ALWAYS_CHAR ("AVIDecoder"),
//             NULL,
//             false)
 , splitter_ (ACE_TEXT_ALWAYS_CHAR ("Splitter"),
              NULL,
              false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , directShowSource_ (ACE_TEXT_ALWAYS_CHAR ("DirectShowSource"),
                      NULL,
                      false)
#endif
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL,
                      false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , graphBuilder_ (NULL)
#else
 , display_ (ACE_TEXT_ALWAYS_CHAR ("Display"),
             NULL,
             false)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::Test_I_Target_Stream"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that all modules which have NOT enqueued onto the
  //         stream (e.g. because initialize() failed...) need to be explicitly
  //         close()d
//  inherited::modules_.push_front (&source_);
//  inherited::modules_.push_front (&decoder_);
  inherited::modules_.push_front (&splitter_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited::modules_.push_front (&directShowSource_);
#endif
  inherited::modules_.push_front (&runtimeStatistic_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  inherited::modules_.push_front (&display_);
#endif

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

Test_I_Target_Stream::~Test_I_Target_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::~Test_I_Target_Stream"));

  inherited::shutdown ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (_DEBUG && graphBuilder_)
    Stream_Module_Device_Tools::debug (graphBuilder_,
                                       std::string ());

  if (graphBuilder_)
    graphBuilder_->Release ();
#endif
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
Test_I_Target_Stream::initialize (const Test_I_Target_StreamConfiguration& configuration_in,
                                  bool setupPipeline_in,
                                  bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::initialize"));

  bool result = true;

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  //if (inherited::isInitialized_)
  //{
  //  // *TODO*: move this to stream_base.inl ?
  //  int result_2 = -1;
  //  const inherited::MODULE_T* module_p = NULL;
  //  inherited::IMODULE_T* imodule_p = NULL;
  //  for (inherited::ITERATOR_T iterator (*this);
  //       (iterator.next (module_p) != 0);
  //       iterator.advance ())
  //  {
  //    if ((module_p == inherited::head ()) ||
  //        (module_p == inherited::tail ()))
  //      continue;

  //    // need a downcast...
  //    imodule_p =
  //      dynamic_cast<inherited::IMODULE_T*> (const_cast<inherited::MODULE_T*> (module_p));
  //    if (!imodule_p)
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
  //                  module_p->name ()));
  //      return false;
  //    } // end IF
  //    if (imodule_p->isFinal ())
  //    {
  //      //ACE_ASSERT (module_p == configuration_in.module);
  //      result_2 = inherited::remove (module_p->name (),
  //                                    ACE_Module_Base::M_DELETE_NONE);
  //      if (result_2 == -1)
  //      {
  //        ACE_DEBUG ((LM_ERROR,
  //                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", aborting\n"),
  //                    module_p->name ()));
  //        return false;
  //      } // end IF
  //      imodule_p->reset ();
  //      ACE_DEBUG ((LM_DEBUG,
  //                  ACE_TEXT ("\"%s\" removed from stream \"%s\"...\n"),
  //                  module_p->name (),
  //                  ACE_TEXT (name ().c_str ())));

  //      break; // done
  //    } // end IF
  //  } // end FOR

  //  if (inherited::sessionData_)
  //  {
  //    inherited::sessionData_->decrease ();
  //    inherited::sessionData_ = NULL;
  //  } // end IF
  //} // end IF

  // allocate a new session state, reset stream
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
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
  Test_I_Target_Stream_SessionData& session_data_r =
    const_cast<Test_I_Target_Stream_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.lock = &(inherited::lock_);
  inherited::state_.currentSessionData = &session_data_r;
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  session_data_r.format = configuration_in.moduleHandlerConfiguration->format;
#endif
  //session_data_r.fileName =
  //  configuration_in.moduleHandlerConfiguration->targetFileName;
  session_data_r.sessionID = configuration_in.sessionID;

  ACE_ASSERT (configuration_in.moduleConfiguration);
  //  configuration_in.moduleConfiguration.streamState = &state_;
  Test_I_Target_StreamConfiguration& configuration_r =
      const_cast<Test_I_Target_StreamConfiguration&> (configuration_in);
  configuration_r.moduleHandlerConfiguration->stateMachineLock =
    &inherited::state_.stateMachineLock;

  // ---------------------------------------------------------------------------

  if (configuration_in.module)
  {
    // *TODO*: (at least part of) this procedure belongs in libACEStream
    //         --> remove type inferences
    inherited::IMODULE_T* imodule_p =
        dynamic_cast<inherited::IMODULE_T*> (configuration_in.module);
    if (!imodule_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    if (!imodule_p->initialize (*configuration_in.moduleConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    imodule_p->reset ();
    inherited::IMODULEHANDLER_T* module_handler_p =
      dynamic_cast<inherited::IMODULEHANDLER_T*> (configuration_in.module->writer ());
    if (!module_handler_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Common_IInitialize_T<HandlerConfigurationType>> failed, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    if (!module_handler_p->initialize (*configuration_in.moduleHandlerConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    inherited::modules_.push_front (configuration_in.module);
  } // end IF

  // ---------------------------------------------------------------------------

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // ******************* Display ************************
  display_.initialize (*configuration_in.moduleConfiguration);
  Test_I_Target_Stream_Module_Display* display_impl_p =
    dynamic_cast<Test_I_Target_Stream_Module_Display*> (display_.writer ());
  if (!display_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Display> failed, aborting\n")));
    return false;
  } // end IF
  if (!display_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                display_.name ()));
    return false;
  } // end IF
#endif

  // ******************* Runtime Statistic *************************
  runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  Test_I_Target_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
      dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF
  if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval, // reporting interval
                                            true,                                        // push statistic messages
                                            configuration_in.printFinalReport,           // print final report ?
                                            configuration_in.messageAllocator))          // message allocator handle
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                runtimeStatistic_.name ()));
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // ******************* DirectShow Source ************************
  std::wstring filter_name;
  std::list<std::wstring> filter_pipeline;

  if (configuration_r.moduleHandlerConfiguration->builder)
    goto continue_;

  HRESULT result_2 = CoInitializeEx (NULL, COINIT_MULTITHREADED);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return false;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (!configuration_r.moduleHandlerConfiguration->builder);
  if (!Stream_Module_Device_Tools::loadTargetRendererGraph (configuration_r.moduleHandlerConfiguration->window,
                                                            configuration_r.moduleHandlerConfiguration->builder,
                                                            filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadTargetRendererGraph(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (configuration_r.moduleHandlerConfiguration->builder);

  if (_DEBUG)
  {
    std::string log_file_name =
      Common_File_Tools::getLogDirectory (std::string (),
                                          0);
    log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
    log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
    Stream_Module_Device_Tools::debug (configuration_r.moduleHandlerConfiguration->builder,
                                       log_file_name);
  } // end IF

  IBaseFilter* ibase_filter_p = NULL;
  //ACE_NEW_NORETURN (ibase_filter_p,
  //                  FILTER_T ());
  //if (!filter_p)
  //{
  //  ACE_DEBUG ((LM_CRITICAL,
  //              ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  //  return false;
  //} // end IF
  result_2 =
    CoCreateInstance (configuration_in.moduleHandlerConfiguration->filterCLSID, NULL,
                      CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                      (void**)&ibase_filter_p);
  if (FAILED (result_2))
  {
    OLECHAR GUID_string[39];
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    int nCount =
      StringFromGUID2 (configuration_in.moduleHandlerConfiguration->filterCLSID,
                       GUID_string, sizeof (GUID_string));
    ACE_ASSERT (nCount == 39);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (ibase_filter_p);
  Test_I_Target_Stream_Module_DirectShowSource::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Test_I_Target_Stream_Module_DirectShowSource::IINITIALIZE_T*> (ibase_filter_p);
  if (!iinitialize_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_IInitialize_T*>(%@), aborting\n"),
                ibase_filter_p));

    // clean up
    ibase_filter_p->Release ();

    goto error;
  } // end IF
  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_r.moduleHandlerConfiguration->filterConfiguration);
  if (!iinitialize_p->initialize (*configuration_in.moduleHandlerConfiguration->filterConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IInitialize_T::initialize(), aborting\n")));

    // clean up
    ibase_filter_p->Release ();

    goto error;
  } // end IF

  filter_name =
    (configuration_r.moduleHandlerConfiguration->push ? TEST_I_STREAM_MODULE_DIRECTSHOW_SOURCE_FILTER_NAME
                                                      : TEST_I_STREAM_MODULE_DIRECTSHOW_ASYNCH_SOURCE_FILTER_NAME);
  //ACE_TEXT_ALWAYS_WCHAR (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ());
  result_2 =
    configuration_r.moduleHandlerConfiguration->builder->AddFilter (ibase_filter_p,
                                                                    filter_name.c_str ());
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  filter_pipeline.push_front (filter_name);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ())));

  ACE_ASSERT (!filter_pipeline.empty ());
  if (!Stream_Module_Device_Tools::connect (configuration_r.moduleHandlerConfiguration->builder,
                                            filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::connect(), aborting\n")));
    goto error;
  } // end IF

  IMediaFilter* media_filter_p = NULL;
  result_2 =
    configuration_r.moduleHandlerConfiguration->builder->QueryInterface (IID_IMediaFilter,
                                                                         (void**)&media_filter_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result_2 = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release ();

  goto continue_;

error:
  // clean up
  configuration_r.moduleHandlerConfiguration->builder->Release ();
  configuration_r.moduleHandlerConfiguration->builder = NULL;

  return false;

continue_:
  directShowSource_.initialize (*configuration_in.moduleConfiguration);
  Test_I_Target_Stream_Module_DirectShowSource* directShowSource_impl_p =
    dynamic_cast<Test_I_Target_Stream_Module_DirectShowSource*> (directShowSource_.writer ());
  if (!directShowSource_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_DirectShowSource>(%@) failed, aborting\n"),
                directShowSource_.writer ()));
    return false;
  } // end IF
  if (!directShowSource_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                directShowSource_.name ()));
    return false;
  } // end IF
#endif

  // ************************ Splitter *****************************
  splitter_.initialize (*configuration_in.moduleConfiguration);
  Test_I_Target_Stream_Module_Splitter* splitter_impl_p =
    dynamic_cast<Test_I_Target_Stream_Module_Splitter*> (splitter_.writer ());
  if (!splitter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Splitter> failed, aborting\n")));
    return false;
  } // end IF
  if (!splitter_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                splitter_.name ()));
    return false;
  } // end IF
  if (!splitter_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                splitter_.name ()));
    return false;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  splitter_.arg (inherited::sessionData_);

//  // ************************ AVI Decoder *****************************
//  decoder_.initialize (*configuration_in.moduleConfiguration);
//  Test_I_Target_Stream_Module_AVIDecoder* decoder_impl_p =
//    dynamic_cast<Test_I_Target_Stream_Module_AVIDecoder*> (decoder_.writer ());
//  if (!decoder_impl_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_AVIDecoder> failed, aborting\n")));
//    return false;
//  } // end IF
//  if (!decoder_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
//                decoder_.name ()));
//    return false;
//  } // end IF
//  result_2 = inherited::push (&decoder_);
//  if (result_2 == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                decoder_.name ()));
//    return false;
//  } // end IF

//  // ******************* Net Reader ***********************
//  source_.initialize (*configuration_in.moduleConfiguration);
//  Test_I_Target_Stream_Module_Net_Writer_t* source_impl_p =
//    dynamic_cast<Test_I_Target_Stream_Module_Net_Writer_t*> (source_.writer ());
//  if (!source_impl_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Net_Writer_T> failed, aborting\n")));
//    return false;
//  } // end IF
//  if (!source_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
//                source_.name ()));
//    return false;
//  } // end IF
//  if (!source_impl_p->initialize (inherited::state_))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
//                source_.name ()));
//    return false;
//  } // end IF
////  netReader_impl_p->reset ();
//  // *NOTE*: push()ing the module will open() it
//  //         --> set the argument that is passed along (head module expects a
//  //             handle to the session data)
//  source_.arg (inherited::sessionData_);
//  result_2 = inherited::push (&source_);
//  if (result_2 == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                source_.name ()));
//    return false;
//  } // end IF

  // -------------------------------------------------------------

  if (setupPipeline_in)
    if (!inherited::setup (configuration_in.notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // set (session) message allocator
  inherited::allocator_ = configuration_in.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;

  return result;
}

bool
Test_I_Target_Stream::collect (Test_I_RuntimeStatistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Target_Stream_SessionData& session_data_r =
        const_cast<Test_I_Target_Stream_SessionData&> (inherited::sessionData_->get ());

  Test_I_Target_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_t> failed, aborting\n")));
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
