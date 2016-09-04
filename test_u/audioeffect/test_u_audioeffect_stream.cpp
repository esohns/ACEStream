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

#include "test_u_audioeffect_stream.h"

#include "ace/Log_Msg.h"

//#include "common_file_tools.h"

#include "stream_macros.h"

#include "test_u_audioeffect_common_modules.h"

// initialize statics
#if defined (ACE_WIN32) || defined (ACE_WIN64)
ACE_Atomic_Op<ACE_Thread_Mutex,
              unsigned long> Test_U_AudioEffect_DirectShow_Stream::currentSessionID = 0;
ACE_Atomic_Op<ACE_Thread_Mutex,
              unsigned long> Test_U_AudioEffect_MediaFoundation_Stream::currentSessionID = 0;
#else
ACE_Atomic_Op<ACE_Thread_Mutex,
              unsigned long> Test_U_AudioEffect_Stream::currentSessionID = 0;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_AudioEffect_DirectShow_Stream::Test_U_AudioEffect_DirectShow_Stream ()
 : inherited (ACE_TEXT_ALWAYS_CHAR ("AudioEffectStream"))
 , graphBuilder_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::Test_U_AudioEffect_DirectShow_Stream"));

}

Test_U_AudioEffect_DirectShow_Stream::~Test_U_AudioEffect_DirectShow_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::~Test_U_AudioEffect_DirectShow_Stream"));

  if (graphBuilder_)
    graphBuilder_->Release ();

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_U_AudioEffect_DirectShow_Stream::load (Stream_ModuleList_t& modules_out,
                                            bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::load"));

  //// initialize return value(s)
  //modules_out.clear ();
  //delete_out = false;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_FileWriter_Module (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                                                                   NULL,
                                                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_WAVEncoder_Module (ACE_TEXT_ALWAYS_CHAR ("WAVEncoder"),
                                                                   NULL,
                                                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_RuntimeStatistic_Module (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                                                                         NULL,
                                                                         false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_DirectShow_Module (ACE_TEXT_ALWAYS_CHAR ("MicSource"),
                                                           NULL,
                                                           false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_DirectShow_Stream::initialize (const Test_U_AudioEffect_DirectShow_StreamConfiguration& configuration_in,
                                                  bool setupPipeline_in,
                                                  bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::initialize"));

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
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  Test_U_AudioEffect_DirectShow_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_DirectShow_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.sessionID =
    ++Test_U_AudioEffect_DirectShow_Stream::currentSessionID;
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->targetFileName;

  // ---------------------------------------------------------------------------
  // sanity check(s)
  //ACE_ASSERT (configuration_in.moduleConfiguration);

  //configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("MicSource")));
  ACE_ASSERT (module_p);
  Test_U_Dev_Mic_Source_DirectShow* source_impl_p =
    dynamic_cast<Test_U_Dev_Mic_Source_DirectShow*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Dev_Mic_Source_DirectShow> failed, aborting\n")));
    return false;
  } // end IF

  // ---------------------------------------------------------------------------

  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  bool COM_initialized = false;
  bool release_builder = false;
  HRESULT result = E_FAIL;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  std::list<std::wstring> filter_pipeline;
  IBaseFilter* filter_p = NULL;
  ISampleGrabber* isample_grabber_p = NULL;
  std::string log_file_name;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED     |
                            COINIT_DISABLE_OLE1DDE   |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  COM_initialized = true;

  if (configuration_in.moduleHandlerConfiguration->builder)
  {
    reference_count =
      configuration_in.moduleHandlerConfiguration->builder->AddRef ();
    graphBuilder_ = configuration_in.moduleHandlerConfiguration->builder;

    // *NOTE*: Stream_Module_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    if (!Stream_Module_Device_Tools::resetDeviceGraph (graphBuilder_,
                                                       CLSID_AudioInputDeviceCategory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::resetDeviceGraph(): \"%s\", aborting\n")));
      goto error;
    } // end IF

    if (!Stream_Module_Device_Tools::getBufferNegotiation (graphBuilder_,
                                                           MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_AUDIO,
                                                           buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getBufferNegotiation(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (buffer_negotiation_p);

    goto continue_;
  } // end IF

  if (!Stream_Module_Device_Tools::loadDeviceGraph (configuration_in.moduleHandlerConfiguration->device,
                                                    CLSID_AudioInputDeviceCategory,
                                                    graphBuilder_,
                                                    buffer_negotiation_p,
                                                    stream_config_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_in.moduleHandlerConfiguration->device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (stream_config_p);

  // clean up
  stream_config_p->Release ();
  stream_config_p = NULL;

  reference_count = graphBuilder_->AddRef ();
  configuration_in.moduleHandlerConfiguration->builder = graphBuilder_;
  release_builder = true;
  ACE_ASSERT (graphBuilder_);
  ACE_ASSERT (buffer_negotiation_p);

continue_:
  if (!Stream_Module_Device_Tools::setCaptureFormat (graphBuilder_,
                                                     CLSID_AudioInputDeviceCategory,
                                                     *configuration_in.moduleHandlerConfiguration->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("capture format: \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (*configuration_in.moduleHandlerConfiguration->format).c_str ())));

  log_file_name =
    Common_File_Tools::getLogDirectory (std::string (),
                                        0);
  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
  Stream_Module_Device_Tools::debug (graphBuilder_,
                                     log_file_name);
#endif

  if (!Stream_Module_Device_Tools::loadAudioRendererGraph (*configuration_in.moduleHandlerConfiguration->format,
                                                           configuration_in.moduleHandlerConfiguration->audioOutput,
                                                           graphBuilder_,
                                                           filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadAudioRendererGraph(), aborting\n")));
    goto error;
  } // end IF

  filter_pipeline.push_front (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_AUDIO);
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
  allocator_properties.cbAlign = -1; // <-- use default
  allocator_properties.cbBuffer = -1; // <-- use default
  allocator_properties.cbPrefix = -1; // <-- use default
  allocator_properties.cBuffers =
    MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
  result =
      buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  if (!Stream_Module_Device_Tools::connect (graphBuilder_,
                                            filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::connect(), aborting\n")));
    goto error;
  } // end IF
  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  //         --> reconnect the AVI decompressor to the (connected) sample
  //             grabber; this seems to work
  if (!Stream_Module_Device_Tools::connected (graphBuilder_,
                                              MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_AUDIO))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("reconnecting...\n")));

    if (!Stream_Module_Device_Tools::connectFirst (graphBuilder_,
                                                   MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_AUDIO))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::connectFirst(), aborting\n")));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (Stream_Module_Device_Tools::connected (graphBuilder_,
                                                     MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_AUDIO));

  // debug info
  // *TODO*: find out why this fails
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result =
      buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result)) // E_FAIL (0x80004005)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    //goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              allocator_properties.cBuffers,
              allocator_properties.cbBuffer,
              allocator_properties.cbAlign,
              allocator_properties.cbPrefix));
  buffer_negotiation_p->Release ();
  buffer_negotiation_p = NULL;

  result = graphBuilder_->QueryInterface (IID_PPV_ARGS (&media_filter_p));
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

  if (session_data_r.format)
    Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);
  ACE_ASSERT (!session_data_r.format);
  if (!Stream_Module_Device_Tools::getOutputFormat (graphBuilder_,
                                                    session_data_r.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (session_data_r.format);

  // ---------------------------------------------------------------------------

  if (!source_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                source_impl_p->mod_->name ()));
    return false;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

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

error:
  if (release_builder)
  {
    configuration_in.moduleHandlerConfiguration->builder->Release ();
    configuration_in.moduleHandlerConfiguration->builder = NULL;
  } // end IF
  if (session_data_r.format)
    Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);
  if (graphBuilder_)
  {
    graphBuilder_->Release ();
    graphBuilder_ = NULL;
  } // end IF

  if (COM_initialized)
    CoUninitialize ();

  return false;
}

bool
Test_U_AudioEffect_DirectShow_Stream::collect (Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  ACE_ASSERT (module_p);
  Test_U_AudioEffect_DirectShow_Statistic_WriterTask_t* runtimeStatistic_impl_p =
    dynamic_cast<Test_U_AudioEffect_DirectShow_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_AudioEffect_DirectShow_Statistic_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  Test_U_AudioEffect_DirectShow_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_DirectShow_SessionData&> (inherited::sessionData_->get ());
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
Test_U_AudioEffect_DirectShow_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::report"));

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

//////////////////////////////////////////

Test_U_AudioEffect_MediaFoundation_Stream::Test_U_AudioEffect_MediaFoundation_Stream ()
 : inherited (ACE_TEXT_ALWAYS_CHAR ("AudioEffectStream"))
 , mediaSession_ (NULL)
 , referenceCount_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::Test_U_AudioEffect_MediaFoundation_Stream"));

}

Test_U_AudioEffect_MediaFoundation_Stream::~Test_U_AudioEffect_MediaFoundation_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::~Test_U_AudioEffect_MediaFoundation_Stream"));

  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result) &&
      (result != MF_E_SHUTDOWN)) // already shut down...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
  } // end IF

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

const Stream_Module_t*
Test_U_AudioEffect_MediaFoundation_Stream::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::find"));

  //if (ACE_OS::strcmp (name_in.c_str (),
  //                    ACE_TEXT_ALWAYS_CHAR ("DisplayNull")) == 0)
  //  return const_cast<Test_U_AudioEffect_MediaFoundation_Module_DisplayNull_Module*> (&displayNull_);

  return inherited::find (name_in);
}
void
Test_U_AudioEffect_MediaFoundation_Stream::start ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::start"));

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

  inherited::start ();
}
void
Test_U_AudioEffect_MediaFoundation_Stream::stop (bool waitForCompletion_in,
                                                 bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::stop"));

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

bool
Test_U_AudioEffect_MediaFoundation_Stream::load (Stream_ModuleList_t& modules_out,
                                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::load"));

  //// initialize return value(s)
  //modules_out.clear ();
  //delete_out = false;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_FileWriter_Module (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                                                                        NULL,
                                                                        false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_WAVEncoder_Module (ACE_TEXT_ALWAYS_CHAR ("WAVEncoder"),
                                                                        NULL,
                                                                        false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_RuntimeStatistic_Module (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                                                                              NULL,
                                                                              false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_MediaFoundation_Module (ACE_TEXT_ALWAYS_CHAR ("MicSource"),
                                                                NULL,
                                                                false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_MediaFoundation_Stream::initialize (const Test_U_AudioEffect_MediaFoundation_StreamConfiguration& configuration_in,
                                                       bool setupPipeline_in,
                                                       bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::initialize"));

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
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  Test_U_AudioEffect_MediaFoundation_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_MediaFoundation_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.sessionID =
    ++Test_U_AudioEffect_MediaFoundation_Stream::currentSessionID;
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->targetFileName;

  // ---------------------------------------------------------------------------
  //// sanity check(s)
  //ACE_ASSERT (configuration_in.moduleConfiguration);
  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("MicSource")));
  ACE_ASSERT (module_p);
  Test_U_Dev_Mic_Source_MediaFoundation* source_impl_p =
    dynamic_cast<Test_U_Dev_Mic_Source_MediaFoundation*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Dev_Mic_Source_MediaFoundation> failed, aborting\n")));
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  IMFTopology* topology_p = NULL;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFMediaType* media_type_p = NULL;
  HRESULT result = E_FAIL;

  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  COM_initialized = true;

  if (configuration_in.moduleHandlerConfiguration->session)
  {
    ULONG reference_count =
      configuration_in.moduleHandlerConfiguration->session->AddRef ();
    mediaSession_ = configuration_in.moduleHandlerConfiguration->session;

    if (!Stream_Module_Device_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
      goto error;
    } // end IF

    // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
    //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
    //         --> (try to) wait for the next MESessionTopologySet event
    // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
    //         still fails with MF_E_INVALIDREQUEST)
    do
    {
      result = mediaSession_->GetFullTopology (flags,
                                               0,
                                               &topology_p);
    } while (result == MF_E_INVALIDREQUEST);
    if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (topology_p);

    if (configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId)
      goto continue_;
    if (!Stream_Module_Device_Tools::getSampleGrabberNodeId (topology_p,
                                                             configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId);

    goto continue_;
  } // end IF

  TOPOID renderer_node_id = 0;
  if (!Stream_Module_Device_Tools::loadAudioRendererTopology (configuration_in.moduleHandlerConfiguration->device,
                                                              configuration_in.moduleHandlerConfiguration->format,
                                                              source_impl_p,
                                                              configuration_in.moduleHandlerConfiguration->audioOutput,
                                                              configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
                                                              renderer_node_id,
                                                              topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadAudioRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_in.moduleHandlerConfiguration->device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;
#if defined (_DEBUG)
  Stream_Module_Device_Tools::dump (topology_p);
#endif
continue_:
  if (!Stream_Module_Device_Tools::setCaptureFormat (topology_p,
                                                     configuration_in.moduleHandlerConfiguration->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("capture format: \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (configuration_in.moduleHandlerConfiguration->format).c_str ())));
#endif

  if (session_data_r.format)
  {
    session_data_r.format->Release ();
    session_data_r.format = NULL;
  } // end IF
  ACE_ASSERT (!session_data_r.format);
  Stream_Module_Device_Tools::copyMediaType (configuration_in.moduleHandlerConfiguration->format,
                                             session_data_r.format);
  //if (!Stream_Module_Device_Tools::getOutputFormat (topology_p,
  //                                                  configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
  //                                                  media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (media_type_p);

  //HRESULT result = E_FAIL;
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

  ACE_ASSERT (!mediaSession_);
  if (!Stream_Module_Device_Tools::setTopology (topology_p,
                                                mediaSession_,
                                                true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setTopology(), aborting\n")));
    goto error;
  } // end IF
  topology_p->Release ();
  topology_p = NULL;
  ACE_ASSERT (mediaSession_);

  if (!configuration_in.moduleHandlerConfiguration->session)
  {
    ULONG reference_count = mediaSession_->AddRef ();
    configuration_in.moduleHandlerConfiguration->session = mediaSession_;
  } // end IF
#endif

  if (!source_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                source_impl_p->mod_->name ()));
    return false;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

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

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (session_data_r.format)
  {
    session_data_r.format->Release ();
    session_data_r.format = NULL;
  } // end IF
  //session_data_r.resetToken = 0;
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

bool
Test_U_AudioEffect_MediaFoundation_Stream::collect (Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  ACE_ASSERT (module_p);
  Test_U_AudioEffect_MediaFoundation_Statistic_WriterTask_t* runtimeStatistic_impl_p =
    dynamic_cast<Test_U_AudioEffect_MediaFoundation_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_AudioEffect_MediaFoundation_Statistic_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  Test_U_AudioEffect_MediaFoundation_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_MediaFoundation_SessionData&> (inherited::sessionData_->get ());
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
Test_U_AudioEffect_MediaFoundation_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::report"));

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

HRESULT
Test_U_AudioEffect_MediaFoundation_Stream::QueryInterface (const IID& IID_in,
                                                           void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (Test_U_AudioEffect_MediaFoundation_Stream, IMFAsyncCallback),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}
ULONG
Test_U_AudioEffect_MediaFoundation_Stream::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
ULONG
Test_U_AudioEffect_MediaFoundation_Stream::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  if (count == 0);
  //delete this;

  return count;
}
HRESULT
Test_U_AudioEffect_MediaFoundation_Stream::GetParameters (DWORD* flags_out,
                                                          DWORD* queue_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::GetParameters"));

  ACE_UNUSED_ARG (flags_out);
  ACE_UNUSED_ARG (queue_out);

  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  return E_NOTIMPL;
}
HRESULT
Test_U_AudioEffect_MediaFoundation_Stream::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::Invoke"));

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

  //Stream_CamSave_SessionData& session_data_r =
  //  const_cast<Stream_CamSave_SessionData&> (inherited::sessionData_->get ());

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
//continue_:
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
    MF_TOPOSTATUS topology_status =
      static_cast<MF_TOPOSTATUS> (attribute_value);
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
#else
Test_U_AudioEffect_Stream::Test_U_AudioEffect_Stream ()
 : inherited (ACE_TEXT_ALWAYS_CHAR ("AudioEffectStream"))
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::Test_U_AudioEffect_Stream"));

}

Test_U_AudioEffect_Stream::~Test_U_AudioEffect_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::~Test_U_AudioEffect_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_U_AudioEffect_Stream::load (Stream_ModuleList_t& modules_out,
                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::load"));

  //// initialize return value(s)
  //modules_out.clear ();
  //delete_out = false;

  Stream_Module_t* module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  Test_U_AudioEffect_Module_FileWriter_Module (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
//                                                               NULL,
//                                                               false),
//                  false);
//  modules_out.push_back (module_p);
//  module_p = NULL;
  // *NOTE*: currently, on UNIX systems, the WAV encoder writes the WAV file
  //         itself
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_WAVEncoder_Module (ACE_TEXT_ALWAYS_CHAR ("WAVEncoder"),
                                                        NULL,
                                                        false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_Vis_SpectrumAnalyzer_Module (ACE_TEXT_ALWAYS_CHAR ("SpectrumAnalyzer"),
                                                                  NULL,
                                                                  false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_Target_ALSA_Module (ACE_TEXT_ALWAYS_CHAR ("ALSAPlayback"),
                                                         NULL,
                                                         false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_Module_RuntimeStatistic_Module (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                                                                     NULL,
                                                                     false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_ALSA_Module (ACE_TEXT_ALWAYS_CHAR ("MicSource"),
                                                     NULL,
                                                     false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_Stream::initialize (const Test_U_AudioEffect_StreamConfiguration& configuration_in,
                                       bool setupPipeline_in,
                                       bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  ACE_ASSERT (configuration_in.moduleHandlerConfiguration->deviceHandle);
//  // *TODO*: remove type inference
//  if (!configuration_in.moduleHandlerConfiguration->format)
//  {
//    if (!Stream_Module_Device_Tools::getCaptureFormat (configuration_in.moduleHandlerConfiguration->deviceHandle,
//                                                       const_cast<Test_U_AudioEffect_StreamConfiguration&> (configuration_in).moduleHandlerConfiguration->format))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(): \"%m\", aborting\n")));
//      return false;
//    } // end IF
//  } // end IF
//  ACE_ASSERT (configuration_in.moduleHandlerConfiguration->format);

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
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  Test_U_AudioEffect_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.sessionID =
    ++Test_U_AudioEffect_Stream::currentSessionID;
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("MicSource")));
  ACE_ASSERT (module_p);
  Test_U_Dev_Mic_Source_ALSA* source_impl_p =
    dynamic_cast<Test_U_Dev_Mic_Source_ALSA*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Dev_Mic_Source_ALSA> failed, aborting\n")));
    return false;
  } // end IF
  if (!source_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                source_impl_p->mod_->name ()));
    return false;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

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
Test_U_AudioEffect_Stream::collect (Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  ACE_ASSERT (module_p);
  Test_U_AudioEffect_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
    dynamic_cast<Test_U_AudioEffect_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_AudioEffect_Module_Statistic_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  Test_U_AudioEffect_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_SessionData&> (inherited::sessionData_->get ());
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
Test_U_AudioEffect_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::report"));

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
#endif
