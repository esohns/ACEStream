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

#include "test_u_module_eventhandler.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Test_U_Module_EventHandler::Test_U_Module_EventHandler (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Module_EventHandler::Test_u_Module_EventHandler"));

}

ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
Test_U_Module_EventHandler:: clone () const
{
  STREAM_TRACE (ACE_TEXT ("Test_u_Module_EventHandler::clone"));

  // initialize return value(s)
  Stream_Task_t* task_p = NULL;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_NORETURN (module_p,
                    Test_U_Module_EventHandler_Module (NULL,//const_cast<ISTREAM_T*> (inherited::getP ()),
                                                       ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
  if (!module_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory(%u): %m, aborting\n"),
                inherited::mod_->name (),
                sizeof (Test_U_Module_EventHandler_Module)));
  else
  {
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);

    Test_U_Module_EventHandler* eventHandler_impl =
        dynamic_cast<Test_U_Module_EventHandler*> (task_p);
    if (!eventHandler_impl)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Test_u_Module_EventHandler> failed, aborting\n"),
                  inherited::mod_->name ()));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
    if (inherited::configuration_)
      eventHandler_impl->initialize (*inherited::configuration_,
                                     inherited::allocator_);
  } // end ELSE

  return task_p;
}
