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

#include "test_i_smtp_send_signalhandler.h"

#include "ace/Log_Msg.h"

#include "common_event_tools.h"

#include "stream_macros.h"

#include "test_i_smtp_send_network.h"

Stream_SMTPSend_SignalHandler::Stream_SMTPSend_SignalHandler ()
: inherited (this) // event handler handle
{
  STREAM_TRACE (ACE_TEXT ("Stream_SMTPSend_SignalHandler::Stream_SMTPSend_SignalHandler"));

}

void
Stream_SMTPSend_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SMTPSend_SignalHandler::handle"));

  bool statistic = false;
  bool shutdown = false;
  switch (signal_in.signal)
  {
    case SIGINT:
// *PORTABILITY*: on Windows SIGQUIT is not defined
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGQUIT:
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case SIGBREAK:
#else
    case SIGUSR1:
#endif // ACE_WIN32 || ACE_WIN64
    {
      // print statistic
      statistic = true;

      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGHUP:
    case SIGUSR2:
#endif // ACE_WIN32 || ACE_WIN64
    case SIGTERM:
    {
      // print statistic
      statistic = true;

      break;
    }
    case SIGCHLD:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case SIGIO:
#endif // ACE_WIN32 || ACE_WIN64
      break;
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: [%d] \"%S\", returning\n"),
                  signal_in.signal, signal_in.signal));
      return;
    }
  } // end SWITCH

  // ------------------------------------
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->dispatchState);

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
    // - leave event loop(s) handling signals, sockets, (maintenance) timers,
    //   exception handlers, ...
    // - activation timers (connection attempts, ...)
    // [- UI dispatch]

    //// step1: stop action timer (if any)
    //if (configuration_.actionTimerId >= 0)
    //{
    //  const void* act_p = NULL;
    //  result =
    //      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (configuration_.actionTimerId,
    //                                                                &act_p);
    //  // *PORTABILITY*: tracing in a signal handler context is not portable
    //  // *TODO*
    //  if (result <= 0)
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to cancel action timer (ID: %d): \"%m\", continuing\n"),
    //                configuration_.actionTimerId));
    //  configuration_.actionTimerId = -1;
    //} // end IF

    // step2: stop/abort(/wait) for connections
    SMTP_IConnection_Manager_t* connection_manager_p =
      SMTP_CONNECTIONMANAGER_SINGLETON::instance ();
    ACE_ASSERT (connection_manager_p);
    connection_manager_p->stop (false, true);
    connection_manager_p->abort ();
    connection_manager_p->wait ();

    // step5: stop reactor (&& proactor, if applicable)
    Common_Event_Tools::finalizeEventDispatch (*inherited::configuration_->dispatchState,
                                               false);                                    // wait ?

    // *IMPORTANT NOTE*: there is no real reason to wait here
  } // end IF
}
