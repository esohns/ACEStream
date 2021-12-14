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

#include "test_u_audioeffect_module_mediafoundation_target.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget::Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget::Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget"));

}

bool
Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget::initialize (const struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration& configuration_in,
                                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget::initialize"));

  inherited::delayStart_ = true;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

bool
Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget::updateMediaSession (IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_MediaFoundationTarget::updateMediaSession"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  if (!inherited::mediaSession_)
    return true; // nothing to do

  bool was_running_b = true;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags_i =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFTopology* topology_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  ACE_Time_Value deadline =
    ACE_Time_Value (STREAM_LIB_MEDIAFOUNDATION_MEDIASESSION_READY_TIMEOUT_S, 0);
  HRESULT result = E_FAIL;
  int result_2 = -1;
  int error = 0;

  // step1: stop the media session
  result = inherited::mediaSession_->Stop ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    was_running_b = false;
  } // end IF

  // step2: retrieve topology
  result = inherited::mediaSession_->GetFullTopology (flags_i,
                                                      0,
                                                      &topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);

  // step3: update topology
  if (!Stream_Module_Decoder_Tools::updateRendererTopology (topology_p,
                                                            mediaType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::updateRendererTopology(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // step4: reset topology
  result = mediaSession_->ClearTopologies ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::ClearTopologies(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = mediaSession_->SetTopology (MFSESSION_SETTOPOLOGY_CLEAR_CURRENT,
                                       NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::SetTopology(MFSESSION_SETTOPOLOGY_CLEAR_CURRENT): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, false);
    inherited::topologyIsReady_ = false;
    result = mediaSession_->SetTopology (MFSESSION_SETTOPOLOGY_IMMEDIATE |
                                         MFSESSION_SETTOPOLOGY_NORESOLUTION,
                                         topology_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    // wait for MF_TOPOSTATUS_READY event
//    deadline = COMMON_TIME_NOW + deadline;
//    result_2 = inherited::condition_.wait (&deadline);
//    if (unlikely (result_2 == -1))
//    {
//      error = ACE_OS::last_error ();
//      if (error != ETIME) // 137: timed out
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to ACE_Condition::wait(%#T): \"%m\", aborting\n"),
//                    inherited::mod_->name (),
//                    &deadline));
//      goto continue_;
//    } // end IF
//continue_:
//    if (!inherited::topologyIsReady_)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: topology not ready%s, aborting\n"),
//                  inherited::mod_->name (),
//                  (error == ETIME) ? ACE_TEXT (" (timed out)") : ACE_TEXT ("")));
//      goto error;
//    } // end IF
  } // end lock scope
  topology_p->Release (); topology_p = NULL;

  // step5: restart the media session ?
  if (likely (was_running_b))
  {
    result = inherited::mediaSession_->Start (&GUID_s,      // time format
                                              &property_s); // start position
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Start(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF
  PropVariantClear (&property_s);

  return true;

error:
  if (topology_p)
    topology_p->Release ();
  PropVariantClear (&property_s);

  return false;
}
