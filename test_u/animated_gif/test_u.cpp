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

#if defined(ACE_WIN32) || defined(ACE_WIN64)
#include "amvideo.h"
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

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

#include "common_log_common.h"
#include "common_log_tools.h"
#if defined (GUI_SUPPORT)
#include "common_logger_queue.h"
#endif // GUI_SUPPORT

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "test_u_common.h"
#include "test_u_defines.h"

#include "test_u_animated_gif_defines.h"
#include "test_u_common_modules.h"
#include "test_u_eventhandler.h"
#include "test_u_signalhandler.h"
#include "test_u_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("AnimatedGIFStream");

int
dirent_selector_cb (const ACE_DIRENT* dirEntry_in)
{
  // *IMPORTANT NOTE*: select all bitmap files (*.bmp)

  std::string filename (ACE_TEXT_ALWAYS_CHAR (dirEntry_in->d_name));
  std::string::size_type position =
      filename.find_last_of ('.', std::string::npos);
  if ((position == 0) || ((position == 1) && filename[0] == '.')) // filter '.' and '..'
    return 0;
  filename.erase (0, position + 1);
  if (!ACE_OS::strncmp (filename.c_str (),
                        ACE_TEXT_ALWAYS_CHAR ("bmp"),
                        ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR ("bmp"))))
    return 1;
  //else if (!ACE_OS::strncmp (filename.c_str (),
  //                           ACE_TEXT_ALWAYS_CHAR ("png"),
  //                           ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR ("png"))))
  //  return 1;

  return 0;
}

int
dirent_comparator_cb (const ACE_DIRENT** f1_in,
                      const ACE_DIRENT** f2_in)
{
  // sanity check(s)
  ACE_ASSERT (f1_in && f2_in);
  ACE_ASSERT (*f1_in && *f2_in);

  std::string filename1 (ACE_TEXT_ALWAYS_CHAR ((*f1_in)->d_name));
  std::string filename2 (ACE_TEXT_ALWAYS_CHAR ((*f2_in)->d_name));
  std::string::size_type position1 =
    filename1.find_last_of ('.', std::string::npos);
  std::string::size_type position2 =
    filename2.find_last_of ('.', std::string::npos);
  filename1.erase (position1, std::string::npos);
  filename2.erase (position2, std::string::npos);
  position1 = filename1.find_last_of ('_', std::string::npos);
  position2 = filename2.find_last_of ('_', std::string::npos);
  filename1.erase (0, position1 + 1);
  filename2.erase (0, position2 + 1);
  int number1, number2;
  std::stringstream converter;
  converter << filename1;
  converter >> number1;
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << filename2;
  converter >> number2;

  return number1 > number2 ? 1 : number1 < number2 ? -1 : 0;
  // return ACE_OS::strcmp ((*f1_in)->d_name, (*f2_in)->d_name);
}

//////////////////////////////////////////

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]    : buffer size (byte(s)) [")
            << TEST_U_ANIMATED_GIF_DEFAULT_BUFFER_SIZE
            << ACE_TEXT ("])")
            << std::endl;
  std::string path = Common_File_Tools::getTempDirectory ();
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [DIRECTORY]: source file(s) directory [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l            : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_ANIMATED_GIF_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o [STRING]   : target file [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]    : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t            : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v            : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
                     std::string& imagesFilePath_out,
                     bool& logToFile_out,
                     std::string& targetFileName_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  std::string path = configuration_path;
  bufferSize_out = TEST_U_ANIMATED_GIF_DEFAULT_BUFFER_SIZE;
  path = Common_File_Tools::getTempDirectory ();
  imagesFilePath_out = path;
  logToFile_out = false;
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_ANIMATED_GIF_DEFAULT_OUTPUT_FILE);
  targetFileName_out = path;
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  printVersionAndExit_out = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("b:f:lo:s:tv"),
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
      case 'f':
      {
        imagesFilePath_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'o':
      {
        targetFileName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
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
      case 'v':
      {
        printVersionAndExit_out = true;
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
#endif // ENABLE_VALGRIND_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (unsigned int bufferSize_in,
         const std::string& imagesFilePath_in,
         const std::string& targetFileName_in,
         unsigned int statisticReportingInterval_in,
         struct Test_U_AnimatedGIF_UI_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  // sanity check(s)
  ACE_ASSERT (CBData_in.configuration);

  // step0a: initialize configuration
  Test_U_EventHandler ui_event_handler (&CBData_in);
  Test_U_Stream stream;
  struct Stream_ModuleConfiguration module_configuration;
  struct Stream_AllocatorConfiguration allocator_configuration;
  Test_U_MessageHandler_Module message_handler (&stream,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

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
  // ********************** module configuration data **************************
  struct Test_U_AnimatedGIF_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_AnimatedGIF_ModuleHandlerConfiguration modulehandler_configuration_2; // file writer
  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
  modulehandler_configuration.printProgressDot = true;
  modulehandler_configuration.fileIdentifier.comparator = dirent_comparator_cb;
  modulehandler_configuration.fileIdentifier.identifier = imagesFilePath_in;
  modulehandler_configuration.fileIdentifier.identifierDiscriminator =
    Common_File_Identifier::DIRECTORY;
  modulehandler_configuration.fileIdentifier.selector = dirent_selector_cb;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration.outputFormat.majortype = MEDIATYPE_Video;
  modulehandler_configuration.outputFormat.subtype = MEDIASUBTYPE_RGB32;
  modulehandler_configuration.outputFormat.bFixedSizeSamples = TRUE;
  modulehandler_configuration.outputFormat.bTemporalCompression = FALSE;
  modulehandler_configuration.outputFormat.formattype = FORMAT_VideoInfo;
  modulehandler_configuration.outputFormat.cbFormat =
    sizeof (struct tagVIDEOINFOHEADER);
  modulehandler_configuration.outputFormat.pbFormat =
    reinterpret_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tagVIDEOINFOHEADER)));
  if (unlikely (!modulehandler_configuration.outputFormat.pbFormat))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  ACE_OS::memset (modulehandler_configuration.outputFormat.pbFormat, 0, sizeof (struct tagVIDEOINFOHEADER));
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (modulehandler_configuration.outputFormat.pbFormat);
  // *NOTE*: empty --> use entire video
  BOOL result_2 = SetRectEmpty (&video_info_header_p->rcSource);
  ACE_ASSERT (result_2);
  result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
  // *NOTE*: empty --> fill entire buffer
  ACE_ASSERT (result_2);
  //video_info_header_p->dwBitRate = ;
  video_info_header_p->dwBitErrorRate = 0;
  video_info_header_p->AvgTimePerFrame = 1;
  video_info_header_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_header_p->bmiHeader.biWidth = 64;
  video_info_header_p->bmiHeader.biHeight = 64;
  //if (video_info_header_p->bmiHeader.biHeight > 0)
  //  video_info_header_p->bmiHeader.biHeight =
  //    -video_info_header_p->bmiHeader.biHeight;
  //ACE_ASSERT (video_info_header_p->bmiHeader.biHeight < 0);
  video_info_header_p->bmiHeader.biPlanes = 1;
  video_info_header_p->bmiHeader.biBitCount =
    Stream_MediaFramework_Tools::toBitCount (modulehandler_configuration.outputFormat.subtype);
  //ACE_ASSERT (video_info_header_p->bmiHeader.biBitCount);
  video_info_header_p->bmiHeader.biCompression = BI_RGB;
  video_info_header_p->bmiHeader.biSizeImage =
    DIBSIZE (video_info_header_p->bmiHeader);
  ////video_info_header_p->bmiHeader.biXPelsPerMeter;
  ////video_info_header_p->bmiHeader.biYPelsPerMeter;
  ////video_info_header_p->bmiHeader.biClrUsed;
  ////video_info_header_p->bmiHeader.biClrImportant;
  modulehandler_configuration.outputFormat.lSampleSize =
    video_info_header_p->bmiHeader.biSizeImage;
#else
  modulehandler_configuration.outputFormat.format.pixelformat =
    V4L2_PIX_FMT_BGRA32;
#endif // ACE_WIN32 || ACE_WIN64
  modulehandler_configuration.slurpFiles = true;
  modulehandler_configuration.statisticReportingInterval =
      ACE_Time_Value (statisticReportingInterval_in, 0);
  modulehandler_configuration.subscriber = &ui_event_handler;

  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.fileIdentifier.identifier = targetFileName_in;

  // ********************** stream configuration data **************************
  if (bufferSize_in)
    allocator_configuration.defaultBufferSize = bufferSize_in;

  struct Stream_Configuration stream_configuration;
  stream_configuration.allocatorConfiguration = &allocator_configuration;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &message_handler;
  stream_configuration.printFinalReport = true;
  CBData_in.configuration->streamConfiguration.initialize (module_configuration,
                                                           modulehandler_configuration,
                                                           stream_configuration);
  CBData_in.configuration->streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (TEST_U_ANIMATED_GIF_DEFAULT_WRITER_MODULE_NAME_STRING),
                                                                       std::make_pair (&module_configuration,
                                                                                       &modulehandler_configuration_2)));

  // step0e: initialize signal handling
  signalHandler_in.initialize (CBData_in.configuration->signalHandlerConfiguration);
  //if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DISPATCH_SIGNAL,
  //                                      signalSet_in,
  //                                      ignoredSignalSet_in,
  //                                      &signalHandler_in,
  //                                      previousSignalActions_inout))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Common_Signal_Tools::initialize(), aborting\n")));
  //  return;
  //} // end IF

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  CBData_in.stream = &stream;
  //CBData_in.userData = &CBData_in;

  if (!stream.initialize (CBData_in.configuration->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, aborting\n")));
    return;
  } // end IF

  stream.start ();
//    if (!stream.isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start stream, aborting\n")));
//      //timer_manager_p->stop ();
//      return;
//    } // end IF
  stream.wait (true,
               false,
               false);

  // step3: clean up

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
  WORD wVersionRequested = MAKEWORD (2, 2);
  WSADATA wsaData;
  result = WSAStartup (wVersionRequested, &wsaData);
  ACE_ASSERT (result == 0);

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

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  unsigned int buffer_size = TEST_U_ANIMATED_GIF_DEFAULT_BUFFER_SIZE;
  std::string path = Common_File_Tools::getTempDirectory ();
  std::string images_file_path_string = path;
  bool log_to_file = false;
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_ANIMATED_GIF_DEFAULT_OUTPUT_FILE);
  std::string target_file_name_string = path;
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool print_version_and_exit = false;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
                            images_file_path_string,
                            log_to_file,
                            target_file_name_string,
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit))
  {
    do_printUsage (ACE::basename (argv_in[0]));
//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_U_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if (!Common_File_Tools::isValidPath (images_file_path_string))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));
//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  //if (run_stress_test)
  //  action_mode = Net_Client_TimeoutHandler::ACTION_STRESS;

  struct Test_U_AnimatedGIF_Configuration configuration;
  struct Test_U_AnimatedGIF_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;

  // step1d: initialize logging and/or tracing
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                          std::string (ACE::basename (argv_in[0])));
  if (!Common_Log_Tools::initialize (std::string (ACE::basename (argv_in[0])), // program name
                                     log_file_name,                            // log file name
                                     false,                                    // log to syslog ?
                                     false,                                    // trace messages ?
                                     trace_information,                        // debug messages ?
                                     NULL))                                    // (ui-) logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
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
                                           COMMON_SIGNAL_DISPATCH_SIGNAL,
                                           false, // using networking ?
                                           false, // using asynch timers ?
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalize ();
//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  Test_U_SignalHandler signal_handler;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_OS_Tools::setResourceLimits (false,  // file descriptors
                                           true,   // stack traces
                                           false)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
           images_file_path_string,
           target_file_name_string,
           statistic_reporting_interval,
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

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
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

  Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
