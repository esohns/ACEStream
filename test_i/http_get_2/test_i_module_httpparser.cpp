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

#include <ace/Synch.h>
#include "test_i_module_httpparser.h"

#include <ace/Log_Msg.h>

#include "stream_macros.h"

Test_I_Module_HTTPParser::Test_I_Module_HTTPParser ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_HTTPParser::Test_I_Module_HTTPParser"));

}

Test_I_Module_HTTPParser::~Test_I_Module_HTTPParser ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Module_HTTPParser::~Test_I_Module_HTTPParser"));

}

void
Test_I_Module_HTTPParser::record (struct HTTP_Record* record_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTTPParser::record"));

  // sanity check(s)
  ACE_ASSERT (record_in);
  ACE_ASSERT (record_in == inherited::record_);
  ACE_ASSERT (!inherited::headFragment_->isInitialized ());

  const Test_I_MessageData_t& data_container_r =
      inherited::headFragment_->get ();
  Test_I_MessageData& data_r =
      const_cast<Test_I_MessageData&> (data_container_r.get ());
  data_r.HTTPRecord = record_in;

  inherited::finished_ = true;

  // debug info
  if (inherited::trace_)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s"),
                ACE_TEXT (HTTP_Tools::dump (*record_in).c_str ())));
}
