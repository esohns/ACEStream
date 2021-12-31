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
#include "Mferror.h"
#include "mfapi.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_log_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_macros.h"
#include "stream_lib_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_defines.h"

#include "stream_dec_defines.h"

#include "test_u_audioeffect_common_modules.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_AudioEffect_DirectShow_Stream::Test_U_AudioEffect_DirectShow_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::Test_U_AudioEffect_DirectShow_Stream"));

}

Test_U_AudioEffect_DirectShow_Stream::~Test_U_AudioEffect_DirectShow_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::~Test_U_AudioEffect_DirectShow_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Test_U_AudioEffect_DirectShow_Stream::load (Stream_ILayout* layout_in,
                                            bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::load"));

  // initialize return value(s)
  delete_out = true;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  //typename inherited::CONFIGURATION_T::ITERATOR_T iterator_2 =
  //  inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING));
  //ACE_ASSERT (iterator_2 != inherited::configuration_->end ());
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_3 =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_3 != inherited::configuration_->end ());
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_4 =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_4 != inherited::configuration_->end ());
  ACE_ASSERT ((*iterator).second.second->generatorConfiguration);

  Stream_Module_t* module_p = NULL, *module_2 = NULL;
  bool device_can_render_format_b = false;
  HRESULT result = E_FAIL;
  bool has_directshow_source_b = true;

  switch (inherited::configuration_->configuration_->sourceType)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    {
      switch (inherited::configuration_->configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          ACE_NEW_RETURN (module_p,
                          Test_U_Dev_Mic_Source_WaveIn_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEIN_CAPTURE_DEFAULT_NAME_STRING)),
                          false);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          ACE_NEW_RETURN (module_p,
                          Test_U_Dev_Mic_Source_WASAPI_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_CAPTURE_DEFAULT_NAME_STRING)),
                          false);
          break;
        }
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
        {
          ACE_NEW_RETURN (module_p,
                          Test_U_Dev_Mic_Source_DirectShow_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)),
                          false);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown capturer (was: %d), aborting\n"),
                      ACE_TEXT (stream_name_string_),
                      inherited::configuration_->configuration_->capturer));
          return false;
        }
      } // end SWITCH
      break;
    }
    case AUDIOEFFECT_SOURCE_NOISE:
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_Dec_Noise_Source_DirectShow_Module (this,
                                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case AUDIOEFFECT_SOURCE_FILE:
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_Dec_MP3Decoder_DirectShow_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_1LAYER3_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown source type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->sourceType));
      return false;
    }
  } // end SWITCH
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_AudioEffect_DirectShow_StatisticReport_Module (this,
  //                                                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

  // sanity check(s)
  ACE_ASSERT (InlineIsEqualGUID (inherited::configuration_->configuration_->format.formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (inherited::configuration_->configuration_->format.pbFormat);

  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (inherited::configuration_->configuration_->format.pbFormat);
  switch (inherited::configuration_->configuration_->renderer)
  {
    case STREAM_DEVICE_RENDERER_WAVEOUT:
    { ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      device_can_render_format_b =
        Stream_MediaFramework_DirectSound_Tools::canRender ((*iterator_3).second.second->deviceIdentifier.identifier._id,
                                                            *waveformatex_p);
      break;
    }
    case STREAM_DEVICE_RENDERER_WASAPI:
    { ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      device_can_render_format_b =
        Stream_MediaFramework_DirectSound_Tools::canRender ((*iterator_3).second.second->deviceIdentifier.identifier._guid,
                                                            STREAM_LIB_WASAPI_RENDER_DEFAULT_SHAREMODE,
                                                            *waveformatex_p);
      break;
    }
    case STREAM_DEVICE_RENDERER_DIRECTSHOW:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->renderer));
      return false;
    }
  } // end SWITCH

  if (!InlineIsEqualGUID ((*iterator).second.second->effect, GUID_NULL) ||
      (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer == STREAM_DEVICE_RENDERER_DIRECTSHOW)) ||
      (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW) && !device_can_render_format_b))
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_DirectShow_Target_Module (this,
                                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF

  has_directshow_source_b =
    (!InlineIsEqualGUID ((*iterator).second.second->effect, GUID_NULL) && !(*iterator_4).second.second->fileIdentifier.empty ()) ||
    (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW) && !device_can_render_format_b);
  if (has_directshow_source_b)
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_DirectShow_Source_Module (this,
                                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_SOURCE_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  if ((!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW)) &&
      !(*iterator_4).second.second->fileIdentifier.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_DirectShow_Distributor_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    branch_p = module_p;
    inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
    inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
    Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_p);
    idistributor_p->initialize (inherited::configuration_->configuration_->branches);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF

  if (!(*iterator).second.second->mute)
    switch (inherited::configuration_->configuration_->renderer)
    {
      case STREAM_DEVICE_RENDERER_WAVEOUT:
      {
        ACE_NEW_RETURN (module_p,
                        Test_U_AudioEffect_DirectShow_WavOut_Module (this,
                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING)),
                        false);
        break;
      }
      case STREAM_DEVICE_RENDERER_WASAPI:
      {
        ACE_NEW_RETURN (module_p,
                        Test_U_AudioEffect_DirectShow_WASAPIOut_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING)),
                        false);
        break;
      }
      case STREAM_DEVICE_RENDERER_DIRECTSHOW:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                    ACE_TEXT (stream_name_string_),
                    inherited::configuration_->configuration_->renderer));
        return false;
      }
    } // end SWITCH
  if (module_p)
  {
    if (!has_directshow_source_b && !device_can_render_format_b)
    {
      ACE_NEW_RETURN (module_2,
                      Test_U_AudioEffect_DirectShow_Delay_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING)),
                      false);
      ACE_ASSERT (module_2);
      layout_in->append (module_2, branch_p, index_i);
      module_2 = NULL;
    } // end IF

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    ACE_NEW_RETURN (module_2,
                    Test_U_AudioEffect_DirectShow_StatisticAnalysis_Module (this,
                                                                            ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_2);
    layout_in->append (module_2, branch_p, index_i);
    module_2 = NULL;
    ACE_NEW_RETURN (module_2,
                    Test_U_AudioEffect_DirectShow_Vis_SpectrumAnalyzer_Module (this,
                                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_2);
    layout_in->append (module_2, branch_p, index_i);
    module_2 = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT

    layout_in->append (module_p, branch_p, index_i);
    ++index_i;
    module_p = NULL;
  } // end IF

  if (!(*iterator_4).second.second->fileIdentifier.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_DirectShow_WAVEncoder_Module (this,
                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_DirectShow_FileWriter_Module (this,
                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
  } // end IF

  return true;
}

bool
Test_U_AudioEffect_DirectShow_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  HRESULT result_2 = E_FAIL;
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
  inherited::CONFIGURATION_T::ITERATOR_T iterator_2 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_2 != configuration_in.end ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator_3 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_3 != configuration_in.end ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator_4 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_4 != configuration_in.end ());

  // *TODO*: remove type inference
  session_data_r.targetFileName =
    (*iterator_4).second.second->fileIdentifier.identifier;

  // ---------------------------------------------------------------------------

  // ********************** Spectrum Analyzer *****************
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Stream_Module_t* module_2 =
      const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_2);
  Stream_Statistic_IDispatch_t* idispatch_p =
    dynamic_cast<Stream_Statistic_IDispatch_t*> (module_2->writer ());
  ACE_ASSERT (idispatch_p);
  (*iterator).second.second->dispatch = idispatch_p;
#endif // GTK_USE
#endif // GUI_SUPPORT

  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  //bool COM_initialized = false;
  bool release_builder = false;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  ISampleGrabber* isample_grabber_p = NULL;
  std::string log_file_name;
  IAMGraphStreams* graph_streams_p = NULL;
  REFERENCE_TIME max_latency_i =
    MILLISECONDS_TO_100NS_UNITS(STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_MAX_LATENCY_MS);
  bool use_framework_renderer_b = false;
  int render_device_id_i = -1;
  Stream_Module_t* module_p = NULL;

  if ((*iterator).second.second->builder)
  {
    Stream_MediaFramework_DirectShow_Tools::shutdown ((*iterator).second.second->builder);
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF

  if (configuration_in.configuration_->capturer != STREAM_DEVICE_CAPTURER_DIRECTSHOW)
  { ACE_ASSERT (!(*iterator).second.second->builder);
    Test_U_AudioEffect_DirectShowFilter_t* filter_p = NULL;
    IBaseFilter* filter_2 = NULL;
    std::wstring filter_name = STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L;

    result_2 =
      CoCreateInstance (CLSID_FilterGraph, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&(*iterator).second.second->builder));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, false).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT ((*iterator).second.second->builder);
    ACE_NEW_NORETURN (filter_p,
                      Test_U_AudioEffect_DirectShowFilter_t ());
    if (!filter_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, aborting\n")));
      goto error;
    } // end IF

    ACE_ASSERT ((*iterator).second.second->filterConfiguration);
    if (!filter_p->initialize (*(*iterator).second.second->filterConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Source_Filter_T::initialize(), aborting\n")));
      delete filter_p; filter_p = NULL;
      goto error;
    } // end IF
    result_2 = filter_p->NonDelegatingQueryInterface (IID_PPV_ARGS (&filter_2));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to NonDelegatingQueryInterface(IID_IBaseFilter): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      delete filter_p; filter_p = NULL;
      goto error;
    } // end IF
    // *WARNING*: invokes IBaseFilter::GetBuffer
    result_2 =
      (*iterator).second.second->builder->AddFilter (filter_2,
                                                     filter_name.c_str ());
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      filter_2->Release (); filter_2 = NULL;
      delete filter_p; filter_p = NULL;
      goto error;
    } // end IF
    filter_2->Release (); filter_2 = NULL;
  } // end IF
  ACE_ASSERT ((*iterator).second.second->builder);

  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_SOURCE_DEFAULT_NAME_STRING)));
  if (module_p)
  {
    // set sample grabber output format ?
    if (!(*iterator).second.second->mute &&
        (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW))
      switch (inherited::configuration_->configuration_->renderer)
      {
        case STREAM_DEVICE_RENDERER_WAVEOUT:
        {
          //struct tWAVEFORMATEX waveformatex_s;
          //ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
          //Stream_MediaFramework_DirectSound_Tools::getBestFormat ((*iterator_3).second.second->deviceIdentifier.identifier._id,
          //                                                        waveformatex_s);
          //result_2 = CreateAudioMediaType (&waveformatex_s,
          //                                 &media_type_s,
          //                                 TRUE);
          //ACE_ASSERT (SUCCEEDED (result_2));
          break;
        }
        case STREAM_DEVICE_RENDERER_WASAPI:
        {
          ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
          struct tWAVEFORMATEX* waveformatex_p =
            Stream_MediaFramework_DirectSound_Tools::getAudioEngineMixFormat ((*iterator_3).second.second->deviceIdentifier.identifier._guid);
          ACE_ASSERT (waveformatex_p);
          result_2 = CreateAudioMediaType (waveformatex_p,
                                           &media_type_s,
                                           TRUE);
          ACE_ASSERT (SUCCEEDED (result_2));
          CoTaskMemFree (waveformatex_p);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                      ACE_TEXT (stream_name_string_),
                      inherited::configuration_->configuration_->renderer));
          return false;
        }
      } // end SWITCH
  } // end IF

  use_framework_renderer_b =
    ((configuration_in.configuration_->renderer == STREAM_DEVICE_RENDERER_DIRECTSHOW) &&
     !(*iterator).second.second->mute);
  switch (inherited::configuration_->configuration_->renderer)
  {
    case STREAM_DEVICE_RENDERER_WAVEOUT:
    {
      ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      render_device_id_i =
        (*iterator_3).second.second->deviceIdentifier.identifier._id;
      break;
    }
    case STREAM_DEVICE_RENDERER_WASAPI:
    {
      ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      render_device_id_i =
        static_cast<int> (Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId ((*iterator_3).second.second->deviceIdentifier.identifier._guid));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->sourceType));
      return false;
    }
  } // end SWITCH
  if (!Stream_Module_Decoder_Tools::loadAudioRendererGraph (((configuration_in.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW) ? CLSID_AudioInputDeviceCategory
                                                                                                                                              : GUID_NULL),
                                                            configuration_in.configuration_->format,
                                                            media_type_s,
                                                            (module_p != NULL),
                                                            (use_framework_renderer_b ? render_device_id_i : -1),
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
  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
  if (!InlineIsEqualGUID ((*iterator).second.second->effect, GUID_NULL))
  { ACE_ASSERT (!graph_configuration.empty ());
    // *NOTE*: this seems to require lSampleSize of 1 to connect successfully....
    graph_configuration.front ().mediaType->lSampleSize = 1;
  } // end IF

  if (configuration_in.configuration_->capturer == STREAM_DEVICE_CAPTURER_DIRECTSHOW)
  {
    result_2 =
      (*iterator).second.second->builder->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L,
                                                            &filter_p);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
    IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                               PINDIR_OUTPUT,
                                                               0);
    ACE_ASSERT (pin_p);
    typename Test_U_AudioEffect_DirectShowFilter_t::OUTPUT_PIN_T* pin_2 =
      dynamic_cast<typename Test_U_AudioEffect_DirectShowFilter_t::OUTPUT_PIN_T*> (pin_p);
    ACE_ASSERT (pin_2);
    if (!pin_2->initialize (configuration_in.configuration_->format))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::initialize(\"%s\"), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (configuration_in.configuration_->format, true).c_str ())));
      pin_p->Release (); pin_p = NULL;
      filter_p->Release (); filter_p = NULL;
      goto error;
    } // end IF
    pin_p->Release (); pin_p = NULL;
    filter_p->Release (); filter_p = NULL;
  } // end IF

  result_2 =
    (*iterator).second.second->builder->QueryInterface (IID_PPV_ARGS (&graph_streams_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IAMGraphStreams): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graph_streams_p);
  result_2 = graph_streams_p->SyncUsingStreamOffset (TRUE);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMGraphStreams::SyncUsingStreamOffset(FALSE): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 = graph_streams_p->SetMaxGraphLatency (max_latency_i);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMGraphStreams::SetMaxGraphLatency(%q): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                max_latency_i,
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  graph_streams_p->Release (); graph_streams_p = NULL;

  if (!Stream_MediaFramework_DirectShow_Tools::connect ((*iterator).second.second->builder,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  result_2 =
    (*iterator).second.second->builder->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result_2 = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release (); media_filter_p = NULL;

  if ((*iterator_2).second.second->builder)
  {
    (*iterator_2).second.second->builder->Release (); (*iterator_2).second.second->builder = NULL;
  } // end IF
  (*iterator).second.second->builder->AddRef ();
  (*iterator_2).second.second->builder = (*iterator).second.second->builder;

  if (inherited::configuration_->configuration_->sourceType != AUDIOEFFECT_SOURCE_FILE)
    session_data_r.formats.push_back (configuration_in.configuration_->format);

  // ---------------------------------------------------------------------------

  if (!inherited::setup (NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

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
  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  return false;
}

//////////////////////////////////////////

Test_U_AudioEffect_MediaFoundation_Stream::Test_U_AudioEffect_MediaFoundation_Stream ()
 : inherited ()
 , condition_ (inherited::lock_)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
 , mediaSession_ (NULL)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
 , frameworkSource_ (this,
                     ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING))
 , mediaFoundationSource_ (this,
                           ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_SOURCE_DEFAULT_NAME_STRING))
 , mediaFoundationTarget_ (this,
                           ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING))
 , referenceCount_ (0)
 , topologyIsReady_ (false)
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
    if (FAILED (result) && (result != MF_E_SHUTDOWN)) // already shut down...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  if (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING),
                       false,
                       false) &&
      !inherited::remove (&frameworkSource_,
                          true,   // lock ?
                          false)) // reset ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::remove(%s): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                frameworkSource_.name ()));
  if (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING),
                       false,
                       false) &&
      !inherited::remove (&mediaFoundationTarget_,
                          true,   // lock ?
                          false)) // reset ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::remove(%s): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                mediaFoundationTarget_.name ()));
  if (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_SOURCE_DEFAULT_NAME_STRING),
                       false,
                       false) &&
      !inherited::remove (&mediaFoundationSource_,
                          true,   // lock ?
                          false)) // reset ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::remove(%s): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                mediaFoundationSource_.name ()));

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

  return inherited::find (name_in,
                          false,
                          false);
}

void
Test_U_AudioEffect_MediaFoundation_Stream::start ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::start"));

  // sanity check(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  ACE_ASSERT (!topologyIsReady_);

  HRESULT result = E_FAIL;
  int result_2 = -1;
  ACE_Time_Value deadline =
    ACE_Time_Value (STREAM_LIB_MEDIAFOUNDATION_MEDIASESSION_READY_TIMEOUT_S, 0);
  int error = 0;
  struct _GUID GUID_s = GUID_NULL;
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  property_s.vt = VT_EMPTY;

  // start receiving media session events/media session ?
  switch (inherited::configuration_->configuration_->sourceType)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    case AUDIOEFFECT_SOURCE_NOISE:
      break;
    case AUDIOEFFECT_SOURCE_FILE:
    {
      // *NOTE*: the source media type(s) is/are unknown at this stage; let the
      //         mediafoundation target module update and manage the media
      //         session
      goto continue_2;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown source type (was: %d), returning\n"),
                  inherited::configuration_->configuration_->sourceType));
      goto error;
    }
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, inherited::lock_);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    result = mediaSession_->BeginGetEvent (this, NULL);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

    // wait for MF_TOPOSTATUS_READY event
    deadline = COMMON_TIME_NOW + deadline;
    result_2 = condition_.wait (&deadline);
    if (unlikely (result_2 == -1))
    { error = ACE_OS::last_error ();
      if (error != ETIME) // 137: timed out
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Condition::wait(%#T): \"%m\", aborting\n"),
                    ACE_TEXT (stream_name_string_),
                    &deadline));
      goto continue_;
    } // end IF
  } // end lock scope
continue_:
  if (!topologyIsReady_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: topology not ready%s, returning\n"),
                ACE_TEXT (stream_name_string_),
                (error == ETIME) ? ACE_TEXT (" (timed out)") : ACE_TEXT ("")));
    goto error;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->Start (&GUID_s,      // time format
                                 &property_s); // start position
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::Start(): \"%s\", returning\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

continue_2:
  PropVariantClear (&property_s);

  inherited::start ();

  return;

error:
  PropVariantClear (&property_s);
}

void
Test_U_AudioEffect_MediaFoundation_Stream::stop (bool waitForCompletion_in,
                                                 bool recurseUpstream_in,
                                                 bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::stop"));

  // stop media session ?
  switch (inherited::configuration_->configuration_->sourceType)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    case AUDIOEFFECT_SOURCE_NOISE:
      break;
    case AUDIOEFFECT_SOURCE_FILE:
    {
      // *NOTE*: the source media type(s) is/are unknown at this stage; let the
      //         mediafoundation target module update and manage the media
      //         session
      goto continue_;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown source type (was: %d), returning\n"),
                  inherited::configuration_->configuration_->sourceType));
      return;
    }
  } // end SWITCH

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

continue_:
  inherited::stop (waitForCompletion_in,
                   recurseUpstream_in,
                   highPriority_in);
}

bool
Test_U_AudioEffect_MediaFoundation_Stream::load (Stream_ILayout* layout_in,
                                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::load"));

  // initialize return value(s)
  delete_out = false; // *TODO*: leaks all allocated modules

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  ACE_ASSERT ((*iterator).second.second->generatorConfiguration);
  //typename inherited::CONFIGURATION_T::ITERATOR_T iterator_2 =
  //  inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING));
  //ACE_ASSERT (iterator_2 != inherited::configuration_->end ());
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_3 =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_3 != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
  bool device_can_render_format_b = false;
  HRESULT result = E_FAIL;
  bool has_mediafoundation_source_b = true;

  switch (inherited::configuration_->configuration_->sourceType)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    {
      switch (inherited::configuration_->configuration_->capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          ACE_NEW_RETURN (module_p,
                          Test_U_Dev_Mic_Source_WaveIn2_Module (this,
                                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEIN_CAPTURE_DEFAULT_NAME_STRING)),
                          false);
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          ACE_NEW_RETURN (module_p,
                          Test_U_Dev_Mic_Source_WASAPI2_Module (this,
                                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_CAPTURE_DEFAULT_NAME_STRING)),
                          false);
          break;
        }
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
        {
          module_p = &frameworkSource_;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown capturer (was: %d), aborting\n"),
                      ACE_TEXT (stream_name_string_),
                      inherited::configuration_->configuration_->capturer));
          return false;
        }
      } // end SWITCH
      break;
    }
    case AUDIOEFFECT_SOURCE_NOISE:
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_Dec_Noise_Source_MediaFoundation_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case AUDIOEFFECT_SOURCE_FILE:
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_Dec_MP3Decoder_MediaFoundation_Module (this,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_1LAYER3_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown source type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->sourceType));
      return false;
    }
  } // end SWITCH
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_AudioEffect_MediaFoundation_StatisticReport_Module (this,
  //                                                                           ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

  switch (inherited::configuration_->configuration_->renderer)
  {
    case STREAM_DEVICE_RENDERER_WAVEOUT:
    {
      struct tWAVEFORMATEX* waveformatex_p = NULL;
      UINT32 cbSize = 0;
      result = MFCreateWaveFormatExFromMFMediaType (inherited::configuration_->configuration_->format,
                                                    &waveformatex_p,
                                                    &cbSize,
                                                    MFWaveFormatExConvertFlag_Normal);
      ACE_ASSERT (SUCCEEDED (result) && waveformatex_p);
      ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      device_can_render_format_b =
        Stream_MediaFramework_DirectSound_Tools::canRender ((*iterator_3).second.second->deviceIdentifier.identifier._id,
                                                            *waveformatex_p);
      CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
      break;
    }
    case STREAM_DEVICE_RENDERER_WASAPI:
    {
      struct tWAVEFORMATEX* waveformatex_p = NULL;
      UINT32 cbSize = 0;
      result = MFCreateWaveFormatExFromMFMediaType (inherited::configuration_->configuration_->format,
                                                    &waveformatex_p,
                                                    &cbSize,
                                                    MFWaveFormatExConvertFlag_Normal);
      ACE_ASSERT (SUCCEEDED (result) && waveformatex_p);
      ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      device_can_render_format_b =
        Stream_MediaFramework_DirectSound_Tools::canRender ((*iterator_3).second.second->deviceIdentifier.identifier._guid,
                                                            STREAM_LIB_WASAPI_RENDER_DEFAULT_SHAREMODE,
                                                            *waveformatex_p);
      CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
      break;
    }
    case STREAM_DEVICE_RENDERER_MEDIAFOUNDATION:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->renderer));
      return false;
    }
  } // end SWITCH

  has_mediafoundation_source_b =
    (!InlineIsEqualGUID ((*iterator).second.second->effect, GUID_NULL) && !(*iterator_3).second.second->fileIdentifier.empty ())                                       ||
    (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_MEDIAFOUNDATION) && !device_can_render_format_b);
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  if (!has_mediafoundation_source_b)
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_StatisticAnalysis_Module (this,
                                                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_Vis_SpectrumAnalyzer_Module (this,
                                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF
#endif // GTK_USE
#endif // GUI_SUPPORT

  if (!InlineIsEqualGUID ((*iterator).second.second->effect, GUID_NULL)                                                                                                   ||
      (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer == STREAM_DEVICE_RENDERER_MEDIAFOUNDATION))                               ||
      (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_MEDIAFOUNDATION) && !device_can_render_format_b))
    layout_in->append (&mediaFoundationTarget_, NULL, 0);

  if (has_mediafoundation_source_b)
  {
    layout_in->append (&mediaFoundationSource_, NULL, 0);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_StatisticAnalysis_Module (this,
                                                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_Vis_SpectrumAnalyzer_Module (this,
                                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT
  } // end IF

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  if ((!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_MEDIAFOUNDATION)) &&
      !(*iterator_3).second.second->fileIdentifier.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_Distributor_Module (this,
                                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    branch_p = module_p;
    inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
    inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
    Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_p);
    idistributor_p->initialize (inherited::configuration_->configuration_->branches);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF

  if (!(*iterator).second.second->mute)
    switch (inherited::configuration_->configuration_->renderer)
    {
      case STREAM_DEVICE_RENDERER_WAVEOUT:
      {
        ACE_NEW_RETURN (module_p,
                        Test_U_AudioEffect_MediaFoundation_WavOut_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING)),
                        false);
        break;
      }
      case STREAM_DEVICE_RENDERER_WASAPI:
      {
        ACE_NEW_RETURN (module_p,
                        Test_U_AudioEffect_MediaFoundation_WASAPIOut_Module (this,
                                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING)),
                        false);
        break;
      }
      case STREAM_DEVICE_RENDERER_MEDIAFOUNDATION:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                    ACE_TEXT (stream_name_string_),
                    inherited::configuration_->configuration_->renderer));
        return false;
      }
    } // end SWITCH
  if (module_p)
  {
    if (!has_mediafoundation_source_b)
    {
      Stream_Module_t* module_2 = NULL;
      ACE_NEW_RETURN (module_2,
                      Test_U_AudioEffect_MediaFoundation_Delay_Module (this,
                                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING)),
                      false);
      layout_in->append (module_2, branch_p, index_i);
    } // end IF

    layout_in->append (module_p, branch_p, index_i);
    ++index_i;
    module_p = NULL;
  } // end IF

  if (!(*iterator_3).second.second->fileIdentifier.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_WAVEncoder_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_MediaFoundation_FileWriter_Module (this,
                                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
  } // end IF

  delete_out = true;

  return true;
}

bool
Test_U_AudioEffect_MediaFoundation_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  if (inherited::isInitialized_)
  {
    if (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING),
                         false,
                         false) &&
        !inherited::remove (&frameworkSource_,
                            true,   // lock ?
                            false)) // reset ?
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::remove(%s): \"%m\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  frameworkSource_.name ()));
    if (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING),
                         false,
                         false) &&
        !inherited::remove (&mediaFoundationTarget_,
                            true,   // lock ?
                            false)) // reset ?
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::remove(%s): \"%m\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  mediaFoundationTarget_.name ()));
    if (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_SOURCE_DEFAULT_NAME_STRING),
                         false,
                         false) &&
        !inherited::remove (&mediaFoundationSource_,
                            true,   // lock ?
                            false)) // reset ?
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::remove(%s): \"%m\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  mediaFoundationSource_.name ()));
    topologyIsReady_ = false;
  } // end IF

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;

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
  ACE_ASSERT (iterator != configuration_in.end ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator_2 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_2 != configuration_in.end ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator_3 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_3 != configuration_in.end ());
  inherited::CONFIGURATION_T::ITERATOR_T iterator_4 =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_4 != configuration_in.end ());

  // *TODO*: remove type inference
  session_data_r.targetFileName =
    (*iterator_4).second.second->fileIdentifier.identifier;

  // ---------------------------------------------------------------------------

  // ********************** Spectrum Analyzer *****************
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  Stream_Statistic_IDispatch_t* idispatch_p =
    dynamic_cast<Stream_Statistic_IDispatch_t*> (module_p->writer ());
  ACE_ASSERT (idispatch_p);
  (*iterator).second.second->dispatch = idispatch_p;
#endif // GTK_SUPPORT
#endif // GUI_USE

  bool graph_loaded = false;
  HRESULT result_2 = E_FAIL;
  IMFTopology* topology_p = NULL;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  ULONG reference_count = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  IMFSampleGrabberSinkCallback2* sample_grabber_p = NULL;
#else
  IMFSampleGrabberSinkCallback* sample_grabber_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  IMFMediaType* media_type_p = NULL;
  bool use_framework_renderer_b = false;
  int render_device_id_i = -1;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  // *TODO*: reusing media sessions is harder than it seems...
  //         --> use a fresh one every time
  if (mediaSession_)
  {
    Stream_MediaFramework_MediaFoundation_Tools::shutdown (mediaSession_);
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF

  IMFMediaSource* media_source_p = NULL;
  if (configuration_in.configuration_->capturer != STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION)
  {
    Test_U_AudioEffect_MediaFoundation_Target* writer_p =
      &const_cast<Test_U_AudioEffect_MediaFoundation_Target&> (getR_3 ());
    if (!writer_p->initialize (*(*iterator).second.second->mediaFoundationConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::initialize(), aborting\n")));
      goto error;
    } // end IF
    result_2 = QueryInterface (IID_PPV_ARGS (&media_source_p));
    if (FAILED (result_2) || !media_source_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_U_AudioEffect_MediaFoundation_Stream::QueryInterface(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
  } // end IF

  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_SOURCE_DEFAULT_NAME_STRING)));
  if (module_p)
  {
    sample_grabber_p =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      dynamic_cast<IMFSampleGrabberSinkCallback2*> (mediaFoundationSource_.writer ());
#else
      dynamic_cast<IMFSampleGrabberSinkCallback*> (mediaFoundationSource_.writer ());
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
    ACE_ASSERT (sample_grabber_p);

    // set sample grabber output format ?
    if (!(*iterator).second.second->mute &&
        (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_MEDIAFOUNDATION))
      switch (inherited::configuration_->configuration_->renderer)
      {
        case STREAM_DEVICE_RENDERER_WAVEOUT:
        {
          struct tWAVEFORMATEX waveformatex_s;
          ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
          Stream_MediaFramework_DirectSound_Tools::getBestFormat ((*iterator_3).second.second->deviceIdentifier.identifier._id,
                                                                  waveformatex_s);
          media_type_p = Stream_MediaFramework_MediaFoundation_Tools::to (waveformatex_s);
          ACE_ASSERT (media_type_p);
          break;
        }
        case STREAM_DEVICE_RENDERER_WASAPI:
        {
          ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
          struct tWAVEFORMATEX* waveformatex_p =
            Stream_MediaFramework_DirectSound_Tools::getAudioEngineMixFormat ((*iterator_3).second.second->deviceIdentifier.identifier._guid);
          media_type_p = Stream_MediaFramework_MediaFoundation_Tools::to (*waveformatex_p);
          ACE_ASSERT (media_type_p);
          CoTaskMemFree (waveformatex_p);
          // *TODO*: remove ASAP
          result_2 = media_type_p->SetGUID (MF_MT_SUBTYPE,
                                            MFAudioFormat_Float);
          ACE_ASSERT (SUCCEEDED (result_2));
          DWORD channel_mask_i = (SPEAKER_FRONT_LEFT |
                                  SPEAKER_FRONT_RIGHT);
          result_2 = media_type_p->SetUINT32 (MF_MT_AUDIO_CHANNEL_MASK,
                                              channel_mask_i);
          ACE_ASSERT (SUCCEEDED (result_2));
          result_2 = media_type_p->DeleteItem (MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
          ACE_ASSERT (SUCCEEDED (result_2));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                      ACE_TEXT (stream_name_string_),
                      inherited::configuration_->configuration_->renderer));
          return false;
        }
      } // end SWITCH
  } // end IF

  if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology ((*iterator).second.second->deviceIdentifier,
                                                                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                media_source_p,
                                                                sample_grabber_p,
                                                                topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString ((*iterator).second.second->deviceIdentifier.identifier._guid).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_source_p);
  ACE_ASSERT (topology_p);
  media_source_p->Release (); media_source_p = NULL;

  if (configuration_in.configuration_->capturer != STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION)
    goto continue_3;

  if (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                              configuration_in.configuration_->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
continue_3:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  use_framework_renderer_b =
    ((configuration_in.configuration_->renderer == STREAM_DEVICE_RENDERER_MEDIAFOUNDATION) &&
     !(*iterator).second.second->mute);
  switch (inherited::configuration_->configuration_->renderer)
  {
    case STREAM_DEVICE_RENDERER_WAVEOUT:
    {
      ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      render_device_id_i = (*iterator_3).second.second->deviceIdentifier.identifier._id;
      break;
    }
    case STREAM_DEVICE_RENDERER_WASAPI:
    {
      ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      render_device_id_i =
        static_cast<int> (Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId ((*iterator_3).second.second->deviceIdentifier.identifier._guid));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->sourceType));
      return false;
    }
  } // end SWITCH
  ACE_ASSERT ((*iterator).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
  if (!Stream_Module_Decoder_Tools::loadAudioRendererTopology ((*iterator).second.second->deviceIdentifier.identifier._guid,
                                                               MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                               (configuration_in.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION),
                                                               configuration_in.configuration_->format,
                                                               media_type_p,
                                                               sample_grabber_p,
                                                               (use_framework_renderer_b ? render_device_id_i : -1),
                                                               (*iterator).second.second->effect,
                                                               (*iterator).second.second->effectOptions,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadAudioRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::GUIDToString ((*iterator).second.second->deviceIdentifier.identifier._guid).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 mediaSession_,
                                                                 false,  // is partial ?
                                                                 false)) // wait for completion ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (mediaSession_);

  switch (inherited::configuration_->configuration_->sourceType)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    case AUDIOEFFECT_SOURCE_NOISE:
    {
      (*iterator_2).second.second->manageMediaSession = false;
      break;
    }
    case AUDIOEFFECT_SOURCE_FILE:
    {
      // *NOTE*: the source media type(s) is/are unknown at this stage; let the
      //         mediafoundation target module update and manage the media
      //         session
      (*iterator_2).second.second->manageMediaSession = true;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown source type (was: %d), returning\n"),
                  inherited::configuration_->configuration_->sourceType));
      goto error;
    }
  } // end SWITCH

  if ((*iterator).second.second->session)
  {
    (*iterator).second.second->session->Release (); (*iterator).second.second->session = NULL;
  } // end IF
  reference_count = mediaSession_->AddRef ();
  (*iterator).second.second->session = mediaSession_;
  if ((*iterator).second.second->mediaFoundationConfiguration->mediaEventGenerator)
  {
    (*iterator).second.second->mediaFoundationConfiguration->mediaEventGenerator->Release (); (*iterator).second.second->mediaFoundationConfiguration->mediaEventGenerator = NULL;
  } // end IF
  result_2 =
    mediaSession_->QueryInterface (IID_PPV_ARGS (&(*iterator).second.second->mediaFoundationConfiguration->mediaEventGenerator));
  ACE_ASSERT (SUCCEEDED (result_2) && (*iterator).second.second->mediaFoundationConfiguration->mediaEventGenerator);

  if (session_data_r.session)
  {
    session_data_r.session->Release (); session_data_r.session = NULL;
  } // end IF
  reference_count = mediaSession_->AddRef ();
  session_data_r.session = mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (topology_p);
  topology_p->Release (); topology_p = NULL;

  if (media_type_p)
  {
    media_type_p->Release (); media_type_p = NULL;
  } // end IF
  if (inherited::configuration_->configuration_->sourceType != AUDIOEFFECT_SOURCE_FILE)
  {
    media_type_p =
      Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration_->format);
    if (!media_type_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    session_data_r.formats.push_back (media_type_p);
  } // end IF

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

  return false;
}

const Test_U_AudioEffect_MediaFoundation_Target&
Test_U_AudioEffect_MediaFoundation_Stream::getR_3 () const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::getR_3"));

  Test_U_AudioEffect_MediaFoundation_Target* writer_p =
    static_cast<Test_U_AudioEffect_MediaFoundation_Target*> (const_cast<Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget_Module&> (mediaFoundationTarget_).writer ());
  ACE_ASSERT (writer_p);

  return *writer_p;
}

const Test_U_AudioEffect_MediaFoundation_Source&
Test_U_AudioEffect_MediaFoundation_Stream::getR_4 () const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::getR_4"));

  Test_U_AudioEffect_MediaFoundation_Source* writer_p =
    static_cast<Test_U_AudioEffect_MediaFoundation_Source*> (const_cast<Test_U_AudioEffect_MediaFoundation_Source_Module&> (mediaFoundationSource_).writer ());
  ACE_ASSERT (writer_p);

  return *writer_p;
}

const Test_U_Dev_Mic_Source_MediaFoundation&
Test_U_AudioEffect_MediaFoundation_Stream::getR_5 () const
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_Stream::getR_5"));

  Test_U_Dev_Mic_Source_MediaFoundation* writer_p =
    static_cast<Test_U_Dev_Mic_Source_MediaFoundation*> (const_cast<Test_U_Dev_Mic_Source_MediaFoundation_Module&> (frameworkSource_).writer ());
  ACE_ASSERT (writer_p);

  return *writer_p;
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

  HRESULT result = QISearch (this,
                             query_interface_table,
                             IID_in,
                             interface_out);
  if (result == E_NOINTERFACE)
  {
    Test_U_AudioEffect_MediaFoundation_Target* writer_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_Target*> (mediaFoundationTarget_.writer ());
    ACE_ASSERT (writer_p);
    result = writer_p->QueryInterface (IID_in,
                                       interface_out);
  } // end IF

  return result;
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
  bool stop_b = false;
  bool request_event_b = true;

  // sanity check(s)
  ACE_ASSERT (result_in);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stop_b = true;
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = media_event_p->GetType (&event_type);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetStatus (&status);
  ACE_ASSERT (SUCCEEDED (result));
  PropVariantInit (&value);
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
    { // MF_E_INVALID_TIMESTAMP : 0xc00d36c0
      // MF_E_STREAMSINK_REMOVED: 0xc00d4a38
      // MF_E_STREAM_ERROR      : 0xc00da7fb
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: received MEError: \"%s\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      request_event_b = false;
      stop_b = true;
      break;
    }
    case MESessionClosed:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                  ACE_TEXT (stream_name_string_)));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      result = mediaSession_->Shutdown ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                    ACE_TEXT (stream_name_string_),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      request_event_b = false;
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
      UINT32 session_capabilities_i = 0, session_capabilities_delta_i = 0;
      result = media_event_p->GetUINT32 (MF_EVENT_SESSIONCAPS,
                                         &session_capabilities_i);
      ACE_ASSERT (SUCCEEDED (result));
      result = media_event_p->GetUINT32 (MF_EVENT_SESSIONCAPS_DELTA,
                                         &session_capabilities_delta_i);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionCapabilitiesChanged: %u/%u\n"),
                  ACE_TEXT (stream_name_string_),
                  session_capabilities_i, session_capabilities_delta_i));
      break;
    }
    case MESessionNotifyPresentationTime:
    {
      UINT64 presentation_time_start_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_START_PRESENTATION_TIME,
                                         &presentation_time_start_i);
      ACE_ASSERT (SUCCEEDED (result));
      UINT64 presentation_time_offset_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_PRESENTATION_TIME_OFFSET,
                                         &presentation_time_offset_i);
      ACE_ASSERT (SUCCEEDED (result));
      UINT64 presentation_time_at_output_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_START_PRESENTATION_TIME_AT_OUTPUT,
                                         &presentation_time_at_output_i);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionNotifyPresentationTime: %Q/%Q/%Q\n"),
                  ACE_TEXT (stream_name_string_),
                  presentation_time_start_i, presentation_time_offset_i, presentation_time_at_output_i));
      break;
    }
    case MESessionStarted:
    { // status MF_E_INVALIDREQUEST    : 0xC00D36B2
      // status MF_E_ATTRIBUTENOTFOUND : 0xC00D36E6
      // status MF_E_STREAMSINK_REMOVED: 0xc00d4a38
      UINT64 presentation_time_offset_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_PRESENTATION_TIME_OFFSET,
                                         &presentation_time_offset_i);
      //ACE_ASSERT (SUCCEEDED (result)); // MF_E_ATTRIBUTENOTFOUND: 0xc00d36e6
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStarted: \"%s\", presentation offset: %q\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ()),
                  presentation_time_offset_i));
      break;
    }
    case MESessionStopped:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStopped, closing sesion\n"),
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
      ACE_ASSERT (SUCCEEDED (result));
      topology_status = static_cast<MF_TOPOSTATUS> (attribute_value);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologyStatus: \"%s\" (status was: \"%s\")\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (topology_status).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      // start media session ?
      if (topology_status == MF_TOPOSTATUS_READY)
      {
        { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, inherited::lock_, E_FAIL);
          if (SUCCEEDED (status))
            topologyIsReady_ = true;
          else
            stop_b = true;
          int result_2 = condition_.broadcast ();
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Condition::broadcast(): \"%m\", aborting\n"),
                        ACE_TEXT (stream_name_string_)));
            stop_b = true;
            goto error;
          } // end IF
        } // end lock scope
      } // end IF
      break;
    }
    case MEExtendedType:
    {
      struct _GUID GUID_s = GUID_NULL;
      result = media_event_p->GetExtendedType (&GUID_s);
      ACE_ASSERT (SUCCEEDED (result));
      // MF_MEEXT_SAR_AUDIO_ENDPOINT_CHANGED: {02E7187D-0087-437E-A27F-CF5ADCCD3112}
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received extended media session event (type was: %s)\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
      break;
    }
    case MEStreamSinkFormatInvalidated:
    {
      //IMFMediaSink* media_sink_p = NULL;
      //// *TODO*: {3EA99C15-A893-4B46-B221-5FAE05C36152}
      //struct _GUID GUID_s =
      //  Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{3EA99C15-A893-4B46-B221-5FAE05C36152}"));
      //result = media_event_p->GetUnknown (GUID_s,
      //                                    IID_PPV_ARGS (&media_sink_p));
      //ACE_ASSERT (SUCCEEDED (result) && media_sink_p);
      TOPOID node_id = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_OUTPUT_NODE,
                                         &node_id);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MEStreamSinkFormatInvalidated (id: %q)\n"),
                  ACE_TEXT (stream_name_string_),
                  node_id));
      //media_sink_p->Release (); media_sink_p = NULL;
      break;
    }
    case MEEndOfPresentationSegment:
    { // *TODO*: {9C86CC50-68CE-4CFF-AA1E-9A5A40D5B4E0}
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MEEndOfPresentationSegment\n"),
                  ACE_TEXT (stream_name_string_)));
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

  if (unlikely (!request_event_b))
    goto continue_;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stop_b = true;
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

continue_:
  if (unlikely (stop_b))
    stop (false,
          true,
          false);

  return S_OK;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
error:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (unlikely (stop_b))
    stop (false,
          true,
          false);

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
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_3 =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_3 != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
  bool add_delay_b = false;
  switch (inherited::configuration_->configuration_->sourceType)
  {
    case AUDIOEFFECT_SOURCE_DEVICE:
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_Dev_Mic_Source_ALSA_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case AUDIOEFFECT_SOURCE_NOISE:
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_Dec_Noise_Source_ALSA_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_NOISE_SOURCE_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case AUDIOEFFECT_SOURCE_FILE:
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_Dec_MP3Decoder_ALSA_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_MPEG_1LAYER3_DEFAULT_NAME_STRING)),
                      false);
      add_delay_b = true;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown source type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->sourceType));
      return false;
    }
  } // end SWITCH
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  Test_U_AudioEffect_StatisticReport_Module (this,
//                                                             ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
//                  false);
//  layout_in->append (module_p, NULL, 0);
//  module_p = NULL;
  if (!(*iterator).second.second->effect.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_AudioEffect_SoXEffect_Module (this,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF
  if (add_delay_b)
  {
    ACE_NEW_RETURN (module_p,
                    Test_U_ALSA_Delay_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING)),
                    false);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF
  ACE_NEW_RETURN (module_p,
                  Test_U_AudioEffect_StatisticAnalysis_Module (this,
                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
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
  // *NOTE*: this processing stream may have branches, depending on:
  //         - whether the output is muted
  //         - whether the output is saved to file
  if (!(*iterator).second.second->mute ||
      !(*iterator_3).second.second->fileIdentifier.empty ())
  {
    typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
    unsigned int index_i = 0;
    if (!(*iterator).second.second->mute &&
        !(*iterator_3).second.second->fileIdentifier.empty ())
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_AudioEffect_Distributor_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                      false);
      branch_p = module_p;
      inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
      inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
      Stream_IDistributorModule* idistributor_p =
        dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
      ACE_ASSERT (idistributor_p);
      idistributor_p->initialize (inherited::configuration_->configuration_->branches);
      layout_in->append (module_p, NULL, 0);
      module_p = NULL;
    } // end IF

    if (!(*iterator).second.second->mute)
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_AudioEffect_Target_ALSA_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING)),
                      false);
      layout_in->append (module_p, branch_p, index_i);
      ++index_i;
      module_p = NULL;
    } // end IF
    if (!(*iterator_3).second.second->fileIdentifier.empty ())
    {
      ACE_NEW_RETURN (module_p,
                      Test_U_AudioEffect_ALSA_WAVEncoder_Module (this,
                                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                      false);
      layout_in->append (module_p, branch_p, index_i);
      ++index_i;
      module_p = NULL;
      // *NOTE*: currently, on UNIX systems, the WAV encoder writes the WAV file
      //         itself
      //  ACE_NEW_RETURN (module_p,
      //                  Test_U_AudioEffect_Module_FileWriter_Module (this,
      //                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_FILE_SINK_DEFAULT_NAME_STRING)),
      //                  false);
      //  modules_out.push_back (module_p);
      //  module_p = NULL;
    } // end IF
  } // end ELSE

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
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_2;
  typename inherited::ISTREAM_T::MODULE_T* module_p = NULL;
  Test_U_AudioEffect_IDispatch_t* idispatch_p = NULL;

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
  iterator_2 =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_2 != configuration_in.end ());
  (*iterator).second.second->outputFormat =
    configuration_in.configuration_->format;
  if (inherited::configuration_->configuration_->sourceType != AUDIOEFFECT_SOURCE_FILE)
    session_data_p->formats.push_back (configuration_in.configuration_->format);
  session_data_p->targetFileName =
    (*iterator_2).second.second->fileIdentifier.identifier;

  // ---------------------------------------------------------------------------

  // ********************** Spectrum Analyzer *****************
  module_p =
      const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  idispatch_p =
      dynamic_cast<Test_U_AudioEffect_IDispatch_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (idispatch_p);
  (*iterator).second.second->dispatch = idispatch_p;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
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
