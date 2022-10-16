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

#include "test_i_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "stream_dec_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_DirectShow_Stream::Test_I_DirectShow_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::Test_I_DirectShow_Stream"));

}

bool
Test_I_DirectShow_Stream::load (Stream_ILayout* layout_in,
                                bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::load"));

  // initialize return value(s)
  deleteModules_out = false;

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

  Stream_Module_t* module_p = NULL, *module_2 = NULL;
  bool device_can_render_format_b = false;
  HRESULT result = E_FAIL;
  bool has_directshow_source_b = true;

  switch (inherited::configuration_->configuration_->capturer)
  {
    case STREAM_DEVICE_CAPTURER_WAVEIN:
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_Mic_Source_DirectShow_WaveIn_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEIN_CAPTURE_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case STREAM_DEVICE_CAPTURER_WASAPI:
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_Mic_Source_DirectShow_WASAPI_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_CAPTURE_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_Mic_Source_DirectShow_Module (this,
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
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_DirectShow_StatisticReport_Module (this,
  //                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

#if defined (FFMPEG_SUPPORT)
  if (!(*iterator).second.second->filtersDescription.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_DirectShow_FfmpegFilter_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_FILTER_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  } // end IF
#endif // FFMPEG_SUPPORT

#if defined (SOX_SUPPORT)
   ACE_NEW_RETURN (module_p,
                   Test_I_DirectShow_SoXResampler_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING)),
                   false);
   ACE_ASSERT (module_p);
   layout_in->append (module_p, NULL, 0);
   module_p = NULL;

   ACE_NEW_RETURN (module_p,
                   Test_I_DirectShow_SoXEffect_Module (this,
                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING)),
                   false);
   ACE_ASSERT (module_p);
   layout_in->append (module_p, NULL, 0);
   module_p = NULL;
#endif // SOX_SUPPORT

  // sanity check(s)
  ACE_ASSERT (InlineIsEqualGUID (inherited::configuration_->configuration_->format.formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (inherited::configuration_->configuration_->format.pbFormat);

  //struct tWAVEFORMATEX* waveformatex_p =
  //  reinterpret_cast<struct tWAVEFORMATEX*> (inherited::configuration_->configuration_->format.pbFormat);
  //switch (inherited::configuration_->configuration_->renderer)
  //{
  //  case STREAM_DEVICE_RENDERER_WAVEOUT:
  //  { ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
  //    device_can_render_format_b =
  //      Stream_MediaFramework_DirectSound_Tools::canRender ((*iterator_3).second.second->deviceIdentifier.identifier._id,
  //                                                          *waveformatex_p);
  //    break;
  //  }
  //  case STREAM_DEVICE_RENDERER_WASAPI:
  //  { ACE_ASSERT ((*iterator_3).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
  //    device_can_render_format_b =
  //      Stream_MediaFramework_DirectSound_Tools::canRender ((*iterator_3).second.second->deviceIdentifier.identifier._guid,
  //                                                          STREAM_LIB_WASAPI_RENDER_DEFAULT_SHAREMODE,
  //                                                          *waveformatex_p);
  //    break;
  //  }
  //  case STREAM_DEVICE_RENDERER_DIRECTSHOW:
  //    break;
  //  default:
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
  //                ACE_TEXT (stream_name_string_),
  //                inherited::configuration_->configuration_->renderer));
  //    return false;
  //  }
  //} // end SWITCH

  //if ((!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer == STREAM_DEVICE_RENDERER_DIRECTSHOW)) ||
  //    (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW) && !device_can_render_format_b))
  //{
  //  ACE_NEW_RETURN (module_p,
  //                  Test_I_DirectShow_Target_Module (this,
  //                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING)),
  //                  false);
  //  ACE_ASSERT (module_p);
  //  layout_in->append (module_p, NULL, 0);
  //  module_p = NULL;
  //} // end IF

  //has_directshow_source_b =
  //  (!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW) && !device_can_render_format_b);
  //if (has_directshow_source_b)
  //{
  //  ACE_NEW_RETURN (module_p,
  //                  Test_I_DirectShow_Source_Module (this,
  //                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_SOURCE_DEFAULT_NAME_STRING)),
  //                  false);
  //  ACE_ASSERT (module_p);
  //  layout_in->append (module_p, NULL, 0);
  //  module_p = NULL;
  //} // end IF

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  Stream_Branches_t branches_a;

  //if ((!(*iterator).second.second->mute && (inherited::configuration_->configuration_->renderer != STREAM_DEVICE_RENDERER_DIRECTSHOW)) &&
  //    !(*iterator_4).second.second->fileIdentifier.empty ())
  //{
    ACE_NEW_RETURN (module_p,
                    Test_I_DirectShow_Distributor_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    branch_p = module_p;
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
    Stream_IDistributorModule* idistributor_p =
      dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
    ACE_ASSERT (idistributor_p);
    idistributor_p->initialize (branches_a);
    layout_in->append (module_p, NULL, 0);
    module_p = NULL;
  //} // end IF

  //if (!(*iterator).second.second->mute)
  //  switch (inherited::configuration_->configuration_->renderer)
  //  {
  //    case STREAM_DEVICE_RENDERER_WAVEOUT:
  //    {
  //      ACE_NEW_RETURN (module_p,
  //                      Test_I_DirectShow_Target_WaveOut_Module (this,
  //                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING)),
  //                      false);
  //      break;
  //    }
  //    case STREAM_DEVICE_RENDERER_WASAPI:
  //    {
  //      ACE_NEW_RETURN (module_p,
  //                      Test_I_DirectShow_Target_WASAPI_Module (this,
  //                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING)),
  //                      false);
  //      break;
  //    }
  //    case STREAM_DEVICE_RENDERER_DIRECTSHOW:
  //      break;
  //    default:
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
  //                  ACE_TEXT (stream_name_string_),
  //                  inherited::configuration_->configuration_->renderer));
  //      return false;
  //    }
  //  } // end SWITCH
  //if (module_p)
  //{
  //  //if (!has_directshow_source_b && !device_can_render_format_b)
  //  //{
  //  //  ACE_NEW_RETURN (module_2,
  //  //                  Test_I_DirectShow_Delay_Module (this,
  //  //                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING)),
  //  //                  false);
  //  //  ACE_ASSERT (module_2);
  //  //  layout_in->append (module_2, branch_p, index_i);
  //  //  module_2 = NULL;
  //  //} // end IF
  //}

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  //ACE_NEW_RETURN (module_2,
  //                Test_I_DirectShow_StatisticAnalysis_Module (this,
  //                                                            ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_2);
  //layout_in->append (module_2, branch_p, index_i);
  //module_2 = NULL;
  ACE_NEW_RETURN (module_2,
                  Test_I_DirectShow_Vis_SpectrumAnalyzer_Module (this,
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

#if defined (DEEPSPEECH_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_DirectShow_DeepSpeechDecoder_Module (this,
                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_DEEPSPEECH_DECODER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#endif // DEEPSPEECH_SUPPORT

  //if (!(*iterator_4).second.second->fileIdentifier.empty ())
  //{
  //  ACE_NEW_RETURN (module_p,
  //                  Test_I_DirectShow_WAVEncoder_Module (this,
  //                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
  //                  false);
  //  ACE_ASSERT (module_p);
  //  layout_in->append (module_p, branch_p, index_i);
  //  module_p = NULL;

  //  ACE_NEW_RETURN (module_p,
  //                  Test_I_DirectShow_FileWriter_Module (this,
  //                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
  //                  false);
  //  ACE_ASSERT (module_p);
  //  layout_in->append (module_p, branch_p, index_i);
  //  module_p = NULL;
  //} // end IF

  return true;
}

//////////////////////////////////////////

Test_I_MediaFoundation_Stream::Test_I_MediaFoundation_Stream ()
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
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::Test_I_MediaFoundation_Stream"));

}

Test_I_MediaFoundation_Stream::~Test_I_MediaFoundation_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::~Test_I_MediaFoundation_Stream"));

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

void
Test_I_MediaFoundation_Stream::start ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::start"));

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

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
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

  PropVariantClear (&property_s);

  inherited::start ();

  return;

error:
  PropVariantClear (&property_s);
}

void
Test_I_MediaFoundation_Stream::stop (bool waitForCompletion_in,
                                     bool recurseUpstream_in,
                                     bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::stop"));

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
                   recurseUpstream_in,
                   highPriority_in);
}

bool
Test_I_MediaFoundation_Stream::load (Stream_ILayout* layout_in,
                                     bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::load"));

  // initialize return value(s)
  deleteModules_out = true;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
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

  switch (inherited::configuration_->configuration_->capturer)
  {
    case STREAM_DEVICE_CAPTURER_WAVEIN:
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_Mic_Source_MediaFoundation_WaveIn_Module (this,
                                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEIN_CAPTURE_DEFAULT_NAME_STRING)),
                      false);
      break;
    }
    case STREAM_DEVICE_CAPTURER_WASAPI:
    {
      ACE_NEW_RETURN (module_p,
                      Test_I_Mic_Source_MediaFoundation_WASAPI_Module (this,
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
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_MediaFoundation_StatisticReport_Module (this,
  //                                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;

#if defined (FFMPEG_SUPPORT)
  //if (!(*iterator).second.second->filtersDescription.empty ())
  //{
  //  ACE_NEW_RETURN (module_p,
  //                  Test_I_MediaFoundation_FfmpegFilter_Module (this,
  //                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_FILTER_DEFAULT_NAME_STRING)),
  //                  false);
  //  ACE_ASSERT (module_p);
  //  layout_in->append (module_p, NULL, 0);
  //  module_p = NULL;
  //} // end IF
#endif // FFMPEG_SUPPORT

#if defined (SOX_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_MediaFoundation_SoXResampler_Module (this,
                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  //ACE_NEW_RETURN (module_p,
  //                Test_I_MediaFoundation_SoXEffect_Module (this,
  //                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  //module_p = NULL;
#endif // SOX_SUPPORT

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  Stream_Branches_t branches_a;

  ACE_NEW_RETURN (module_p,
                  Test_I_MediaFoundation_Distributor_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  branch_p = module_p;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
  if (!(*iterator_3).second.second->fileIdentifier.empty ())
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DECODE_NAME));
  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  if (!idistributor_p->initialize (branches_a))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Miscellaneous_Distributor_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  //ACE_NEW_RETURN (module_p,
  //                Test_I_MediaFoundation_StatisticAnalysis_Module (this,
  //                                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, branch_p, index_i);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_MediaFoundation_Vis_SpectrumAnalyzer_Module (this,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT

  ++index_i;
  if (!(*iterator_3).second.second->fileIdentifier.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_MediaFoundation_WAVEncoder_Module (this,
                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;

    ACE_NEW_RETURN (module_p,
                    Test_I_MediaFoundation_FileWriter_Module (this,
                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
  } // end IF

#if defined (DEEPSPEECH_SUPPORT)
  ++index_i;
  ACE_NEW_RETURN (module_p,
                  Test_I_MediaFoundation_DeepSpeechDecoder_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_DEEPSPEECH_DECODER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
#endif // DEEPSPEECH_SUPPORT

  return true;
}

bool
Test_I_MediaFoundation_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

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
  Test_I_SpeechCommand_MediaFoundation_SessionData& session_data_r =
    const_cast<Test_I_SpeechCommand_MediaFoundation_SessionData&> (inherited::sessionData_->getR ());
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
  IMFMediaType* media_type_p = NULL, *media_type_2 = NULL; // capture-
  bool use_framework_renderer_b = false;
  int render_device_id_i = -1;
  std::string effect_options; // *TODO*

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
    Test_I_MediaFoundation_Target* writer_p =
      &const_cast<Test_I_MediaFoundation_Target&> (getR_4 ());
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
                  ACE_TEXT ("failed to Test_I_MediaFoundation_Stream::QueryInterface(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
  } // end IF

  Stream_Module_t* module_p =
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
          Stream_MediaFramework_DirectSound_Tools::getBestFormat ((*iterator_3).second.second->deviceIdentifier,
                                                                  waveformatex_s);
          media_type_p = Stream_MediaFramework_MediaFoundation_Tools::to (waveformatex_s);
          ACE_ASSERT (media_type_p);
          break;
        }
        case STREAM_DEVICE_RENDERER_WASAPI:
        {
          struct tWAVEFORMATEX* waveformatex_p =
            Stream_MediaFramework_DirectSound_Tools::getAudioEngineMixFormat ((*iterator_3).second.second->deviceIdentifier);
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
          goto error;
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
  use_framework_renderer_b = false;
  switch (inherited::configuration_->configuration_->renderer)
  {
    case STREAM_DEVICE_RENDERER_WAVEOUT:
    {
      render_device_id_i = (*iterator_3).second.second->deviceIdentifier;
      break;
    }
    case STREAM_DEVICE_RENDERER_WASAPI:
    {
      render_device_id_i =
        static_cast<int> (Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId ((*iterator_3).second.second->deviceIdentifier));
      break;
    }
    case STREAM_DEVICE_RENDERER_MEDIAFOUNDATION:
    {
      render_device_id_i =
        static_cast<int> (Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId ((*iterator_3).second.second->deviceIdentifier));
      use_framework_renderer_b = !(*iterator).second.second->mute;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown renderer type (was: %d), aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  inherited::configuration_->configuration_->renderer));
      goto error;
    }
  } // end SWITCH
  media_type_2 =
    ((configuration_in.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION) ? Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration_->format)
                                                                                           : NULL); // use preset source format
  if (!Stream_Module_Decoder_Tools::loadAudioRendererTopology ((*iterator).second.second->deviceIdentifier,
                                                               MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                               (configuration_in.configuration_->capturer == STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION),
                                                               media_type_2,
                                                               media_type_p,
                                                               sample_grabber_p,
                                                               (use_framework_renderer_b ? render_device_id_i : -1),
                                                               GUID_NULL,
                                                               effect_options,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadAudioRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::GUIDToString ((*iterator).second.second->deviceIdentifier).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_2 && topology_p);
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

  (*iterator_2).second.second->manageMediaSession = false;

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
  if (media_type_2)
  {
    media_type_2->Release (); media_type_2 = NULL;
  } // end IF
  media_type_p =
    Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration_->format);
  ACE_ASSERT (media_type_p);
  session_data_r.formats.push_back (media_type_p);

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
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
  if (media_type_2)
    media_type_2->Release ();
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

const Test_I_MediaFoundation_Target&
Test_I_MediaFoundation_Stream::getR_4 () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::getR_4"));

  Test_I_MediaFoundation_Target* writer_p =
    static_cast<Test_I_MediaFoundation_Target*> (const_cast<Test_I_MediaFoundation_Target_Module&> (mediaFoundationTarget_).writer ());
  ACE_ASSERT (writer_p);

  return *writer_p;
}

const Test_I_MediaFoundation_Source&
Test_I_MediaFoundation_Stream::getR_5 () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::getR_5"));

  Test_I_MediaFoundation_Source* writer_p =
    static_cast<Test_I_MediaFoundation_Source*> (const_cast<Test_I_MediaFoundation_Source_Module&> (mediaFoundationSource_).writer ());
  ACE_ASSERT (writer_p);

  return *writer_p;
}

const Test_I_Mic_Source_MediaFoundation&
Test_I_MediaFoundation_Stream::getR_6 () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::getR_6"));

  Test_I_Mic_Source_MediaFoundation* writer_p =
    static_cast<Test_I_Mic_Source_MediaFoundation*> (const_cast<Test_I_Mic_Source_MediaFoundation_Module&> (frameworkSource_).writer ());
  ACE_ASSERT (writer_p);

  return *writer_p;
}

HRESULT
Test_I_MediaFoundation_Stream::QueryInterface (const IID& IID_in,
                                               void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (Test_I_MediaFoundation_Stream, IMFAsyncCallback),
    { 0 },
  };

  HRESULT result = QISearch (this,
                             query_interface_table,
                             IID_in,
                             interface_out);
  if (result == E_NOINTERFACE)
  {
    Test_I_MediaFoundation_Target* writer_p =
      static_cast<Test_I_MediaFoundation_Target*> (mediaFoundationTarget_.writer ());
    ACE_ASSERT (writer_p);
    result = writer_p->QueryInterface (IID_in,
                                       interface_out);
  } // end IF

  return result;
}

HRESULT
Test_I_MediaFoundation_Stream::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::Invoke"));

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
                  ACE_TEXT ("%s: received MESessionCapabilitiesChanged (now/delta): 0x%x/0x%x\n"),
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
                  ACE_TEXT ("%s: received MESessionStopped, closing session\n"),
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
        { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, E_FAIL);
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
Test_I_ALSA_Stream::Test_I_ALSA_Stream ()
  : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::Test_I_ALSA_Stream"));

}

bool
Test_I_ALSA_Stream::load (Stream_ILayout* layout_in,
                          bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::load"));

  // initialize return value(s)
  deleteModules_out = true;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_3 =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_3 != inherited::configuration_->end ());
  Stream_Branches_t branches_a;
  Stream_Module_t* module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_Mic_Source_ALSA_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  //  ACE_NEW_RETURN (module_p,
  //                  Test_I_ALSA_StatisticReport_Module (this,
  //                                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                  false);
  //  layout_in->append (module_p, NULL, 0);
  //  module_p = NULL;

#if defined (SOX_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_SoXResampler_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_SoXEffect_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_EFFECT_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#endif // SOX_SUPPORT

  typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
  unsigned int index_i = 0;
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_Distributor_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  branch_p = module_p;
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME));
  if (!(*iterator_3).second.second->fileIdentifier.empty ())
    branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
  branches_a.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DECODE_NAME));
  Stream_IDistributorModule* idistributor_p =
    dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
  ACE_ASSERT (idistributor_p);
  if (!idistributor_p->initialize (branches_a))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Miscellaneous_Distributor_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

//  ACE_NEW_RETURN (module_p,
//                  Test_I_ALSA_StatisticAnalysis_Module (this,
//                                                        ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
//                  false);
//  layout_in->append (module_p, branch_p, index_i);
//  module_p = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_Vis_SpectrumAnalyzer_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT

  ++index_i;
  if (!(*iterator_3).second.second->fileIdentifier.empty ())
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_ALSA_WAVEncoder_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    layout_in->append (module_p, branch_p, index_i);
    module_p = NULL;
  } // end IF

#if defined (DEEPSPEECH_SUPPORT)
  ++index_i;
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_DeepSpeechDecoder_Module (this,
                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_DEEPSPEECH_DECODER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, branch_p, index_i);
  module_p = NULL;
#endif // DEEPSPEECH_SUPPORT

  return true;
}

bool
Test_I_ALSA_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.configuration_);

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_I_SpeechCommand_ALSA_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator_2;

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
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<Test_I_SpeechCommand_ALSA_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  // sanity check(s)
  iterator =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  iterator_2 =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING));
  ACE_ASSERT (iterator_2 != configuration_in.end ());
  session_data_p->formats.push_back (configuration_in.configuration_->format);
  session_data_p->targetFileName =
    (*iterator_2).second.second->fileIdentifier.identifier;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

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
