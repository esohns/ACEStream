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

#ifndef TEST_I_INPUT_HANDLER_T_H
#define TEST_I_INPUT_HANDLER_T_H

#include "common_input_handler_base.h"

template <typename ConfigurationType, // implements Common_Input_Configuration
          typename MessageType>       // implements ACE_Message_Block
class Test_I_InputHandler_T
 : public Common_InputHandler_Base_T<ConfigurationType>
{
  typedef Common_InputHandler_Base_T<ConfigurationType> inherited;

 public:
  Test_I_InputHandler_T ();
  inline virtual ~Test_I_InputHandler_T () {}

  // *NOTE*: the default action is to drop the message block into the queue (if any)
  // *IMPORTANT NOTE*: fire-and-forget first argument
  virtual bool handle_input (ACE_Message_Block*); // message block containing input

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_InputHandler_T (const Test_I_InputHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_InputHandler_T& operator= (const Test_I_InputHandler_T&))
};

// include template definition
#include "test_i_input_handler.inl"

#endif
