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
#include "test_u_audioeffect_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif

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

  // sanity check(s)
  //ACE_ASSERT (inherited::configuration_);
  //ACE_ASSERT (inherited::configuration_->moduleHandlerConfiguration);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_FileWriter_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                                                                   NULL,
                                                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_WAVEncoder_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR ("WAVEncoder"),
                                                                   NULL,
                                                                   false),
                  false);
  modules_out.push_back (module_p);
  //if (inherited::configuration_->moduleHandlerConfiguration->GdkWindow2D ||
  //    inherited::configuration_->moduleHandlerConfiguration->GdkGLContext)
  //{
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_DirectShow_Vis_SpectrumAnalyzer_Module (this,
                                                                               ACE_TEXT_ALWAYS_CHAR ("SpectrumAnalyzer"),
                                                                               NULL,
                                                                               false),
                    false);
    modules_out.push_back (module_p);
  //} // end IF
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_StatisticAnalysis_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR ("StatisticAnalysis"),
                                                                          NULL,
                                                                          false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_StatisticReport_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                                        NULL,
                                                                        false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_DirectShow_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR ("MicSource"),
                                                           NULL,
                                                           false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_DirectShow_Stream::initialize (const struct Test_U_AudioEffect_DirectShow_StreamConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<struct Test_U_AudioEffect_DirectShow_StreamConfiguration&> (configuration_in).setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF
  const_cast<struct Test_U_AudioEffect_DirectShow_StreamConfiguration&> (configuration_in).setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  struct Test_U_AudioEffect_DirectShow_SessionData& session_data_r =
    const_cast<struct Test_U_AudioEffect_DirectShow_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.sessionID =
    ++Test_U_AudioEffect_DirectShow_Stream::currentSessionID;
  // sanity check(s)
  Test_U_AudioEffect_DirectShow_ModuleHandlerConfigurationsIterator_t iterator =
    const_cast<struct Test_U_AudioEffect_DirectShow_StreamConfiguration&> (configuration_in).moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.moduleHandlerConfigurations.end ());
  session_data_r.targetFileName = (*iterator).second.targetFileName;

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
                ACE_TEXT ("%s: dynamic_cast<Test_U_Dev_Mic_Source_DirectShow> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF

  // ---------------------------------------------------------------------------

  // sanity check(s)
  ACE_ASSERT (configuration_in.allocatorConfiguration);

  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  //bool COM_initialized = false;
  bool release_builder = false;
  HRESULT result_2 = E_FAIL;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  Stream_Module_Device_DirectShow_Graph_t graph_configuration;
  struct Stream_Module_Device_DirectShow_GraphEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  ISampleGrabber* isample_grabber_p = NULL;
  std::string log_file_name;
  IAMGraphStreams* graph_streams_p = NULL;

  //result = CoInitializeEx (NULL,
  //                         (COINIT_MULTITHREADED     |
  //                          COINIT_DISABLE_OLE1DDE   |
  //                          COINIT_SPEED_OVER_MEMORY));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return false;
  //} // end IF
  //COM_initialized = true;

  if ((*iterator).second.builder)
  {
    reference_count = (*iterator).second.builder->AddRef ();
    graphBuilder_ = (*iterator).second.builder;

    // *NOTE*: Stream_Module_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    if (!Stream_Module_Device_DirectShow_Tools::resetGraph (graphBuilder_,
                                                            CLSID_AudioInputDeviceCategory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::resetGraph(): \"%s\", aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF

    if (!Stream_Module_Device_DirectShow_Tools::getBufferNegotiation (graphBuilder_,
                                                                      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO,
                                                                      buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::getBufferNegotiation(), aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (buffer_negotiation_p);

    goto continue_;
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::loadDeviceGraph ((*iterator).second.device,
                                                               CLSID_AudioInputDeviceCategory,
                                                               graphBuilder_,
                                                               buffer_negotiation_p,
                                                               stream_config_p,
                                                               const_cast<Test_U_AudioEffect_DirectShow_StreamConfiguration&> (configuration_in).filterGraphConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT ((*iterator).second.device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (stream_config_p);

  // clean up
  stream_config_p->Release ();
  stream_config_p = NULL;

  reference_count = graphBuilder_->AddRef ();
  (*iterator).second.builder = graphBuilder_;
  release_builder = true;
  ACE_ASSERT (graphBuilder_);
  ACE_ASSERT (buffer_negotiation_p);

continue_:
  //if (!Stream_Module_Device_Tools::setCaptureFormat (graphBuilder_,
  //                                                   CLSID_AudioInputDeviceCategory,
  //                                                   *configuration_in.moduleHandlerConfiguration->format))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
#if defined (_DEBUG)
  log_file_name =
    Common_File_Tools::getLogDirectory (std::string (),
                                        0);
  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
  Stream_Module_Device_DirectShow_Tools::debug (graphBuilder_,
                                                log_file_name);
#endif

  if (!Stream_Module_Device_DirectShow_Tools::loadAudioRendererGraph (*(*iterator).second.format,
                                                                      ((*iterator).second.mute ? -1
                                                                                               : (*iterator).second.audioOutput),
                                                                      graphBuilder_,
                                                                      (*iterator).second.effect,
                                                                      (*iterator).second.effectOptions,
                                                                      graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::loadAudioRendererGraph(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF

  graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*(*iterator).second.format,
                                                             graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  graph_configuration.push_front (graph_entry);
  result_2 =
    (*iterator).second.builder->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB,
                                                   &filter_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                       (void**)&isample_grabber_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (isample_grabber_p);
  filter_p->Release ();
  filter_p = NULL;

  result_2 = isample_grabber_p->SetBufferSamples (false);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 = isample_grabber_p->SetCallback (source_impl_p, 0);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  isample_grabber_p->Release ();
  isample_grabber_p = NULL;

  ACE_ASSERT (buffer_negotiation_p);
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  allocator_properties.cbAlign = 1;
  //allocator_properties.cbAlign = -1; // <-- use default
  allocator_properties.cbBuffer =
    configuration_in.allocatorConfiguration->defaultBufferSize;
  allocator_properties.cbPrefix = -1; // <-- use default
  allocator_properties.cBuffers =
    MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
  result_2 =
      buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::connect (graphBuilder_,
                                                       graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::connect(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  //         --> reconnect the AVI decompressor to the (connected) sample
  //             grabber; this seems to work
  if (!Stream_Module_Device_DirectShow_Tools::connected (graphBuilder_,
                                                         MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reconnecting\n"),
                ACE_TEXT (inherited::name_.c_str ())));

    if (!Stream_Module_Device_DirectShow_Tools::connectFirst (graphBuilder_,
                                                              MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::connectFirst(), aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (Stream_Module_Device_DirectShow_Tools::connected (graphBuilder_,
                                                                MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO));

  // debug info
  // *TODO*: find out why this fails
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result_2 =
      buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result_2)) // E_FAIL (0x80004005)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    //goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              ACE_TEXT (inherited::name_.c_str ()),
              allocator_properties.cBuffers,
              allocator_properties.cbBuffer,
              allocator_properties.cbAlign,
              allocator_properties.cbPrefix));
  buffer_negotiation_p->Release ();
  buffer_negotiation_p = NULL;

  result_2 = graphBuilder_->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result_2 = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release ();
  media_filter_p = NULL;

  result_2 = graphBuilder_->QueryInterface (IID_PPV_ARGS (&graph_streams_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IAMGraphStreams): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graph_streams_p);
  result_2 = graph_streams_p->SyncUsingStreamOffset (FALSE);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMGraphStreams::SyncUsingStreamOffset(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  graph_streams_p->Release ();
  graph_streams_p = NULL;

  if (session_data_r.format)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_r.format);
  ACE_ASSERT (!session_data_r.format);
  if (!Stream_Module_Device_DirectShow_Tools::getOutputFormat (graphBuilder_,
                                                               MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB,
                                                               session_data_r.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::getOutputFormat(\"%s\"), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));
    goto error;
  } // end IF
  ACE_ASSERT (session_data_r.format);

  // ---------------------------------------------------------------------------

  source_impl_p->set (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (!inherited::setup ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<struct Test_U_AudioEffect_DirectShow_StreamConfiguration&> (configuration_in).setupPipeline =
      setup_pipeline;
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  if (stream_config_p)
    stream_config_p->Release ();
  if (media_filter_p)
    media_filter_p->Release ();
  if (filter_p)
    filter_p->Release ();
  if (isample_grabber_p)
    isample_grabber_p->Release ();
  if (graph_streams_p)
    graph_streams_p->Release ();

  if (release_builder)
  {
    (*iterator).second.builder->Release ();
    (*iterator).second.builder = NULL;
  } // end IF
  if (graphBuilder_)
  {
    graphBuilder_->Release ();
    graphBuilder_ = NULL;
  } // end IF

  if (session_data_r.format)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_r.format);

  //if (COM_initialized)
  //  CoUninitialize ();

  return false;
}

bool
Test_U_AudioEffect_DirectShow_Stream::collect (Test_U_AudioEffect_RuntimeStatistic& data_out)
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
                ACE_TEXT ("%s: dynamic_cast<Test_U_AudioEffect_DirectShow_Statistic_WriterTask_T> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
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
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
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
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
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
                  ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
  } // end IF

  // *NOTE*: this implements an ordered shutdown on destruction
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
                ACE_TEXT ("%s: failed to IMFMediaSession::Start(): \"%s\", returning\n"),
                ACE_TEXT (inherited::name_.c_str ()),
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
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (inherited::name_.c_str ()),
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
                  ACE_TEXT ("%s: failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ()),
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  //ACE_ASSERT (inherited::configuration_->moduleHandlerConfiguration);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_FileWriter_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                                                                        NULL,
                                                                        false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_WAVEncoder_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR ("WAVEncoder"),
                                                                        NULL,
                                                                        false),
                  false);
  modules_out.push_back (module_p);
  //if (inherited::configuration_->moduleHandlerConfiguration->GdkWindow2D ||
  //    inherited::configuration_->moduleHandlerConfiguration->GdkGLContext)
  //{
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_Vis_SpectrumAnalyzer_Module (this,
                                                                                    ACE_TEXT_ALWAYS_CHAR ("SpectrumAnalyzer"),
                                                                                    NULL,
                                                                                    false),
                    false);
    modules_out.push_back (module_p);
  //} // end IF
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_StatisticAnalysis_Module (this,
                                                                               ACE_TEXT_ALWAYS_CHAR ("StatisticAnalysis"),
                                                                               NULL,
                                                                               false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_StatisticReport_Module (this,
                                                                             ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                                             NULL,
                                                                             false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_MediaFoundation_Module (this,
                                                                ACE_TEXT_ALWAYS_CHAR ("MicSource"),
                                                                NULL,
                                                                false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_MediaFoundation_Stream::initialize (const struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration&> (configuration_in).setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF
  const_cast<struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration&> (configuration_in).setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  struct Test_U_AudioEffect_MediaFoundation_SessionData& session_data_r =
    const_cast<struct Test_U_AudioEffect_MediaFoundation_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.sessionID =
    ++Test_U_AudioEffect_MediaFoundation_Stream::currentSessionID;
  Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfigurationsIterator_t iterator =
    const_cast<struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration&> (configuration_in).moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.moduleHandlerConfigurations.end ());
  session_data_r.targetFileName = (*iterator).second.targetFileName;

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
                ACE_TEXT ("%s: dynamic_cast<Test_U_Dev_Mic_Source_MediaFoundation> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
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
  ULONG reference_count = 0;

  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  COM_initialized = true;

  if (mediaSession_)
  {
    mediaSession_->Release ();
    mediaSession_ = NULL;
  } // end IF

  if ((*iterator).second.session)
  {
    reference_count = (*iterator).second.session->AddRef ();
    mediaSession_ = (*iterator).second.session;

    if (!Stream_Module_Device_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF

    // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
    //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
    //         --> (try to) wait for the next MESessionTopologySet event
    // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
    //         still fails with MF_E_INVALIDREQUEST)
    do
    {
      result_2 = mediaSession_->GetFullTopology (flags,
                                                 0,
                                                 &topology_p);
    } while (result_2 == MF_E_INVALIDREQUEST);
    if (FAILED (result_2)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (topology_p);

    if ((*iterator).second.sampleGrabberNodeId)
      goto continue_;
    if (!Stream_Module_Device_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                             (*iterator).second.sampleGrabberNodeId))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT ((*iterator).second.sampleGrabberNodeId);

    goto continue_;
  } // end IF

  TOPOID renderer_node_id = 0;
  if (!Stream_Module_Device_MediaFoundation_Tools::loadAudioRendererTopology ((*iterator).second.device,
                                                                              (*iterator).second.format,
                                                                              source_impl_p,
                                                                              ((*iterator).second.mute ? -1
                                                                                                        : (*iterator).second.audioOutput),
                                                                              (*iterator).second.sampleGrabberNodeId,
                                                                              renderer_node_id,
                                                                              topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::loadAudioRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT ((*iterator).second.device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;
#if defined (_DEBUG)
  Stream_Module_Device_MediaFoundation_Tools::dump (topology_p);
#endif

  ACE_ASSERT (!mediaSession_);
  if (!Stream_Module_Device_MediaFoundation_Tools::setTopology (topology_p,
                                                                mediaSession_,
                                                                true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (mediaSession_);

  reference_count = mediaSession_->AddRef ();
  (*iterator).second.session = mediaSession_;
continue_:
  ACE_ASSERT (topology_p);
  if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                     (*iterator).second.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  topology_p->Release ();
  topology_p = NULL;
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: capture format: \"%s\"\n"),
              ACE_TEXT (inherited::name_.c_str ()),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString ((*iterator).second.format).c_str ())));
#endif

  if (session_data_r.format)
  {
    session_data_r.format->Release ();
    session_data_r.format = NULL;
  } // end IF
  ACE_ASSERT (!session_data_r.format);
  Stream_Module_Device_MediaFoundation_Tools::copyMediaType ((*iterator).second.format,
                                                             session_data_r.format);
  //if (!Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (topology_p,
  //                                                                  configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
  //                                                                  media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getOutputFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (media_type_p);

  if (session_data_r.session)
  {
    session_data_r.session->Release ();
    session_data_r.session = NULL;
  } // end IF
  reference_count = mediaSession_->AddRef ();
  session_data_r.session = mediaSession_;
#endif

  source_impl_p->set (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      return false;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration&> (configuration_in).setupPipeline =
      setup_pipeline;
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
Test_U_AudioEffect_MediaFoundation_Stream::collect (Test_U_AudioEffect_RuntimeStatistic& data_out)
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
                ACE_TEXT ("%s: dynamic_cast<Test_U_AudioEffect_MediaFoundation_Statistic_WriterTask_T> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF

  // synch access
  struct Test_U_AudioEffect_MediaFoundation_SessionData& session_data_r =
    const_cast<struct Test_U_AudioEffect_MediaFoundation_SessionData&> (inherited::sessionData_->get ());
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
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
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
  } // end IF

  return result_2;
}

void
Test_U_AudioEffect_MediaFoundation_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::report"));

//   Net_Module_Statistic_ReaderTask_t* runtimeStatistic_impl =
//     dynamic_cast<Net_Module_Statistic_ReaderTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Net_Module_Statistic_ReaderTask_t> failed, returning\n")));
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
  //if (count == 0)
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
                ACE_TEXT ("%s: failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
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
                ACE_TEXT ("%s: received MEEndOfPresentation\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    break;
  }
  case MEError:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: received MEError: \"%s\"\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (status).c_str ())));
    break;
  }
  case MESessionClosed:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                ACE_TEXT (inherited::name_.c_str ())));
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
                ACE_TEXT ("%s: received MESessionEnded, closing sesion\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    result = mediaSession_->Close ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    break;
  }
  case MESessionCapabilitiesChanged:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionCapabilitiesChanged\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    break;
  }
  case MESessionNotifyPresentationTime:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionNotifyPresentationTime\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    break;
  }
  case MESessionStarted:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionStarted\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    break;
  }
  case MESessionStopped:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionStopped, stopping\n"),
                ACE_TEXT (inherited::name_.c_str ())));

    if (isRunning ())
      stop (false,
            true);
    break;
  }
  case MESessionTopologySet:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionTopologySet (status was: \"%s\")\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Common_Tools::error2String (status).c_str ())));
    break;
  }
  case MESessionTopologyStatus:
  {
    UINT32 attribute_value = 0;
    MF_TOPOSTATUS topology_status = MF_TOPOSTATUS_INVALID;

    result = media_event_p->GetUINT32 (MF_EVENT_TOPOLOGY_STATUS,
                                       &attribute_value);
    if (FAILED (result))
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to IMFMediaEvent::GetUINT32(MF_EVENT_TOPOLOGY_STATUS): \"%s\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    else
      topology_status = static_cast<MF_TOPOSTATUS> (attribute_value);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionTopologyStatus: \"%s\"\n"),
                ACE_TEXT (inherited::name_.c_str ()),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::topologyStatusToString (topology_status).c_str ())));
    break;
  }
  default:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: received unknown/invalid media session event (type was: %d), continuing\n"),
                ACE_TEXT (inherited::name_.c_str ()),
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
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::name_.c_str ()),
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  Test_U_AudioEffect_ModuleHandlerConfigurationsIterator_t iterator =
      inherited::configuration_->moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->moduleHandlerConfigurations.end ());
  struct Test_U_AudioEffect_ModuleHandlerConfiguration* configuration_p =
      dynamic_cast<struct Test_U_AudioEffect_ModuleHandlerConfiguration*> ((*iterator).second);
  // sanity check(s)
  ACE_ASSERT (configuration_p);

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
                  Test_U_AudioEffect_ALSA_WAVEncoder_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR ("WAVEncoder"),
                                                             NULL,
                                                             false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_Vis_SpectrumAnalyzer_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR ("SpectrumAnalyzer"),
                                                                  NULL,
                                                                  false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  if (!configuration_p->mute)
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_Target_ALSA_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR ("ALSAPlayback"),
                                                           NULL,
                                                           false),
                    false);
    modules_out.push_back (module_p);
    module_p = NULL;
  } // end IF
  if (!configuration_p->effect.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_SoXEffect_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR ("AudioEffect"),
                                                         NULL,
                                                         false),
                    false);
    modules_out.push_back (module_p);
    module_p = NULL;
  } // end IF
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_StatisticAnalysis_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR ("StatisticAnalysis"),
                                                               NULL,
                                                               false),
                  false);
  modules_out.push_back (module_p);
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_StatisticReport_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                             NULL,
                                                             false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_ALSA_Module (this,
                                                     ACE_TEXT_ALWAYS_CHAR ("MicSource"),
                                                     NULL,
                                                     false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_Stream::initialize (const struct Test_U_AudioEffect_StreamConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_U_AudioEffect_SessionData* session_data_p = NULL;
  Test_U_AudioEffect_ModuleHandlerConfigurationsIterator_t iterator;
  struct Test_U_AudioEffect_ModuleHandlerConfiguration* configuration_p = NULL;
  typename inherited::ISTREAM_T::MODULE_T* module_p = NULL;
  Test_U_Dev_Mic_Source_ALSA* source_impl_p = NULL;

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
  const_cast<struct Test_U_AudioEffect_StreamConfiguration&> (configuration_in).setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  const_cast<struct Test_U_AudioEffect_StreamConfiguration&> (configuration_in).setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<struct Test_U_AudioEffect_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_p->sessionID = ++Test_U_AudioEffect_Stream::currentSessionID;
  // sanity check(s)
  iterator =
      const_cast<struct Test_U_AudioEffect_StreamConfiguration&> (configuration_in).moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.moduleHandlerConfigurations.end ());
  configuration_p =
      dynamic_cast<struct Test_U_AudioEffect_ModuleHandlerConfiguration*> ((*iterator).second);
  ACE_ASSERT (configuration_p);
  session_data_p->targetFileName = configuration_p->fileName;
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  module_p =
    const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("MicSource")));
  ACE_ASSERT (module_p);
  source_impl_p =
    dynamic_cast<Test_U_Dev_Mic_Source_ALSA*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_U_Dev_Mic_Source_ALSA> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF
  source_impl_p->set (&(inherited::state_));
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (!inherited::setup ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    goto error;
  } // end IF

  // -------------------------------------------------------------

  // OK: all went well
  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<struct Test_U_AudioEffect_StreamConfiguration&> (configuration_in).setupPipeline =
      setup_pipeline;

  return false;
}

bool
Test_U_AudioEffect_Stream::collect (Test_U_AudioEffect_RuntimeStatistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("StatisticReport")));
  ACE_ASSERT (module_p);
  Test_U_AudioEffect_Module_Statistic_WriterTask_t* statistic_impl_p =
    dynamic_cast<Test_U_AudioEffect_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_U_AudioEffect_Module_Statistic_WriterTask_T> failed, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
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
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
      return false;
    } // end IF
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (inherited::name_.c_str ())));
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
