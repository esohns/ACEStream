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
Test_I_DirectShow_Stream::load (Stream_ILayout* layout_inout,
                                bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::load"));

  // initialize return value(s)
  deleteModules_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_->dispatchConfiguration);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  return true;
}

bool
Test_I_DirectShow_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);

  inherited::isInitialized_ = true;

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
Test_I_MediaFoundation_Stream::load (Stream_ILayout* layout_inout,
                                     bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::load"));

  // initialize return value(s)
  deleteModules_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_->dispatchConfiguration);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  return true;
}

bool
Test_I_MediaFoundation_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);

  inherited::isInitialized_ = true;

  return true;
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
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_StatisticAnalysis_Module (this,
                                                        ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_ANALYSIS_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_Vis_SpectrumAnalyzer_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (DEEPSPEECH_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_I_ALSA_DeepSpeechDecoder_Module (this,
                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_DEEPSPEECH_DECODER_DEFAULT_NAME_STRING)),
                  false);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#endif // DEEPSPEECH_SUPPORT

//  // *NOTE*: this processing stream may have branches, depending on:
//  //         - whether the output is muted
//  //         - whether the output is saved to file
//  if (!(*iterator).second.second->mute ||
//      !(*iterator_3).second.second->fileIdentifier.empty ())
//  {
//    typename inherited::MODULE_T* branch_p = NULL; // NULL: 'main' branch
//    unsigned int index_i = 0;
//    if (!(*iterator).second.second->mute &&
//        !(*iterator_3).second.second->fileIdentifier.empty ())
//    {
//      ACE_NEW_RETURN (module_p,
//                      Test_I_ALSA_Distributor_Module (this,
//                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
//                      false);
//      branch_p = module_p;
//      inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME));
//      inherited::configuration_->configuration_->branches.push_back (ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_SAVE_NAME));
//      Stream_IDistributorModule* idistributor_p =
//        dynamic_cast<Stream_IDistributorModule*> (module_p->writer ());
//      ACE_ASSERT (idistributor_p);
//      idistributor_p->initialize (inherited::configuration_->configuration_->branches);
//      layout_in->append (module_p, NULL, 0);
//      module_p = NULL;
//    } // end IF

//    if (!(*iterator).second.second->mute)
//    {
//      ACE_NEW_RETURN (module_p,
//                      Test_I_Target_ALSA_Module (this,
//                                                 ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING)),
//                      false);
//      layout_in->append (module_p, branch_p, index_i);
//      ++index_i;
//      module_p = NULL;
//    } // end IF
//    if (!(*iterator_3).second.second->fileIdentifier.empty ())
//    {
//      ACE_NEW_RETURN (module_p,
//                      Test_I_ALSA_WAVEncoder_Module (this,
//                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING)),
//                      false);
//      layout_in->append (module_p, branch_p, index_i);
//      ++index_i;
//      module_p = NULL;
//      // *NOTE*: currently, on UNIX systems, the WAV encoder writes the WAV file
//      //         itself
//      //  ACE_NEW_RETURN (module_p,
//      //                  Test_I_ALSA_FileWriter_Module (this,
//      //                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_FILE_SINK_DEFAULT_NAME_STRING)),
//      //                  false);
//      //  modules_out.push_back (module_p);
//      //  module_p = NULL;
//    } // end IF
//  } // end ELSE

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
  typename inherited::ISTREAM_T::MODULE_T* module_p = NULL;
  Stream_Statistic_IDispatch_t* idispatch_p = NULL;

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

  // **************************** Spectrum Analyzer ****************************
  module_p =
    const_cast<typename inherited::ISTREAM_T::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  idispatch_p =
    dynamic_cast<Stream_Statistic_IDispatch_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
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
