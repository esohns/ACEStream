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
#include <regex>
#include <string>

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif
#include "ace/Log_Msg.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/POSIX_Proactor.h"
#include "ace/Proactor.h"
#endif
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
//#include "ace/Synch.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common.h"
//#include "common_file_tools.h"
#include "common_tools.h"

#include "common_log_tools.h"
#if defined (GUI_SUPPORT)
#include "common_logger.h"
#endif // GUI_SUPPORT

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_defines.h"
#endif // GUI_SUPPORT

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"

#include "http_defines.h"

#include "test_u_defines.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "http_get_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "http_get_common.h"
#include "http_get_connection_manager_common.h"
#include "http_get_defines.h"
#include "http_get_eventhandler.h"
#include "http_get_module_eventhandler.h"
#include "http_get_signalhandler.h"
#include "http_get_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("HTTPGetStream");

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string working_directory =
    Common_File_Tools::getWorkingDirectory ();
  std::string temp_directory = Common_File_Tools::getTempDirectory ();
  std::string configuration_directory = working_directory;
  configuration_directory += ACE_DIRECTORY_SEPARATOR_STR_A;
  configuration_directory +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]  : buffer size (byte(s)) [")
            << NET_STREAM_MESSAGE_DATA_BUFFER_SIZE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : show console [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#endif
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug parser [")
            << COMMON_PARSER_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::string output_file_path = temp_directory;
  output_file_path += ACE_DIRECTORY_SEPARATOR_STR_A;
  output_file_path += ACE_TEXT_ALWAYS_CHAR (HTTP_GET_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : (output) file name [")
            << output_file_path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string UI_file_path = configuration_directory;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_STR_A;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_DEFINITION_FILE_NAME);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: interface definition file [\"")
            << UI_file_path
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [STRING] : URL")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]  : #dispatch threads [")
            << TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-z          : debug parser [")
            << COMMON_PARSER_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#endif
                     bool& debugParser_out,
                     std::string& outputFileName_out,
                     std::string& interfaceDefinitionFile_out,
                     std::string& hostName_out,
                     bool& logToFile_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     std::string& URL_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out,
                     ACE_INET_Addr& remoteHost_out,
                     bool& useSSL_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string working_directory =
    Common_File_Tools::getWorkingDirectory ();
  std::string temp_directory = Common_File_Tools::getTempDirectory ();
  std::string configuration_directory = working_directory;
  configuration_directory += ACE_DIRECTORY_SEPARATOR_STR_A;
  configuration_directory +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  // initialize results
  bufferSize_out = NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#endif
  debugParser_out = COMMON_PARSER_DEFAULT_YACC_TRACE;
  outputFileName_out = temp_directory;
  outputFileName_out += ACE_DIRECTORY_SEPARATOR_STR_A;
  outputFileName_out += ACE_TEXT_ALWAYS_CHAR (HTTP_GET_DEFAULT_OUTPUT_FILE);
  interfaceDefinitionFile_out = configuration_directory;
  interfaceDefinitionFile_out += ACE_DIRECTORY_SEPARATOR_STR_A;
  interfaceDefinitionFile_out +=
    ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_DEFINITION_FILE_NAME);
  hostName_out.clear ();
  logToFile_out = false;
  port_out = 0;
  useReactor_out =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  URL_out.clear ();
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
  int result =
    remoteHost_out.set (static_cast<u_short> (HTTP_DEFAULT_SERVER_PORT),
                        static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                        1,
                        0);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set (): \"%m\", aborting\n")));
    return false;
  } // end IF
  useSSL_out = false;

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               ACE_TEXT ("b:cdf:g::lrs:tu:vx:z"),
#else
                               ACE_TEXT ("b:df:g::lrs:tu:vx:z"),
#endif
                               1,                         // skip command name
                               1,                         // report parsing errors
                               ACE_Get_Opt::PERMUTE_ARGS, // ordering
                               0);                        // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argument_parser ()) != EOF)
  {
    switch (option)
    {
      case 'b':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argument_parser.opt_arg ();
        converter >> bufferSize_out;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'c':
      {
        showConsole_out = true;
        break;
      }
#endif
      case 'd':
      {
        debugParser_out = true;
        break;
      }
      case 'f':
      {
        outputFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          interfaceDefinitionFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          interfaceDefinitionFile_out.clear ();
        break;
      }
      case 'l':
      {
        logToFile_out = true;
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
        converter << argument_parser.opt_arg ();
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
        URL_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());

        // step1: parse URL
        std::string URI_s;
        if (!HTTP_Tools::parseURL (URL_out,
                                   hostName_out,
                                   URI_s,
                                   useSSL_out))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                      ACE_TEXT (URL_out.c_str ())));
          return false;
        } // end IF

        std::string hostname_string = hostName_out;
        size_t position =
          hostname_string.find_last_of (':', std::string::npos);
        if (position == std::string::npos)
        {
          hostname_string += ':';
          std::ostringstream converter;
          converter << (useSSL_out ? HTTPS_DEFAULT_SERVER_PORT
                                   : HTTP_DEFAULT_SERVER_PORT);
          hostname_string += converter.str ();
        } // end IF
        result = remoteHost_out.set (hostname_string.c_str (),
                                     AF_INET);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                      ACE_TEXT (hostname_string.c_str ())));
          return false;
        } // end IF

        // step2: validate address/verify host name exists
        //        --> resolve
        ACE_TCHAR buffer[HOST_NAME_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = remoteHost_out.get_host_name (buffer,
                                               sizeof (buffer));
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::get_host_name(): \"%m\", aborting\n")));
          return false;
        } // end IF
        std::string hostname = ACE_TEXT_ALWAYS_CHAR (buffer);
        std::string dotted_decimal_string;
        if (!Net_Common_Tools::getAddress (hostname,
                                           dotted_decimal_string))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_Common_Tools::getAddress(\"%s\"), aborting\n"),
                      ACE_TEXT (hostname.c_str ())));
          return false;
        } // end IF

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
        converter << argument_parser.opt_arg ();
        converter >> numberOfDispatchThreads_out;
        break;
      }
      case 'z':
      {
        debugParser_out = true;
        break;
      }
      // error handling
      case ':':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("option \"%c\" requires an argument, aborting\n"),
                    argument_parser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argument_parser.last_option ())));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    ACE_TEXT (argument_parser.long_option ())));
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
do_initializeSignals (bool useReactor_in,
                      bool allowUserRuntimeConnect_in,
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
  signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */

  // *IMPORTANT NOTE*: "...NPTL makes internal use of the first two real-time
  //                   signals (see also signal(7)); these signals cannot be
  //                   used in applications. ..." (see 'man 7 pthreads')
  // --> on POSIX platforms, make sure that ACE_SIGRTMIN == 34
  //  for (int i = ACE_SIGRTMIN;
  //       i <= ACE_SIGRTMAX;
  //       i++)
  //    signals_out.sig_del (i);

  if (!useReactor_in)
  {
    ACE_Proactor* proactor_p = ACE_Proactor::instance ();
    ACE_ASSERT (proactor_p);
    ACE_POSIX_Proactor* proactor_impl_p =
      dynamic_cast<ACE_POSIX_Proactor*> (proactor_p->implementation ());
    ACE_ASSERT (proactor_impl_p);
    if (proactor_impl_p->get_impl_type () == ACE_POSIX_Proactor::PROACTOR_SIG)
      signals_out.sig_del (COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);
  } // end IF
#endif

  // *NOTE*: gdb sends some signals (when running in an IDE ?)
  //         --> remove signals (and let IDE handle them)
  // *TODO*: clean this up
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DEBUG_DEBUGGER)
  //  signals_out.sig_del (SIGINT);
  signals_out.sig_del (SIGCONT);
  signals_out.sig_del (SIGHUP);
#endif
#endif

  // *TODO*: improve valgrind support
#ifdef LIBACESTREAM_ENABLE_VALGRIND_SUPPORT
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif
}

void
do_work (unsigned int bufferSize_in,
         bool debugParser_in,
         const std::string& fileName_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         const std::string& URL_in,
         const ACE_INET_Addr& remoteHost_in,
         bool useSSL_in,
         unsigned int numberOfDispatchThreads_in,
         const std::string& interfaceDefinitionFile_in,
         struct HTTPGet_UI_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#endif
         HTTPGet_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  // sanity check(s)
  ACE_ASSERT (CBData_in.configuration);

  ACE_thread_t thread_id = 0;

  // step0a: initialize configuration and stream
  Stream_IStreamControlBase* istream_base_p = NULL;
  Common_IInitialize_T<HTTPGet_StreamConfiguration_t>* iinitialize_p = NULL;
  Stream_IStream_t* istream_p = NULL;
  HTTPGet_Stream_t stream;
  HTTPGet_AsynchStream_t asynch_stream;
  if (useReactor_in)
    istream_p = &stream;
  else
    istream_p = &asynch_stream;

  HTTPGet_EventHandler event_handler (&CBData_in,
                                      interfaceDefinitionFile_in.empty ());
  HTTPGet_Module_EventHandler_Module event_handler_module (istream_p,
                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize message allocator, returning\n")));
    return;
  } // end IF
  HTTPGet_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                                &heap_allocator,         // heap allocator handle
                                                true);                   // block ?

  HTTPGet_ConnectionManager_t* connection_manager_p =
    HTTPGET_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

  // *********************** socket configuration data ************************
  HTTPGet_ConnectionConfiguration_t connection_configuration;
  connection_configuration.address = remoteHost_in;
  connection_configuration.useLoopBackDevice =
    connection_configuration.address.is_loopback ();

  connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;

  connection_configuration.allocatorConfiguration = &allocator_configuration;
  connection_configuration.allocatorConfiguration->defaultBufferSize = bufferSize_in;
  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.initialize (CBData_in.configuration->streamConfiguration);

  CBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                            &connection_configuration));
  Net_ConnectionConfigurationsIterator_t iterator =
    CBData_in.configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->connectionConfigurations.end ());

  // ********************** stream configuration data **************************
  // ********************** parser configuration data **************************
  CBData_in.configuration->parserConfiguration.debugParser = debugParser_in;
  if (debugParser_in)
    CBData_in.configuration->parserConfiguration.debugScanner = true;
  // ********************** module configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct HTTPGet_ModuleHandlerConfiguration modulehandler_configuration;
  modulehandler_configuration.configuration = CBData_in.configuration;
  modulehandler_configuration.connectionConfigurations =
    &CBData_in.configuration->connectionConfigurations;
  modulehandler_configuration.connectionManager = connection_manager_p;
  modulehandler_configuration.parserConfiguration =
    &CBData_in.configuration->parserConfiguration;
  modulehandler_configuration.passive = false;
  modulehandler_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  //modulehandler_configuration.stream = istream_p;
  modulehandler_configuration.streamConfiguration =
    &CBData_in.configuration->streamConfiguration;
  if (!interfaceDefinitionFile_in.empty ())
    modulehandler_configuration.subscriber = &event_handler;
  modulehandler_configuration.targetFileName = fileName_in;
  modulehandler_configuration.URL = URL_in;
  // ******************** (sub-)stream configuration data *********************
  struct Stream_Configuration steam_configuration;
  steam_configuration.messageAllocator = &message_allocator;
  steam_configuration.module =
    (!interfaceDefinitionFile_in.empty () ? &event_handler_module
                                          : NULL);
  steam_configuration.printFinalReport = true;
  CBData_in.configuration->streamConfiguration.initialize (module_configuration,
                                                           modulehandler_configuration,
                                                           steam_configuration);

  if (bufferSize_in)
    CBData_in.configuration->streamConfiguration.configuration->allocatorConfiguration->defaultBufferSize =
      bufferSize_in;

  CBData_in.stream = istream_p;

  //module_handler_p->initialize (configuration.moduleHandlerConfiguration);

  // step0c: initialize connection manager
  struct Net_UserData user_data_s;
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*dynamic_cast<HTTPGet_ConnectionConfiguration_t*> ((*iterator).second),
                             &user_data_s);

  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  struct Common_TimerConfiguration timer_configuration;
  int group_id = -1;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT
  // step0b: initialize event dispatch
  CBData_in.configuration->dispatchConfiguration.numberOfReactorThreads =
      (useReactor_in ? numberOfDispatchThreads_in : 0);
  CBData_in.configuration->dispatchConfiguration.numberOfProactorThreads =
      (!useReactor_in ? numberOfDispatchThreads_in : 0);
  if (!Common_Tools::initializeEventDispatch (CBData_in.configuration->dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    goto clean;
  } // end IF
  CBData_in.dispatchState.configuration =
      &CBData_in.configuration->dispatchConfiguration;

  //// step0d: initialize regular (global) statistic reporting
  //Stream_StatisticHandler_Reactor_t statistic_handler (ACTION_REPORT,
  //                                                     connection_manager_p,
  //                                                     false);
  ////Stream_StatisticHandler_Proactor_t statistic_handler_proactor (ACTION_REPORT,
  ////                                                               connection_manager_p,
  ////                                                               false);
  //long timer_id = -1;
  //if (statisticReportingInterval_in)
  //{
  //  ACE_Event_Handler* handler_p = &statistic_handler;
  //  ACE_Time_Value interval (statisticReportingInterval_in, 0);
  //  timer_id =
  //    timer_manager_p->schedule_timer (handler_p,                  // event handler
  //                                     NULL,                       // ACT
  //                                     COMMON_TIME_NOW + interval, // first wakeup time
  //                                     interval);                  // interval
  //  if (timer_id == -1)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));

  //    // clean up
  //    timer_manager_p->stop ();
  //    delete stream_p;

  //    return;
  //  } // end IF
  //} // end IF

  // step0c: initialize signal handling
  CBData_in.configuration->signalHandlerConfiguration.dispatchState =
    &CBData_in.dispatchState;
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerID = timer_id;
  if (!signalHandler_in.initialize (CBData_in.configuration->signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    goto clean;
  } // end IF
  if (!Common_Signal_Tools::initialize ((useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                       : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    goto clean;
  } // end IF

  // intialize timers
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (thread_id);
  ACE_UNUSED_ARG (thread_id);

  // step1a: start UI event loop ?
  if (!interfaceDefinitionFile_in.empty ())
  {
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    gtk_manager_p->start (thread_id);
    ACE_UNUSED_ARG (thread_id);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION * 1000);
    int result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      goto clean;
    } // end IF
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      goto clean;
    } // end IF
    BOOL was_visible_b = false;
    if (!showConsole_in)
      was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  // *WARNING*: from this point on, clean up any remote connections !

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
  if (!Common_Tools::startEventDispatch (CBData_in.dispatchState))
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
    goto clean;
  } // end IF

  if (interfaceDefinitionFile_in.empty ())
  {
    iinitialize_p =
      dynamic_cast<Common_IInitialize_T<HTTPGet_StreamConfiguration_t>*> (istream_p);
    ACE_ASSERT (iinitialize_p);
    istream_base_p = dynamic_cast<Stream_IStreamControlBase*> (istream_p);
    ACE_ASSERT (istream_base_p);
    // initialize processing stream
    if (!iinitialize_p->initialize (CBData_in.configuration->streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));
      goto clean;
    } // end IF

    istream_base_p->start ();
    //if (!stream_base_p->isRunning ())
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to start stream, aborting\n")));
    //  goto clean;
    //} // end IF
    istream_base_p->wait (true,   // wait for any worker thread(s) ?
                          false,  // wait for upstream (if any) ?
                          false); // wait for downstream (if any) ?
  } // end IF
  else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    gtk_manager_p->wait ();
#else
    ;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step3: clean up
  connection_manager_p->stop ();
  //Common_Tools::finalizeEventDispatch (useReactor_in,
  //                                     !useReactor_in,
  //                                     group_id);
  connection_manager_p->wait ();

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

  //if (!interfaceDefinitionFile_in.empty ())
  //{
  //  int result = event_handler_module.close (ACE_Module_Base::M_DELETE_NONE);
  //  if (result == -1)
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
  //                event_handler_module.name ()));
  //} // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

clean:
  Common_Tools::finalizeEventDispatch (useReactor_in,
                                       !useReactor_in,
                                       group_id);
  timer_manager_p->stop ();
  if (!interfaceDefinitionFile_in.empty ())
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
#else
    ;
#endif // GTK_USE
#endif // GUI_SUPPORT
}

COMMON_DEFINE_PRINTVERSION_FUNCTION(do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

  int result;

  unsigned int buffer_size;
  bool debug_parser;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console;
#endif
  bool log_to_file;
  bool use_reactor;
  unsigned int statistic_reporting_interval;
  unsigned int number_of_dispatch_threads;
  bool trace_information;
  bool print_version_and_exit;
  std::string configuration_directory;
  std::string working_directory;
  std::string temp_directory;
  std::string output_file_path;
  std::string UI_file_path;
  std::string host_name;
  unsigned short port;
  std::string URL;
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  Common_SignalActions_t previous_signal_actions;
  sigset_t previous_signal_mask;
  std::string log_file_name;
  struct HTTPGet_UI_CBData ui_cb_data;
  //Common_Logger_t logger (&ui_cb_data.UIState.logStack,
  //                        &ui_cb_data.UIState.lock);
  ACE_SYNCH_RECURSIVE_MUTEX* lock_p = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
  lock_p = &state_r.subscribersLock;
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
#endif // GTK_USE
#endif // GUI_SUPPORT
  struct HTTPGet_Configuration configuration;
  HTTPGet_SignalHandler signal_handler (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                        lock_p);
  ACE_Profile_Timer process_profile;
  ACE_High_Res_Timer timer;
  std::string working_time_string;
  ACE_Time_Value working_time;
  ACE_Time_Value user_time;
  ACE_Time_Value system_time;
  std::string user_time_string;
  std::string system_time_string;
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_INET_Addr remote_host;
  bool use_SSL;

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

#if defined (VALGRIND_SUPPORT)
  if (RUNNING_ON_VALGRIND)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("running on valgrind...\n")));
#endif // VALGRIND_SUPPORT

  // step0: process profile
  result = process_profile.start ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::start(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  Common_Tools::initialize ();

  ui_cb_data.configuration = &configuration;
  working_directory = Common_File_Tools::getWorkingDirectory ();
  temp_directory = Common_File_Tools::getTempDirectory ();
  configuration_directory = working_directory;
  configuration_directory += ACE_DIRECTORY_SEPARATOR_STR_A;
  configuration_directory +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  // step1a: set defaults
  buffer_size = NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
  debug_parser = COMMON_PARSER_DEFAULT_YACC_TRACE;
  output_file_path = temp_directory;
  output_file_path += ACE_DIRECTORY_SEPARATOR_STR_A;
  output_file_path +=
    ACE_TEXT_ALWAYS_CHAR (HTTP_GET_DEFAULT_OUTPUT_FILE);
  UI_file_path = configuration_directory;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_STR_A;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (HTTPGET_UI_DEFINITION_FILE_NAME);
  log_to_file = false;
  use_reactor =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  trace_information = false;
  print_version_and_exit = false;
  number_of_dispatch_threads =
    TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#endif
                            debug_parser,
                            output_file_path,
                            UI_file_path,
                            host_name,
                            log_to_file,
                            port,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            URL,
                            print_version_and_exit,
                            number_of_dispatch_threads,
                            remote_host,
                            use_SSL))
  {
    // help the user: print usage instructions
    do_printUsage (ACE::basename (argv_in[0]));
    goto error;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (NET_STREAM_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if ((!UI_file_path.empty () &&
       !Common_File_Tools::isReadable (UI_file_path))                      ||
      (host_name.empty () && URL.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    // help the user: print usage instructions
    do_printUsage (ACE::basename (argv_in[0]));
    goto error;
  } // end IF
  if (number_of_dispatch_threads == 0)
    number_of_dispatch_threads = 1;

  // step2: run program ?
  if (print_version_and_exit)
  {
    do_printVersion (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                          ACE_DIRECTORY_SEPARATOR_CHAR)));
    goto done;
  } // end IF

  // step3: initialize logging and/or tracing
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                             ACE_DIRECTORY_SEPARATOR_CHAR)));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),           // program name
                                            log_file_name,                        // log file name
                                            false,                                // log to syslog ?
                                            false,                                // trace messages ?
                                            trace_information,                    // debug messages ?
                                            NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));
    goto error;
  } // end IF

  // step4: pre-initialize signal handling
  do_initializeSignals (use_reactor,
                        true, // allow SIGUSR1/SIGBREAK
                        signal_set,
                        ignored_signal_set);
  result = ACE_OS::sigemptyset (&previous_signal_mask);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sigemptyset(): \"%m\", aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    goto error;
  } // end IF
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           use_reactor,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    goto error;
  } // end IF

  // step5: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false, // file descriptors
                                        true,  // stack traces
                                        true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    goto error;
  } // end IF

  // step6: initialize configuration
  //configuration.userData = &user_data;

  // step7: initialize user interface, if any
  if (UI_file_path.empty ())
    goto continue_;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ui_cb_data.progressData.state = &state_r;

  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_ui_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_ui_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (UI_file_path, static_cast<GtkBuilder*> (NULL));
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
#endif // GUI_SUPPORT

continue_:
  timer.start ();
  // step8: run program
  do_work (buffer_size,
           debug_parser,
           output_file_path,
           use_reactor,
           statistic_reporting_interval,
           URL,
           remote_host,
           use_SSL,
           number_of_dispatch_threads,
           UI_file_path,
           ui_cb_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#endif
           signal_handler);
  timer.stop ();

  // debug info
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

done:
  process_profile.stop ();

  // debug info
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  result = process_profile.elapsed_time (elapsed_time);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    goto error;
  } // end IF
  process_profile.elapsed_rusage (elapsed_rusage);
  user_time.set (elapsed_rusage.ru_utime);
  system_time.set (elapsed_rusage.ru_stime);
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
#endif

  // step9: clean up
  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif
  return EXIT_FAILURE;
} // end main
