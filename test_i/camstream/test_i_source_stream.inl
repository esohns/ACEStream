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
 , displayNull_ (ACE_TEXT_ALWAYS_CHAR ("DisplayNull"),
                 NULL,
                 false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaSession_ (NULL)
 , referenceCount_ (1)
#endif
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
  //inherited::modules_.push_front (&displayNull_);

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

  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
  } // end IF

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename ConnectorType>
Stream_Module_t*
Test_I_Source_Stream_T<ConnectorType>::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::find"));

  if (ACE_OS::strcmp (name_in.c_str (),
                      ACE_TEXT_ALWAYS_CHAR ("DisplayNull")) == 0)
    return const_cast<Test_I_Source_Stream_Module_DisplayNull_Module*> (&displayNull_);

  return inherited::find (name_in);
}
template <typename ConnectorType>
void
Test_I_Source_Stream_T<ConnectorType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::start"));

  inherited::start ();

  // sanity check(s)
  ACE_ASSERT (mediaSession_);

  struct _GUID GUID_s = GUID_NULL;
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  HRESULT result = mediaSession_->Start (&GUID_s,      // time format
    &property_s); // start position
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::Start(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    PropVariantClear (&property_s);

    return;
  } // end IF
  PropVariantClear (&property_s);

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
}
template <typename ConnectorType>
void
Test_I_Source_Stream_T<ConnectorType>::stop (bool waitForCompletion_in,
                                             bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::stop"));

  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF

  inherited::stop (waitForCompletion_in,
    lockedAccess_in);
}

template <typename ConnectorType>
HRESULT
Test_I_Source_Stream_T<ConnectorType>::QueryInterface (const IID& IID_in,
                                                       void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (OWN_TYPE_T, IMFAsyncCallback),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}
template <typename ConnectorType>
ULONG
Test_I_Source_Stream_T<ConnectorType>::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
template <typename ConnectorType>
ULONG
Test_I_Source_Stream_T<ConnectorType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  if (count == 0);
  //delete this;

  return count;
}
template <typename ConnectorType>
HRESULT
Test_I_Source_Stream_T<ConnectorType>::GetParameters (DWORD* flags_out,
                                                      DWORD* queue_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::GetParameters"));

  ACE_UNUSED_ARG (flags_out);
  ACE_UNUSED_ARG (queue_out);

  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  return E_NOTIMPL;
}
template <typename ConnectorType>
HRESULT
Test_I_Source_Stream_T<ConnectorType>::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::Invoke"));

  HRESULT result = E_FAIL;
  IMFMediaEvent* media_event_p = NULL;
  MediaEventType event_type = MEUnknown;
  HRESULT status = E_FAIL;
  struct tagPROPVARIANT value;
  PropVariantInit (&value);

  // sanity check(s)
  ACE_ASSERT (result_in);
  ACE_ASSERT (mediaSession_);
  ACE_ASSERT (inherited::sessionData_);

  Test_I_Source_Stream_SessionData& session_data_r =
    const_cast<Test_I_Source_Stream_SessionData&> (inherited::sessionData_->get ());

  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = media_event_p->GetType (&event_type);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetStatus (&status);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetValue (&value);
  ACE_ASSERT (SUCCEEDED (result));
  switch (event_type)
  {
  case MEEndOfPresentation:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MEEndOfPresentation...\n")));
    break;
  }
  case MEError:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("received MEError: \"%s\"\n"),
                ACE_TEXT (Common_Tools::error2String (status).c_str ())));
    break;
  }
  case MESessionClosed:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionClosed, shutting down...\n")));
    //IMFMediaSource* media_source_p = NULL;
    //if (!Stream_Module_Device_Tools::getMediaSource (mediaSession_,
    //                                                 media_source_p))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), continuing\n")));
    //  goto continue_;
    //} // end IF
    //ACE_ASSERT (media_source_p);
    //result = media_source_p->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSource::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    //media_source_p->Release ();
continue_:
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    break;
  }
  case MESessionEnded:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionEnded, closing sesion...\n")));
    result = mediaSession_->Close ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    break;
  }
  case MESessionCapabilitiesChanged:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionCapabilitiesChanged...\n")));
    break;
  }
  case MESessionNotifyPresentationTime:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionNotifyPresentationTime...\n")));
    break;
  }
  case MESessionStarted:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionStarted...\n")));
    break;
  }
  case MESessionStopped:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionStopped, stopping...\n")));
    if (isRunning ())
      stop (false,
            true);
    break;
  }
  case MESessionTopologySet:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionTopologySet (status was: \"%s\")...\n"),
                ACE_TEXT (Common_Tools::error2String (status).c_str ())));
    break;
  }
  case MESessionTopologyStatus:
  {
    UINT32 attribute_value = 0;
    result = media_event_p->GetUINT32 (MF_EVENT_TOPOLOGY_STATUS,
                                       &attribute_value);
    ACE_ASSERT (SUCCEEDED (result));
    enum MF_TOPOSTATUS topology_status =
      static_cast<enum MF_TOPOSTATUS> (attribute_value);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionTopologyStatus: \"%s\"...\n"),
                ACE_TEXT (Stream_Module_Device_Tools::topologyStatusToString (topology_status).c_str ())));
    break;
  }
  default:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("received unknown/invalid media session event (type was: %d), continuing\n"),
                event_type));
    break;
  }
  } // end SWITCH
  PropVariantClear (&value);
  media_event_p->Release ();

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  return S_OK;

error:
  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}
#endif

template <typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectorType>::initialize (const Test_I_Source_StreamConfiguration& configuration_in,
                                                   bool setupPipeline_in,
                                                   bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::initialize"));

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
  source_impl_p =
    dynamic_cast<Test_I_Stream_Module_CamSource*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Module_CamSource> failed, aborting\n")));
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result = E_FAIL;
  IMFTopology* topology_p = NULL;

  result = CoInitializeEx (NULL,
                            (COINIT_MULTITHREADED    |
                            COINIT_DISABLE_OLE1DDE  |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  COM_initialized = true;

  if (!Stream_Module_Device_Tools::loadRendererTopology (configuration_in.moduleHandlerConfiguration->device,
                                                          configuration_in.moduleHandlerConfiguration->format,
                                                          source_impl_p,
                                                          NULL,
                                                          //configuration_in.moduleHandlerConfiguration->window,
                                                          configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
                                                          session_data_r.rendererNodeId,
                                                          topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_in.moduleHandlerConfiguration->device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;

  if (!Stream_Module_Device_Tools::setCaptureFormat (topology_p,
                                                      configuration_in.moduleHandlerConfiguration->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
  if (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("capture format: \"%s\"...\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (configuration_in.moduleHandlerConfiguration->format).c_str ())));

  if (session_data_r.format)
  {
    session_data_r.format->Release ();
    session_data_r.format = NULL;
  } // end IF
  ACE_ASSERT (!session_data_r.format);
  if (!Stream_Module_Device_Tools::getOutputFormat (topology_p,
                                                    configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
                                                    session_data_r.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (session_data_r.format);

  if (mediaSession_)
  {
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
    mediaSession_ = NULL;
  } // end IF
  ULONG reference_count = 0;
  if (configuration_in.moduleHandlerConfiguration->session)
  {
    reference_count =
      configuration_in.moduleHandlerConfiguration->session->AddRef ();
    mediaSession_ = configuration_in.moduleHandlerConfiguration->session;

    if (!Stream_Module_Device_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end IF

  if (!Stream_Module_Device_Tools::setTopology (topology_p,
                                                mediaSession_,
                                                true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setTopology(), aborting\n")));
    goto error;
  } // end IF
  topology_p->Release ();

  if (!configuration_in.moduleHandlerConfiguration->session)
  {
    reference_count = mediaSession_->AddRef ();
    configuration_in.moduleHandlerConfiguration->session = mediaSession_;
  } // end IF
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
  if (topology_p)
  {
    Stream_Module_Device_Tools::dump (topology_p);
    topology_p->Release ();
  } // end IF
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release ();
    session_data_r.direct3DDevice = NULL;
  } // end IF
  if (session_data_r.format)
  {
    //Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);
    session_data_r.format->Release ();
    session_data_r.format = NULL;
  } // end IF
  session_data_r.resetToken = 0;
  if (session_data_r.session)
  {
    session_data_r.session->Release ();
    session_data_r.session = NULL;
  } // end IF
  if (mediaSession_)
  {
    mediaSession_->Release ();
    mediaSession_ = NULL;
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
