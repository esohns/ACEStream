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
#include "ace/OS.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"
#include "ace/Thread_Manager.h"

#include "common_defines.h"
#include "common_macros.h"

template <typename ConfigurationType,
          typename MessageType>
Test_I_InputHandler_T<ConfigurationType,
                      MessageType>::Test_I_InputHandler_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::Test_I_InputHandler_T"));

}

template <typename ConfigurationType,
          typename MessageType>
bool
Test_I_InputHandler_T<ConfigurationType,
                      MessageType>::handle_input (ACE_Message_Block* messageBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_InputHandler_T::handle_input"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->queue);
  ACE_ASSERT (messageBlock_in);

  int result = -1;

  result = inherited::configuration_->queue->enqueue (messageBlock_in, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue_Base::enqueue(): \"%m\", aborting\n")));
    messageBlock_in->release ();
    return false;
  } // end IF

  return true;
}
