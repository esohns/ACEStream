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

#ifndef TEST_U_TOOLS_H
#define TEST_U_TOOLS_H

#include <string>

#include "ace/config-macros.h"
#include "ace/OS_NS_Thread.h"

class Test_U_Tools
{
 public:
  static bool initialize ();

  template <typename ThreadDataType,
            typename CallbackDataType>
  static bool spawn (const std::string&,      // thread name
                     ACE_THR_FUNC,            // thread function
                     int,                     // group id
                     const CallbackDataType&, // callback data
                     ACE_Thread_ID&);         // return value: thread id

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_Tools (const Test_U_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Tools& operator= (const Test_U_Tools&))
};

// include template definition
#include "test_u_tools.inl"

#endif
