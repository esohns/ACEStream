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

#include "test_u_camsave_signalhandler.h"

#include "ace/Log_Msg.h"

#include "common_ui_gtk_manager.h"

#include "stream_macros.h"

Stream_CamSave_SignalHandler::Stream_CamSave_SignalHandler ()
 : inherited (this, // event handler handle
              true) // use reactor ?
 , configuration_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_SignalHandler::Stream_CamSave_SignalHandler"));

}

Stream_CamSave_SignalHandler::~Stream_CamSave_SignalHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_SignalHandler::~Stream_CamSave_SignalHandler"));

}

bool
Stream_CamSave_SignalHandler::initialize (const Stream_CamSave_SignalHandlerConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_SignalHandler::initialize"));

  configuration_ = configuration_in;

  return true;
}

bool
Stream_CamSave_SignalHandler::handleSignal (int signal_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_SignalHandler::handleSignal"));

//  int result = -1;

  bool statistics = false;
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
      // print statistics
      statistics = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGUSR2:
#endif
    case SIGTERM:
    {
      // print statistics
      statistics = true;

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

  // print statistics ?
  if (statistics)
  {
    try
    {
      //handle = configuration_.connector->connect (configuration_.peerAddress);
    }
    catch (...)
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

    // step2: stop GTK event processing
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, true);
  } // end IF

  return true;
}