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

//  int result = -1;

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  //if (inherited::isInitialized_)
  //{
  //  // *TODO*: move this to stream_base.inl ?
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
  //      result = inherited::remove (module_p->name (),
  //                                  ACE_Module_Base::M_DELETE_NONE);
  //      if (result == -1)
  //      {
  //        ACE_DEBUG ((LM_ERROR,
  //                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", aborting\n"),
  //                    module_p->name ()));
  //        return false;
  //      } // end IF
  //      imodule_p->reset ();

  //      break; // done
  //    } // end IF
  //  } // end FOR

  //  if (!inherited::finalize ())
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to Stream_Base_T::finalize(): \"%m\", continuing\n")));
  //} // end IF

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Source_Stream_SessionData& session_data_r =
    const_cast<Test_I_Source_Stream_SessionData&> (inherited::sessionData_->get ());
#endif

//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleConfiguration);
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

  if (configuration_in.module)
  {
    // *TODO*: (at least part of) this procedure belongs in libACEStream
    //         --> remove type inferences
    typename inherited::IMODULE_T* imodule_p = NULL;
    try
    {
      imodule_p =
          dynamic_cast<typename inherited::IMODULE_T*> (configuration_in.module);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in dynamic_cast<Stream_IModule_T>(%@), aborting\n"),
                  configuration_in.module->name (),
                  configuration_in.module));
      imodule_p = NULL;
    }
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
                  ACE_TEXT ("%s: failed to initialize module writer, aborting\n"),
                  configuration_in.module->name ()));
      return false;
    } // end IF
    inherited::modules_.push_front (configuration_in.module);
  } // end IF

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
    return false;
  } // end IF
  if (!netTarget_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                netTarget_.name ()));
    return false;
  } // end IF

  // ******************* Runtime Statistic ************************
  runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  runtimeStatistic_impl_p =
      dynamic_cast<Test_I_Source_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_Stream_Module_RuntimeStatistic> failed, aborting\n")));
    return false;
  } // end IF
  if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval, // reporting interval (seconds)
                                            true,                                        // push 1-second interval statistic messages downstream ?
                                            configuration_in.printFinalReport,           // print final report ?
                                            configuration_in.messageAllocator))          // message allocator handle
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                runtimeStatistic_.name ()));
    return false;
  } // end IF

  // ******************* Camera Source ************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::list<std::wstring> filter_pipeline;
  bool graph_loaded = false;
  bool COM_initialized = false;
  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;

  if (configuration_in.moduleHandlerConfiguration->builder)
  {
    // *NOTE*: Stream_Module_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    //if (!Stream_Module_Device_Tools::resetDeviceGraph (configuration_in.moduleHandlerConfiguration->builder))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Stream_Module_Device_Tools::resetDeviceGraph(): \"%s\", aborting\n")));
    //  return false;
    //} // end IF

    if (!Stream_Module_Device_Tools::getBufferNegotiation (configuration_in.moduleHandlerConfiguration->builder,
                                                           buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getBufferNegotiation(): \"%s\", aborting\n")));
      return false;
    } // end IF
    ACE_ASSERT (buffer_negotiation_p);

    goto continue_;
  } // end IF
  else
  {
    HRESULT result_2 = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      return false;
    } // end IF
    COM_initialized = true;

    // sanity check(s)
    ACE_ASSERT (!configuration_in.moduleHandlerConfiguration->builder);
    IAMStreamConfig* stream_config_p = NULL;
    if (!Stream_Module_Device_Tools::loadDeviceGraph (configuration_in.moduleHandlerConfiguration->device,
                                                      configuration_in.moduleHandlerConfiguration->builder,
                                                      buffer_negotiation_p,
                                                      stream_config_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                  ACE_TEXT (configuration_in.moduleHandlerConfiguration->device.c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (configuration_in.moduleHandlerConfiguration->builder);
    ACE_ASSERT (buffer_negotiation_p);
    ACE_ASSERT (stream_config_p);
    graph_loaded = true;

    // clean up
    stream_config_p->Release ();

    if (_DEBUG)
    {
      std::string log_file_name =
        Common_File_Tools::getLogDirectory (std::string (),
                                            0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
      Stream_Module_Device_Tools::debug (configuration_in.moduleHandlerConfiguration->builder,
                                         log_file_name);
    } // end IF
  } // end IF

continue_:
  ACE_ASSERT (!session_data_r.mediaType);
  IMediaFilter* media_filter_p = NULL;

  if (!Stream_Module_Device_Tools::getCaptureFormat (configuration_in.moduleHandlerConfiguration->builder,
                                                     session_data_r.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (session_data_r.mediaType);
  if (!Stream_Module_Device_Tools::loadRendererGraph (*session_data_r.mediaType,
                                                      configuration_in.moduleHandlerConfiguration->window,
                                                      configuration_in.moduleHandlerConfiguration->builder,
                                                      filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererGraph(), aborting\n")));

    // clean up
    Stream_Module_Device_Tools::deleteMediaType (session_data_r.mediaType);
    session_data_r.mediaType = NULL;

    goto error;
  } // end IF
  Stream_Module_Device_Tools::deleteMediaType (session_data_r.mediaType);
  session_data_r.mediaType = NULL;
  filter_pipeline.push_front (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO);

  IBaseFilter* filter_p = NULL;
  result =
    configuration_in.moduleHandlerConfiguration->builder->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB,
                                                                            &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  ISampleGrabber* isample_grabber_p = NULL;
  result = filter_p->QueryInterface (IID_ISampleGrabber,
                                     (void**)&isample_grabber_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (isample_grabber_p);
  filter_p->Release ();
  filter_p = NULL;

  result = isample_grabber_p->SetBufferSamples (false);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
#endif

  source_impl_p =
    dynamic_cast<Test_I_Stream_Module_CamSource*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_CamSource> failed, aborting\n")));
    goto error;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = isample_grabber_p->SetCallback (source_impl_p, 0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  isample_grabber_p->Release ();
  isample_grabber_p = NULL;

  ACE_ASSERT (buffer_negotiation_p);
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  allocator_properties.cbAlign = 1;
  allocator_properties.cbBuffer = 0;
  allocator_properties.cbPrefix = 0;
  allocator_properties.cBuffers =
    MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
  result = buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  if (!Stream_Module_Device_Tools::connect (configuration_in.moduleHandlerConfiguration->builder,
                                            filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::connect(), aborting\n")));
    goto error;
  } // end IF

    // debug info
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result = buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              allocator_properties.cBuffers,
              allocator_properties.cbBuffer,
              allocator_properties.cbAlign,
              allocator_properties.cbPrefix));
  buffer_negotiation_p->Release ();
  buffer_negotiation_p = NULL;

  ACE_ASSERT (!session_data_r.mediaType);
  if (!Stream_Module_Device_Tools::getOutputFormat (configuration_in.moduleHandlerConfiguration->builder,
    session_data_r.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (session_data_r.mediaType);

  result =
    configuration_in.moduleHandlerConfiguration->builder->QueryInterface (IID_IMediaFilter,
                                                                          (void**)&media_filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release ();
  media_filter_p = NULL;
#endif

  source_.initialize (*configuration_in.moduleConfiguration);
  if (!source_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module writer, aborting\n"),
                source_.name ()));
    goto error;
  } // end IF
  if (!source_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize module writer, aborting\n"),
                source_.name ()));
    goto error;
  } // end IF
  //fileReader_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  source_.arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      goto error;
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (COM_initialized)
    CoUninitialize ();
#endif

  return true;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  if (isample_grabber_p)
    isample_grabber_p->Release ();
  if (filter_p)
    filter_p->Release ();
  if (media_filter_p)
    media_filter_p->Release ();
  if (graph_loaded)
    if (configuration_in.moduleHandlerConfiguration->builder)
    {
      configuration_in.moduleHandlerConfiguration->builder->Release ();
      configuration_in.moduleHandlerConfiguration->builder = NULL;
    } // end IF
  if (session_data_r.mediaType)
  {
    Stream_Module_Device_Tools::deleteMediaType (session_data_r.mediaType);
    session_data_r.mediaType = NULL;
  } // end IF

  if (COM_initialized)
    CoUninitialize ();
#endif

  return false;
}

template <typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectorType>::collect (Test_I_Source_Stream_StatisticData& data_out)
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
