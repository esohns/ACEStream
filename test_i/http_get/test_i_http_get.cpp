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
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H
#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_dec_common.h"

#include "stream_file_sink.h"

#include "stream_net_http_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H
#include "net_common_tools.h"

#include "test_i_common.h"

#include "test_i_module_databasewriter.h"

#include "test_i_connection_manager_common.h"
#include "test_i_http_get_common.h"
#include "test_i_http_get_defines.h"
#include "test_i_http_get_signalhandler.h"
#include "test_i_http_get_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("HTTPGetStream");

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string path =
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
  std::string database_options_file = path;
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_OPTIONS_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [PATH]   : database options file [")
            << database_options_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : write to database [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e [STRING] : write to database table [")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_TABLE)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : (output) file name [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : port number [")
            << HTTP_DEFAULT_SERVER_PORT
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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [STRING] : URL [")
            << ACE_TEXT_ALWAYS_CHAR (TEST_T_DEFAULT_URL)
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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-z          : debug parser [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
                     std::string& dataBaseOptionsFileName_out,
                     bool& outputToDataBase_out,
                     std::string& outputDataBaseTable_out,
                     std::string& outputFileName_out,
                     bool& logToFile_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     std::string& URI_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out,
                     bool& debugParser_out,
                     std::string& hostName_out,
                     ACE_INET_Addr& remoteHost_out,
                     bool& useSSL_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  bufferSize_out = TEST_I_DEFAULT_BUFFER_SIZE;
  dataBaseOptionsFileName_out = path;
  dataBaseOptionsFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  dataBaseOptionsFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  dataBaseOptionsFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  dataBaseOptionsFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_OPTIONS_FILE);
  outputToDataBase_out = false;
  outputDataBaseTable_out =
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_TABLE);
  outputFileName_out = path;
  outputFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  outputFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  hostName_out.clear ();
  logToFile_out = false;
  port_out = HTTP_DEFAULT_SERVER_PORT;
  useReactor_out =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  URI_out = ACE_TEXT_ALWAYS_CHAR (TEST_T_DEFAULT_URL);
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
  debugParser_out = 0;
  hostName_out.clear ();
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

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("b:c:de:f:lp:rs:tu:vx:z"),
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
        dataBaseOptionsFileName_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'd':
      {
        outputToDataBase_out = true;
        break;
      }
      case 'e':
      {
        outputDataBaseTable_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'f':
      {
        outputFileName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
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
        URI_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
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
        debugParser_out = true;
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

  // step1: parse URL
  std::string URI_s;
  if (!HTTP_Tools::parseURL (URI_out,
                             remoteHost_out,
                             hostName_out,
                             URI_s,
                             useSSL_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                ACE_TEXT (URI_out.c_str ())));
    return false;
  } // end IF

//  std::string hostname_string = hostName_out;
//  size_t position =
//    hostname_string.find_last_of (':', std::string::npos);
//  if (position == std::string::npos)
//  {
//    port_out = (useSSL_out ? HTTPS_DEFAULT_SERVER_PORT
//                           : HTTP_DEFAULT_SERVER_PORT);
//    hostname_string += ':';
//    std::ostringstream converter;
//    converter << port_out;
//    hostname_string += converter.str ();
//  } // end IF
//  else
//  {
//    std::istringstream converter (hostname_string.substr (position + 1,
//                                                          std::string::npos));
//    converter >> port_out;
//  } // end ELSE
//  result = remoteHost_out.set (hostname_string.c_str (),
//                               AF_INET);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
//                ACE_TEXT (hostname_string.c_str ())));
//    return false;
//  } // end IF

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
         const std::string& dataBaseOptionsFileName_in,
         bool dataBase_in,
         const std::string& dataBaseTable_in,
         const std::string& fileName_in,
         const std::string& hostName_in,
         unsigned short port_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         const std::string& URL_in,
         unsigned int numberOfDispatchThreads_in,
         bool debugParser_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_Source_SignalHandler& signalHandler_in,
         const ACE_INET_Addr& address_in,
         bool useSSL_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  // step0a: initialize configuration and stream
  struct Test_I_HTTPGet_Configuration configuration;
  Test_I_StreamBase_t* stream_p = NULL;
  if (useReactor_in)
  {
    if (useSSL_in)
      ACE_NEW_NORETURN (stream_p,
                        Test_I_HTTPGet_SSL_Stream_t ());
    else
      ACE_NEW_NORETURN (stream_p,
                        Test_I_HTTPGet_Stream_t ());
  } // end IF
  else
  {
    if (useSSL_in)
      ACE_ASSERT (false); // *TODO*
    else
      ACE_NEW_NORETURN (stream_p,
                        Test_I_HTTPGet_AsynchStream_t ());
  } // end ELSE
  if (!stream_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF
  if (useReactor_in)
    configuration.dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    configuration.dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;

  Stream_Module_t* module_p = NULL;
  Test_I_Module_DataBaseWriter_Module database_writer (stream_p,
                                                       ACE_TEXT_ALWAYS_CHAR (MODULE_DB_MYSQL_TARGET_DEFAULT_NAME_STRING));
  Test_I_FileWriter_Module file_writer (stream_p,
                                        ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING));
  module_p = &file_writer;
  if (dataBase_in)
    module_p = &database_writer;
  //Test_I_IModuleHandler_t* module_handler_p =
  //  dynamic_cast<Test_I_IModuleHandler_t*> (module_p->writer ());
  //if (!module_handler_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: dynamic_cast<Test_I_IModuleHandler_t>(0x%@) failed, returning\n"),
  //              module_p->name (),
  //              module_p->writer ()));

  //  // clean up
  //  delete stream_p;

  //  return;
  //} // end IF

  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Test_I_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?

  Test_I_Stream_InetConnectionManager_t* connection_manager_p =
    TEST_I_STREAM_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

  // *********************** socket configuration data ************************
  Test_I_HTTPGet_ConnectionConfiguration_t connection_configuration;
  int result =
    connection_configuration.socketConfiguration.address.set (port_in,
                                                              hostName_in.c_str (),
                                                              1,
                                                              AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));

    // clean up
    delete stream_p;

    return;
  } // end IF
  connection_configuration.socketConfiguration.useLoopBackDevice =
    connection_configuration.socketConfiguration.address.is_loopback ();
//  connection_configuration.socketHandlerConfiguration.socketConfiguration_2.writeOnly =
//    true;
  connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.allocatorConfiguration = &allocator_configuration;
  connection_configuration.allocatorConfiguration->defaultBufferSize =
    bufferSize_in;
  connection_configuration.streamConfiguration =
    &configuration.streamConfiguration_2;

  configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                 &connection_configuration));
  //Net_ConnectionConfigurationsIterator_t iterator =
  //  configuration.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  //ACE_ASSERT (iterator != configuration.connectionConfigurations.end ());

  // ********************** stream configuration data **************************
  // ********************** prser configuration data ***************************
#if defined (_DEBUG)
  configuration.parserConfiguration.debugParser = debugParser_in;
  if (debugParser_in)
    configuration.parserConfiguration.debugScanner = true;
#endif // _DEBUG
  // ********************** module configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct Test_I_HTTPGet_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_I_HTTPGet_ModuleHandlerConfiguration modulehandler_configuration_2; // connection-
  struct Test_I_HTTPGet_StreamConfiguration stream_configuration;
  struct Test_I_HTTPGet_StreamConfiguration stream_configuration_2; // connection-

  modulehandler_configuration.connectionConfigurations =
    &configuration.connectionConfigurations;
  modulehandler_configuration.connectionManager =
      connection_manager_p;
  modulehandler_configuration.dataBaseOptionsFileName =
      dataBaseOptionsFileName_in;
  modulehandler_configuration.dataBaseTable = dataBaseTable_in;
  modulehandler_configuration.targetFileName = fileName_in;
  modulehandler_configuration.loginOptions.database = dataBase_in;
  //modulehandler_configuration.loginOptions.password =;
  //modulehandler_configuration.loginOptions.user = ;
  modulehandler_configuration.parserConfiguration =
    &configuration.parserConfiguration;
  modulehandler_configuration.passive = false;
  modulehandler_configuration.streamConfiguration =
    &configuration.streamConfiguration;
  modulehandler_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  //modulehandler_configuration.stream = stream_p;
  modulehandler_configuration.URL = URL_in;
  // ******************** (sub-)stream configuration data *********************
  if (bufferSize_in)
    allocator_configuration.defaultBufferSize = bufferSize_in;

  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = module_p;
  stream_configuration.printFinalReport = true;
  configuration.streamConfiguration.initialize (module_configuration,
                                                modulehandler_configuration,
                                                stream_configuration);

  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration_2.streamConfiguration =
    &configuration.streamConfiguration_2;
  stream_configuration_2 = stream_configuration;
  stream_configuration_2.module = NULL;
  configuration.streamConfiguration_2.initialize (module_configuration,
                                                  modulehandler_configuration_2,
                                                  stream_configuration_2);

  //module_handler_p->initialize (configuration.moduleHandlerConfiguration);

  // step0b: initialize event dispatch
  configuration.dispatchConfiguration.numberOfProactorThreads =
          (!useReactor_in ? numberOfDispatchThreads_in : 0);
  configuration.dispatchConfiguration.numberOfReactorThreads =
          (useReactor_in ? numberOfDispatchThreads_in : 0);
  if (!Common_Event_Tools::initializeEventDispatch (configuration.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    delete stream_p; stream_p = NULL;
    return;
  } // end IF

  // step0c: initialize connection manager
  struct Net_UserData user_data_s;
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (connection_configuration,
                             &user_data_s);

  // step0d: initialize regular (global) statistic reporting
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
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
      delete stream_p; stream_p = NULL;
      return;
    } // end IF
  } // end IF

  // step0c: initialize signal handling
  struct Common_EventDispatchState dispatch_state_s;
  dispatch_state_s.configuration = &configuration.dispatchConfiguration;
  configuration.signalHandlerConfiguration.dispatchState = &dispatch_state_s;
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerID = timer_id;
  if (!signalHandler_in.initialize (configuration.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    timer_manager_p->stop ();
    delete stream_p; stream_p = NULL;
    return;
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
    timer_manager_p->stop ();
    delete stream_p; stream_p = NULL;
    return;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
    timer_manager_p->stop ();
    delete stream_p; stream_p = NULL;
    return;
  } // end IF

  if (!stream_p->initialize (configuration.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, returning\n")));
    Common_Event_Tools::finalizeEventDispatch (dispatch_state_s,
                                               true); // wait ?
    timer_manager_p->stop ();
    delete stream_p; stream_p = NULL;
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
  stream_p->wait (true,   // wait for any worker thread(s) ?
                  false,  // wait for upstream (if any) ?
                  false); // wait for downstream (if any) ?

  // clean up
  connection_manager_p->stop (false, true);
  connection_manager_p->abort ();
  connection_manager_p->wait ();
  Common_Event_Tools::finalizeEventDispatch (dispatch_state_s,
                                             true); // wait ?

  // step3: clean up
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
  delete stream_p; stream_p = NULL;

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
  // *PORTABILITY*: on Windows, initialize ACE...
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
  // start profile timer...
  process_profile.start ();

#if defined(ACE_WIN32) || defined(ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false);        // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  unsigned int buffer_size = TEST_I_DEFAULT_BUFFER_SIZE;
  std::string database_options_file = configuration_path;
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_OPTIONS_FILE);
  bool output_to_database = false;
  std::string output_database_table =
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_TABLE);
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string output_file = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  bool log_to_file = false;
  unsigned short port = HTTP_DEFAULT_SERVER_PORT;
  bool use_reactor =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  std::string URL = ACE_TEXT_ALWAYS_CHAR (TEST_T_DEFAULT_URL);
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
  bool debug_parser = false;
  std::string host_name;
  ACE_INET_Addr address;
  bool use_ssl;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
                            database_options_file,
                            output_to_database,
                            output_database_table,
                            output_file,
                            log_to_file,
                            port,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            URL,
                            print_version_and_exit,
                            number_of_dispatch_threads,
                            debug_parser,
                            host_name,
                            address,
                            use_ssl))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
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
  if ((output_to_database && output_database_table.empty ())                ||
      (!database_options_file.empty () &&
       !Common_File_Tools::isReadable (database_options_file))              ||
      host_name.empty ()                                                    ||
//      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool) ||
      URL.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
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

  // step1d: initialize logging and/or tracing
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),           // program name
                                            log_file_name,                        // log file name
                                            false,                                // log to syslog ?
                                            false,                                // trace messages ?
                                            trace_information,                    // debug messages ?
                                            NULL))                                // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
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

    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
//  ACE_SYNCH_RECURSIVE_MUTEX signal_lock;
  Stream_Source_SignalHandler signal_handler;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
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
  if (!Common_OS_Tools::setResourceLimits (false, // file descriptors
                                           true,  // stack traces
                                           true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
           database_options_file,
           output_to_database,
           output_database_table,
           output_file,
           host_name,
           port,
           use_reactor,
           statistic_reporting_interval,
           URL,
           number_of_dispatch_threads,
           debug_parser,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler,
           address,
           use_ssl);
  timer.stop ();

  // debug info
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // stop profile timer...
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

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
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

  // debug info
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
  Common_Log_Tools::finalizeLogging ();
  Common_Tools::finalize ();
  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
