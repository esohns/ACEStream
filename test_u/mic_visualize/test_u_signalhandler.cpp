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

#include "test_u_signalhandler.h"

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#if defined (GLUT_SUPPORT)
#include "GL/freeglut.h"
#endif // GLUT_SUPPORT

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Test_U_SignalHandler::Test_U_SignalHandler ()
 : inherited (this) // event handler handle
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SignalHandler::Test_U_SignalHandler"));

}

void
Test_U_SignalHandler::handle (const struct Common_Signal& signal_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_SignalHandler::handle"));

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
      //case SIGPOLL:
      break;
    default:
    {
      // *PORTABILITY*: tracing in a signal handler context is not portable
      // *TODO*
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", returning\n"),
                  signal_in.signal));
      return;
    }
  } // end SWITCH

  // ------------------------------------

  // print statistic ?
  if (statistic)
  {
    try {
      //handle = configuration_.connector->connect (configuration_.peerAddress);
    } catch (...)
    {
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

#if defined (GLUT_SUPPORT)
    glutLeaveMainLoop ();
#endif // GLUT_SUPPORT
  } // end IF
}
