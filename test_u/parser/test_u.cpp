#include "stdafx.h"

#include <iostream>
#include <string>

#include "ace/config-lite.h"
#include "ace/ACE.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/Init_ACE.h"
#include "ace/OS.h"
#include "ace/Profile_Timer.h"
#include "ace/Synch.h"
#include "ace/Time_Value.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_log_tools.h"

#include "common_timer_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_misc_defines.h"

#include "parser_eventhandler.h"
#include "parser_module_eventhandler.h"
#include "parser_stream.h"

void
do_print_usage (const std::string& programName_in)
{
  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string path_root =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug scanner [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e          : debug parser [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string source_file_path = path_root;
  source_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  source_file_path += ACE_TEXT_ALWAYS_CHAR ("test_1.txt");
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [PATH]   : source (bencoded) file [")
            << source_file_path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
                      bool& debugScanner_out,
                      bool& debugParser_out,
                      std::string& sourceFilePath_out,
                      bool& logToFile_out,
                      bool& traceInformation_out)
{
  std::string path_root =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  debugScanner_out = false;
  debugParser_out = false;
  sourceFilePath_out = path_root;
  sourceFilePath_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  sourceFilePath_out += ACE_TEXT_ALWAYS_CHAR ("test_1.txt");
  logToFile_out = false;
  traceInformation_out = false;

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
                               ACE_TEXT ("def:lt"),
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
      case 'd':
      {
        debugScanner_out = true;
        break;
      }
      case 'e':
      {
        debugParser_out = true;
        break;
      }
      case 'f':
      {
        sourceFilePath_out =
          ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
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
do_work (int argc_in,
         ACE_TCHAR* argv_in[],
         bool debugScanner_in,
         bool debugParser_in,
         const std::string& sourceFilePath_in)
{
  // step1: load data into a message block
  ACE_UINT64 file_size_i = Common_File_Tools::size (sourceFilePath_in);
  uint8_t* data_p = NULL, *data_2 = NULL;
  Parser_Message* message_p = NULL, *message_2 = NULL;
  struct Common_FlexBisonParserConfiguration parser_configuration;
  struct Parser_ModuleHandlerConfiguration modulehandler_configuration;
  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;
  struct Stream_ModuleConfiguration module_configuration;
  struct Stream_Configuration stream_configuration;
  Parser_StreamConfiguration_t stream_configuration_2;
  Parser_Stream parser_stream;
  Parser_EventHandler event_handler (false);
  Parser_Module_EventHandler_Module module (&parser_stream,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Stream_MessageQueueBase_T<ACE_MT_SYNCH, Common_TimePolicy_t> message_queue (STREAM_QUEUE_MAX_SLOTS,
                                                                              NULL);
  std::string file_path_2 = ACE::dirname (sourceFilePath_in.c_str (), '\\');
  file_path_2 += ACE_DIRECTORY_SEPARATOR_STR;
  file_path_2 += ACE_TEXT ("test_2.txt");
  int result = -1;

  if (!Common_File_Tools::load (sourceFilePath_in,
                                data_p,
                                file_size_i,
                                COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to slurp file (was: \"\"), returning\n"),
                ACE_TEXT (sourceFilePath_in.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (data_p);

  ACE_NEW_NORETURN (message_p,
                    Parser_Message (0,                                                                                   // session id
                                    static_cast<unsigned int> (file_size_i + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE))); // size
  ACE_ASSERT (message_p);
  message_p->base (reinterpret_cast<char*> (data_p),
                   file_size_i + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                   ACE_Message_Block::DONT_DELETE);
  message_p->size (file_size_i);
  message_p->wr_ptr (file_size_i);

  if (!Common_File_Tools::load (file_path_2,
                                data_2,
                                file_size_i,
                                COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to slurp file (was: \"\"), returning\n"),
                ACE_TEXT (file_path_2.c_str ())));
    goto clean;
  } // end IF
  ACE_ASSERT (data_2);

  ACE_NEW_NORETURN (message_2,
                    Parser_Message (0,                                                                                   // session id
                                    static_cast<unsigned int> (file_size_i + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE))); // size
  ACE_ASSERT (message_2);
  message_2->base (reinterpret_cast<char*> (data_2),
                   file_size_i + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE,
                   ACE_Message_Block::DONT_DELETE);
  message_2->size (file_size_i);
  message_2->wr_ptr (file_size_i);

  // step2: initialize parser
  parser_configuration.block = true;
#if defined (_DEBUG)
  parser_configuration.debugParser = debugParser_in;
  parser_configuration.debugScanner = debugScanner_in;
#endif // _DEBUG
  parser_configuration.messageQueue = NULL;
  parser_configuration.useYYScanBuffer =
    COMMON_PARSER_DEFAULT_FLEX_USE_YY_SCAN_BUFFER;
  modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration.parserConfiguration = &parser_configuration;
  modulehandler_configuration.queue = &message_queue;
  modulehandler_configuration.subscriber = &event_handler;
  stream_configuration.module = &module;
  stream_configuration_2.initialize (module_configuration,
                                     modulehandler_configuration,
                                     stream_configuration);
  if (!parser_stream.initialize (stream_configuration_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, returning\n")));
    goto clean;
  } // end IF

  // step3: parse data
  parser_stream.start ();
  result = message_queue.enqueue (message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put: \"%m\", returning\n")));
    goto clean;
  } // end IF
  message_p = NULL;

  ACE_OS::sleep (ACE_Time_Value (1, 0));

  result = message_queue.enqueue (message_2, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put: \"%m\", returning\n")));
    goto clean;
  } // end IF
  message_2 = NULL;

  parser_stream.stop ();
  parser_stream.wait ();

clean:
  if (message_p)
  {
    message_p->release (); message_p = NULL;
  } // end IF
  if (data_p)
  {
    delete[] data_p; data_p = NULL;
  } // end IF
  if (message_2)
  {
    message_2->release (); message_2 = NULL;
  } // end IF
  if (data_2)
  {
    delete[] data_2; data_2 = NULL;
  } // end IF
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  int result = EXIT_FAILURE, result_2 = -1;

  // step0: initialize
  // *PORTABILITY*: on Windows, initialize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result_2 = ACE::init ();
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Profile_Timer process_profile;
  process_profile.start ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  ACE_High_Res_Timer timer;
  ACE_Time_Value working_time;
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_Time_Value user_time, system_time;

  // step1a set defaults
  bool debug_scanner = false;
  bool debug_parser = false;
  std::string path_root = Common_File_Tools::getWorkingDirectory ();
  std::string source_file_path = path_root;
  source_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  source_file_path += ACE_TEXT_ALWAYS_CHAR ("test_1.txt");
  bool log_to_file = false;
  std::string log_file_name;
  bool trace_information = false;

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
                             debug_scanner,
                             debug_parser,
                             source_file_path,
                             log_to_file,
                             trace_information))
  {
    do_print_usage (ACE::basename (argv_in[0]));
    goto clean;
  } // end IF

  if (!Common_File_Tools::isReadable (source_file_path))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument(s), aborting\n")));
    do_print_usage (ACE::basename (argv_in[0]));
    goto clean;
  } // end IF

  // step1c: initialize logging and/or tracing
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]), // program name
                                            log_file_name,              // log file name
                                            false,                      // log to syslog ?
                                            false,                      // trace messages ?
                                            trace_information,          // debug messages ?
                                            NULL))                      // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));
    goto clean;
  } // end IF

  timer.start ();
  // step2: do actual work
  do_work (argc_in,
           argv_in,
           debug_scanner,
           debug_parser,
           source_file_path);
  timer.stop ();

  // debug info
  timer.elapsed_time (working_time);
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"\n"),
              ACE_TEXT (Common_Timer_Tools::periodToString (working_time).c_str ())));

  // step3: shut down
  process_profile.stop ();

  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  result_2 = process_profile.elapsed_time (elapsed_time);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));
    goto clean;
  } // end IF
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  user_time.set (elapsed_rusage.ru_utime);
  system_time.set (elapsed_rusage.ru_stime);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (Common_Timer_Tools::periodToString (user_time).c_str ()),
              ACE_TEXT (Common_Timer_Tools::periodToString (system_time).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\nmaximum resident set size = %d\nintegral shared memory size = %d\nintegral unshared data size = %d\nintegral unshared stack size = %d\npage reclaims = %d\npage faults = %d\nswaps = %d\nblock input operations = %d\nblock output operations = %d\nmessages sent = %d\nmessages received = %d\nsignals received = %d\nvoluntary context switches = %d\ninvoluntary context switches = %d\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (Common_Timer_Tools::periodToString (user_time).c_str ()),
              ACE_TEXT (Common_Timer_Tools::periodToString (system_time).c_str ()),
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

  result = EXIT_SUCCESS;

clean:
  Common_Log_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}
