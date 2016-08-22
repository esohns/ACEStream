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

#include "test_i_module_splitter.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_Target_DirectShow_Module_Splitter::Test_I_Target_DirectShow_Module_Splitter ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_Module_Splitter::Test_I_Target_DirectShow_Module_Splitter"));

}

Test_I_Target_DirectShow_Module_Splitter::~Test_I_Target_DirectShow_Module_Splitter ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_Module_Splitter::~Test_I_Target_DirectShow_Module_Splitter"));

}

bool
Test_I_Target_DirectShow_Module_Splitter::initialize (const Test_I_Target_DirectShow_ModuleHandlerConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_DirectShow_Module_Splitter::initialize"));

  inherited::PDUSize_ = configuration_in.format->lSampleSize;

  return inherited::initialize (configuration_in);
}

//////////////////////////////////////////

Test_I_Target_MediaFoundation_Module_Splitter::Test_I_Target_MediaFoundation_Module_Splitter ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_Module_Splitter::Test_I_Target_MediaFoundation_Module_Splitter"));

}

Test_I_Target_MediaFoundation_Module_Splitter::~Test_I_Target_MediaFoundation_Module_Splitter ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_Module_Splitter::~Test_I_Target_MediaFoundation_Module_Splitter"));

}

bool
Test_I_Target_MediaFoundation_Module_Splitter::initialize (const Test_I_Target_MediaFoundation_ModuleHandlerConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_MediaFoundation_Module_Splitter::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.format);

  HRESULT result = configuration_in.format->GetUINT32 (MF_MT_SAMPLE_SIZE,
                                                       &(inherited::PDUSize_));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_SAMPLE_SIZE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  return inherited::initialize (configuration_in);
}
#endif
