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

#include <iostream>
#include <string>

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
//#include "ace/Synch.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_logger_queue.h"
#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#include "common_ui_defines.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H
#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_misc_defines.h"

#if defined (GTK_SUPPORT)
#include "test_i_callbacks.h"
#endif // GTK_SUPPORT
#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_module_eventhandler.h"
#include "test_i_target_common.h"
#include "test_i_target_eventhandler.h"
#include "test_i_target_listener_common.h"
#include "test_i_target_signalhandler.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("FileStream");

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]  : buffer size (byte(s)) [")
            << TEST_I_DEFAULT_BUFFER_SIZE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [VALUE]  : maximum number of connections [")
            << TEST_I_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string gtk_rc_file = path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e          : Gtk .rc file [\"")
            << gtk_rc_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : (target) file name [\"")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [STRING] : network interface [\"")
            << ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  // *TODO*: this doesn't really make sense (yet)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o          : use loopback [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : listening port [")
            << NET_ADDRESS_DEFAULT_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u          : use UDP [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]  : #dispatch threads [")
            << TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
                     unsigned int& maximumNumberOfConnections_out,
                     std::string& gtkRcFile_out,
                     std::string& outputFile_out,
                     std::string& gtkGladeFile_out,
                     bool& logToFile_out,
                     std::string& netWorkInterface_out,
                     bool& useLoopBack_out,
                     unsigned short& listeningPortNumber_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& useUDP_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  bufferSize_out = TEST_I_DEFAULT_BUFFER_SIZE;
  gtkRcFile_out = path;
  gtkRcFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkRcFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  maximumNumberOfConnections_out =
    TEST_I_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  outputFile_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  gtkGladeFile_out = path;
  gtkGladeFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkGladeFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_GLADE_FILE);
  logToFile_out = false;
  netWorkInterface_out = ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET);
  useLoopBack_out = false;
  listeningPortNumber_out = NET_ADDRESS_DEFAULT_PORT;
  useReactor_out =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  useUDP_out = false;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out = TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("b:c:e:f:g::ln:op:rs:tuvx:"),
                              1,                         // skip command name
                              1,                         // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS, // ordering
                              0);                        // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'b':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> bufferSize_out;
        break;
      }
      case 'c':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> maximumNumberOfConnections_out;
        break;
      }
      case 'e':
      {
        gtkRcFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'f':
      {
        outputFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          gtkGladeFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          gtkGladeFile_out.clear ();
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'n':
      {
        netWorkInterface_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'o':
      {
        useLoopBack_out = true;
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> listeningPortNumber_out;
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 's':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> statisticReportingInterval_out;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        useUDP_out = true;
        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      case 'x':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> numberOfDispatchThreads_out;
        break;
      }
      // error handling
      case ':':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("option \"%c\" requires an argument, aborting\n"),
                    argumentParser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argumentParser.last_option ())));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    ACE_TEXT (argumentParser.long_option ())));
        return false;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("parse error, aborting\n")));
        return false;
      }
    } // end SWITCH
  } // end WHILE

  return true;
}

void
do_initializeSignals (bool allowUserRuntimeConnect_in,
                      ACE_Sig_Set& signals_out,
                      ACE_Sig_Set& ignoredSignals_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initializeSignals"));

  int result = -1;

  // initialize return value(s)
  result = signals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF
  result = ignoredSignals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF

  // *PORTABILITY*: on Windows(TM) platforms most signals are not defined, and
  //                ACE_Sig_Set::fill_set() doesn't really work as specified
  // --> add valid signals (see <signal.h>)...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signals_out.sig_add (SIGINT);            // 2       /* interrupt */
  signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
  signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
  //  signals_out.sig_add (SIGSEGV);           // 11      /* segment violation */
  signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
  if (allowUserRuntimeConnect_in)
  {
    signals_out.sig_add (SIGBREAK);        // 21      /* Ctrl-Break sequence */
    ignoredSignals_out.sig_add (SIGBREAK); // 21      /* Ctrl-Break sequence */
  } // end IF
  signals_out.sig_add (SIGABRT);           // 22      /* abnormal termination triggered by abort call */
  signals_out.sig_add (SIGABRT_COMPAT);    // 6       /* SIGABRT compatible with other platforms, same as SIGABRT */
#else
  result = signals_out.fill_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::fill_set(): \"%m\", returning\n")));
    return;
  } // end IF
  // *NOTE*: cannot handle some signals --> registration fails for these...
  signals_out.sig_del (SIGKILL);           // 9       /* Kill signal */
  signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */
  // ---------------------------------------------------------------------------
  if (!allowUserRuntimeConnect_in)
  {
    signals_out.sig_del (SIGUSR1);         // 10      /* User-defined signal 1 */
    ignoredSignals_out.sig_add (SIGUSR1);  // 10      /* User-defined signal 1 */
  } // end IF
  // *NOTE* core dump on SIGSEGV
  signals_out.sig_del (SIGSEGV);           // 11      /* Segmentation fault: Invalid memory reference */
  // *NOTE* don't care about SIGPIPE
  signals_out.sig_del (SIGPIPE);           // 12      /* Broken pipe: write to pipe with no readers */

#if defined (VALGRIND_USE)
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif // VALGRIND_USE
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (unsigned int bufferSize_in,
         unsigned int maximumNumberOfConnections_in,
         const std::string& fileName_in,
         const std::string& UIDefinitionFile_in,
         const std::string& networkInterface_in,
         bool useLoopBack_in,
         unsigned short listeningPortNumber_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         bool useUDP_in,
         unsigned int numberOfDispatchThreads_in,
         struct Test_I_Target_UI_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_Target_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  ACE_UNUSED_ARG (networkInterface_in);

  int result = -1;

  // step0a: initialize configuration
  if (useReactor_in)
    CBData_in.configuration->dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    CBData_in.configuration->dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;
  CBData_in.configuration->protocol = (useUDP_in ? NET_TRANSPORTLAYER_UDP
                                                 : NET_TRANSPORTLAYER_TCP);

  struct Stream_AllocatorConfiguration allocator_configuration;

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Test_I_Target_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                      &heap_allocator,     // heap allocator handle
                                                      true);               // block ?

//  CBData_in.configuration = &configuration;
//  Stream_GTK_CBData* cb_data_base_p = &CBData_in;
//  cb_data_base_p->configuration = &configuration;
  Test_I_Target_EventHandler ui_event_handler (&CBData_in);
  Test_I_Stream_Target_EventHandler_Module event_handler (NULL,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_Stream_Target_EventHandler* event_handler_p =
    dynamic_cast<Test_I_Stream_Target_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Target_EventHandler> failed, returning\n")));
    return;
  } // end IF

  Test_I_Target_TCPConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  // ********************** connection configuration data **********************
  Test_I_Target_TCPConnectionConfiguration_t tcp_connection_configuration;
  Test_I_Target_UDPConnectionConfiguration_t udp_connection_configuration;
  tcp_connection_configuration.socketConfiguration.address.set_port_number (listeningPortNumber_in,
                                                                            1);
  tcp_connection_configuration.socketConfiguration.useLoopBackDevice =
    useLoopBack_in;
  if (useLoopBack_in)
  {
    result =
      tcp_connection_configuration.socketConfiguration.address.set (listeningPortNumber_in,
                                                                    INADDR_LOOPBACK,
                                                                    1,
                                                                    0);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  } // end IF
  tcp_connection_configuration.allocatorConfiguration = &allocator_configuration;
  tcp_connection_configuration.allocatorConfiguration->defaultBufferSize = bufferSize_in;
  tcp_connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
//  connection_configuration.connectionManager = connection_manager_p;
  tcp_connection_configuration.messageAllocator = &message_allocator;
  tcp_connection_configuration.streamConfiguration =
    &CBData_in.configuration->streamConfiguration;

  CBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                            &tcp_connection_configuration));
  CBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                            &tcp_connection_configuration));
  CBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("UDP"),
                                                                            &udp_connection_configuration));
  Net_ConnectionConfigurationsIterator_t iterator =
    CBData_in.configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("TCP"));
  ACE_ASSERT (iterator != CBData_in.configuration->connectionConfigurations.end ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    CBData_in.configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("UDP"));
  ACE_ASSERT (iterator_2 != CBData_in.configuration->connectionConfigurations.end ());
  // ********************** stream configuration data **************************
  if (bufferSize_in)
    allocator_configuration.defaultBufferSize = bufferSize_in;

  // ********************** module configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct Test_I_Target_ModuleHandlerConfiguration modulehandler_configuration;
//  modulehandler_configuration.configuration = &configuration;
  modulehandler_configuration.connectionConfigurations =
    &CBData_in.configuration->connectionConfigurations;
//  modulehandler_configuration.connectionManager = connection_manager_p;
  modulehandler_configuration.inbound = true;
  modulehandler_configuration.printProgressDot =
      UIDefinitionFile_in.empty ();
  modulehandler_configuration.statisticReportingInterval =
      ACE_Time_Value (statisticReportingInterval_in, 0);
  modulehandler_configuration.streamConfiguration =
      &CBData_in.configuration->streamConfiguration;
#if defined (GTK_USE)
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.subscribers = &CBData_in.subscribers;
  modulehandler_configuration.lock = &state_r.subscribersLock;
#endif // GTK_USE
  modulehandler_configuration.fileIdentifier.identifier = fileName_in;

  // ******************** (sub-)stream configuration data *********************
  struct Test_I_Target_StreamConfiguration stream_configuration;
  if (bufferSize_in)
    allocator_configuration.defaultBufferSize = bufferSize_in;

  stream_configuration.allocatorConfiguration = &allocator_configuration;
  stream_configuration.cloneModule = true;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
    (!UIDefinitionFile_in.empty () ? &event_handler
                                   : NULL);
  stream_configuration.printFinalReport = true;
  CBData_in.configuration->streamConfiguration.initialize (module_configuration,
                                                           modulehandler_configuration,
                                                           stream_configuration);
  Test_I_Target_StreamConfiguration_t::ITERATOR_T iterator_3 =
    CBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != CBData_in.configuration->streamConfiguration.end ());

  // ********************* listener configuration data ************************
//  configuration.listenerConfiguration.connectionConfiguration =
//    dynamic_cast<Test_I_Target_TCPConnectionConfiguration_t*> ((*iterator).second);
//  configuration.listenerConfiguration.connectionManager = connection_manager_p;
//  configuration.listenerConfiguration.statisticReportingInterval =
//      statisticReportingInterval_in;
  //configuration.listenerConfiguration.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice =
  //  useLoopBack_in;

  // step0b: initialize event dispatch
  CBData_in.configuration->dispatchConfiguration.numberOfProactorThreads =
    numberOfDispatchThreads_in;
  CBData_in.configuration->dispatchConfiguration.numberOfReactorThreads =
    numberOfDispatchThreads_in;
  if (!Common_Event_Tools::initializeEventDispatch (CBData_in.configuration->dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step0c: initialize connection manager
  struct Net_UserData user_data_s;
  connection_manager_p->initialize ((maximumNumberOfConnections_in ? maximumNumberOfConnections_in
                                                                   : std::numeric_limits<unsigned int>::max ()),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*static_cast<Test_I_Target_TCPConnectionConfiguration_t*> ((*iterator).second),
                             &user_data_s);

  // step0d: initialize regular (global) statistic reporting
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  struct Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);
  Net_StreamStatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                                  connection_manager_p,
                                                  false);
  long timer_id = -1;
  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
                                       NULL,                       // asynchronous completion token
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));
      timer_manager_p->stop ();
      return;
    } // end IF
  } // end IF

  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
      &CBData_in.configuration->dispatchConfiguration;

  // step0e: initialize signal handling
  CBData_in.configuration->signalHandlerConfiguration.dispatchState =
    &event_dispatch_state_s;
//  if (useReactor_in)
//    CBData_in.configuration->signalHandlerConfiguration.listener =
//      TEST_I_TARGET_LISTENER_SINGLETON::instance ();
//  else
    CBData_in.configuration->signalHandlerConfiguration.listener =
      TEST_I_TARGET_ASYNCHLISTENER_SINGLETON::instance ();
  CBData_in.configuration->signalHandlerConfiguration.statisticReportingHandler =
    connection_manager_p;
  CBData_in.configuration->signalHandlerConfiguration.statisticReportingTimerId =
    timer_id;
  if (!signalHandler_in.initialize (CBData_in.configuration->signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    timer_manager_p->stop ();
    return;
  } // end IF
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    timer_manager_p->stop ();
    return;
  } // end IF

  // step1: handle events (signals, incoming connections/data[, timers], ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // [timer events:]
  // - perform (connection) statistic collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step1a: start UI event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    //CBData_in.UIState.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));

    gtk_manager_p->start (NULL);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
    result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      timer_manager_p->stop ();
      return;
    } // end IF
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = ::GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      timer_manager_p->stop ();
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#endif // GTK_USE
      return;
    } // end IF
    BOOL was_visible_b = ::ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  // step1b: initialize worker(s)
//  int group_id = -1;
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));

    //		{ // synch access
    //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

    //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
    //					 iterator != CBData_in.event_source_ids.end();
    //					 iterator++)
    //				g_source_remove(*iterator);
    //		} // end lock scope
    if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#endif // GTK_USE
    timer_manager_p->stop ();
    return;
  } // end IF

  // step1c: start listening ?
  if (UIDefinitionFile_in.empty ())
  {
    if (useUDP_in)
    {
      Test_I_Target_UDPConnectionManager_t::INTERFACE_T* iconnection_manager_p =
        TEST_I_TARGET_UDP_CONNECTIONMANAGER_SINGLETON::instance ();;
      ACE_ASSERT (iconnection_manager_p);
      Test_I_Target_IUDPConnector_t* connector_p = NULL;
//      if (useReactor_in)
//        ACE_NEW_NORETURN (connector_p,
//                          Test_I_InboundUDPConnector_t (true));
//      else
        ACE_NEW_NORETURN (connector_p,
                          Test_I_InboundUDPAsynchConnector_t (true));
      if (!connector_p)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory, returning\n")));

        Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                   true); // wait ?
        //		{ // synch access
        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
        //					 iterator != CBData_in.event_source_ids.end();
        //					 iterator++)
        //				g_source_remove(*iterator);
        //		} // end lock scope
        if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
          gtk_manager_p->stop (true, true);
#endif // GTK_USE
        timer_manager_p->stop ();
        return;
      } // end IF
      //  Stream_IInetConnector_t* iconnector_p = &connector;
      if (!connector_p->initialize (*static_cast<Test_I_Target_UDPConnectionConfiguration_t*> ((*iterator).second)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));

        Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                   true); // wait ?
        //		{ // synch access
        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
        //					 iterator != CBData_in.event_source_ids.end();
        //					 iterator++)
        //				g_source_remove(*iterator);
        //		} // end lock scope
        if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
          gtk_manager_p->stop (true, true);
#endif // GTK_USE
        timer_manager_p->stop ();
        delete connector_p; connector_p = NULL;
        return;
      } // end IF

      // connect
      // *TODO*: support one-thread operation by scheduling a signal and manually
      //         running the dispatch loop for a limited time
      CBData_in.configuration->handle =
        connector_p->connect (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress);
//      if (!useReactor_in)
//      {
        ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S,
                                0);
        ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
        Test_I_InboundUDPAsynchConnector_t::ICONNECTION_T* connection_p = NULL;
        // *TODO*: avoid tight loop here
        do
        {
          connection_p =
            iconnection_manager_p->get (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.peerAddress,
                                        true);
          if (connection_p)
          {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            CBData_in.configuration->handle =
              reinterpret_cast<ACE_HANDLE> (connection_p->id ());
#else
            CBData_in.configuration->handle =
              static_cast<ACE_HANDLE> (connection_p->id ());
#endif
            connection_p->decrease (); connection_p = NULL;
            break;
          } // end IF
        } while (COMMON_TIME_NOW < deadline);
//      } // end IF
      if (CBData_in.configuration->handle == ACE_INVALID_HANDLE)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to connect to %s, returning\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress,
                                                                   false,
                                                                   false).c_str ())));

        Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                   true); // wait ?
        //		{ // synch access
        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
        //					 iterator != CBData_in.event_source_ids.end();
        //					 iterator++)
        //				g_source_remove(*iterator);
        //		} // end lock scope
        if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
          gtk_manager_p->stop (true, true);
#endif // GTK_USE
        timer_manager_p->stop ();
        delete connector_p; connector_p = NULL;
        return;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("listening to UDP %s\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress).c_str ())));

      delete connector_p; connector_p = NULL;
    } // end IF
    else
    {
      if (!CBData_in.configuration->signalHandlerConfiguration.listener->initialize (tcp_connection_configuration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize listener, returning\n")));

        Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                   true); // wait ?
        //		{ // synch access
        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
        //					 iterator != CBData_in.event_source_ids.end();
        //					 iterator++)
        //				g_source_remove(*iterator);
        //		} // end lock scope
        if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
          gtk_manager_p->stop (true, true);
#endif // GTK_USE
        timer_manager_p->stop ();
        return;
      } // end IF
      CBData_in.configuration->signalHandlerConfiguration.listener->start (NULL);
//      if (!CBData_in.configuration->signalHandlerConfiguration.listener->isRunning ())
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to start listener (port: %u), returning\n"),
//                    listeningPortNumber_in));
//
//        Common_Tools::finalizeEventDispatch (event_dispatch_state_s.proactorGroupId,
//                                             event_dispatch_state_s.reactorGroupId,
//                                             true);
//        //		{ // synch access
//        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);
//
//        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
//        //					 iterator != CBData_in.event_source_ids.end();
//        //					 iterator++)
//        //				g_source_remove(*iterator);
//        //		} // end lock scope
//        if (!UIDefinitionFile_in.empty ())
//#if defined (GTK_USE)
//          gtk_manager_p->stop (true, true, true);
//#endif // GTK_USE
//        timer_manager_p->stop ();
//        return;
//      } // end IF
    } // end ELSE
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

  // clean up
  // *NOTE*: listener has stopped, interval timer has been cancelled,
  // and connections have been aborted...
  //		{ // synch access
  //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //					 iterator != CBData_in.event_source_ids.end();
  //					 iterator++)
  //				g_source_remove(*iterator);
  //		} // end lock scope
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p->wait (false);
#endif // GTK_USE
  } // end IF
  else
    Common_Event_Tools::dispatchEvents (event_dispatch_state_s);

  // wait for connection processing to complete
  connection_manager_p->stop (false, true);
  connection_manager_p->abort ();
  connection_manager_p->wait ();

  timer_manager_p->stop ();

  result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
                event_handler.name ()));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

COMMON_DEFINE_PRINTVERSION_FUNCTION(do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result = -1;
#endif // ACE_WIN32 || ACE_WIN64

  // step0: initialize
  // *PORTABILITY*: on Windows, initialize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::init ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer
  process_profile.start ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  unsigned int buffer_size = TEST_I_DEFAULT_BUFFER_SIZE;
  unsigned int maximum_number_of_connections =
    TEST_I_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string gtk_rc_file = path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  std::string output_file = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::string gtk_glade_file = path;
  gtk_glade_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_glade_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_GLADE_FILE);
  bool log_to_file = false;
  std::string network_interface = ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET);
  bool use_loopback = false;
  unsigned short listening_port_number = NET_ADDRESS_DEFAULT_PORT;
  bool use_reactor =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
      STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool use_UDP = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
                            maximum_number_of_connections,
                            gtk_rc_file,
                            output_file,
                            gtk_glade_file,
                            log_to_file,
                            network_interface,
                            use_loopback,
                            listening_port_number,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            use_UDP,
                            print_version_and_exit,
                            number_of_dispatch_threads))
  {
    do_printUsage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_I_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
//  if (use_reactor                      &&
//      (number_of_dispatch_threads > 1) &&
//      !use_thread_pool)
//  { // *NOTE*: see also: man (2) select
//    // *TODO*: verify this for MS Windows based systems
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
//    use_thread_pool = true;
//  } // end IF
  if ((gtk_glade_file.empty () &&
       !Common_File_Tools::isReadable (output_file))                       ||
      (!gtk_glade_file.empty () &&
       !Common_File_Tools::isReadable (gtk_glade_file))                    ||
      (!gtk_rc_file.empty () &&
       !Common_File_Tools::isReadable (gtk_rc_file))
//      (use_thread_pool && !use_reactor)                                    ||
//      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool)
      )
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s\n"),
                output_file.c_str ()));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s\n"),
                gtk_glade_file.c_str ()));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s\n"),
                gtk_rc_file.c_str ()));

    do_printUsage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  if (number_of_dispatch_threads == 0)
    number_of_dispatch_threads = 1;

//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
  struct Test_I_Target_Configuration configuration;
//  struct Common_UI_CBData* ui_cb_data_p = NULL;
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
  struct Test_I_Target_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
  //ui_cb_data.progressData.state = &ui_cb_data;
  ui_cb_data.UIState = &state_r;

  // step1d: initialize logging and/or tracing
  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);

  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
  if (!Common_Log_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])), // program name
                                     log_file_name,                        // log file name
                                     false,                                // log to syslog ?
                                     false,                                // trace messages ?
                                     trace_information,                    // debug messages ?
                                     NULL))                                // (ui-) logger ?
//                                            (gtk_glade_file.empty () ? NULL
//                                                                     : &logger))) // (ui-) logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                        : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                           true,  // using networking ?
                                           false, // using asynch timers ?
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  Stream_Target_SignalHandler signal_handler;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));
    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_OS_Tools::setResourceLimits (true,  // file descriptors
                                           true,  // stack traces
                                           true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

#if defined (GTK_USE)
  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_target_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (gtk_rc_file);
  if (!gtk_glade_file.empty ())
    if (!gtk_manager_p->initialize (ui_cb_data.configuration->GTKConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), aborting\n")));

      Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                  : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                     previous_signal_actions,
                                     previous_signal_mask);
      Common_Log_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_FAILURE;
    } // end IF
#endif // GTK_USE

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
           maximum_number_of_connections,
           output_file,
           gtk_glade_file,
           network_interface,
           use_loopback,
           listening_port_number,
           use_reactor,
           statistic_reporting_interval,
           use_UDP,
           number_of_dispatch_threads,
           ui_cb_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // stop profile timer
  process_profile.stop ();

  // only process profile left to do
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  if (process_profile.elapsed_time (elapsed_time) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
  user_time_string = Common_Timer_Tools::periodToString (user_time);
  system_time_string = Common_Timer_Tools::periodToString (system_time);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\nmaximum resident set size = %d\nintegral shared memory size = %d\nintegral unshared data size = %d\nintegral unshared stack size = %d\npage reclaims = %d\npage faults = %d\nswaps = %d\nblock input operations = %d\nblock output operations = %d\nmessages sent = %d\nmessages received = %d\nsignals received = %d\nvoluntary context switches = %d\ninvoluntary context switches = %d\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ()),
              elapsed_rusage.ru_maxrss,
              elapsed_rusage.ru_ixrss,
              elapsed_rusage.ru_idrss,
              elapsed_rusage.ru_isrss,
              elapsed_rusage.ru_minflt,
              elapsed_rusage.ru_majflt,
              elapsed_rusage.ru_nswap,
              elapsed_rusage.ru_inblock,
              elapsed_rusage.ru_oublock,
              elapsed_rusage.ru_msgsnd,
              elapsed_rusage.ru_msgrcv,
              elapsed_rusage.ru_nsignals,
              elapsed_rusage.ru_nvcsw,
              elapsed_rusage.ru_nivcsw));
#endif // ACE_WIN32 || ACE_WIN64

  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();

  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
