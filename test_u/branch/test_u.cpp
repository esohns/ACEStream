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

#include "test_u_defines.h"

#include "branch_session_message.h"
#include "branch_common_modules.h"
#include "branch_eventhandler.h"
#include "branch_module_eventhandler.h"
#include "branch_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("Branch_Stream");
const char stream_name_string_2[] = ACE_TEXT_ALWAYS_CHAR ("Branch_Stream_2");

enum Test_U_ModeType
{
  TEST_U_MODE_DEFAULT = 0,
  TEST_U_MODE_AGGREGATION,
  ////////////////////////////////////////
  TEST_U_MODE_MAX,
  TEST_U_MODE_INVALID
};

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m          : program mode [")
            << 0
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
                      bool& logToFile_out,
                      enum Test_U_ModeType& mode_out,
                      bool& traceInformation_out)
{
  std::string path_root =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  logToFile_out = false;
  traceInformation_out = false;

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
                               ACE_TEXT ("lm:t"),
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
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'm':
      {
        std::istringstream converter (ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ()),
                                      std::ios_base::in);
        int i = 0;
        converter >> i;
        mode_out = static_cast<enum Test_U_ModeType> (i);
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
         enum Test_U_ModeType mode_in)
{
  int result = -1;
  struct Branch_ModuleHandlerConfiguration modulehandler_configuration;
  struct Common_AllocatorConfiguration allocator_configuration;
  struct Stream_ModuleConfiguration module_configuration;
  struct Branch_StreamConfiguration stream_configuration;
  Branch_StreamConfiguration_t stream_configuration_2;

  // step2: initialize stream
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Branch_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?
  modulehandler_configuration.messageAllocator = &message_allocator;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration_2.initialize (module_configuration,
                                     modulehandler_configuration,
                                     stream_configuration);

  switch (mode_in)
  {
    case TEST_U_MODE_DEFAULT:
    {
      Branch_Message* message_p = NULL;
      modulehandler_configuration.concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;
      Stream_MessageQueueBase_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t> message_queue (STREAM_QUEUE_MAX_SLOTS,
                                                                    NULL);
      modulehandler_configuration.queue = &message_queue;
      Branch_EventHandler event_handler (false);
      modulehandler_configuration.subscriber = &event_handler;

      Branch_Stream branch_stream;
      module_configuration.stream = &branch_stream;
      Branch_Module_EventHandler_Module module (&branch_stream,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
      stream_configuration.module = &module;

      ACE_NEW_NORETURN (message_p,
                        Branch_Message (1,                                         // session id
                                        STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE)); // size
      ACE_ASSERT (message_p);
      message_p->initialize (1,     // session id
                             NULL); // data block [NULL --> do not change]

      if (!branch_stream.initialize (stream_configuration_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, returning\n")));
        goto clean;
      } // end IF

      // step3: parse data
      branch_stream.start ();
      result = message_queue.enqueue (message_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::put: \"%m\", returning\n")));
        goto clean;
      } // end IF
      message_p = NULL;

      ACE_OS::sleep (ACE_Time_Value (1, 0));

      branch_stream.stop ();
      branch_stream.wait ();

clean:
      if (message_p)
      {
        message_p->release (); message_p = NULL;
      } // end IF
      break;
    }
    case TEST_U_MODE_AGGREGATION:
    {
      Branch_Message* message_p = NULL, *message_2 = NULL;
      modulehandler_configuration.concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
      Stream_MessageQueueBase_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t> message_queue (STREAM_QUEUE_MAX_SLOTS,
                                                                    NULL);
      Stream_MessageQueueBase_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t> message_queue_2 (STREAM_QUEUE_MAX_SLOTS,
                                                                      NULL);
      modulehandler_configuration.queue = &message_queue;
      struct Branch_ModuleHandlerConfiguration modulehandler_configuration_2;
      Branch_EventHandler event_handler (false);
      modulehandler_configuration.subscriber = &event_handler;

      modulehandler_configuration_2.concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
      modulehandler_configuration_2.queue = &message_queue_2;
      modulehandler_configuration_2.subscriber = &event_handler;

      Branch_Stream_2 branch_stream, branch_stream_2;
      Branch_Module_EventHandler_Module module (&branch_stream,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
      stream_configuration.module = &module;
      Branch_Aggregator_Module module_2 (&branch_stream,
                                         ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_AGGREGATOR_DEFAULT_NAME_STRING));
      stream_configuration.module_2 = &module_2;

      Branch_StreamConfiguration_t stream_configuration_4;
      stream_configuration_4.initialize (module_configuration,
                                         modulehandler_configuration_2,
                                         stream_configuration);

      ACE_NEW_NORETURN (message_p,
                        Branch_Message (1,                                         // session id
                                        STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE)); // size
      ACE_ASSERT (message_p);
      message_p->initialize (1,     // session id
                             NULL); // data block [NULL --> do not change]
      message_2 = static_cast<Branch_Message*> (message_p->duplicate ());
      ACE_ASSERT (message_2);

      bool result_2 = false;

      if (!branch_stream.initialize (stream_configuration_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, returning\n")));
        goto clean_2;
      } // end IF
      branch_stream_2.name (stream_name_string_2);
      if (!branch_stream_2.initialize (stream_configuration_4))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, returning\n")));
        goto clean_2;
      } // end IF
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("initialization complete\n")));

      branch_stream.start ();
      branch_stream_2.start ();
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("streams started\n")));

      result = message_queue.enqueue (message_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::put: \"%m\", returning\n")));
        goto clean_2;
      } // end IF
      message_p = NULL;

      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("three...")));
      ACE_OS::sleep (ACE_Time_Value (1, 0));
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("two...")));
      ACE_OS::sleep (ACE_Time_Value (1, 0));
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("one...")));
      ACE_OS::sleep (ACE_Time_Value (1, 0));
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("STOP !")));

      branch_stream.stop (false,  // wait ?
                          false,  // recurse ?
                          false); // high priority ?
      branch_stream_2.stop (false,
                            false,
                            false);
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("streams stopped\n")));

      branch_stream.wait (true,   // wait for threads ?
                          false,  // wait for upstream ?
                          false); // wait for downstream ?
      branch_stream_2.wait (true,
                            false,
                            false);
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("streams complete\n")));

      ////////////////////////////////////

      // remove aggregator
      result_2 = branch_stream.remove (&module_2,
                                       true,  // lock ?
                                       true); // reset ?
      ACE_ASSERT (result_2);
      result_2 = branch_stream_2.remove (&module_2,
                                         true,  // lock ?
                                         true); // reset ?
      ACE_ASSERT (result_2);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("removed aggregator module\n")));

      if (!branch_stream.initialize (stream_configuration_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, returning\n")));
        goto clean_2;
      } // end IF
      if (!branch_stream_2.initialize (stream_configuration_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, returning\n")));
        goto clean_2;
      } // end IF
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("initialization (2) complete\n")));

      branch_stream.start ();
      branch_stream_2.start ();
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("streams (2) started\n")));

      message_2->initialize (4,     // session id
                             NULL); // data block [NULL --> do not change]
      result = message_queue.enqueue (message_2, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::put: \"%m\", returning\n")));
        goto clean_2;
      } // end IF
      message_2 = NULL;

      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("three...")));
      ACE_OS::sleep (ACE_Time_Value (1, 0));
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("two...")));
      ACE_OS::sleep (ACE_Time_Value (1, 0));
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("one...")));
      ACE_OS::sleep (ACE_Time_Value (1, 0));
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("STOP !")));

      branch_stream.stop (false,  // wait ?
                          false,  // recurse ?
                          false); // high priority ?
      branch_stream_2.stop (false,
                            false,
                            false);
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("streams (2) stopped\n")));

      branch_stream.wait (true,   // wait for threads ?
                          false,  // wait for upstream ?
                          false); // wait for downstream ?
      branch_stream_2.wait (true,
                            false,
                            false);
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("streams (2) complete\n")));

      // remove aggregator
      result_2 = branch_stream.remove (&module_2,
                                       true,  // lock ?
                                       true); // reset ?
      ACE_ASSERT (result_2);
      result_2 = branch_stream_2.remove (&module_2,
                                         true,  // lock ?
                                         true); // reset ?
      ACE_ASSERT (result_2);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("removed aggregator module (2)\n")));

      result_2 = branch_stream.remove (&module,
                                       true,  // lock ?
                                       true); // reset ?
      ACE_ASSERT (result_2);
      result_2 = branch_stream_2.remove (&module,
                                         true,  // lock ?
                                         true); // reset ?
      ACE_ASSERT (result_2);
      ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("removed message handler module\n")));

clean_2:
      if (message_p)
      {
        message_p->release (); message_p = NULL;
      } // end IF
      if (message_2)
      {
        message_2->release (); message_2 = NULL;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown program mode (was: %d), returning\n"),
                  mode_in));
      return;    
    }
  } // end SWITCH
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
  enum Test_U_ModeType program_mode_e = TEST_U_MODE_INVALID;
  bool trace_information = false;

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
                             log_to_file,
                             program_mode_e,
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
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]), // program name
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
           program_mode_e);
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
