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

  // step1: stop the media session
  HRESULT result = inherited::mediaSession_->Stop ();
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 mediaSession_,
                                                                 false, // is partial ?
                                                                 true)) // wait for completion ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

  // step4: restart the media session ?
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
