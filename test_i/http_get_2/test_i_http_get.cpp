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

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#ifdef LIBACESTREAM_ENABLE_VALGRIND_SUPPORT
#include "valgrind/valgrind.h"
#endif

#include "common.h"
#include "common_file_tools.h"
#include "common_logger.h"
#include "common_tools.h"

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_net_http_defines.h"

#ifdef HAVE_CONFIG_H
#include "libACEStream_config.h"
#endif

#include "net_common_tools.h"

#include "http_defines.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_module_spreadsheetwriter.h"

#include "test_i_http_get_common.h"
#include "test_i_http_get_signalhandler.h"
#include "test_i_http_get_stream.h"

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::string configuration_file = path;
#if defined (DEBUG_DEBUGGER)
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file += ACE_TEXT_ALWAYS_CHAR ("http_get_2");
#endif
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [FILE]   : LibreOffice bootstrap .ini [")
            << configuration_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug HTTP parser [")
            << NET_PROTOCOL_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  configuration_file = path;
#if defined (DEBUG_DEBUGGER)
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file += ACE_TEXT_ALWAYS_CHAR ("http_get_2");
#endif
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_PORTFOLIO_CONFIGURATION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [FILE]   : stock index portfolio .ini [")
            << configuration_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-h [HOST]   : LibreOffice application hostname [")
            << ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o [FILE]   : (output) LibreOffice spreadsheet .ods [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : LibreOffice application port number [")
            << TEST_I_DEFAULT_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [STRING] : stock index value URL")
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
                     std::string& bootstrapFileName_out,
                     bool& debug_out,
                     std::string& configurationFileName_out,
                     std::string& hostName_out,
                     bool& logToFile_out,
                     std::string& outputFileName_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     bool& traceInformation_out,
                     std::string& URI_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out,
                     ACE_INET_Addr& remoteHost_out,
                     bool& useSSL_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string path = Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  bootstrapFileName_out = path;
#if defined (DEBUG_DEBUGGER)
  bootstrapFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrapFileName_out += ACE_TEXT_ALWAYS_CHAR ("http_get_2");
#endif
  bootstrapFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrapFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  bootstrapFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrapFileName_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE);
  debug_out = NET_PROTOCOL_DEFAULT_YACC_TRACE;
  configurationFileName_out = path;
#if defined (DEBUG_DEBUGGER)
  configurationFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFileName_out += ACE_TEXT_ALWAYS_CHAR ("http_get_2");
#endif
  configurationFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  configurationFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFileName_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_PORTFOLIO_CONFIGURATION_FILE);
  hostName_out = ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST);
  logToFile_out = false;
  outputFileName_out = path;
  outputFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  outputFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  port_out = TEST_I_DEFAULT_PORT;
  useReactor_out = NET_EVENT_USE_REACTOR;
  traceInformation_out = false;
  URI_out.clear ();
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
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
                              ACE_TEXT ("c:df:h:lo:p:rtu:vx:"),
                              1,                         // skip command name
                              1,                         // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS, // ordering
                              0);                        // for now, don't use long options

  int option = 0;
  std::istringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'c':
      {
        bootstrapFileName_out =
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'd':
      {
        debug_out = true;
        break;
      }
      case 'f':
      {
        configurationFileName_out =
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
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
        outputFileName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.str (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
        converter >> port_out;
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        // step1: parse URL
        URI_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());

        if (!HTTP_Tools::parseURL (URI_out,
                                   remoteHost_out,
                                   URI_out))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                      ACE_TEXT (URI_out.c_str ())));
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
                      ACE_TEXT ("failed to Net_Common_Tools::getAddress(), aborting\n")));
          return false;
        } // end IF

        //// step3: validate URI
        //std::string regex_string =
        //    ACE_TEXT_ALWAYS_CHAR ("^(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
        ////regex_string =
        ////    ACE_TEXT_ALWAYS_CHAR ("^(?:http(?:s)?://)?((.+\\.)+([^\\/]+))(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
        //std::regex regex;
        //regex.assign (regex_string,
        //              (std::regex_constants::ECMAScript |
        //               std::regex_constants::icase));
        //std::smatch match_results_3;
        //if (!std::regex_match (URI_out,
        //                       match_results_3,
        //                       regex,
        //                       std::regex_constants::match_default))
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("invalid URI (was: \"%s\"), aborting\n"),
        //              ACE_TEXT (URI_out.c_str ())));
        //  return false;
        //} // end IF
        //ACE_ASSERT (match_results_3.ready () && !match_results_3.empty ());

        //if (!match_results_3[2].matched)
        //  URI_out += ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_NET_SOURCE_HTTP_GET_DEFAULT_URL);
        ////else if (!match_results_3[3].matched)
        ////  URI_out += ACE_TEXT_ALWAYS_CHAR (HTML_DEFAULT_SUFFIX);

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
        converter.str (argumentParser.opt_arg ());
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

#ifdef ENABLE_VALGRIND_SUPPORT
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif
#endif
}

bool
do_parseConfigurationFile (const std::string& fileName_in,
                           Test_I_StockItems_t& stockItems_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_parseConfigurationFile"));

  // initialize return value(s)
  stockItems_out.clear ();

  int result = -1;
  ACE_Configuration_Heap configuration_heap;
  result = configuration_heap.open ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Configuration_Heap::open() failed, aborting\n")));
    return false;
  } // end IF

  ACE_Ini_ImpExp ini_import_export (configuration_heap);
  result = ini_import_export.import_config (fileName_in.c_str ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Ini_ImpExp::import_config(\"%s\") failed, aborting\n"),
                ACE_TEXT (fileName_in.c_str ())));
    return false;
  } // end IF

  // step1: find/open "timestamp" section...
  ACE_Configuration_Section_Key section_key;
  result =
    configuration_heap.open_section (configuration_heap.root_section (),
                                     ACE_TEXT (TEST_I_CNF_SYMBOLS_SECTION_HEADER),
                                     0, // MUST exist !
                                     section_key);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(\"%s\"), aborting\n"),
                ACE_TEXT (TEST_I_CNF_SYMBOLS_SECTION_HEADER)));
    return false;
  } // end IF

  // import values...
  int index = 0;
  ACE_TString item_name, item_value;
  ACE_Configuration::VALUETYPE item_type;
  Test_I_StockItem stock_item;
  while (configuration_heap.enumerate_values (section_key,
                                              index,
                                              item_name,
                                              item_type) == 0)
  {
    result =
      configuration_heap.get_string_value (section_key,
                                           item_name.c_str (),
                                           item_value);
    if (result == -1)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"), aborting\n"),
                  item_name.c_str ()));
      return false;
    } // end IF

    stock_item.symbol = ACE_TEXT_ALWAYS_CHAR (item_name.c_str ());
    stock_item.ISIN =
        ((item_value.length () == TEST_I_ISIN_LENGTH) ? ACE_TEXT_ALWAYS_CHAR (item_value.c_str ())
                                                      : ACE_TEXT_ALWAYS_CHAR (""));
    stockItems_out.insert (stock_item);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added symbol: \"%s\"...\n"),
                ACE_TEXT (stock_item.symbol.c_str ())));

    ++index;
  } // end WHILE
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("loaded %u symbol(s)...\n"),
              stockItems_out.size ()));

  return true;
}

void
do_work (const std::string& bootstrapFileName_in,
         bool debug_in,
         const std::string& configurationFileName_in,
         const std::string& hostName_in,
         bool useThreadPool_in,
         const std::string& fileName_in,
         unsigned short port_in,
         bool useReactor_in,
         const ACE_INET_Addr& remoteHost_in,
         bool useSSL_in,
         const std::string& URL_in,
         unsigned int numberOfDispatchThreads_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_Source_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  bool finalize_event_dispatch = false;
  bool stop_timers = false;
  int group_id = -1;
  int result = -1;
  Test_I_StreamBase_t* stream_p = NULL;
  Test_I_StockItem stock_item;

  Stream_AllocatorHeap_T<Test_I_AllocatorConfiguration> heap_allocator;
  Test_I_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?

  Common_Timer_Manager_t* timer_manager_p = NULL;
  Common_TimerConfiguration timer_configuration;

  Test_I_Stream_InetConnectionManager_t* connection_manager_p =
    TEST_I_STREAM_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  Test_I_Configuration configuration;
  connection_manager_p->set (configuration,
                             &configuration.userData);
  Stream_StatisticHandler_Reactor_t statistic_handler (ACTION_REPORT,
                                                       connection_manager_p,
                                                       false);

  struct Common_DispatchThreadData thread_data;

  // step0a: initialize configuration and stream
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
    ACE_NEW_NORETURN (stream_p,
                      Test_I_HTTPGet_AsynchStream_t ());
  if (!stream_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  configuration.userData.configuration = &configuration;
  configuration.userData.streamConfiguration =
      &configuration.streamConfiguration;
  configuration.useReactor = useReactor_in;

  if (!do_parseConfigurationFile (configurationFileName_in,
                                  configuration.moduleHandlerConfiguration.stockItems))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to do_parseConfigurationFile(\"%s\"), returning\n"),
                ACE_TEXT (configurationFileName_in.c_str ())));
    goto error;
  } // end IF
  stock_item.ISIN = ACE_TEXT_ALWAYS_CHAR (TEST_I_ISIN_DAX);
  configuration.moduleHandlerConfiguration.stockItems.insert (stock_item);

  // *********************** socket configuration data ************************
  configuration.socketConfiguration.address = remoteHost_in;
  configuration.socketConfiguration.useLoopBackDevice =
    configuration.socketConfiguration.address.is_loopback ();
  //configuration.socketConfiguration.writeOnly = true;
  // ******************** socket handler configuration data *******************
  configuration.socketHandlerConfiguration.messageAllocator =
    &message_allocator;
  configuration.socketHandlerConfiguration.PDUSize = TEST_I_DEFAULT_BUFFER_SIZE;
  configuration.socketHandlerConfiguration.socketConfiguration =
    &configuration.socketConfiguration;
  configuration.socketHandlerConfiguration.userData =
    &configuration.userData;

  // ********************** stream configuration data **************************
  // ********************** module configuration data **************************
  configuration.moduleConfiguration.streamConfiguration =
    &configuration.streamConfiguration;

  configuration.moduleHandlerConfiguration.streamConfiguration =
    &configuration.streamConfiguration;

  configuration.moduleHandlerConfiguration.configuration = &configuration;
  configuration.moduleHandlerConfiguration.connectionManager =
    connection_manager_p;
  result =
    configuration.moduleHandlerConfiguration.libreOfficeHost.set (port_in,
                                                                  hostName_in.c_str (),
                                                                  1,
                                                                  ACE_ADDRESS_FAMILY_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));
    goto error;
  } // end IF
  configuration.moduleHandlerConfiguration.libreOfficeRc = bootstrapFileName_in;
  configuration.moduleHandlerConfiguration.traceParsing = debug_in;
  if (debug_in)
    configuration.moduleHandlerConfiguration.traceScanning = true;
  configuration.moduleHandlerConfiguration.targetFileName = fileName_in;
  //configuration.moduleHandlerConfiguration.hostName = hostName_in;

  //configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_ACCEPT_HEADER_STRING),
  //                                                                             ACE_TEXT_ALWAYS_CHAR ("text/html, application/xhtml+xml, */*")));
  //configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_ACCEPT_REFERER_STRING),
  //                                                                             ACE_TEXT_ALWAYS_CHAR ("http://kurse.boerse.ard.de/ard/kurse_einzelkurs_uebersicht.htn?i=118700&suchbegriff=US0079031078&exitPoint=")));
  //configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_LANGUAGE_HEADER_STRING),
  //                                                                             ACE_TEXT_ALWAYS_CHAR ("de-DE,de;q=0.8,en-US;q=0.5,en;q=0.3")));
  //configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_ENCODING_HEADER_STRING),
  //                                                                             ACE_TEXT_ALWAYS_CHAR ("gzip, deflate")));
  //configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_COOKIE_HEADER_STRING),
  //                                                                             ACE_TEXT_ALWAYS_CHAR ("xtvrn=$452061$; backlink=http://boerse.ard.de/index.html; usf_mobil=1; USF-C-usf_mobil=1")));
  configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                                                               ACE_TEXT_ALWAYS_CHAR ("kurse.boerse.ard.de")));
  //configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_AGENT_HEADER_STRING),
  //                                                                             ACE_TEXT_ALWAYS_CHAR ("Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko")));
  //configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_TRACKING_HEADER_STRING),
  //                                                                             ACE_TEXT_ALWAYS_CHAR ("1")));
  configuration.moduleHandlerConfiguration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONNECTION_STRING),
                                                                               ACE_TEXT_ALWAYS_CHAR ("Keep-Alive")));

  configuration.moduleHandlerConfiguration.URL = URL_in;
  configuration.moduleHandlerConfiguration.socketConfiguration =
    &configuration.socketConfiguration;
  configuration.moduleHandlerConfiguration.socketHandlerConfiguration =
    &configuration.socketHandlerConfiguration;
  configuration.moduleHandlerConfiguration.stream = stream_p;
  // ******************** (sub-)stream configuration data *********************
  configuration.streamConfiguration.messageAllocator = &message_allocator;
  //configuration.streamConfiguration.module = module_p;
  configuration.streamConfiguration.moduleConfiguration =
    &configuration.moduleConfiguration;
  configuration.streamConfiguration.moduleHandlerConfiguration =
    &configuration.moduleHandlerConfiguration;
  configuration.streamConfiguration.printFinalReport = true;

  //module_handler_p->initialize (configuration.moduleHandlerConfiguration);

  // step0b: initialize event dispatch
  thread_data.numberOfDispatchThreads = numberOfDispatchThreads_in;
  thread_data.useReactor = useReactor_in;
  if (!Common_Tools::initializeEventDispatch (useReactor_in,
                                              useThreadPool_in,
                                              numberOfDispatchThreads_in,
                                              thread_data.proactorType,
                                              thread_data.reactorType,
                                              configuration.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    goto error;
  } // end IF

  // step0c: (re-)configure connection manager
  connection_manager_p->set (configuration,
                             &configuration.userData);

  // step0d: initialize regular (global) statistic reporting
  timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();
  stop_timers = true;
  //Stream_StatisticHandler_Proactor_t statistics_handler_proactor (ACTION_REPORT,
  //                                                                connection_manager_p,
  //                                                                false);
  //long timer_id = -1;
  //if (statisticReportingInterval_in)
  //{
  //  ACE_Event_Handler* handler_p = &statistics_handler;
  //  ACE_Time_Value interval (statisticReportingInterval_in, 0);
  //  timer_id =
  //    timer_manager_p->schedule_timer (handler_p,                  // event handler
  //                                     NULL,                       // ACT
  //                                     COMMON_TIME_NOW + interval, // first wakeup time
  //                                     interval);                  // interval
  //  if (timer_id == -1)
  //  {
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));

  //    // clean up
  //    timer_manager_p->stop ();
  //    delete stream_p;

  //    return;
  //  } // end IF
  //} // end IF

  // step0c: initialize signal handling
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerID = timer_id;
  signalHandler_in.initialize (configuration.signalHandlerConfiguration);
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), aborting\n")));
    goto error;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
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

    goto error;
  } // end IF
  finalize_event_dispatch = true;

  if (!stream_p->initialize (configuration.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, aborting\n")));
    goto error;
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
  stream_p->waitForCompletion ();

  // step3: clean up
  connection_manager_p->stop ();
  Common_Tools::finalizeEventDispatch (useReactor_in,
                                       !useReactor_in,
                                       group_id);
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

  //result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  //if (result == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
  //              event_handler.name ()));
  delete stream_p;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

error:
  if (finalize_event_dispatch)
    Common_Tools::finalizeEventDispatch (useReactor_in,
                                         !useReactor_in,
                                         group_id);
  if (stop_timers)
    timer_manager_p->stop ();
  if (stream_p)
    delete stream_p;
}

void
do_printVersion (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printVersion"));

  std::ostringstream converter;

  // compiler version string...
  converter << ACE::compiler_major_version ();
  converter << ACE_TEXT (".");
  converter << ACE::compiler_minor_version ();
  converter << ACE_TEXT (".");
  converter << ACE::compiler_beta_version ();

  std::cout << programName_in
            << ACE_TEXT (" compiled on ")
            << ACE::compiler_name ()
            << ACE_TEXT (" ")
            << converter.str ()
            << std::endl << std::endl;

  std::cout << ACE_TEXT ("libraries: ")
            << std::endl
#ifdef HAVE_CONFIG_H
            << ACE_TEXT (LIBACESTREAM_PACKAGE)
            << ACE_TEXT (": ")
            << ACE_TEXT (LIBACESTREAM_PACKAGE_VERSION)
            << std::endl
#endif
            ;

  converter.str ("");
  // ACE version string...
  converter << ACE::major_version ();
  converter << ACE_TEXT (".");
  converter << ACE::minor_version ();
  converter << ACE_TEXT (".");
  converter << ACE::beta_version ();

  // *NOTE*: cannot use ACE_VERSION, as it doesn't contain the (potential) beta
  // version number... Need this, as the library soname is compared to this
  // string
  std::cout << ACE_TEXT ("ACE: ")
//             << ACE_VERSION
            << converter.str ()
            << std::endl;
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

  int result = EXIT_FAILURE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool finalize_ACE = false;
#endif
  bool finalize_logging = false;
  bool finalize_signals = false;
  ACE_Profile_Timer process_profile;

  std::string path, bootstrap_file, configuration_file, host_name, output_file;
  bool debug, log_to_file, use_thread_pool, use_reactor, trace_information;
  unsigned short port;
  std::string URL;
  bool print_version_and_exit;
  unsigned int number_of_dispatch_threads;
  ACE_INET_Addr remote_host;
  bool use_SSL;

  std::string log_file_name;

  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  Common_SignalActions_t previous_signal_actions;
  sigset_t previous_signal_mask;
  Stream_Source_SignalHandler signal_handler;

  ACE_High_Res_Timer timer;
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_Time_Value user_time, system_time, working_time;
  std::string user_time_string, system_time_string, working_time_string;

  // step0: initialize
  // *PORTABILITY*: on Windows, initialize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result_2 = ACE::init ();
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  finalize_ACE = true;
#endif
  Common_Tools::initialize ();

  // start profile timer
  process_profile.start ();

  path = Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  bootstrap_file = path;
#if defined (DEBUG_DEBUGGER)
  bootstrap_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrap_file += ACE_TEXT_ALWAYS_CHAR ("http_get_2");
#endif
  bootstrap_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrap_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  bootstrap_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrap_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE);
  debug = NET_PROTOCOL_DEFAULT_YACC_TRACE;
  configuration_file = path;
#if defined (DEBUG_DEBUGGER)
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file += ACE_TEXT_ALWAYS_CHAR ("http_get_2");
#endif
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_PORTFOLIO_CONFIGURATION_FILE);
  host_name.clear ();
  log_to_file = false;
  output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  use_thread_pool = NET_EVENT_USE_THREAD_POOL;
  port = TEST_I_DEFAULT_PORT;
  use_reactor = NET_EVENT_USE_REACTOR;
  trace_information = false;
  URL.clear ();
  print_version_and_exit = false;
  number_of_dispatch_threads =
    TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
  result = remote_host.set (static_cast<u_short> (HTTP_DEFAULT_SERVER_PORT),
                            static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                            1,
                            0);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  use_SSL = false;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            bootstrap_file,
                            debug,
                            configuration_file,
                            host_name,
                            log_to_file,
                            output_file,
                            port,
                            use_reactor,
                            trace_information,
                            URL,
                            print_version_and_exit,
                            number_of_dispatch_threads,
                            remote_host,
                            use_SSL))
  {
    do_printUsage (ACE::basename (argv_in[0]));
    goto error;
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
  if ((bootstrap_file.empty () ||
       !Common_File_Tools::isReadable (bootstrap_file))                     ||
      (configuration_file.empty () ||
       !Common_File_Tools::isReadable (configuration_file))                 ||
      output_file.empty ()                                                  ||
      host_name.empty ()                                                    ||
      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool) ||
      URL.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));
    do_printUsage (ACE::basename (argv_in[0]));
    goto error;
  } // end IF
  if (number_of_dispatch_threads == 0) number_of_dispatch_threads = 1;

  // step1d: initialize logging and/or tracing
  if (log_to_file)
    log_file_name =
      Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACESTREAM_PACKAGE_NAME),
                                         ACE::basename (argv_in[0]));
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]),           // program name
                                        log_file_name,                        // log file name
                                        false,                                // log to syslog ?
                                        false,                                // trace messages ?
                                        trace_information,                    // debug messages ?
                                        NULL))                                // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));
    goto error;
  } // end IF
  finalize_logging = true;

  // step1e: pre-initialize signal handling
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
                        signal_set,
                        ignored_signal_set);
  result = ACE_OS::sigemptyset (&previous_signal_mask);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sigemptyset(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (!Common_Tools::preInitializeSignals (signal_set,
                                           use_reactor,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::preInitializeSignals(), aborting\n")));
    goto error;
  } // end IF
  finalize_signals = true;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    result = EXIT_SUCCESS;

    goto continue_;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false, // file descriptors
                                        true,  // stack traces
                                        true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));
    goto error;
  } // end IF

  timer.start ();
  // step2: do actual work
  do_work (bootstrap_file,
           debug,
           configuration_file,
           host_name,
           use_thread_pool,
           output_file,
           port,
           use_reactor,
           remote_host,
           use_SSL,
           URL,
           number_of_dispatch_threads,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  timer.elapsed_time (working_time);
  Common_Tools::period2String (working_time,
                               working_time_string);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // stop profile timer
  process_profile.stop ();
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  result = process_profile.elapsed_time (elapsed_time);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  user_time.set (elapsed_rusage.ru_utime);
  system_time.set (elapsed_rusage.ru_stime);
  Common_Tools::period2String (user_time,
                               user_time_string);
  Common_Tools::period2String (system_time,
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

  result = EXIT_SUCCESS;

continue_:
error:
  if (finalize_signals)
    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
  if (finalize_logging)
    Common_Tools::finalizeLogging ();
  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (finalize_ACE)
  {
    int result_2 = ACE::fini ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
  } // end IF
#endif

  return result;
} // end main
