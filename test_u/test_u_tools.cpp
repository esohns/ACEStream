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

#include "test_u_tools.h"

#include "stream_macros.h"

#include "common_file_tools.h"

#include "common_error_tools.h"

bool
Test_U_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Tools::initialize"));

  if (likely (!Common_Error_Tools::inDebugSession ()))
  {
    if (unlikely (!Common_File_Tools::setWorkingDirectory (ACE_TEXT_ALWAYS_CHAR (".."))))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::setWorkingDirectory(\"..\"), aborting\n")));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("set working directory to \"%s\"\n"),
                ACE_TEXT (Common_File_Tools::getWorkingDirectory ().c_str ())));
  } // end IF

  return true;
}
