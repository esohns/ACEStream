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
Test_I_ALSA_Stream::load (Stream_ILayout* layout_inout,
                          bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::load"));

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
Test_I_ALSA_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::initialize"));

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
#endif // ACE_WIN32 || ACE_WIN64
