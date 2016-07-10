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

#include "ace/Synch.h"
#include "test_i_source_signalhandler.h"

#include "ace/Log_Msg.h"

//#include "common_timer_manager.h"
#include "common_tools.h"

#include "stream_macros.h"

#include "test_i_source_common.h"
#include "test_i_connection_manager_common.h"

Test_I_Source_SignalHandler::Test_I_Source_SignalHandler ()
 : inherited (this) // event handler handle
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_SignalHandler::Test_I_Source_SignalHandler"));

}

Test_I_Source_SignalHandler::~Test_I_Source_SignalHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_SignalHandler::~Test_I_Source_SignalHandler"));

}

bool
Test_I_Source_SignalHandler::handleSignal (int signal_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_SignalHandler::handleSignal"));

//  int result = -1;

  bool statistic = false;
  bool shutdown = false;
  switch (signal_in)
  {
    case SIGINT:
// *PORTABILITY*: on Windows SIGQUIT is not defined
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGQUIT:
#endif
    {
//       // *PORTABILITY*: tracing in a signal handler context is not portable
//       // *TODO*
      //ACE_DEBUG((LM_DEBUG,
      //           ACE_TEXT("shutting down...\n")));

      shutdown = true;

      break;
    }
// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) and SIGTERM (15) instead...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR1:
#else
    case SIGBREAK:
#endif
    {
      // print statistic
      statistic = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGUSR2:
#endif
    case SIGTERM:
    {
      // print statistic
      statistic = true;

      break;
    }
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", aborting\n"),
                  signal_in));
      return false;
    }
  } // end SWITCH

  // ------------------------------------

  // print statistic ?
  if (statistic)
  {
    try {
      //handle = configuration_.connector->connect (configuration_.peerAddress);
    } catch (...) {
      //// *PORTABILITY*: tracing in a signal handler context is not portable
      //// *TODO*
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("caught exception in Net_Client_IConnector::connect(): \"%m\", continuing\n")));
    }
  } // end IF

//check_shutdown:
  // ...shutdown ?
  if (shutdown)
  {
    // stop everything, i.e.
    // - stop processing stream
    // - leave event loop(s) handling signals, sockets, (maintenance) timers,
    //   exception handlers, ...
    // - activation timers (connection attempts, ...)
    // [- UI dispatch]

    // step1: stop processing stream
    ACE_ASSERT (inherited::configuration_->stream);
    inherited::configuration_->stream->stop (false, // don't block
                                             true); // locked access

    // step2: stop/abort(/wait) for connections
    Test_I_Source_IInetConnectionManager_t* connection_manager_p =
        TEST_I_SOURCE_CONNECTIONMANAGER_SINGLETON::instance ();
    ACE_ASSERT (connection_manager_p);
    connection_manager_p->stop ();
//    connection_manager_p->abort ();

//    // step3: stop reactor (&& proactor, if applicable)
//    Common_Tools::finalizeEventDispatch (inherited::configuration_->useReactor,  // stop reactor ?
//                                         !inherited::configuration_->useReactor, // stop proactor ?
//                                         -1);                                    // group ID (--> don't block)
  } // end IF

  return true;
}
