#include "stdafx.h"

#include <iostream>
#include <string>

#include "ace/ACE.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/Init_ACE.h"
#include "ace/OS.h"
#include "ace/Profile_Timer.h"
//#include "ace/Synch.h"
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

#include "test_u_defines.h"

#include "test_u_common_modules.h"
#include "test_u_eventhandler.h"
#include "test_u_stream.h"

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
  std::string filename_string = path_root;
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  filename_string +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_INPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f          : libreOffice template (.ods) filename [")
            << filename_string.c_str ()
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p          : export as PDF [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  filename_string = path_root;
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  filename_string +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : libreOffice .rc filename [")
            << filename_string.c_str ()
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
                      std::string& templateFileName_out,
                      std::string& rcFileName_out,
                      bool& logToFile_out,
                      bool& exportAsPDF_out,
                      bool& traceInformation_out)
{
  std::string path_root =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  templateFileName_out = path_root;
  templateFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  templateFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  templateFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  templateFileName_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_INPUT_FILE);
  rcFileName_out = path_root;
  rcFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  rcFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  rcFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  rcFileName_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_RC_FILE);
  logToFile_out = false;
  exportAsPDF_out = false;
  traceInformation_out = false;

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
                               ACE_TEXT ("f:lr:pt"),
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
      case 'f':
      {
        templateFileName_out =
            ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'p':
      {
        exportAsPDF_out = true;
        break;
      }
      case 'r':
      {
        rcFileName_out =
            ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
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
         const std::string& templateFileName_in,
         bool exportAsPDF_in,
         const std::string& rcFileName_in)
{
  int result = -1;
  Test_U_Message* message_p = NULL;
  //struct Common_ParserConfiguration parser_configuration;
  struct Test_U_Document_ModuleHandlerConfiguration modulehandler_configuration;
  struct Common_AllocatorConfiguration allocator_configuration;
  struct Stream_ModuleConfiguration module_configuration;
  struct Stream_Configuration stream_configuration;
  Test_U_StreamConfiguration_t stream_configuration_2;
  Test_U_EventHandler event_handler (false);
  Test_U_MessageHandler_Module module (NULL,
                                       ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Stream_MessageQueue_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Test_U_SessionMessage> message_queue (0,
                                                              NULL);
  Test_U_Stream stream;

  ACE_NEW_NORETURN (message_p,
                    Test_U_Message (1,                                         // session id
                                    STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE)); // size
  ACE_ASSERT (message_p);
  message_p->initialize (1,     // session id
                         NULL); // data block (NULL --> do not change)

  // step2: initialize stream
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Test_U_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?
  module_configuration.stream = &stream;
  modulehandler_configuration.concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  modulehandler_configuration.fileIdentifier.identifierDiscriminator =
    Common_File_Identifier::FILE;
  std::string filename_string = Common_File_Tools::getWorkingDirectory ();
  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
  filename_string += Common_File_Tools::basename (templateFileName_in);
  if (exportAsPDF_in)
  {
    filename_string = Common_File_Tools::cropExtension (filename_string);
    filename_string += '.';
    filename_string +=
      ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_PDF_FILENAME_EXTENSION);
  } // end IF
  modulehandler_configuration.fileIdentifier.identifier = filename_string;
  modulehandler_configuration.fileName = templateFileName_in;
  modulehandler_configuration.libreOfficeRc = rcFileName_in;
  modulehandler_configuration.messageAllocator = &message_allocator;
  modulehandler_configuration.queue = &message_queue;
  modulehandler_configuration.subscriber = &event_handler;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &module;
  stream_configuration_2.initialize (module_configuration,
                                     modulehandler_configuration,
                                     stream_configuration);
  if (!stream.initialize (stream_configuration_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, returning\n")));
    goto clean;
  } // end IF

  // step3: parse data
  stream.start ();
  result = message_queue.enqueue (message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put: \"%m\", returning\n")));
    goto clean;
  } // end IF
  message_p = NULL;

  ACE_OS::sleep (ACE_Time_Value (1, 0));

  stream.stop (true, true, false);
  stream.wait ();

clean:
  if (message_p)
  {
    message_p->release (); message_p = NULL;
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
  bool log_to_file = false;
  std::string log_file_name;
  bool export_as_pdf_b = false;
  bool trace_information = false;
  std::string template_filename;
  template_filename = Common_File_Tools::getWorkingDirectory ();
  template_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  template_filename +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  template_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  template_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_INPUT_FILE);
  std::string rc_filename = Common_File_Tools::getWorkingDirectory ();
  rc_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  rc_filename +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  rc_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  rc_filename +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_RC_FILE);

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
                             template_filename,
                             rc_filename,
                             log_to_file,
                             export_as_pdf_b,
                             trace_information))
  {
    do_print_usage (ACE::basename (argv_in[0]));
    goto clean;
  } // end IF

//  if (!Common_File_Tools::isReadable (source_file_path))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid argument(s), aborting\n")));
//    do_print_usage (ACE::basename (argv_in[0]));
//    goto clean;
//  } // end IF

  // step1c: initialize logging and/or tracing
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR)));
  if (!Common_Log_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR)), // program name
                                     log_file_name,              // log file name
                                     false,                      // log to syslog ?
                                     false,                      // trace messages ?
                                     trace_information,          // debug messages ?
                                     NULL))                      // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));
    goto clean;
  } // end IF

  timer.start ();
  // step2: do actual work
  do_work (argc_in,
           argv_in,
           template_filename,
           export_as_pdf_b,
           rc_filename);
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
  Common_Log_Tools::finalize ();

  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return result;
}
