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
#include <limits>
#include <string>

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Synch.h"
#include "ace/Version.h"

#include "common_file_tools.h"
#include "common_logger.h"
#include "common_tools.h"

#include "common_ui_defines.h"
//#include "common_ui_glade_definition.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#ifdef HAVE_CONFIG_H
#include "libACEStream_config.h"
#endif

#include "test_i_callbacks.h"
#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_module_eventhandler.h"
#include "test_i_source_common.h"
#include "test_i_source_eventhandler.h"
#include "test_i_source_signalhandler.h"
#include "test_i_source_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("FileStream");

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("filestream");
#endif // #ifdef DEBUG_DEBUGGER

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
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  std::string gtk_rc_file = path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e          : Gtk .rc file [\"")
            << gtk_rc_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : (source payload) file name")
            << std::endl;
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-h [STRING] : target host [\"")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_HOSTNAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o          : use thread pool [")
            << NET_EVENT_USE_THREAD_POOL
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : port number [")
            << NET_ADDRESS_DEFAULT_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL
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
            << NET_CLIENT_DEFAULT_NUMBER_OF_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-z [VALUE]  : loop [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("] {-1: forever}")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
                     std::string& gtkRcFile_out,
                     std::string& sourceFile_out,
                     std::string& gtkGladeFile_out,
                     std::string& hostName_out,
                     bool& logToFile_out,
                     bool& useThreadPool_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& useUDP_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out,
                     size_t& loop_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("filestream");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  bufferSize_out = TEST_I_DEFAULT_BUFFER_SIZE;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  gtkRcFile_out = path;
  gtkRcFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkRcFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  //SourceFile_out.clear ();
  hostName_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_HOSTNAME);
  gtkGladeFile_out = path;
  gtkGladeFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkGladeFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  logToFile_out = false;
  useThreadPool_out = NET_EVENT_USE_THREAD_POOL;
  port_out = NET_ADDRESS_DEFAULT_PORT;
  useReactor_out = NET_EVENT_USE_REACTOR;
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  useUDP_out = false;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    NET_CLIENT_DEFAULT_NUMBER_OF_DISPATCH_THREADS;
  loop_out = 0;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("b:e:f:g::h:lop:rs:tuvx:z:"),
                              1,                          // skip command name
                              1,                          // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS,  // ordering
                              0);                         // for now, don't use long options

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
      case 'e':
      {
        gtkRcFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'f':
      {
        sourceFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
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
      case 'h':
      {
        hostName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'o':
      {
        useThreadPool_out = true;
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> port_out;
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
      case 'z':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> loop_out;
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

  // *PORTABILITY*: on Windows(TM) platforms most signals have not been defined,
  //                and ACE_Sig_Set::fill_set() doesn't work as specified
  //                --> add valid signals manually (see <signal.h>)
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
  // *NOTE*: on UNIX platforms some signals cannot be managed (handler
  //         registration fails)
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
#ifdef ENABLE_VALGRIND_SUPPORT
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif
#endif
}

void
do_work (unsigned int bufferSize_in,
         const std::string& fileName_in,
         const std::string& UIDefinitionFile_in,
         const std::string& hostName_in,
         bool useThreadPool_in,
         unsigned short port_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         bool useUDP_in,
         unsigned int numberOfDispatchThreads_in,
         Test_I_Source_GTK_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_I_Source_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  // step0a: initialize event dispatch
  struct Common_DispatchThreadData thread_data;
  thread_data.numberOfDispatchThreads = numberOfDispatchThreads_in;
  thread_data.useReactor = useReactor_in;
  struct Test_I_Source_Configuration configuration;
  if (!Common_Tools::initializeEventDispatch (useReactor_in,
                                              useThreadPool_in,
                                              numberOfDispatchThreads_in,
                                              thread_data.proactorType,
                                              thread_data.reactorType,
                                              configuration.streamConfiguration.configuration_.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step0b: initialize configuration and stream
  configuration.protocol = (useUDP_in ? NET_TRANSPORTLAYER_UDP
                                      : NET_TRANSPORTLAYER_TCP);
  if (useReactor_in)
    ACE_NEW_NORETURN (CBData_in.stream,
                      Test_I_Source_TCPStream_t ());
  else
    ACE_NEW_NORETURN (CBData_in.stream,
                      Test_I_Source_AsynchTCPStream_t ());
  if (useReactor_in)
    ACE_NEW_NORETURN (CBData_in.UDPStream,
                      Test_I_Source_UDPStream_t ());
  else
    ACE_NEW_NORETURN (CBData_in.UDPStream,
                      Test_I_Source_AsynchUDPStream_t ());
  if (!CBData_in.stream || !CBData_in.UDPStream)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF
  //configuration.userData.connectionConfiguration =
  //    &configuration.connectionConfiguration;
  //configuration.userData.streamConfiguration =
  //  &configuration.streamConfiguration;
  configuration.useReactor = useReactor_in;

  Stream_AllocatorHeap_T<struct Stream_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (configuration.streamConfiguration.allocatorConfiguration_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));

    // clean up
    delete CBData_in.stream;
    delete CBData_in.UDPStream;

    return;
  } // end IF
  Test_I_Source_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                      &heap_allocator,     // heap allocator handle
                                                      true);               // block ?

  CBData_in.configuration = &configuration;
  Test_I_Source_EventHandler ui_event_handler (&CBData_in);
  Test_I_Stream_Source_EventHandler_Module event_handler ((useUDP_in ? CBData_in.UDPStream : CBData_in.stream),
                                                          ACE_TEXT_ALWAYS_CHAR ("EventHandler"));
  Test_I_Stream_Source_EventHandler* event_handler_p =
    dynamic_cast<Test_I_Stream_Source_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Stream_Source_EventHandler> failed, returning\n")));

    // clean up
    delete CBData_in.stream;
    delete CBData_in.UDPStream;

    return;
  } // end IF

  Test_I_Source_InetConnectionManager_t* iconnection_manager_p =
    TEST_I_SOURCE_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);

  // ********************** connection configuration data **********************
  struct Test_I_Source_ConnectionConfiguration connection_configuration;
  int result =
    connection_configuration.socketHandlerConfiguration.socketConfiguration_2.address.set (port_in,
                                                                                           hostName_in.c_str (),
                                                                                           1,
                                                                                           ACE_ADDRESS_FAMILY_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));

    // clean up
    delete CBData_in.stream;
    delete CBData_in.UDPStream;

    return;
  } // end IF
  connection_configuration.socketHandlerConfiguration.socketConfiguration_2.useLoopBackDevice =
    connection_configuration.socketHandlerConfiguration.socketConfiguration_2.address.is_loopback ();
  connection_configuration.socketHandlerConfiguration.socketConfiguration_3.writeOnly =
    true;

  connection_configuration.socketHandlerConfiguration.statisticReportingInterval =
    statisticReportingInterval_in;
  connection_configuration.socketHandlerConfiguration.userData =
    &configuration.userData;

  connection_configuration.connectionManager =
    iconnection_manager_p;
  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.PDUSize = bufferSize_in;
  connection_configuration.streamConfiguration =
    &configuration.streamConfiguration;
  connection_configuration.userData = &configuration.userData;
  configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                 connection_configuration));

  Test_I_Source_ConnectionConfigurationIterator_t iterator =
    configuration.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration.connectionConfigurations.end ());
  (*iterator).second.socketHandlerConfiguration.connectionConfiguration =
    &((*iterator).second);

  // ********************** stream configuration data **************************
  // ********************** module configuration data **************************
  struct Test_I_Source_ModuleHandlerConfiguration modulehandler_configuration;
  modulehandler_configuration.allocatorConfiguration =
    &configuration.streamConfiguration.allocatorConfiguration_;
  modulehandler_configuration.connectionManager = iconnection_manager_p;
  modulehandler_configuration.fileName = fileName_in;
  modulehandler_configuration.printProgressDot =
    UIDefinitionFile_in.empty ();
  modulehandler_configuration.connectionConfigurations =
    &configuration.connectionConfigurations;
  modulehandler_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  modulehandler_configuration.stream =
    ((configuration.protocol == NET_TRANSPORTLAYER_TCP) ? CBData_in.stream
                                                        : CBData_in.UDPStream);
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.subscribers = &CBData_in.subscribers;
  modulehandler_configuration.subscribersLock = &CBData_in.subscribersLock;

  // ********************* (sub-)stream configuration data *********************
  if (bufferSize_in)
    configuration.streamConfiguration.allocatorConfiguration_.defaultBufferSize =
        bufferSize_in;

  configuration.streamConfiguration.configuration_.messageAllocator =
      &message_allocator;
  configuration.streamConfiguration.configuration_.module =
    (!UIDefinitionFile_in.empty () ? &event_handler
                                   : NULL);
  configuration.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                            modulehandler_configuration));
  configuration.streamConfiguration.configuration_.printFinalReport = true;

  // step0c: initialize connection manager
  iconnection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  iconnection_manager_p->set ((*iterator).second,
                              &configuration.userData);

  // step0d: initialize regular (global) statistic reporting
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();
  Test_I_StatisticHandler_t statistic_handler (ACTION_REPORT,
                                               iconnection_manager_p,
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
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));

      // clean up
      timer_manager_p->stop ();
      delete CBData_in.stream;
      delete CBData_in.UDPStream;

      return;
    } // end IF
  } // end IF

  // step0e: initialize signal handling
  configuration.signalHandlerConfiguration.useReactor = useReactor_in;
  configuration.signalHandlerConfiguration.stream = CBData_in.stream;
  if (!signalHandler_in.initialize (configuration.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));

    // clean up
    timer_manager_p->stop ();
    delete CBData_in.stream;
    delete CBData_in.UDPStream;

    return;
  } // end IF
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), returning\n")));

    // clean up
    timer_manager_p->stop ();
    delete CBData_in.stream;
    delete CBData_in.UDPStream;

    return;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step1a: start GTK event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
    CBData_in.finalizationHook = idle_finalize_UI_cb;
    CBData_in.initializationHook = idle_initialize_source_UI_cb;
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
    CBData_in.userData = &CBData_in;

    TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->start ();
    ACE_Time_Value one_second (1, 0);
    int result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
    if (!TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));

      // clean up
      timer_manager_p->stop ();
      delete CBData_in.stream;
      delete CBData_in.UDPStream;

      return;
    } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));

      // clean up
      TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->stop (true);
      timer_manager_p->stop ();
      delete CBData_in.stream;
      delete CBData_in.UDPStream;

      return;
    } // end IF
    BOOL was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif
  } // end IF

  // step1b: initialize worker(s)
  int group_id = -1;
  if (!Common_Tools::startEventDispatch (thread_data,
                                         group_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));

    // clean up
    //		{ // synch access
    //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

    //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
    //					 iterator != CBData_in.event_source_ids.end();
    //					 iterator++)
    //				g_source_remove(*iterator);
    //		} // end lock scope
    if (!UIDefinitionFile_in.empty ())
      TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->stop ();
    timer_manager_p->stop ();
    delete CBData_in.stream;
    delete CBData_in.UDPStream;

    return;
  } // end IF

  if (UIDefinitionFile_in.empty ())
  {
    Test_I_StreamBase_t* stream_p =
      ((configuration.protocol == NET_TRANSPORTLAYER_TCP) ? CBData_in.stream
                                                          : CBData_in.UDPStream);
    ACE_ASSERT (stream_p);

    unsigned int counter = 0;
loop:
    if (!stream_p->initialize (configuration.streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));

      // clean up
      Common_Tools::finalizeEventDispatch (useReactor_in,
                                           !useReactor_in,
                                           group_id);
      timer_manager_p->stop ();
      delete CBData_in.stream;
      delete CBData_in.UDPStream;

      return;
    } // end IF

    // *NOTE*: this call blocks until the file has been sent (or an error
    //         occurs)
    stream_p->start ();
    //    if (!stream_p->isRunning ())
    //    {
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to start stream, aborting\n")));

    //      // clean up
    //      //timer_manager_p->stop ();

    //      return;
    //    } // end IF
    stream_p->wait (true, false, false);
    if (CBData_in.loop)
    {
      ++counter;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("iteration #%u complete...\n"),
                  counter));
      if ((static_cast<int> (CBData_in.loop) == -1) ||
          (counter != CBData_in.loop))
        goto loop;
    } // end IF

    // clean up
    iconnection_manager_p->stop ();
    iconnection_manager_p->wait ();
    Common_Tools::finalizeEventDispatch (useReactor_in,
                                         !useReactor_in,
                                         group_id);
  } // end IF
  else
    Common_Tools::dispatchEvents (useReactor_in,
                                  group_id);

  // step3: clean up
  if (!UIDefinitionFile_in.empty ())
    TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->stop ();
  timer_manager_p->stop ();

  //		{ // synch access
  //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //					 iterator != CBData_in.event_source_ids.end();
  //					 iterator++)
  //				g_source_remove(*iterator);
  //		} // end lock scope
  //timer_manager_p->stop ();

  //  { // synch access
  //    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //		for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //				 iterator != CBData_in.event_source_ids.end();
  //				 iterator++)
  //			g_source_remove(*iterator);
  //	} // end lock scope

  //result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  //if (result == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
  //              event_handler.name ()));
  delete CBData_in.stream;
  delete CBData_in.UDPStream;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

void
do_printVersion (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printVersion"));

  std::ostringstream converter;

  // compiler version string
  converter << ACE::compiler_major_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::compiler_minor_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::compiler_beta_version ();

  std::cout << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" compiled on ")
            << ACE::compiler_name ()
            << ACE_TEXT_ALWAYS_CHAR (" ")
            << converter.str ()
            << std::endl << std::endl;

  std::cout << ACE_TEXT_ALWAYS_CHAR ("libraries: ")
            << std::endl
#ifdef HAVE_CONFIG_H
            << ACE_TEXT_ALWAYS_CHAR (LIBACESTREAM_PACKAGE)
            << ACE_TEXT_ALWAYS_CHAR (": ")
            << ACE_TEXT_ALWAYS_CHAR (LIBACESTREAM_PACKAGE_VERSION)
            << std::endl
#endif
            ;

  converter.str ("");
  // ACE version string...
  converter << ACE::major_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::minor_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::beta_version ();

  // *NOTE*: cannot use ACE_VERSION, as it doesn't contain the (potential) beta
  // version number... Need this, as the library soname is compared to this
  // string
  std::cout << ACE_TEXT_ALWAYS_CHAR ("ACE: ")
//             << ACE_VERSION
            << converter.str ()
            << std::endl;
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

  int result = -1;

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
#endif
  Common_Tools::initialize ();

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer
  process_profile.start ();

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
  //configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //configuration_path += ACE_TEXT_ALWAYS_CHAR ("filestream");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  unsigned int buffer_size = TEST_I_DEFAULT_BUFFER_SIZE;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  std::string gtk_rc_file = path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  std::string source_file;
  std::string gtk_glade_file = path;
  gtk_glade_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_glade_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  std::string host_name = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_HOSTNAME);
  bool log_to_file = false;
  bool use_thread_pool = NET_EVENT_USE_THREAD_POOL;
  unsigned short port = NET_ADDRESS_DEFAULT_PORT;
  bool use_reactor = NET_EVENT_USE_REACTOR;
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information = false;
  bool use_UDP = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    NET_CLIENT_DEFAULT_NUMBER_OF_DISPATCH_THREADS;
  size_t loop = 0;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
                            gtk_rc_file,
                            source_file,
                            gtk_glade_file,
                            host_name,
                            log_to_file,
                            use_thread_pool,
                            port,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            use_UDP,
                            print_version_and_exit,
                            number_of_dispatch_threads,
                            loop))
  {
    do_printUsage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

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
  if (use_reactor                      &&
      (number_of_dispatch_threads > 1) &&
      !use_thread_pool)
  { // *NOTE*: see also: man (2) select
    // *TODO*: verify this for MS Windows based systems
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
    use_thread_pool = true;
  } // end IF
  if ((gtk_glade_file.empty () &&
       !Common_File_Tools::isReadable (source_file))                        ||
      (!gtk_glade_file.empty () &&
       !Common_File_Tools::isReadable (gtk_glade_file))                     ||
      (!gtk_rc_file.empty () &&
       !Common_File_Tools::isReadable (gtk_rc_file))                        ||
      (use_thread_pool && !use_reactor)                                     ||
      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool) ||
      (loop && !gtk_glade_file.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (number_of_dispatch_threads == 0) number_of_dispatch_threads = 1;

  struct Test_I_Source_GTK_CBData gtk_cb_user_data;
  gtk_cb_user_data.progressData.GTKState = &gtk_cb_user_data;
  gtk_cb_user_data.loop = loop;
  // step1d: initialize logging and/or tracing
  Common_Logger_t logger (&gtk_cb_user_data.logStack,
                          &gtk_cb_user_data.lock);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACESTREAM_PACKAGE_NAME),
                                         ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
  if (!Common_Tools::initializeLogging (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])), // program name
                                        log_file_name,                        // log file name
                                        false,                                // log to syslog ?
                                        false,                                // trace messages ?
                                        trace_information,                    // debug messages ?
                                        (gtk_glade_file.empty () ? NULL
                                                                 : &logger))) // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  sigset_t previous_signal_mask;
  result = ACE_OS::sigemptyset (&previous_signal_mask);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sigemptyset(): \"%m\", aborting\n")));

    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (!Common_Tools::preInitializeSignals (signal_set,
                                           use_reactor,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::preInitializeSignals(), aborting\n")));

    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  Test_I_Source_SignalHandler signal_handler;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false, // file descriptors
                                        true,  // stack traces
                                        true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
  gtk_cb_user_data.RCFiles.push_back (gtk_rc_file);
  Test_I_Source_GtkBuilderDefinition_t ui_definition (argc_in,
                                                      argv_in);
  if (!gtk_glade_file.empty ())
    if (!TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                                       argv_in,
                                                                       &gtk_cb_user_data,
                                                                       &ui_definition))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager::initialize(), aborting\n")));

      Common_Tools::finalizeSignals (signal_set,
                                     previous_signal_actions,
                                     previous_signal_mask);
      Common_Tools::finalizeLogging ();
      // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

      return EXIT_FAILURE;
    } // end IF

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
           source_file,
           gtk_glade_file,
           host_name,
           use_thread_pool,
           port,
           use_reactor,
           statistic_reporting_interval,
           use_UDP,
           number_of_dispatch_threads,
           gtk_cb_user_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  Common_Tools::periodToString (working_time,
                                working_time_string);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // stop profile timer
  process_profile.stop ();

  // only process profile left to do...
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  if (process_profile.elapsed_time (elapsed_time) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
  Common_Tools::periodToString (user_time,
                               user_time_string);
  Common_Tools::periodToString (system_time,
                               system_time_string);

  // debug info
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
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
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ())));
#endif

  Common_Tools::finalizeSignals (signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;
} // end main
