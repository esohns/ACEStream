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

#include "stream_lib_alsa_tools.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"

bool
Stream_MediaFramework_ALSA_Tools::getCaptureVolumeLevels (const std::string& cardName_in,
                                                          const std::string& simpleElementName_in,
                                                          long& minLevel_out,
                                                          long& maxLevel_out,
                                                          long& currentLevel_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::getCaptureVolumeLevels"));

  // initialize return value(s)
  minLevel_out = 0;
  maxLevel_out = 0;
  currentLevel_out = 0;
  bool result = false;

  snd_mixer_t* handle_p = NULL;
  snd_mixer_selem_id_t* simple_elem_id_p = NULL;
  snd_mixer_elem_t* simple_elem_p = NULL;
  int result_2 = -1;
  int mode = 0;

  result_2 = snd_mixer_open (&handle_p, mode);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (handle_p);
  result_2 = snd_mixer_attach (handle_p, cardName_in.c_str ());
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_attach(0x%@,\"%s\"): \"%m\", aborting\n"),
                handle_p, ACE_TEXT (cardName_in.c_str ())));
    goto error;
  } // end IF
  result_2 = snd_mixer_selem_register (handle_p,
                                       NULL,  // options
                                       NULL); // classp
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_selem_register(0x%@): \"%m\", aborting\n"),
                handle_p));
    goto error;
  } // end IF
  result_2 = snd_mixer_load (handle_p);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_load(0x%@): \"%m\", aborting\n"),
                handle_p));
    goto error;
  } // end IF

  result_2 = snd_mixer_selem_id_malloc (&simple_elem_id_p);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_mixer_selem_id_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (simple_elem_id_p);
  snd_mixer_selem_id_set_index (simple_elem_id_p, 0);
  snd_mixer_selem_id_set_name (simple_elem_id_p, simpleElementName_in.c_str ());
  simple_elem_p = snd_mixer_find_selem (handle_p, simple_elem_id_p);
  if (!simple_elem_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_find_selem(0x%@,\"%s\"): \"%m\", aborting\n"),
                handle_p, ACE_TEXT (simpleElementName_in.c_str ())));
    goto error;
  } // end IF
  result_2 = snd_mixer_selem_get_capture_volume_range (simple_elem_p,
                                                       &minLevel_out,
                                                       &maxLevel_out);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_selem_get_capture_volume_range(0x%@): \"%m\", aborting\n"),
                simple_elem_p));
    goto error;
  } // end IF
  result_2 = snd_mixer_selem_get_capture_volume (simple_elem_p,
                                                 SND_MIXER_SCHN_FRONT_LEFT,
                                                 &currentLevel_out);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_selem_get_capture_volume(0x%@,%d): \"%m\", aborting\n"),
                simple_elem_p, SND_MIXER_SCHN_FRONT_LEFT));
    goto error;
  } // end IF
  snd_mixer_selem_id_free (simple_elem_id_p); simple_elem_id_p = NULL;
  result_2 = snd_mixer_close (handle_p);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_close(0x%@): \"%m\", aborting\n"),
                handle_p));
    handle_p = NULL;
    goto error;
  } // end IF
  handle_p = NULL;

  result = true;

error:
  if (simple_elem_id_p)
    snd_mixer_selem_id_free (simple_elem_id_p);
  if (handle_p)
    snd_mixer_close (handle_p);
  return result;
}

bool
Stream_MediaFramework_ALSA_Tools::setCaptureVolumeLevel (const std::string& cardName_in,
                                                         const std::string& simpleElementName_in,
                                                         long level_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::setCaptureVolumeLevel"));

  bool result = false;

  snd_mixer_t* handle_p = NULL;
  snd_mixer_selem_id_t* simple_elem_id_p = NULL;
  snd_mixer_elem_t* simple_elem_p = NULL;
  int result_2 = -1;
  int mode = 0;

  result_2 = snd_mixer_open (&handle_p, mode);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (handle_p);
  result_2 = snd_mixer_attach (handle_p, cardName_in.c_str ());
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_attach(0x%@,\"%s\"): \"%m\", aborting\n"),
                handle_p, ACE_TEXT (cardName_in.c_str ())));
    goto error;
  } // end IF
  result_2 = snd_mixer_selem_register (handle_p,
                                       NULL,  // options
                                       NULL); // classp
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_selem_register(0x%@): \"%m\", aborting\n"),
                handle_p));
    goto error;
  } // end IF
  result_2 = snd_mixer_load (handle_p);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_load(0x%@): \"%m\", aborting\n"),
                handle_p));
    goto error;
  } // end IF

  result_2 = snd_mixer_selem_id_malloc (&simple_elem_id_p);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_mixer_selem_id_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (simple_elem_id_p);
  snd_mixer_selem_id_set_index (simple_elem_id_p, 0);
  snd_mixer_selem_id_set_name (simple_elem_id_p, simpleElementName_in.c_str ());
  simple_elem_p = snd_mixer_find_selem (handle_p, simple_elem_id_p);
  if (!simple_elem_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_find_selem(0x%@,\"%s\"): \"%m\", aborting\n"),
                handle_p, ACE_TEXT (simpleElementName_in.c_str ())));
    goto error;
  } // end IF
  result_2 = snd_mixer_selem_set_capture_volume_all (simple_elem_p,
                                                     level_in);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_selem_set_capture_volume_all(0x%@,%d): \"%m\", aborting\n"),
                simple_elem_p, level_in));
    goto error;
  } // end IF
  snd_mixer_selem_id_free (simple_elem_id_p); simple_elem_id_p = NULL;
  result_2 = snd_mixer_close (handle_p);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_mixer_close(0x%@): \"%m\", aborting\n"),
                handle_p));
    handle_p = NULL;
    goto error;
  } // end IF
  handle_p = NULL;

  result = true;

error:
  if (simple_elem_id_p)
    snd_mixer_selem_id_free (simple_elem_id_p);
  if (handle_p)
    snd_mixer_close (handle_p);
  return result;
}
