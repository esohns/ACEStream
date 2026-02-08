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

#include "stream_dec_whisper_decoder.h"

#include "stream_dec_defines.h"

const char libacestream_default_dec_whisper_decoder_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_WHISPERCPP_DECODER_DEFAULT_NAME_STRING);

void
acestream_dec_whispercpp_on_log_cb (ggml_log_level level_in,
                                    const char* text_in,
                                    void* userData_in)
{
  //STREAM_TRACE (ACE_TEXT ("acestream_dec_whispercpp_on_log_cb"));

  // sanity check(s)
  ACE_ASSERT (text_in);

  ACE_UNUSED_ARG (userData_in);

  static enum ACE_Log_Priority priority_e = LM_DEBUG;
  switch (level_in)
  {
    case GGML_LOG_LEVEL_NONE: // fallthrough
    case GGML_LOG_LEVEL_DEBUG:
      break;
    case GGML_LOG_LEVEL_INFO:
      priority_e = LM_INFO;
      break;
    case GGML_LOG_LEVEL_WARN:
      priority_e = LM_WARNING;
      break;
    case GGML_LOG_LEVEL_ERROR:
      priority_e = LM_ERROR;
      break;
    case GGML_LOG_LEVEL_CONT:
      //priority_e = LM_DEBUG;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown log level (was: %d), using LM_DEBUG\n"),
                  ACE_TEXT (libacestream_default_dec_whisper_decoder_module_name_string),
                  level_in));
      priority_e = LM_DEBUG;
      break;
    }
  } // end SWITCH

  ACE_DEBUG ((priority_e,
              ACE_TEXT ("%s: %s"),
              ACE_TEXT (libacestream_default_dec_whisper_decoder_module_name_string),
              ACE_TEXT (text_in)));
}

void
acestream_dec_whispercpp_on_progress_cb (struct whisper_context* context_in,
                                         struct whisper_state* state_in,
                                         int progress_in,
                                         void* userData_in)
{
  // STREAM_TRACE (ACE_TEXT ("acestream_dec_whispercpp_on_progress_cb"));

  ACE_UNUSED_ARG (context_in);
  ACE_UNUSED_ARG (state_in);
  ACE_UNUSED_ARG (userData_in);

  // *TODO*
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: progress: %d%%\n"),
  //            ACE_TEXT (libacestream_default_dec_whisper_decoder_module_name_string),
  //            progress_in));
}

bool
acestream_dec_whispercpp_on_begin_cb (struct whisper_context* context_in,
                                      struct whisper_state* state_in,
                                      void* userData_in)
{
  // STREAM_TRACE (ACE_TEXT ("acestream_dec_whispercpp_on_begin_cb"));

  ACE_UNUSED_ARG (context_in);
  ACE_UNUSED_ARG (state_in);

  return !acestream_dec_whispercpp_abort_cb (userData_in);
}

bool
acestream_dec_whispercpp_abort_cb (void* userData_in)
{
  // STREAM_TRACE (ACE_TEXT ("acestream_dec_whispercpp_abort_cb"));

  bool* aborted_b = static_cast<bool*> (userData_in);

  return (aborted_b ? *aborted_b : false);
}
