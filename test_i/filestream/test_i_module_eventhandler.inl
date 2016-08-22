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

#include "ace/Log_Msg.h"
#include "ace/Module.h"
#include "ace/OS_Memory.h"

#include "stream_macros.h"

template <typename ModuleConfigurationType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Test_I_Stream_Module_EventHandler_T<ModuleConfigurationType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    MessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::Test_I_Stream_Module_EventHandler_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_EventHandler_T::Test_I_Stream_Module_EventHandler_T"));

}

template <typename ModuleConfigurationType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Test_I_Stream_Module_EventHandler_T<ModuleConfigurationType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    MessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::~Test_I_Stream_Module_EventHandler_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_EventHandler_T::~Test_I_Stream_Module_EventHandler_T"));

}

template <typename ModuleConfigurationType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
ACE_Module<ACE_MT_SYNCH,
           Common_TimePolicy_t>*
Test_I_Stream_Module_EventHandler_T<ModuleConfigurationType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    MessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::clone ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_EventHandler_T::clone"));

  // initialize return value(s)
  ACE_Module<ACE_MT_SYNCH,
             Common_TimePolicy_t>* module_p = NULL;

  ACE_NEW_NORETURN (module_p,
                    MODULE_T (ACE_TEXT_ALWAYS_CHAR (inherited::name ()),
                    NULL));
  if (!module_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  else
  {
    inherited* messageHandler_impl_p =
      dynamic_cast<inherited*> (module_p->writer ());
    if (!messageHandler_impl_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("dynamic_cast<Stream_Module_MessageHandler_T> failed, aborting\n")));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
    messageHandler_impl_p->initialize (inherited::subscribers_,
                                       inherited::lock_);
  } // end ELSE

  return module_p;
}
