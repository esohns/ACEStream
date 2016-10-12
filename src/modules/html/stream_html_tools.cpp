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

#include "stream_html_tools.h"

#include <ace/Log_Msg.h>

#include "stream_macros.h"

ACE_Log_Priority
Stream_HTML_Tools::errorLevelToLogPriority (xmlErrorLevel errorLevel_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HTML_Tools::errorLevelToLogPriority"));

  switch (errorLevel_in)
  {
    case XML_ERR_NONE:
      return LM_DEBUG;
    case XML_ERR_WARNING:
      return LM_WARNING;
    case XML_ERR_ERROR:
      break;
    case XML_ERR_FATAL:
      return LM_CRITICAL;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown error level (was: %d), aborting\n"),
                  errorLevel_in));
      break;
    }
  } // end SWITCH

  return LM_ERROR;
}
