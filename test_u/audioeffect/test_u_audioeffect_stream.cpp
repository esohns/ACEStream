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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include "dshow.h"
#include "Mferror.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_log_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

//#include "stream_dec_defines.h"

//#include "stream_dev_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_audioeffect_common_modules.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_AudioEffect_DirectShow_Stream::Test_U_AudioEffect_DirectShow_Stream ()
 : inherited ()
 //, graphBuilder_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::Test_U_AudioEffect_DirectShow_Stream"));

}

Test_U_AudioEffect_DirectShow_Stream::~Test_U_AudioEffect_DirectShow_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::~Test_U_AudioEffect_DirectShow_Stream"));

  //if (graphBuilder_)
  //  graphBuilder_->Release ();

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_U_AudioEffect_DirectShow_Stream::load (Stream_ILayout* layout_in,
                                            bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
  if (inherited::configuration_->configuration_->useFrameworkSource)
    ACE_NEW_RETURN (module_p,
                    Test_U_Dev_Mic_Source_DirectShow_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)),
                    false);
  else
    ACE_NEW_RETURN (module_p,
                    Test_U_Dev_Mic_Source_WaveIn_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_WAVEIN_DEFAULT_NAME_STRING)),
                    false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_AudioEffect_DirectShow_StatisticReport_Module (this,
  //                                                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_StatisticAnalysis_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  if (!(*iterator).second.second->mute)
  {
    ACE_NEW_RETURN (module_p,
                    //Test_U_AudioEffect_DirectShow_WavOut_Module (this,
                    //                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_WAVOUT_DEFAULT_NAME_STRING)),
                    Test_U_AudioEffect_DirectShow_Target_Module (this,
                                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_Vis_SpectrumAnalyzer_Module (this,
                                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
#endif // GTK_USE
#endif // GUI_SUPPORT
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_WAVEncoder_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_DirectShow_FileWriter_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);

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
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  Test_U_AudioEffect_DirectShow_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_DirectShow_SessionData&> (inherited::sessionData_->getR ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());

  // *TODO*: remove type inference
  session_data_r.targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  std::string head_module_name_string =
    (configuration_in.configuration_->useFrameworkSource ? ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)
                                                         : ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_WAVEIN_DEFAULT_NAME_STRING));
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (head_module_name_string));
  ACE_ASSERT (module_p);
  Common_ISetP_T<struct Test_U_AudioEffect_DirectShow_StreamState>* iset_p =
    dynamic_cast<Common_ISetP_T<struct Test_U_AudioEffect_DirectShow_StreamState>*> (module_p->writer ());
  ACE_ASSERT (iset_p);

  // ---------------------------------------------------------------------------

  // ********************** Spectrum Analyzer *****************
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  Stream_Module_t* module_2 =
      const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_2);
  Test_U_AudioEffect_IDispatch_t* idispatch_p = dynamic_cast<Test_U_AudioEffect_IDispatch_t*> (module_2->writer ());
  ACE_ASSERT (idispatch_p);
  (*iterator).second.second->dispatch = idispatch_p;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

  //struct _AllocatorProperties allocator_properties;
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
  //ISampleGrabber* isample_grabber_p = NULL;
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
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  return false;
  //} // end IF
  //COM_initialized = true;

  ACE_ASSERT ((*iterator).second.second->builder);

  //if (!Stream_Device_Tools::setCaptureFormat (graphBuilder_,
  //                                            CLSID_AudioInputDeviceCategory,
  //                                            *configuration_in.moduleHandlerConfiguration->format))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Device_Tools::setCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
//#if defined (_DEBUG)
//  log_file_name =
//    Common_Log_Tools::getLogDirectory (std::string (),
//                                       0);
//  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
//  log_file_name += STREAM_LIB_DIRECTSHOW_LOGFILE_NAME;
//  Stream_MediaFramework_DirectShow_Tools::debug (graphBuilder_,
//                                                 log_file_name);
//#endif // _DEBUG

  if (!Stream_Module_Decoder_Tools::loadAudioRendererGraph ((configuration_in.configuration_->useFrameworkSource ? CLSID_AudioInputDeviceCategory
                                                                                                                 : GUID_NULL),
                                                            configuration_in.configuration_->format,
                                                            ((*iterator).second.second->mute ? -1
                                                                                            : (*iterator).second.second->audioOutput),
                                                            (*iterator).second.second->builder,
                                                            (*iterator).second.second->effect,
                                                            (*iterator).second.second->effectOptions,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadAudioRendererGraph(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  graph_entry.filterName =
    (configuration_in.configuration_->useFrameworkSource ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO
                                                         : STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L);
  graph_entry.mediaType =
    Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format);
  if (!graph_entry.mediaType)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  if (!InlineIsEqualGUID ((*iterator).second.second->effect, GUID_NULL))
  {
    // *NOTE*: this seems to require lSampleSize of 1 to connect successfully....
    graph_entry.mediaType->lSampleSize = 1;
  } // end IF
  graph_configuration.push_front (graph_entry);

//  result_2 =
//    (*iterator).second.second->builder->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB,
//                                                         &filter_p);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (filter_p);
//  result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
//                                       (void**)&isample_grabber_p);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (isample_grabber_p);
//  filter_p->Release (); filter_p = NULL;
//
//  result_2 = isample_grabber_p->SetBufferSamples (false);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  result_2 = isample_grabber_p->SetCallback (source_impl_p, 0);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  isample_grabber_p->Release (); isample_grabber_p = NULL;
//
//  ACE_ASSERT (buffer_negotiation_p);
//  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
//  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
//  //         if this is -1/0 (why ?)
//  allocator_properties.cbAlign = 1;
//  //allocator_properties.cbAlign = -1; // <-- use default
//  allocator_properties.cbBuffer =
//    configuration_in.configuration->allocatorConfiguration->defaultBufferSize;
//  allocator_properties.cbPrefix = -1; // <-- use default
//  allocator_properties.cBuffers =
//    STREAM_DEV_MIC_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
//  result_2 =
//      buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//
  if (!Stream_MediaFramework_DirectShow_Tools::connect ((*iterator).second.second->builder,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
//  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
//  //         null renderer 'breaks' the connection between the AVI decompressor
//  //         and the sample grabber (go ahead, try it in with graphedit.exe)
//  //         --> reconnect the AVI decompressor to the (connected) sample
//  //             grabber; this seems to work
//  if (!Stream_MediaFramework_DirectShow_Tools::connected (graphBuilder_,
//                                                          STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: reconnecting\n"),
//                ACE_TEXT (stream_name_string_)));
//
//    if (!Stream_MediaFramework_DirectShow_Tools::connectFirst (graphBuilder_,
//                                                               STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connectFirst(), aborting\n"),
//                  ACE_TEXT (stream_name_string_)));
//      goto error;
//    } // end IF
//  } // end IF
//  ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::connected (graphBuilder_,
//                                                                 STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO));
//
//#if defined (_DEBUG)
//  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
//  result_2 =
//      buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
//  if (FAILED (result_2)) // E_FAIL (0x80004005)
//  {
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
//    //goto error;
//  } // end IF
//  else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
//                ACE_TEXT (stream_name_string_),
//                allocator_properties.cBuffers,
//                allocator_properties.cbBuffer,
//                allocator_properties.cbAlign,
//                allocator_properties.cbPrefix));
//#endif // _DEBUG
//  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;
//
//  result_2 = graphBuilder_->QueryInterface (IID_PPV_ARGS (&media_filter_p));
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (media_filter_p);
//  result_2 = media_filter_p->SetSyncSource (NULL);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  media_filter_p->Release (); media_filter_p = NULL;
//
//  result_2 = graphBuilder_->QueryInterface (IID_PPV_ARGS (&graph_streams_p));
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IAMGraphStreams): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (graph_streams_p);
//  result_2 = graph_streams_p->SyncUsingStreamOffset (FALSE);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to IAMGraphStreams::SyncUsingStreamOffset(): \"%s\", aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  graph_streams_p->Release (); graph_streams_p = NULL;
//
//  if (!Stream_MediaFramework_DirectShow_Tools::getOutputFormat (graphBuilder_,
//                                                                STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB,
//                                                                media_type_s))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::getOutputFormat(\"%s\"), aborting\n"),
//                ACE_TEXT (stream_name_string_),
//                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB)));
//    goto error;
//  } // end IF
  //media_type_s.bFixedSizeSamples = TRUE;
  //media_type_s.bTemporalCompression = FALSE;
  //media_type_s.cbFormat = sizeof (struct tWAVEFORMATEX);
  //media_type_s.formattype = FORMAT_WaveFormatEx;
  //media_type_s.majortype = MEDIATYPE_Audio;
  //media_type_s.subtype = MEDIASUBTYPE_PCM;
  session_data_r.formats.push_back (configuration_in.configuration_->format);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: input format: %s\n"),
              ACE_TEXT (stream_name_string_),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (configuration_in.configuration_->format, false).c_str ())));

  //Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
  //ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));

  // ---------------------------------------------------------------------------

  iset_p->setP (&(inherited::state_));

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
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  if (stream_config_p)
    stream_config_p->Release ();
  if (media_filter_p)
    media_filter_p->Release ();
  if (filter_p)
    filter_p->Release ();
  //if (isample_grabber_p)
  //  isample_grabber_p->Release ();
  if (graph_streams_p)
    graph_streams_p->Release ();

  if (release_builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF

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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
}

bool
Test_U_AudioEffect_MediaFoundation_Stream::load (Stream_ILayout* layout_in,
                                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_MediaFoundation_Module (this,
                                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_AudioEffect_MediaFoundation_StatisticReport_Module (this,
  //                                                                           ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_StatisticAnalysis_Module (this,
                                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_Vis_SpectrumAnalyzer_Module (this,
                                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
#endif // GTK_USE
#endif // GUI_SUPPORT
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_WAVEncoder_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_MediaFoundation_FileWriter_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);

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
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  IMFMediaType* media_type_p = NULL;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  Test_U_AudioEffect_MediaFoundation_SessionData& session_data_r =
    const_cast<Test_U_AudioEffect_MediaFoundation_SessionData&> (inherited::sessionData_->getR ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());

  // *TODO*: remove type inference
  session_data_r.targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING)));
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
  ULONG reference_count = 0;

  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
  COM_initialized = true;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF

  if ((*iterator).second.second->session)
  {
    reference_count = (*iterator).second.second->session->AddRef ();
    mediaSession_ = (*iterator).second.second->session;

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
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (topology_p);

    if ((*iterator).second.second->sampleGrabberNodeId)
      goto continue_;
    if (!Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                              (*iterator).second.second->sampleGrabberNodeId))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    ACE_ASSERT ((*iterator).second.second->sampleGrabberNodeId);

    goto continue_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  TOPOID renderer_node_id = 0;
  if (!Stream_Module_Decoder_Tools::loadAudioRendererTopology ((*iterator).second.second->deviceIdentifier.identifier._string,
                                                               configuration_in.configuration_->format,
                                                               source_impl_p,
                                                               ((*iterator).second.second->mute ? -1
                                                                                               : (*iterator).second.second->audioOutput),
                                                               (*iterator).second.second->sampleGrabberNodeId,
                                                               renderer_node_id,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadAudioRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second->deviceIdentifier.identifier._string)));
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
  (*iterator).second.second->session = mediaSession_;
continue_:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (topology_p);
  if (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                              configuration_in.configuration_->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  topology_p->Release (); topology_p = NULL;
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: capture format: \"%s\"\n"),
              ACE_TEXT (stream_name_string_),
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (configuration_in.configuration_->format).c_str ())));
#endif // _DEBUG

  //media_type_p =
  //  Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration->format);
  //if (!media_type_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n")));
  //  goto error;
  //} // end IF
  //session_data_r.formats.push_back (media_type_p);

  if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                     (*iterator).second.second->sampleGrabberNodeId,
                                                                     media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  session_data_r.formats.push_back (media_type_p);

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

  if (configuration_in.configuration_->setupPipeline)
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
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionClosed:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                  ACE_TEXT (stream_name_string_)));
      //IMFMediaSource* media_source_p = NULL;
      //if (!Stream_Device_Tools::getMediaSource (mediaSession_,
      //                                                 media_source_p))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Device_Tools::getMediaSource(), continuing\n")));
      //  goto continue_;
      //} // end IF
      //ACE_ASSERT (media_source_p);
      //result = media_source_p->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSource::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      //media_source_p->Release ();
  //continue_:
      // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
      //result = mediaSession_->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
        this->stop (false, // wait ?
                    true); // locked access ?
      break;
    }
    case MESessionTopologySet:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologySet (status was: \"%s\")\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      else
        topology_status = static_cast<MF_TOPOSTATUS> (attribute_value);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologyStatus: \"%s\"\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (topology_status).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  return S_OK;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
error:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}
#else
Test_U_AudioEffect_ALSA_Stream::Test_U_AudioEffect_ALSA_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_ALSA_Stream::Test_U_AudioEffect_ALSA_Stream"));

}

Test_U_AudioEffect_ALSA_Stream::~Test_U_AudioEffect_ALSA_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_ALSA_Stream::~Test_U_AudioEffect_ALSA_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_U_AudioEffect_ALSA_Stream::load (Stream_ILayout* layout_in,
                                      bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_ALSA_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
      inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Dev_Mic_Source_ALSA_Module (this,
                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  Test_U_AudioEffect_StatisticReport_Module (this,
//                                                             ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
//                  false);
//  layout_in->append (module_p, NULL, 0);
//  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_StatisticAnalysis_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  if (!(*iterator).second.second->effect.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_SoXEffect_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF
  if (!(*iterator).second.second->mute)
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_Target_ALSA_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_Vis_SpectrumAnalyzer_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_ALSA_WAVEncoder_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  Test_U_AudioEffect_Module_FileWriter_Module (this,
//                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_FILE_SINK_DEFAULT_NAME_STRING)),
//                  false);
//  modules_out.push_back (module_p);
//  module_p = NULL;
  // *NOTE*: currently, on UNIX systems, the WAV encoder writes the WAV file
  //         itself

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_ALSA_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_ALSA_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  ACE_ASSERT (configuration_in.configuration_);
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_U_AudioEffect_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration* directshow_configuration_p =
      NULL;
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration* mediafoundation_configuration_p =
      NULL;
#else
  struct Test_U_AudioEffect_ModuleHandlerConfiguration* configuration_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  typename inherited::ISTREAM_T::MODULE_T* module_p = NULL;
  Test_U_AudioEffect_IDispatch_t* idispatch_p = NULL;
  Test_U_Dev_Mic_Source_ALSA* source_impl_p = NULL;

//  ACE_ASSERT (configuration_in.moduleHandlerConfiguration->deviceHandle);
//  // *TODO*: remove type inference
//  if (!configuration_in.moduleHandlerConfiguration->format)
//  {
//    if (!Stream_Device_Tools::getCaptureFormat (configuration_in.moduleHandlerConfiguration->deviceHandle,
//                                                       const_cast<Test_U_AudioEffect_ALSA_StreamConfiguration&> (configuration_in).moduleHandlerConfiguration->format))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_Device_Tools::getCaptureFormat(): \"%m\", aborting\n")));
//      return false;
//    } // end IF
//  } // end IF
//  ACE_ASSERT (configuration_in.moduleHandlerConfiguration->format);

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<Test_U_AudioEffect_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  // sanity check(s)
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (configuration_in.useMediaFoundation)
  {
    mediafoundation_configuration_p = (*iterator).second.second;
    session_data_p->targetFileName =
        mediafoundation_configuration_p->fileIdentifier.identifier;
  } // end IF
  else
  {
    directshow_configuration_p = (*iterator).second.second;
    session_data_p->targetFileName =
        directshow_configuration_p->fileIdentifier.identifier;
  } // end ELSE
#else
  configuration_p = (*iterator).second.second;
  session_data_p->targetFileName = configuration_p->fileIdentifier.identifier;
#endif // ACE_WIN32 || ACE_WIN64
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);
  session_data_p->formats.push_back (configuration_in.configuration_->format);

  // ---------------------------------------------------------------------------

  // ********************** Spectrum Analyzer *****************
  module_p =
      const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
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
#endif // ACE_WIN32 || ACE_WIN64

  // ---------------------------------------------------------------------------

  // ******************* Mic Source ************************
  module_p =
    const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING)));
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
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
#endif // ACE_WIN32 || ACE_WIN64
