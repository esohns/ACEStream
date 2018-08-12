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
#include "test_u_audioeffect_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <dshow.h>

#include "stream_dec_defines.h"

#include "stream_dev_defines.h"
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"

#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_audioeffect_common_modules.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_AudioEffect_DirectShow_Stream::Test_U_AudioEffect_DirectShow_Stream ()
 : inherited ()
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
                                                                   ACE_TEXT_ALWAYS_CHAR (MODULE_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_WAVEncoder_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (MODULE_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  //if (inherited::configuration_->moduleHandlerConfiguration->GdkWindow2D ||
  //    inherited::configuration_->moduleHandlerConfiguration->GdkGLContext)
  //{
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_DirectShow_Vis_SpectrumAnalyzer_Module (this,
                                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                    false);
    modules_out.push_back (module_p);
  //} // end IF
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_StatisticAnalysis_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_StatisticReport_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_DirectShow_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_DirectShow_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  struct Test_U_AudioEffect_DirectShow_SessionData& session_data_r =
    const_cast<struct Test_U_AudioEffect_DirectShow_SessionData&> (inherited::sessionData_->getR ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());

  // *TODO*: remove type inference
  session_data_r.targetFileName = (*iterator).second.second.targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  Test_U_Dev_Mic_Source_DirectShow* source_impl_p =
    dynamic_cast<Test_U_Dev_Mic_Source_DirectShow*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Dev_Mic_Source_DirectShow_T> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

  // ---------------------------------------------------------------------------

  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  //bool COM_initialized = false;
  bool release_builder = false;
  HRESULT result_2 = E_FAIL;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
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
  //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  //  return false;
  //} // end IF
  //COM_initialized = true;

  if ((*iterator).second.second.builder)
  {
    reference_count = (*iterator).second.second.builder->AddRef ();
    graphBuilder_ = (*iterator).second.second.builder;

    // *NOTE*: Stream_Module_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    if (!Stream_MediaFramework_DirectShow_Tools::resetGraph (graphBuilder_,
                                                             CLSID_AudioInputDeviceCategory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::resetGraph(): \"%s\", aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

    if (!Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation (graphBuilder_,
                                                                       MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO,
                                                                       buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    ACE_ASSERT (buffer_negotiation_p);

    goto continue_;
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::loadDeviceGraph ((*iterator).second.second.deviceIdentifier,
                                                               CLSID_AudioInputDeviceCategory,
                                                               graphBuilder_,
                                                               buffer_negotiation_p,
                                                               stream_config_p,
                                                               const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.filterGraphConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second.deviceIdentifier.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (stream_config_p);

  // clean up
  stream_config_p->Release (); stream_config_p = NULL;

  reference_count = graphBuilder_->AddRef ();
  (*iterator).second.second.builder = graphBuilder_;
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
  log_file_name += MODULE_LIB_DIRECTSHOW_LOGFILE_NAME;
  Stream_MediaFramework_DirectShow_Tools::debug (graphBuilder_,
                                                 log_file_name);
#endif // _DEBUG

  if (!Stream_Module_Decoder_Tools::loadAudioRendererGraph (*(*iterator).second.second.inputFormat,
                                                            ((*iterator).second.second.mute ? -1
                                                                                            : (*iterator).second.second.audioOutput),
                                                            graphBuilder_,
                                                            (*iterator).second.second.effect,
                                                            (*iterator).second.second.effectOptions,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadAudioRendererGraph(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  graph_entry.filterName = MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  if (!Stream_MediaFramework_DirectShow_Tools::copyMediaType (*(*iterator).second.second.inputFormat,
                                                              graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::copyMediaType(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  graph_configuration.push_front (graph_entry);
  result_2 =
    (*iterator).second.second.builder->FindFilterByName (MODULE_LIB_DIRECTSHOW_FILTER_NAME_GRAB,
                                                         &filter_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_GRAB),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                       (void**)&isample_grabber_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 = isample_grabber_p->SetCallback (source_impl_p, 0);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
    configuration_in.allocatorConfiguration_.defaultBufferSize;
  allocator_properties.cbPrefix = -1; // <-- use default
  allocator_properties.cBuffers =
    MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
  result_2 =
      buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF

  if (!Stream_MediaFramework_DirectShow_Tools::connect (graphBuilder_,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  //         --> reconnect the AVI decompressor to the (connected) sample
  //             grabber; this seems to work
  if (!Stream_MediaFramework_DirectShow_Tools::connected (graphBuilder_,
                                                          MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reconnecting\n"),
                ACE_TEXT (stream_name_string_)));

    if (!Stream_MediaFramework_DirectShow_Tools::connectFirst (graphBuilder_,
                                                               MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connectFirst(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::connected (graphBuilder_,
                                                                 MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO));

  // debug info
  // *TODO*: find out why this fails
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result_2 =
      buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result_2)) // E_FAIL (0x80004005)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    //goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              ACE_TEXT (stream_name_string_),
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
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result_2 = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release ();
  media_filter_p = NULL;

  result_2 = graphBuilder_->QueryInterface (IID_PPV_ARGS (&graph_streams_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IAMGraphStreams): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graph_streams_p);
  result_2 = graph_streams_p->SyncUsingStreamOffset (FALSE);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMGraphStreams::SyncUsingStreamOffset(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  graph_streams_p->Release ();
  graph_streams_p = NULL;

  if (session_data_r.inputFormat)
    Stream_MediaFramework_DirectShow_Tools::deleteMediaType (session_data_r.inputFormat);
  ACE_ASSERT (!session_data_r.inputFormat);
  if (!Stream_MediaFramework_DirectShow_Tools::getOutputFormat (graphBuilder_,
                                                                MODULE_LIB_DIRECTSHOW_FILTER_NAME_GRAB,
                                                                session_data_r.inputFormat))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::getOutputFormat(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_GRAB)));
    goto error;
  } // end IF
  ACE_ASSERT (session_data_r.inputFormat);

  // ---------------------------------------------------------------------------

  source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (!inherited::setup (NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
#if defined (_DEBUG)
  inherited::dump_state ();
#endif // _DEBUG

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
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
    (*iterator).second.second.builder->Release (); (*iterator).second.second.builder = NULL;
  } // end IF
  if (graphBuilder_)
  {
    graphBuilder_->Release (); graphBuilder_ = NULL;
  } // end IF

  if (session_data_r.inputFormat)
    Stream_MediaFramework_DirectShow_Tools::deleteMediaType (session_data_r.inputFormat);

  //if (COM_initialized)
  //  CoUninitialize ();

  return false;
}

//////////////////////////////////////////

Test_U_AudioEffect_MediaFoundation_Stream::Test_U_AudioEffect_MediaFoundation_Stream ()
 : inherited ()
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
 , mediaSession_ (NULL)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
 , referenceCount_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::Test_U_AudioEffect_MediaFoundation_Stream"));

}

Test_U_AudioEffect_MediaFoundation_Stream::~Test_U_AudioEffect_MediaFoundation_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::~Test_U_AudioEffect_MediaFoundation_Stream"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result) &&
      (result != MF_E_SHUTDOWN)) // already shut down...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
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
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    PropVariantClear (&property_s);
    return;
  } // end IF
  PropVariantClear (&property_s);

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  inherited::start ();
}
void
Test_U_AudioEffect_MediaFoundation_Stream::stop (bool waitForCompletion_in,
                                                 bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::stop"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
}

bool
Test_U_AudioEffect_MediaFoundation_Stream::load (Stream_ModuleList_t& modules_out,
                                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_FileWriter_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (MODULE_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_WAVEncoder_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (MODULE_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  //if (inherited::configuration_->moduleHandlerConfiguration->GdkWindow2D ||
  //    inherited::configuration_->moduleHandlerConfiguration->GdkGLContext)
  //{
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_Vis_SpectrumAnalyzer_Module (this,
                                                                                    ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                    false);
    modules_out.push_back (module_p);
  //} // end IF
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_StatisticAnalysis_Module (this,
                                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_StatisticReport_Module (this,
                                                                             ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_MediaFoundation_Module (this,
                                                                ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_MediaFoundation_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  struct Test_U_AudioEffect_MediaFoundation_SessionData& session_data_r =
    const_cast<struct Test_U_AudioEffect_MediaFoundation_SessionData&> (inherited::sessionData_->getR ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());

  // *TODO*: remove type inference
  session_data_r.targetFileName = (*iterator).second.second.targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  Test_U_Dev_Mic_Source_MediaFoundation* source_impl_p =
    dynamic_cast<Test_U_Dev_Mic_Source_MediaFoundation*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_U_Dev_Mic_Source_MediaFoundation> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

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
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
  COM_initialized = true;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF

  if ((*iterator).second.second.session)
  {
    reference_count = (*iterator).second.second.session->AddRef ();
    mediaSession_ = (*iterator).second.second.session;

    if (!Stream_MediaFramework_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
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
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (topology_p);

    if ((*iterator).second.second.sampleGrabberNodeId)
      goto continue_;
    if (!Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                              (*iterator).second.second.sampleGrabberNodeId))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    ACE_ASSERT ((*iterator).second.second.sampleGrabberNodeId);

    goto continue_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  TOPOID renderer_node_id = 0;
  if (!Stream_Module_Decoder_Tools::loadAudioRendererTopology ((*iterator).second.second.deviceIdentifier,
                                                               (*iterator).second.second.inputFormat,
                                                               source_impl_p,
                                                               ((*iterator).second.second.mute ? -1
                                                                                               : (*iterator).second.second.audioOutput),
                                                               (*iterator).second.second.sampleGrabberNodeId,
                                                               renderer_node_id,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadAudioRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second.deviceIdentifier.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;
#if defined (_DEBUG)
  Stream_MediaFramework_MediaFoundation_Tools::dump (topology_p);
#endif // _DEBUG

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (!mediaSession_);
  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 mediaSession_,
                                                                 true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (mediaSession_);

  reference_count = mediaSession_->AddRef ();
  (*iterator).second.second.session = mediaSession_;
continue_:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (topology_p);
  if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                     (*iterator).second.second.inputFormat))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  topology_p->Release (); topology_p = NULL;
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: capture format: \"%s\"\n"),
              ACE_TEXT (stream_name_string_),
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::mediaTypeToString ((*iterator).second.second.inputFormat).c_str ())));
#endif

  if (session_data_r.inputFormat)
  {
    session_data_r.inputFormat->Release (); session_data_r.inputFormat = NULL;
  } // end IF
  ACE_ASSERT (!session_data_r.inputFormat);
  Stream_MediaFramework_MediaFoundation_Tools::copyMediaType ((*iterator).second.second.inputFormat,
                                                              session_data_r.inputFormat);
  //if (!Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (topology_p,
  //                                                                  configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
  //                                                                  media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::getOutputFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (media_type_p);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (session_data_r.session)
  {
    session_data_r.session->Release (); session_data_r.session = NULL;
  } // end IF
  reference_count = mediaSession_->AddRef ();
  session_data_r.session = mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      return false;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (session_data_r.inputFormat)
  {
    session_data_r.inputFormat->Release (); session_data_r.inputFormat = NULL;
  } // end IF
  //session_data_r.resetToken = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (session_data_r.session)
  {
    session_data_r.session->Release (); session_data_r.session = NULL;
  } // end IF
  if (mediaSession_)
  {
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  if (COM_initialized)
    CoUninitialize ();

  return false;
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (inherited::sessionData_);

  //Stream_CamSave_SessionData& session_data_r =
  //  const_cast<Stream_CamSave_SessionData&> (inherited::sessionData_->get ());

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MEError:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: received MEError: \"%s\"\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionClosed:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                  ACE_TEXT (stream_name_string_)));
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
      //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      //media_source_p->Release ();
  //continue_:
      // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
      //result = mediaSession_->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      break;
    }
    case MESessionEnded:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionEnded, closing sesion\n"),
                  ACE_TEXT (stream_name_string_)));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      result = mediaSession_->Close ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                    ACE_TEXT (stream_name_string_),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      break;
    }
    case MESessionCapabilitiesChanged:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionCapabilitiesChanged\n"),
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MESessionNotifyPresentationTime:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionNotifyPresentationTime\n"),
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MESessionStarted:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStarted\n"),
                  ACE_TEXT (stream_name_string_)));
      break;
    }
    case MESessionStopped:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStopped, stopping\n"),
                  ACE_TEXT (stream_name_string_)));

      if (isRunning ())
        stop (false,
              true);
      break;
    }
    case MESessionTopologySet:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologySet (status was: \"%s\")\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (status).c_str ())));
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
                    ACE_TEXT (stream_name_string_),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      else
        topology_status = static_cast<MF_TOPOSTATUS> (attribute_value);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologyStatus: \"%s\"\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::topologyStatusToString (topology_status).c_str ())));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: received unknown/invalid media session event (type was: %d), continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  event_type));
      break;
    }
  } // end SWITCH
  PropVariantClear (&value);
  media_event_p->Release (); media_event_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  return S_OK;

error:
  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}
#else
Test_U_AudioEffect_Stream::Test_U_AudioEffect_Stream ()
 : inherited ()
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

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
      inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  // sanity check(s)
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  struct Test_U_AudioEffect_ModuleHandlerConfiguration* configuration_p =
      dynamic_cast<struct Test_U_AudioEffect_ModuleHandlerConfiguration*> (&((*iterator).second.second));
  // sanity check(s)
  ACE_ASSERT (configuration_p);

  //// initialize return value(s)
  //modules_out.clear ();
  //delete_out = false;

  Stream_Module_t* module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  Test_U_AudioEffect_Module_FileWriter_Module (this,
//                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_FILE_SINK_DEFAULT_NAME_STRING)),
//                  false);
//  modules_out.push_back (module_p);
//  module_p = NULL;
  // *NOTE*: currently, on UNIX systems, the WAV encoder writes the WAV file
  //         itself
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_ALSA_WAVEncoder_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (MODULE_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_Vis_SpectrumAnalyzer_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  if (!configuration_p->mute)
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_Target_ALSA_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_TARGET_ALSA_DEFAULT_NAME_STRING)),
                    false);
    modules_out.push_back (module_p);
    module_p = NULL;
  } // end IF
  if (!configuration_p->effect.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_SoXEffect_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (MODULE_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING)),
                    false);
    modules_out.push_back (module_p);
    module_p = NULL;
  } // end IF
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_StatisticAnalysis_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_StatisticReport_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_ALSA_Module (this,
                                                     ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING)),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_U_AudioEffect_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration* directshow_configuration_p =
      NULL;
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration* mediafoundation_configuration_p =
      NULL;
#else
  struct Test_U_AudioEffect_ModuleHandlerConfiguration* configuration_p = NULL;
#endif
  typename inherited::ISTREAM_T::MODULE_T* module_p = NULL;
  Test_U_AudioEffect_IDispatch_t* idispatch_p = NULL;
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
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<struct Test_U_AudioEffect_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  // sanity check(s)
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (configuration_in.useMediaFoundation)
  {
    mediafoundation_configuration_p =
        dynamic_cast<struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration*> (&((*iterator).second.second));
    ACE_ASSERT (mediafoundation_configuration_p);
  } // end IF
  else
  {
    directshow_configuration_p =
        dynamic_cast<struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration*> (&((*iterator).second.second));
    ACE_ASSERT (directshow_configuration_p);
  } // end ELSE
#else
  configuration_p =
      dynamic_cast<struct Test_U_AudioEffect_ModuleHandlerConfiguration*> (&((*iterator).second.second));
  ACE_ASSERT (configuration_p);
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (configuration_in.useMediaFoundation)
    session_data_p->targetFileName = mediafoundation_configuration_p->fileName;
  else
    session_data_p->targetFileName = directshow_configuration_p->fileName;
#else
  session_data_p->targetFileName = configuration_p->fileName;
#endif
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // ---------------------------------------------------------------------------

  // ********************** Spectrum Analyzer *****************
  module_p =
      const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  idispatch_p =
      dynamic_cast<Test_U_AudioEffect_IDispatch_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (idispatch_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (configuration_in.useMediaFoundation)
    mediafoundation_configuration_p->dispatch = idispatch_p;
  else
    directshow_configuration_p->dispatch = idispatch_p;
#else
  configuration_p->dispatch = idispatch_p;
#endif

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  module_p =
    const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  source_impl_p =
    dynamic_cast<Test_U_Dev_Mic_Source_ALSA*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_U_Dev_Mic_Source_ALSA> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  source_impl_p->setP (&(inherited::state_));
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (!inherited::setup ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  // -------------------------------------------------------------

  // OK: all went well
  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;

  return false;
}
#endif // ACE_WIN32 || ACE_WIN64
