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

#include <sstream>

#if defined (SOX_SUPPORT)
#include "sox.h"
#endif // SOX_SUPPORT

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"

bool
Stream_MediaFramework_ALSA_Tools::canRender (struct _snd_pcm* handle_in,
                                             const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::canRender"));

  // sanity check(s)
  ACE_ASSERT (handle_in);

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
}

bool
Stream_MediaFramework_ALSA_Tools::getDefaultFormat (const std::string& cardName_in,
                                                    bool isCapture_in,
                                                    struct Stream_MediaFramework_ALSA_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::getDefaultFormat"));

  // initialize return value(s)
  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_ALSA_MediaType));

  bool result = false;
  int result_2 = -1;
  struct _snd_pcm* handle_p = NULL;
  int mode_i = SND_PCM_NO_AUTO_RESAMPLE |
               SND_PCM_NO_AUTO_CHANNELS |
               SND_PCM_NO_AUTO_FORMAT;
  struct _snd_pcm_hw_params* snd_pcm_hw_params_p = NULL;
  unsigned int sample_rate_denominator_i = 0;

  result_2 =
    snd_pcm_open (&handle_p,
                  cardName_in.c_str (),
                  (isCapture_in ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK),
                  mode_i);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_open(%s): \"%s\", aborting\n"),
                ACE_TEXT (cardName_in.c_str ()),
                ACE_TEXT (snd_strerror (result_2))));
    goto error;
  } // end IF

  snd_pcm_hw_params_malloc (&snd_pcm_hw_params_p);
  if (unlikely (!snd_pcm_hw_params_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  result_2 = snd_pcm_hw_params_any (handle_p,
                                    snd_pcm_hw_params_p);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    goto error;
  } // end IF

  result_2 = snd_pcm_hw_params_get_format (snd_pcm_hw_params_p,
                                           &mediaType_out.format);
  if (unlikely (result_2 < 0))
  {
    if (unlikely (result_2 != -EINVAL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", aborting\n"),
                  ACE_TEXT (snd_strerror (result_2))));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("format not set, setting to default: \"%s\", continuing\n"),
                ACE_TEXT (snd_pcm_format_name (STREAM_LIB_ALSA_DEFAULT_FORMAT))));
    mediaType_out.format = STREAM_LIB_ALSA_DEFAULT_FORMAT;
  } // end IF
  result_2 = snd_pcm_hw_params_get_subformat (snd_pcm_hw_params_p,
                                              &mediaType_out.subFormat);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_subformat(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    goto error;
  } // end IF

  result_2 = snd_pcm_hw_params_get_channels (snd_pcm_hw_params_p,
                                             &mediaType_out.channels);
  if (unlikely (result_2 < 0))
  {
    if (unlikely (result_2 != -EINVAL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", aborting\n"),
                  ACE_TEXT (snd_strerror (result_2))));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("#channels not set, setting to default: %u, continuing\n"),
                STREAM_LIB_ALSA_DEFAULT_CHANNELS));
    mediaType_out.channels = STREAM_LIB_ALSA_DEFAULT_CHANNELS;
  } // end IF
  result_2 = snd_pcm_hw_params_get_rate_numden (snd_pcm_hw_params_p,
                                                &mediaType_out.rate,
                                                &sample_rate_denominator_i);
  if (unlikely (result_2 < 0))
  {
    if (unlikely (result_2 != -EINVAL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_numden(): \"%s\", aborting\n"),
                  ACE_TEXT (snd_strerror (result_2))));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("sample rate not set, setting to default: %u, continuing\n"),
                STREAM_LIB_ALSA_DEFAULT_SAMPLE_RATE));
    mediaType_out.rate = STREAM_LIB_ALSA_DEFAULT_SAMPLE_RATE;
  } // end IF
//  ACE_ASSERT (sample_rate_denominator_i == 1); // *TODO*

  result = true;

error:
  if (snd_pcm_hw_params_p)
  { ACE_ASSERT (handle_p);
    Stream_MediaFramework_ALSA_Tools::dump (handle_p,
                                            false); // any
    snd_pcm_hw_params_free (snd_pcm_hw_params_p);
  } // end IF
  if (handle_p)
  {
    result_2 = snd_pcm_close (handle_p);
    if (unlikely (result_2 < 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(%s): \"%s\", continuing\n"),
                  ACE_TEXT (cardName_in.c_str ()),
                  ACE_TEXT (snd_strerror (result_2))));
  } // end IF

  return result;
}

bool
Stream_MediaFramework_ALSA_Tools::setFormat (struct _snd_pcm* handle_in,
                                             const struct Stream_MediaFramework_ALSA_Configuration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::setFormat"));

  // sanity check(s)
  ACE_ASSERT (handle_in);
  ACE_ASSERT (configuration_in.format);

  // step1: set hardware parameters
  int result = -1;
  struct _snd_pcm_hw_params* snd_pcm_hw_params_p = NULL;
  int subunit_direction = 0;
  unsigned int periods_i = configuration_in.periods;
  snd_pcm_uframes_t period_size_i = configuration_in.periodSize;
  unsigned int period_time_i = configuration_in.periodTime;
  snd_pcm_uframes_t buffer_size_i = configuration_in.bufferSize;
  unsigned int buffer_time_i = configuration_in.bufferTime;
  snd_pcm_sw_params_t* snd_pcm_sw_params_p = NULL;
  snd_pcm_uframes_t threshold_i =
      ((snd_pcm_stream (handle_in) == SND_PCM_STREAM_PLAYBACK) ? configuration_in.bufferSize : 1);

  snd_pcm_hw_params_malloc (&snd_pcm_hw_params_p);
  if (unlikely (!snd_pcm_hw_params_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = snd_pcm_hw_params_any (handle_in, snd_pcm_hw_params_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_access (handle_in, snd_pcm_hw_params_p,
                                         configuration_in.access);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_access(%d): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.access,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_format (handle_in, snd_pcm_hw_params_p,
                                         configuration_in.format->format);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_format(%d): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.format->format,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result =
    snd_pcm_hw_params_set_channels (handle_in, snd_pcm_hw_params_p,
                                    configuration_in.format->channels);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_channels(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.format->channels,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
    snd_pcm_hw_params_set_rate_resample (handle_in,
                                         snd_pcm_hw_params_p,
                                         (configuration_in.rateResample ? 1 : 0));
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_rate_resample(%d): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                (configuration_in.rateResample ? 1 : 0),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_rate (handle_in, snd_pcm_hw_params_p,
                                       configuration_in.format->rate,
                                       subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_rate(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.format->rate,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  /* Set buffer size (in frames). The resulting latency is given by */
  /* latency = periodsize * periods / (rate * bytes_per_frame)      */
  result =
    snd_pcm_hw_params_set_periods_min (handle_in, snd_pcm_hw_params_p,
                                       &periods_i,
                                       &subunit_direction);
//    snd_pcm_hw_params_set_periods_near (handle_in, snd_pcm_hw_params_p,
//                                        &periods_i,
//                                        &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_periods_min(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.periods,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result =
    snd_pcm_hw_params_set_periods_first (handle_in, snd_pcm_hw_params_p,
                                         &periods_i,
                                         &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_periods_first(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.periods,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  if (periods_i != configuration_in.periods)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to set periods (was: %u, result: %u), continuing\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                configuration_in.periods, periods_i));
  subunit_direction = 0;
  result =
    snd_pcm_hw_params_set_period_size_near (handle_in, snd_pcm_hw_params_p,
                                            &period_size_i,
                                            &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_period_size_near(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.periodSize,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  if (period_size_i != configuration_in.periodSize)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to set period size (was: %u, result: %u), continuing\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                configuration_in.periodSize, period_size_i));
  subunit_direction = 0;
  result =
    snd_pcm_hw_params_set_period_time_near (handle_in, snd_pcm_hw_params_p,
                                            &period_time_i,
                                            &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_period_time_near(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.periodTime,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  if (period_time_i != configuration_in.periodTime)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to set period time (was: %u, result: %u), continuing\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                configuration_in.periodTime, period_time_i));

  subunit_direction = 0;
  result =
    snd_pcm_hw_params_set_buffer_size_near (handle_in, snd_pcm_hw_params_p,
                                            &buffer_size_i);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_buffer_size_near(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.bufferSize,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  if (buffer_size_i != configuration_in.bufferSize)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to set buffer size (was: %u, result: %u), continuing\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                configuration_in.bufferSize, buffer_size_i));
  subunit_direction = 0;
  result =
    snd_pcm_hw_params_set_buffer_time_near (handle_in, snd_pcm_hw_params_p,
                                            &buffer_time_i,
                                            &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_buffer_time_near(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), configuration_in.bufferTime,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  if (buffer_time_i != configuration_in.bufferTime)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to set buffer time (was: %u, result: %u), continuing\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                configuration_in.bufferTime, buffer_time_i));

  if (snd_pcm_stream (handle_in) == SND_PCM_STREAM_CAPTURE)
  {
    // *NOTE*: "...For playback, the buffer size determines the latency. For
    //         capture, it does not; ..."
    result = snd_pcm_hw_params_get_buffer_size_max (snd_pcm_hw_params_p,
                                                    &buffer_size_i);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_hw_params_get_buffer_size_max(): \"%s\", aborting\n"),
                  ACE_TEXT (snd_pcm_name (handle_in)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    result = snd_pcm_hw_params_set_buffer_size_max (handle_in, snd_pcm_hw_params_p,
                                                    &buffer_size_i);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_hw_params_set_buffer_time_max(%u): \"%s\", aborting\n"),
                  ACE_TEXT (snd_pcm_name (handle_in)), buffer_size_i,
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
  } // end IF

  result = snd_pcm_hw_params (handle_in,
                              snd_pcm_hw_params_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_hw_params(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  snd_pcm_hw_params_free (snd_pcm_hw_params_p); snd_pcm_hw_params_p = NULL;

  // step2: set software parameters
  result = snd_pcm_sw_params_malloc (&snd_pcm_sw_params_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to snd_pcm_sw_params_malloc(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_sw_params_current (handle_in,
                                      snd_pcm_sw_params_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_sw_params_current(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_sw_params_set_avail_min (handle_in,
                                            snd_pcm_sw_params_p,
                                            period_size_i);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_sw_params_set_avail_min(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), period_size_i,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_sw_params_set_start_threshold (handle_in,
                                                  snd_pcm_sw_params_p,
                                                  std::numeric_limits<snd_pcm_uframes_t>::max ());
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_sw_params_set_start_threshold(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)), std::numeric_limits<snd_pcm_uframes_t>::max (),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  snd_pcm_sw_params_get_boundary (snd_pcm_sw_params_p,
                                  &threshold_i);
  result = snd_pcm_sw_params_set_stop_threshold (handle_in,
                                                 snd_pcm_sw_params_p,
                                                 threshold_i);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_sw_params_set_stop_threshold(%u): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                threshold_i,
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  if (!configuration_in.asynch)
  {
    result = snd_pcm_sw_params_set_period_event (handle_in,
                                                 snd_pcm_sw_params_p,
                                                 1);
    if (result < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_sw_params_set_period_event(1): \"%s\", aborting\n"),
                  ACE_TEXT (snd_pcm_name (handle_in)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
  } // end IF
  result = snd_pcm_sw_params_set_tstamp_mode (handle_in,
                                              snd_pcm_sw_params_p,
                                              SND_PCM_TSTAMP_NONE);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_sw_params_set_tstamp_mode(SND_PCM_TSTAMP_NONE): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_sw_params (handle_in,
                              snd_pcm_sw_params_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to snd_pcm_sw_params(): \"%s\", aborting\n"),
                ACE_TEXT (snd_pcm_name (handle_in)),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  snd_pcm_sw_params_free (snd_pcm_sw_params_p); snd_pcm_sw_params_p = NULL;

  return true;

error:
  if (snd_pcm_hw_params_p)
    snd_pcm_hw_params_free (snd_pcm_hw_params_p);
  if (snd_pcm_sw_params_p)
    snd_pcm_sw_params_free (snd_pcm_sw_params_p);
#if defined (_DEBUG)
  Stream_MediaFramework_ALSA_Tools::dump (handle_in, false);
#endif // _DEBUG

  return false;
}

bool
Stream_MediaFramework_ALSA_Tools::getFormat (struct _snd_pcm* handle_in,
                                             struct Stream_MediaFramework_ALSA_Configuration& configuration_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::getFormat"));

  // sanity check(s)
  ACE_ASSERT (handle_in);
  ACE_ASSERT (configuration_out.format);

  // initialize return value(s)
  struct Stream_MediaFramework_ALSA_MediaType* media_type_p =
    configuration_out.format;
  ACE_OS::memset (&configuration_out, 0, sizeof (struct Stream_MediaFramework_ALSA_Configuration));
  configuration_out.format = media_type_p;
  ACE_OS::memset (&configuration_out.format, 0, sizeof (struct Stream_MediaFramework_ALSA_MediaType));

  int result = -1;
  struct _snd_pcm_hw_params* snd_pcm_hw_params_p = NULL;
  unsigned int sample_rate_numerator, sample_rate_denominator;
  int subunit_direction = 0;

  snd_pcm_hw_params_malloc (&snd_pcm_hw_params_p);
  if (unlikely (!snd_pcm_hw_params_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_current (handle_in,
                                      snd_pcm_hw_params_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_current(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  ACE_ASSERT (snd_pcm_hw_params_p);

  result = snd_pcm_hw_params_get_access (snd_pcm_hw_params_p,
                                         &configuration_out.access);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  configuration_out.asynch =
      ((snd_pcm_stream (handle_in) == SND_PCM_STREAM_PLAYBACK) ? STREAM_LIB_ALSA_PLAYBACK_DEFAULT_ASYNCH
                                                                     : STREAM_LIB_ALSA_CAPTURE_DEFAULT_ASYNCH);
  configuration_out.handle = handle_in;

  result = snd_pcm_hw_params_get_format (snd_pcm_hw_params_p,
                                         &configuration_out.format->format);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_subformat (snd_pcm_hw_params_p,
                                            &configuration_out.format->subFormat);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_subformat(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_channels (snd_pcm_hw_params_p,
                                           &configuration_out.format->channels);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_get_rate_numden (snd_pcm_hw_params_p,
                                              &sample_rate_numerator,
                                              &sample_rate_denominator);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_numden(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  configuration_out.format->rate = sample_rate_numerator;
  ACE_ASSERT (sample_rate_denominator == 1); // *TODO*

  result = snd_pcm_hw_params_get_periods (snd_pcm_hw_params_p,
                                          &configuration_out.periods,
                                          &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_periods(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  subunit_direction = 0;
  result = snd_pcm_hw_params_get_period_size (snd_pcm_hw_params_p,
                                              &configuration_out.periodSize,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  subunit_direction = 0;
  result = snd_pcm_hw_params_get_period_time (snd_pcm_hw_params_p,
                                              &configuration_out.periodTime,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_buffer_size (snd_pcm_hw_params_p,
                                              &configuration_out.bufferSize);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  subunit_direction = 0;
  result = snd_pcm_hw_params_get_buffer_time (snd_pcm_hw_params_p,
                                              &configuration_out.bufferTime,
                                              &subunit_direction);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  snd_pcm_hw_params_free (snd_pcm_hw_params_p); snd_pcm_hw_params_p = NULL;

  return true;

error:
  if (snd_pcm_hw_params_p)
    snd_pcm_hw_params_free (snd_pcm_hw_params_p);

  return false;
}

int
Stream_MediaFramework_ALSA_Tools::getCardNumber (const std::string& cardName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::getCardNumber"));

  int return_value = -1;

  std::string device_id_string = cardName_in;
  std::string::size_type position_i = device_id_string.find (':', 0);
  std::istringstream converter;
  if (position_i != std::string::npos)
  {
    device_id_string = device_id_string.substr (position_i + 1,
                                                std::string::npos);
    converter.str (device_id_string);
    converter >> return_value;
    return return_value;
  } // end IF

  int result = snd_card_get_index (device_id_string.c_str ());
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_card_get_index(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (device_id_string.c_str ()),
                ACE_TEXT (snd_strerror (result))));
    return -1;
  } // end IF
  return_value = result;

  return return_value;
}

std::string
Stream_MediaFramework_ALSA_Tools::getDeviceName (enum _snd_pcm_stream direction_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::getDeviceName"));

  std::string result_string;

  // sanity check(s)
  ACE_ASSERT ((direction_in == SND_PCM_STREAM_CAPTURE) ||
              (direction_in == SND_PCM_STREAM_PLAYBACK));

  void** hints_p = NULL;
  int result =
      snd_device_name_hint (-1,
                            ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_PCM_INTERFACE_NAME),
                            &hints_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_device_name_hint(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    return result_string;
  } // end IF

  char* string_p = NULL;
  std::string hint_string, device_type;
  std::string::size_type position_i = std::string::npos;
  for (void** i = hints_p; *i; ++i)
  {
    string_p = NULL;
    string_p = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("IOID"));
    if (!string_p)
    { // *NOTE*: NULL: device is i/o
      goto continue_;
    } // end IF
    hint_string = string_p;
    free (string_p); string_p = NULL;
    if (ACE_OS::strcmp (hint_string.c_str (),
                        (direction_in == SND_PCM_STREAM_PLAYBACK) ? ACE_TEXT_ALWAYS_CHAR ("Output")
                                                                  : ACE_TEXT_ALWAYS_CHAR ("Input")))
      continue;

continue_:
    string_p = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("NAME"));
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    hint_string = string_p;
    free (string_p); string_p = NULL;

    // filter hardware devices
    device_type = hint_string;
    position_i = hint_string.find (':', 0);
    if (position_i != std::string::npos)
      device_type = device_type.substr (0, position_i);
    if (ACE_OS::strcmp (device_type.c_str (),
                        (direction_in == SND_PCM_STREAM_PLAYBACK) ? ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEVICE_PLAYBACK_PREFIX)
                                                                  : ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEVICE_CAPTURE_PREFIX)))
      continue;
    result_string = hint_string;

//    string_p = snd_device_name_get_hint (*i, "DESC");
//    if (!string_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", aborting\n")));
//      goto clean;
//    } // end IF

//    // clean up
//    free (string_p); string_p = NULL;
    break;
  } // end FOR

clean:
  if (hints_p)
  {
    result = snd_device_name_free_hint (hints_p);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_free_hint(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF

  return result_string;
}

std::string
Stream_MediaFramework_ALSA_Tools::formatToString (const struct _snd_pcm* handle_in,
                                                  const struct _snd_pcm_hw_params* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::formatToString"));

  std::string result;

  // sanity check(s)
  ACE_ASSERT (format_in);

  std::ostringstream converter;
  enum _snd_pcm_access access;
  enum _snd_pcm_format format;
  enum _snd_pcm_subformat sub_format;
  unsigned int channels;
  unsigned int sample_rate_numerator, sample_rate_denominator;
  int subunit_direction = 0;
  unsigned int period_time;
  snd_pcm_uframes_t period_size;
  unsigned int periods;
  unsigned int buffer_time;
  snd_pcm_uframes_t buffer_size;
  unsigned int rate_resample;

  int result_2 = snd_pcm_hw_params_get_access (format_in,
                                               &access);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("access: ");
  result += snd_pcm_access_name (access);
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_format (format_in,
                                           &format);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("format: ");
  result += snd_pcm_format_name (format);
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_subformat (format_in,
                                              &sub_format);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_subformat(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("subformat: ");
  result += snd_pcm_subformat_name (sub_format);
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_channels (format_in,
                                             &channels);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("channels: ");
  converter << channels;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_rate_numden (format_in,
                                                &sample_rate_numerator,
                                                &sample_rate_denominator);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_numden(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("rate: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << sample_rate_numerator;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("/");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << sample_rate_denominator;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result_2 =
      snd_pcm_hw_params_get_rate_resample (const_cast<struct _snd_pcm*> (handle_in),
                                           const_cast<struct _snd_pcm_hw_params*> (format_in),
                                           &rate_resample);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate_resample(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("rate resample: ");
  result += (rate_resample ? ACE_TEXT_ALWAYS_CHAR ("yes")
                           : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result_2 = snd_pcm_hw_params_get_period_time (format_in,
                                                &period_time,
                                                &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("period time: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << period_time;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  subunit_direction = 0;
  result_2 = snd_pcm_hw_params_get_period_size (format_in,
                                                &period_size,
                                                &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_period_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("period size: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << period_size;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  subunit_direction = 0;
  result_2 = snd_pcm_hw_params_get_periods (format_in,
                                            &periods,
                                            &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_periods(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("periods: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << periods;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  subunit_direction = 0;
  result_2 = snd_pcm_hw_params_get_buffer_time (format_in,
                                                &buffer_time,
                                                &subunit_direction);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_time(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("buffer time: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << buffer_time;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result_2 = snd_pcm_hw_params_get_buffer_size (format_in,
                                                &buffer_size);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_buffer_size(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return result;
  } // end IF
  result += ACE_TEXT_ALWAYS_CHAR ("buffer size: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << buffer_size;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] supports sample-resolution mmap: ");
  result +=
      (snd_pcm_hw_params_can_mmap_sample_resolution (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                                : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] double buffering (start/stop): ");
  result +=
      (snd_pcm_hw_params_is_double (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                               : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] double buffering (data transfers): ");
  result +=
      (snd_pcm_hw_params_is_batch (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                              : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] sample block transfer: ");
  result +=
      (snd_pcm_hw_params_is_block_transfer (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                       : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[cfg] monotonic timestamps: ");
  result +=
      (snd_pcm_hw_params_is_monotonic (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                  : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports overrange detection: ");
  result +=
      (snd_pcm_hw_params_can_overrange (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                   : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports pause: ");
  result +=
      (snd_pcm_hw_params_can_pause (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                               : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports resume: ");
  result +=
      (snd_pcm_hw_params_can_resume (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] half-duplex only: ");
  result +=
      (snd_pcm_hw_params_is_half_duplex (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                    : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] joint duplex (capture/playback): ");
  result +=
      (snd_pcm_hw_params_is_joint_duplex (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                     : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports sample-resolution synchronized start: ");
  result +=
      (snd_pcm_hw_params_can_sync_start (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                    : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports disabling period wakeups: ");
  result +=
      (snd_pcm_hw_params_can_disable_period_wakeup (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                               : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("[hw] supports audio wallclock timestamps: ");
  result +=
      (snd_pcm_hw_params_supports_audio_wallclock_ts (format_in) ? ACE_TEXT_ALWAYS_CHAR ("yes")
                                                                 : ACE_TEXT_ALWAYS_CHAR ("no"));
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  result += ACE_TEXT_ALWAYS_CHAR ("significant bits: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << snd_pcm_hw_params_get_sbits (format_in);
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");
  result += ACE_TEXT_ALWAYS_CHAR ("FIFO size (frames): ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << snd_pcm_hw_params_get_fifo_size (format_in);
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("\n");

  return result;
}

void
Stream_MediaFramework_ALSA_Tools::dump (struct _snd_pcm* handle_in,
                                        bool current_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::dump"));

  struct _snd_pcm_hw_params* format_p = NULL;
  int result = -1;
  struct _snd_pcm_format_mask* format_mask_p = NULL;
  std::string formats_string;
  int subunit_direction = 0;
  unsigned int value_min_i, value_max_i;
  snd_pcm_uframes_t period_size_min, period_size_max;
  snd_pcm_uframes_t buffer_size_min, buffer_size_max;

  result = snd_pcm_hw_params_malloc (&format_p);
  if ((result < 0) || !format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  result =
    (current_in ? snd_pcm_hw_params_current (handle_in, format_p)
                : snd_pcm_hw_params_any (handle_in, format_p));
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_%s(): \"%s\", aborting\n"),
                (current_in ? ACE_TEXT ("current") : ACE_TEXT ("any")),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  snd_pcm_format_mask_malloc (&format_mask_p);
  ACE_ASSERT (format_mask_p);
  snd_pcm_hw_params_get_format_mask (format_p,
                                     format_mask_p);
  ACE_ASSERT (result >= 0);
  for (int i = 0, prev_i = 0;
       i <= SND_PCM_FORMAT_LAST;
       ++i)
    if (snd_pcm_format_mask_test (format_mask_p, static_cast<snd_pcm_format_t> (i)))
    {
      if (prev_i)
        formats_string += ACE_TEXT_ALWAYS_CHAR (",");
      formats_string += snd_pcm_format_name (static_cast<snd_pcm_format_t> (i));
      prev_i = 1;
    } // end IF
  snd_pcm_format_mask_free (format_mask_p); format_mask_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sformats: %s...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              ACE_TEXT (formats_string.c_str ())));

  result = snd_pcm_hw_params_get_channels_min (format_p,
                                               &value_min_i);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_channels_max (format_p,
                                               &value_max_i);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %schannels: %u-%u...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              value_min_i, value_max_i));

  result = snd_pcm_hw_params_get_rate_min (format_p,
                                           &value_min_i,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  subunit_direction = 0;
  result = snd_pcm_hw_params_get_rate_max (format_p,
                                           &value_max_i,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %srates: %u-%u...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              value_min_i, value_max_i));

  subunit_direction = 0;
  result =
    snd_pcm_hw_params_get_period_time_min (format_p,
                                           &value_min_i,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  subunit_direction = 0;
  result =
    snd_pcm_hw_params_get_period_time_max (format_p,
                                           &value_max_i,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %speriod times: %u-%u (us)...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              value_min_i, value_max_i));
  subunit_direction = 0;
  result =
    snd_pcm_hw_params_get_period_size_min (format_p,
                                           &period_size_min,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  subunit_direction = 0;
  result =
    snd_pcm_hw_params_get_period_size_max (format_p,
                                           &period_size_max,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %speriod sizes: %u-%u (frames)...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              period_size_min, period_size_max));
  subunit_direction = 0;
  result = snd_pcm_hw_params_get_periods_min (format_p,
                                              &value_min_i,
                                              &subunit_direction);
  ACE_ASSERT (result >= 0);
  subunit_direction = 0;
  result = snd_pcm_hw_params_get_periods_max (format_p,
                                              &value_max_i,
                                              &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %speriods: %u-%u...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              value_min_i, value_max_i));

  subunit_direction = 0;
  result =
    snd_pcm_hw_params_get_buffer_time_min (format_p,
                                           &value_min_i,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  subunit_direction = 0;
  result =
    snd_pcm_hw_params_get_buffer_time_max (format_p,
                                           &value_max_i,
                                           &subunit_direction);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sbuffer times: %u-%u (us)...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              value_min_i, value_max_i));
  result = snd_pcm_hw_params_get_buffer_size_min (format_p,
                                                  &buffer_size_min);
  ACE_ASSERT (result >= 0);
  result = snd_pcm_hw_params_get_buffer_size_max (format_p,
                                                  &buffer_size_max);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sbuffer sizes: %u-%u (frames)...\n"),
              ACE_TEXT (snd_pcm_name (handle_in)),
              (current_in ? ACE_TEXT ("") : ACE_TEXT ("available ")),
              buffer_size_min, buffer_size_max));

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);
}

bool
Stream_MediaFramework_ALSA_Tools::getVolumeLevels (const std::string& cardName_in,
                                                   const std::string& simpleElementName_in,
                                                   bool isCapture_in,
                                                   long& minLevel_out,
                                                   long& maxLevel_out,
                                                   long& currentLevel_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::getVolumeLevels"));

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
                ACE_TEXT ("failed to snd_mixer_find_selem(%@,\"%s\"): \"%m\", aborting\n"),
                handle_p, ACE_TEXT (simpleElementName_in.c_str ())));
    goto error;
  } // end IF
  result_2 =
    (isCapture_in ? snd_mixer_selem_get_capture_volume_range (simple_elem_p,
                                                              &minLevel_out,
                                                              &maxLevel_out)
                  : snd_mixer_selem_get_playback_volume_range (simple_elem_p,
                                                               &minLevel_out,
                                                               &maxLevel_out));
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to %s(0x%@): \"%m\", aborting\n"),
                (isCapture_in ? ACE_TEXT ("snd_mixer_selem_get_capture_volume_range") : ACE_TEXT ("snd_mixer_selem_get_playback_volume_range")),
                simple_elem_p));
    goto error;
  } // end IF
  result_2 =
    (isCapture_in ? snd_mixer_selem_get_capture_volume (simple_elem_p,
                                                        SND_MIXER_SCHN_FRONT_LEFT,
                                                        &currentLevel_out)
                  : snd_mixer_selem_get_playback_volume (simple_elem_p,
                                                         SND_MIXER_SCHN_FRONT_LEFT,
                                                         &currentLevel_out));
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to %s(0x%@,%d): \"%m\", aborting\n"),
                (isCapture_in ? ACE_TEXT ("snd_mixer_selem_get_capture_volume") : ACE_TEXT ("snd_mixer_selem_get_playback_volume")),
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
Stream_MediaFramework_ALSA_Tools::setVolumeLevel (const std::string& cardName_in,
                                                  const std::string& simpleElementName_in,
                                                  bool isCapture_in,
                                                  long level_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::setVolumeLevel"));

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
                ACE_TEXT ("failed to snd_mixer_find_selem(%@,\"%s\"): \"%m\", aborting\n"),
                handle_p, ACE_TEXT (simpleElementName_in.c_str ())));
    goto error;
  } // end IF
  result_2 =
    (isCapture_in ? snd_mixer_selem_set_capture_volume_all (simple_elem_p,
                                                            level_in)
                  : snd_mixer_selem_set_playback_volume_all (simple_elem_p,
                                                             level_in));
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to %s(0x%@,%d): \"%m\", aborting\n"),
                (isCapture_in ? ACE_TEXT ("snd_mixer_selem_set_capture_volume_all") : ACE_TEXT ("snd_mixer_selem_set_playback_volume_all")),
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

#if defined(FFMPEG_SUPPORT)
enum AVSampleFormat
Stream_MediaFramework_ALSA_Tools::ALSAFormatToffmpegFormat (enum _snd_pcm_format format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::ALSAFormatToffmpegFormat"));

  switch (format_in)
  {
    case SND_PCM_FORMAT_S8:
      break;
    case SND_PCM_FORMAT_S16_BE:
    case SND_PCM_FORMAT_S16_LE:
      return AV_SAMPLE_FMT_S16;
    case SND_PCM_FORMAT_S18_3BE:
    case SND_PCM_FORMAT_S18_3LE:
    case SND_PCM_FORMAT_S20_3BE:
    case SND_PCM_FORMAT_S20_3LE:
    case SND_PCM_FORMAT_S20_BE:
    case SND_PCM_FORMAT_S20_LE:
    case SND_PCM_FORMAT_S24_BE:
    case SND_PCM_FORMAT_S24_LE:
    case SND_PCM_FORMAT_S24_3BE:
    case SND_PCM_FORMAT_S24_3LE:
      break;
    case SND_PCM_FORMAT_S32_BE:
    case SND_PCM_FORMAT_S32_LE:
      return AV_SAMPLE_FMT_S32;
    case SND_PCM_FORMAT_DSD_U8:
    case SND_PCM_FORMAT_DSD_U16_BE:
    case SND_PCM_FORMAT_DSD_U16_LE:
    case SND_PCM_FORMAT_DSD_U32_BE:
    case SND_PCM_FORMAT_DSD_U32_LE:
      break;
    case SND_PCM_FORMAT_U8:
      return AV_SAMPLE_FMT_U8;
    case SND_PCM_FORMAT_U16_BE:
    case SND_PCM_FORMAT_U16_LE:
    case SND_PCM_FORMAT_U18_3BE:
    case SND_PCM_FORMAT_U18_3LE:
    case SND_PCM_FORMAT_U20_3BE:
    case SND_PCM_FORMAT_U20_3LE:
    case SND_PCM_FORMAT_U20_BE:
    case SND_PCM_FORMAT_U20_LE:
    case SND_PCM_FORMAT_U24_3BE:
    case SND_PCM_FORMAT_U24_3LE:
    case SND_PCM_FORMAT_U24_BE:
    case SND_PCM_FORMAT_U24_LE:
    case SND_PCM_FORMAT_U32_BE:
    case SND_PCM_FORMAT_U32_LE:
      break;
    case SND_PCM_FORMAT_FLOAT_BE:
    case SND_PCM_FORMAT_FLOAT_LE:
      return AV_SAMPLE_FMT_FLT;
    case SND_PCM_FORMAT_FLOAT64_BE:
    case SND_PCM_FORMAT_FLOAT64_LE:
      return AV_SAMPLE_FMT_DBL;
    case SND_PCM_FORMAT_MU_LAW:
    case SND_PCM_FORMAT_A_LAW:
    case SND_PCM_FORMAT_IMA_ADPCM:
    case SND_PCM_FORMAT_GSM:
    case SND_PCM_FORMAT_G723_24:
    case SND_PCM_FORMAT_G723_24_1B:
    case SND_PCM_FORMAT_G723_40:
    case SND_PCM_FORMAT_G723_40_1B:
    case SND_PCM_FORMAT_IEC958_SUBFRAME_LE:
    case SND_PCM_FORMAT_IEC958_SUBFRAME_BE:
    case SND_PCM_FORMAT_MPEG:
    case SND_PCM_FORMAT_SPECIAL:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ALSA audio frame format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return AV_SAMPLE_FMT_NONE;
}
#endif // FFMPEG_SUPPORT

#if defined (SOX_SUPPORT)
enum sox_encoding_t
Stream_MediaFramework_ALSA_Tools::to (enum _snd_pcm_format format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::to"));

  switch (format_in)
  {
    case SND_PCM_FORMAT_S8:
    case SND_PCM_FORMAT_S16_BE:
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S18_3BE:
    case SND_PCM_FORMAT_S18_3LE:
    case SND_PCM_FORMAT_S20_3BE:
    case SND_PCM_FORMAT_S20_3LE:
    case SND_PCM_FORMAT_S20_BE:
    case SND_PCM_FORMAT_S20_LE:
    case SND_PCM_FORMAT_S24_BE:
    case SND_PCM_FORMAT_S24_LE:
    case SND_PCM_FORMAT_S24_3BE:
    case SND_PCM_FORMAT_S24_3LE:
    case SND_PCM_FORMAT_S32_BE:
    case SND_PCM_FORMAT_S32_LE:
      return SOX_ENCODING_SIGN2;
    case SND_PCM_FORMAT_DSD_U8:
    case SND_PCM_FORMAT_DSD_U16_BE:
    case SND_PCM_FORMAT_DSD_U16_LE:
    case SND_PCM_FORMAT_DSD_U32_BE:
    case SND_PCM_FORMAT_DSD_U32_LE:
    case SND_PCM_FORMAT_U8:
    case SND_PCM_FORMAT_U16_BE:
    case SND_PCM_FORMAT_U16_LE:
    case SND_PCM_FORMAT_U18_3BE:
    case SND_PCM_FORMAT_U18_3LE:
    case SND_PCM_FORMAT_U20_3BE:
    case SND_PCM_FORMAT_U20_3LE:
    case SND_PCM_FORMAT_U20_BE:
    case SND_PCM_FORMAT_U20_LE:
    case SND_PCM_FORMAT_U24_3BE:
    case SND_PCM_FORMAT_U24_3LE:
    case SND_PCM_FORMAT_U24_BE:
    case SND_PCM_FORMAT_U24_LE:
    case SND_PCM_FORMAT_U32_BE:
    case SND_PCM_FORMAT_U32_LE:
      return SOX_ENCODING_UNSIGNED;
    case SND_PCM_FORMAT_FLOAT_BE:
    case SND_PCM_FORMAT_FLOAT_LE:
    case SND_PCM_FORMAT_FLOAT64_BE:
    case SND_PCM_FORMAT_FLOAT64_LE:
      return SOX_ENCODING_FLOAT;
    case SND_PCM_FORMAT_MU_LAW:
      return SOX_ENCODING_ULAW;
    case SND_PCM_FORMAT_A_LAW:
      return SOX_ENCODING_ALAW;
    case SND_PCM_FORMAT_IMA_ADPCM:
      return SOX_ENCODING_IMA_ADPCM;
    case SND_PCM_FORMAT_GSM:
      return SOX_ENCODING_GSM;
    case SND_PCM_FORMAT_G723_24:
    case SND_PCM_FORMAT_G723_24_1B:
    case SND_PCM_FORMAT_G723_40:
    case SND_PCM_FORMAT_G723_40_1B:
      return SOX_ENCODING_G723;
    case SND_PCM_FORMAT_IEC958_SUBFRAME_LE:
    case SND_PCM_FORMAT_IEC958_SUBFRAME_BE:
    case SND_PCM_FORMAT_MPEG:
    case SND_PCM_FORMAT_SPECIAL:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ALSA audio frame format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return SOX_ENCODING_UNKNOWN;
}

void
Stream_MediaFramework_ALSA_Tools::to (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in,
                                      struct sox_encodinginfo_t& encodingInfo_inout,
                                      struct sox_signalinfo_t& signalInfo_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_ALSA_Tools::to"));

  // initialize return value(s)
  ACE_OS::memset (&encodingInfo_inout, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&signalInfo_inout, 0, sizeof (struct sox_signalinfo_t));

  encodingInfo_inout.bits_per_sample =
    snd_pcm_format_width (mediaType_in.format);
  encodingInfo_inout.encoding =
    Stream_MediaFramework_ALSA_Tools::to (mediaType_in.format);

  encodingInfo_inout.reverse_bytes = sox_option_default;
  encodingInfo_inout.reverse_nibbles = sox_option_default;
  encodingInfo_inout.reverse_bits = sox_option_default;
  encodingInfo_inout.opposite_endian = sox_false;
  switch (mediaType_in.format)
  {
    case SND_PCM_FORMAT_DSD_U16_BE:
    case SND_PCM_FORMAT_DSD_U32_BE:
    case SND_PCM_FORMAT_S16_BE:
    case SND_PCM_FORMAT_S18_3BE:
    case SND_PCM_FORMAT_S20_3BE:
    case SND_PCM_FORMAT_S20_BE:
    case SND_PCM_FORMAT_S24_BE:
    case SND_PCM_FORMAT_S24_3BE:
    case SND_PCM_FORMAT_S32_BE:
    case SND_PCM_FORMAT_U16_BE:
    case SND_PCM_FORMAT_U18_3BE:
    case SND_PCM_FORMAT_U20_3BE:
    case SND_PCM_FORMAT_U20_BE:
    case SND_PCM_FORMAT_U24_3BE:
    case SND_PCM_FORMAT_U24_BE:
    case SND_PCM_FORMAT_U32_BE:
#if defined (ACE_LITTLE_ENDIAN)
      encodingInfo_inout.opposite_endian = sox_true;
#endif // ACE_LITTLE_ENDIAN
      break;
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S18_3LE:
    case SND_PCM_FORMAT_S20_3LE:
    case SND_PCM_FORMAT_S20_LE:
    case SND_PCM_FORMAT_S24_LE:
    case SND_PCM_FORMAT_S24_3LE:
    case SND_PCM_FORMAT_S32_LE:
    case SND_PCM_FORMAT_DSD_U16_LE:
    case SND_PCM_FORMAT_DSD_U32_LE:
    case SND_PCM_FORMAT_U16_LE:
    case SND_PCM_FORMAT_U18_3LE:
    case SND_PCM_FORMAT_U20_3LE:
    case SND_PCM_FORMAT_U20_LE:
    case SND_PCM_FORMAT_U24_3LE:
    case SND_PCM_FORMAT_U24_LE:
    case SND_PCM_FORMAT_U32_LE:
#if defined (ACE_BIG_ENDIAN)
      encodingInfo_inout.opposite_endian = sox_true;
#endif // ACE_BIG_ENDIAN
      break;
    case SND_PCM_FORMAT_FLOAT_BE:
    case SND_PCM_FORMAT_FLOAT_LE:
    case SND_PCM_FORMAT_FLOAT64_BE:
    case SND_PCM_FORMAT_FLOAT64_LE:
    case SND_PCM_FORMAT_MU_LAW:
    case SND_PCM_FORMAT_A_LAW:
    case SND_PCM_FORMAT_IMA_ADPCM:
    case SND_PCM_FORMAT_GSM:
    case SND_PCM_FORMAT_G723_24:
    case SND_PCM_FORMAT_G723_24_1B:
    case SND_PCM_FORMAT_G723_40:
    case SND_PCM_FORMAT_G723_40_1B:
    case SND_PCM_FORMAT_IEC958_SUBFRAME_LE:
    case SND_PCM_FORMAT_IEC958_SUBFRAME_BE:
    case SND_PCM_FORMAT_MPEG:
    case SND_PCM_FORMAT_SPECIAL:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ALSA audio frame format (was: %d), continuing\n"),
                  mediaType_in.format));
      break;
    }
  } // end SWITCH

  signalInfo_inout.rate = mediaType_in.rate;
  signalInfo_inout.channels = mediaType_in.channels;
  signalInfo_inout.precision = snd_pcm_format_width (mediaType_in.format);
}
#endif // SOX_SUPPORT
