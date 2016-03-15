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

#include "stream_dec_tools.h"

#include "ace/Log_Msg.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#ifdef __cplusplus
extern "C"
{
#include "libavutil/avutil.h"
}
#endif
#endif

#include "stream_macros.h"

void
Stream_Module_Decoder_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::initialize"));

}

std::string
Stream_Module_Decoder_Tools::errorToString (int error_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::errorToString"));

  std::string result;

  int result_2 = -1;
  char buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));

  result_2 = av_strerror (error_in,
                          buffer,
                          sizeof (buffer));
  if (result_2 < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to av_strerror(%d): \"%m\", continuing\n"),
                error_in));

  result = buffer;

  return result;
}