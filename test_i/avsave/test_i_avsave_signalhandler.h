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

#ifndef TEST_I_AVSAVE_SIGNALHANDLER_H
#define TEST_I_AVSAVE_SIGNALHANDLER_H

#include "ace/Global_Macros.h"

#include "common_isignal.h"
#include "common_signal_handler.h"

#include "test_i_avsave_common.h"

class Stream_AVSave_SignalHandler
 : public Common_SignalHandler_T<struct Stream_AVSave_SignalHandlerConfiguration>
{
  typedef Common_SignalHandler_T<struct Stream_AVSave_SignalHandlerConfiguration> inherited;

 public:
  Stream_AVSave_SignalHandler ();
  inline virtual ~Stream_AVSave_SignalHandler () {}

  // implement Common_ISignal
  virtual void handle (const struct Common_Signal&); // signal

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_SignalHandler (const Stream_AVSave_SignalHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_SignalHandler& operator= (const Stream_AVSave_SignalHandler&))
};

#endif
