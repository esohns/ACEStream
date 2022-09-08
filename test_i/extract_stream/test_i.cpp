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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

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

#include "common_tools.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_defines.h"
#include "common_ui_tools.h"
#if defined(GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#include "stream_lib_tools.h"

#include "stream_misc_defines.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_common_modules.h"
#include "test_i_eventhandler.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "test_i_signalhandler.h"
#include "test_i_stream.h"

#include "test_i_extract_stream_defines.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("Extract_Stream_Video_Stream");
const char stream_name_string_2[] = ACE_TEXT_ALWAYS_CHAR ("Extract_Stream_Audio_Stream");

void
print_usage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::print_usage"));

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-a          : extract audio [")
            << true
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b          : extract video [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : source filename")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
#endif // GUI_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
process_arguments (int argc_in,
                   ACE_TCHAR** argv_in, // cannot be const...
                   enum Test_I_ExtractStream_ProgramMode& mode_out,
                   std::string& sourceFileName_out,
#if defined (GUI_SUPPORT)
                   std::string& UIFile_out,
#endif // GUI_SUPPORT
                   bool& logToFile_out,
                   bool& traceInformation_out)
{
  STREAM_TRACE (ACE_TEXT ("::process_arguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  mode_out = TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY;
  sourceFileName_out.clear ();
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT
  logToFile_out = false;
  traceInformation_out = false;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("abf:g::ltv");
  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
                               ACE_TEXT (options_string.c_str ()),
                               1,                          // skip command name
                               1,                          // report parsing errors
                               ACE_Get_Opt::PERMUTE_ARGS,  // ordering
                               0);                         // for now, don't use long options
  int option = 0;
  std::stringstream converter;
  while ((option = argument_parser ()) != EOF)
  {
    switch (option)
    {
      case 'a':
      {
        mode_out = TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY;
        break;
      }
      case 'b':
      {
        mode_out = TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_VIDEO_ONLY;
        break;
      }
      case 'f':
      {
        sourceFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
#if defined (GUI_SUPPORT)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
#endif // GUI_SUPPORT
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
      case 'v':
      {
        mode_out = TEST_I_EXTRACTSTREAM_PROGRAMMODE_PRINT_VERSION;
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
                    argument_parser.last_option ()));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    argument_parser.long_option ()));
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
initialize_signals (ACE_Sig_Set& signals_out,
                    ACE_Sig_Set& ignoredSignals_out)
{
  STREAM_TRACE (ACE_TEXT ("::initialize_signals"));

  int result = -1;

  // initialize return value(s)
  result = signals_out.empty_set ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF
  result = ignoredSignals_out.empty_set ();
  if (unlikely (result == -1))
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
  signals_out.sig_add (SIGBREAK);          // 21      /* Ctrl-Break sequence */
  signals_out.sig_add (SIGABRT);           // 22      /* abnormal termination triggered by abort call */
  signals_out.sig_add (SIGABRT_COMPAT);    // 6       /* SIGABRT compatible with other platforms, same as SIGABRT */
#else
  result = signals_out.fill_set ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::fill_set(): \"%m\", returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (enum Test_I_ExtractStream_ProgramMode mode_in,
         const std::string& sourceFilename_in,
         const std::string& targetFilename_in,
         struct Test_I_ExtractStream_Configuration& configuration_in,
#if defined (GUI_SUPPORT)
         const std::string& UIDefinitionFilename_in,
         struct Test_I_ExtractStream_UI_CBData& CBData_in,
#endif // GUI_SUPPORT
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_I_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Test_I_GTK_Manager_t* gtk_manager_p =
    TEST_I_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  struct Test_I_GTK_State& state_r =
    const_cast<struct Test_I_GTK_State&> (gtk_manager_p->getR ());
  //CBData_in.UIState->gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
  //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
  state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (UIDefinitionFilename_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_Tools::initialize (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
#endif // ACE_WIN32 || ACE_WIN64

  // ********************** module configuration data **************************
  struct Stream_AllocatorConfiguration allocator_configuration; // video
  allocator_configuration.defaultBufferSize = 131072; // 128 kB
  struct Stream_AllocatorConfiguration allocator_configuration_2; // audio

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF

  struct Stream_ModuleConfiguration module_configuration;
  struct Test_I_ExtractStream_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_I_ExtractStream_ModuleHandlerConfiguration modulehandler_configuration_2; // wav encoder
  struct Test_I_ExtractStream_StreamConfiguration stream_configuration;

#if defined (GUI_SUPPORT)
  Test_I_EventHandler_t ui_event_handler (&CBData_in
#if defined (GTK_USE)
                                         );
#elif defined (WXWIDGETS_USE)
                                         ,iapplication_in);
#endif
#else
  Test_I_EventHandler_t ui_event_handler;
#endif // GUI_SUPPORT

//  Test_I_ExtractStream_StreamConfiguration_t::ITERATOR_T stream_iterator;
  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration.deviceType = AV_HWDEVICE_TYPE_DXVA2;
#else
  modulehandler_configuration.deviceType = AV_HWDEVICE_TYPE_VAAPI;
#endif // ACE_WIN32 || ACE_WIN64
  //modulehandler_configuration.display = displayDevice_in;
  modulehandler_configuration.subscriber = &ui_event_handler;
  //modulehandler_configuration.subscribers = &CBData_in.subscribers;
  modulehandler_configuration.outputFormat.audio.channels = 2;
  modulehandler_configuration.outputFormat.audio.format = AV_SAMPLE_FMT_S16;
  modulehandler_configuration.outputFormat.audio.sampleRate = 48000;
  modulehandler_configuration.outputFormat.video.format = AV_PIX_FMT_RGB32;
  modulehandler_configuration.outputFormat.video.frameRate.num = 30;
  modulehandler_configuration.outputFormat.video.resolution = {640, 480};
  modulehandler_configuration.targetFileName = targetFilename_in;
  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.manageSoX = false;
  CBData_in.progressData.audioFrameSize =
    av_get_bytes_per_sample (modulehandler_configuration.outputFormat.audio.format);

  Test_I_MessageHandler_Module message_handler (NULL,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_Stream stream;

  Test_I_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?
  modulehandler_configuration.messageAllocator = &message_allocator;

  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
      (!UIDefinitionFilename_in.empty () ? &message_handler
                                         : NULL);

  stream_configuration.allocatorConfiguration =
    &allocator_configuration;
  stream_configuration.mode = mode_in;

  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_2)));

//  stream_iterator =
//    configuration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (stream_iterator != configuration_in.streamConfiguration.end ());

  struct Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  int result = -1;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step0e: initialize signal handling
  //configuration_in.signalHandlerConfiguration.messageAllocator =
  //  &message_allocator;
  signalHandler_in.initialize (configuration_in.signalHandlerConfiguration);
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    return;
  } // end IF

  // intialize timers
  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

  // step0f: (initialize) processing stream

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

#if defined (GUI_SUPPORT)
  // step1a: start UI event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
    CBData_in.stream = &stream;
#if defined (GTK_USE)
    CBData_in.UIState = &state_r;
    CBData_in.progressData.state = &state_r;

    Common_UI_GTK_Tools::initialize (CBData_in.configuration->GTKConfiguration.argc,
                                     CBData_in.configuration->GTKConfiguration.argv);

    gtk_manager_p->initialize (configuration_in.GTKConfiguration);
#elif defined (WXWIDGETS_USE)
    struct Common_UI_wxWidgets_State& state_r =
      const_cast<struct Common_UI_wxWidgets_State&> (iapplication_in->getR ());
    state_r.resources[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<wxObject*> (NULL));
#endif // GTK_USE || WXWIDGETS_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      goto clean;
    } // end IF
    BOOL was_visible_b = false;
    //if (!UIDefinitionFilename_in.empty ())
      was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_USE)
    ACE_ASSERT (gtk_manager_p);
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
      goto clean;
    } // end IF
    gtk_manager_p->wait (false); // wait for message queue ?
#elif (WXWIDGETS_USE)
    if (unlikely (!iapplication_in->run ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_wxWidgets_IApplicationBase_T::run(), returning\n")));
      goto clean;
    } // end IF
#endif // GTK_USE || WXWIDGETS_USE
  } // end IF
  else
  {
#endif // GUI_SUPPORT
    Stream_IStreamControlBase* stream_p = NULL;

    if (!stream.initialize (configuration_in.streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));
      goto clean;
    } // end IF
    stream_p = &stream;
    ACE_ASSERT (stream_p);
    stream_p->start ();
//    if (!stream_p->isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start stream, aborting\n")));
//      //timer_manager_p->stop ();
//      return;
//    } // end IF
    stream_p->wait (true, false, false);
#if defined (GUI_SUPPORT)
  } // end ELSE
#endif // GUI_SUPPORT

  // step3: clean up
clean:
  timer_manager_p->stop ();

  result = stream.remove (&message_handler,
                          true,   // lock ?
                          false); // reset ?

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

COMMON_DEFINE_PRINTVERSION_FUNCTION (print_version,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  // initialize framework(s)
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
  if (!Common_UI_WxWidgets_Tools::initialize (argc_in,
                                              argv_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_WxWidgets_Tools::initialize(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (COM_initialized) Common_Tools::finalizeCOM ();
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

  // step1a set defaults
  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();
  enum Test_I_ExtractStream_ProgramMode program_mode_e =
    TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY;
  std::string source_filename;
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  std::string target_filename = path;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  std::string UI_definition_filename = path;
  UI_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT
  bool log_to_file = false;
  //struct Common_UI_DisplayDevice display_device_s =
  //  Common_UI_Tools::getDefaultDisplay ();
  bool trace_information = false;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  bool result_2 = false;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step1b: parse/process/validate configuration
  if (!process_arguments (argc_in,
                          argv_in, 
                          program_mode_e,
                          source_filename,
#if defined (GUI_SUPPORT)
                          UI_definition_filename,
#endif // GUI_SUPPORT
                          log_to_file,
                          //display_device_s,
                          trace_information))
  {
    print_usage (ACE::basename (argv_in[0]));
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
                ACE_TEXT ("limiting the number of message buffers could (!) lead to a deadlock --> ensure the streaming elements are sufficiently efficient in this regard\n")));
  if (
#if defined (GUI_SUPPORT)
      (!UI_definition_filename.empty () &&
       !Common_File_Tools::isReadable (UI_definition_filename))// ||
#endif // GUI_SUPPORT
     )
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    print_usage (ACE::basename (argv_in[0]));
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

  // step1d: initialize logging and/or tracing
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_MessageStack_t* logstack_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  logstack_p = &state_r.logStack;
  lock_p = &state_r.logStackLock;

  Common_Logger_t logger (logstack_p,
                          lock_p);
#endif // GTK_USE
#endif // GUI_SUPPORT
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                          ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),                   // program name
                                            log_file_name,                                // log file name
                                            false,                                        // log to syslog ?
                                            false,                                        // trace messages ?
                                            trace_information,                            // debug messages ?
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                            (UI_definition_filename.empty () ? NULL
                                                                             : &logger))) // (ui) logger ?
#elif defined (QT_USE)
                                            NULL))                                        // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                            NULL))                                        // (ui) logger ?
#else
                                            NULL))                                        // (ui) logger ?
#endif
#else
                                            NULL))                                        // (ui) logger ?
#endif // GUI_SUPPORT
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
    Common_Tools::finalize ();
#if defined(ACE_WIN32) || defined(ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1f: handle specific program modes
  switch (program_mode_e)
  {
    case TEST_I_EXTRACTSTREAM_PROGRAMMODE_PRINT_VERSION:
    {
      print_version (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

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
    }
    case TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY:
    {
      target_filename +=
        ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_AUDIO_FILE);
      break;
    }
    case TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_VIDEO_ONLY:
    case TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AV:
    {
      target_filename +=
        ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_AUDIO_VIDEO_FILE);
      break;
    }
    break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown program mode (was: %d), aborting\n"),
                  program_mode_e));

      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_FAILURE;
    }
  } // end SWITCH

#if defined (GUI_SUPPORT)
  struct Test_I_ExtractStream_UI_CBData* ui_cb_data_p = NULL;
  struct Test_I_ExtractStream_Configuration configuration;
  struct Test_I_ExtractStream_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
  ui_cb_data_p = &ui_cb_data;

  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  ACE_ASSERT (ui_cb_data_p);
#endif // GUI_SUPPORT

  // step1h: initialize UI framework
#if defined (GUI_SUPPORT)
  struct Common_UI_State* ui_state_p = NULL;
#if defined (GTK_USE)
  ui_state_p = &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  Common_UI_wxWidgets_IApplicationBase_t* iapplication_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      ACE_NEW_NORETURN (iapplication_p,
                        Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                                          argc_in,
                                                                          Common_UI_WxWidgets_Tools::convertArgV (argc_in, argv_in),
                                                                          COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
      Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
      iinitialize_p->initialize (directshow_ui_cb_data);
      Test_I_ExtractStream_DirectShow_WxWidgetsIApplication_t* iapplication_2 =
        dynamic_cast<Test_I_ExtractStream_DirectShow_WxWidgetsIApplication_t*> (iapplication_p);
      ACE_ASSERT (iapplication_2);
      Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t::STATE_T& state_r =
        const_cast<Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t::STATE_T&> (iapplication_2->getR ());
      Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
        const_cast<Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t::CONFIGURATION_T&> (iapplication_2->getR_2 ());
      configuration_r.UIState = &state_r;
      ACE_ASSERT (configuration_r.UIState);
      ui_state_p =
        const_cast<Test_I_ExtractStream_DirectShow_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ACE_NEW_NORETURN (iapplication_p,
                        Test_I_ExtractStream_MediaFoundation_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                                               argc_in,
                                                                               Common_UI_WxWidgets_Tools::convertArgV (argc_in,
                                                                                                                       argv_in),
                                                                               COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
      Test_I_ExtractStream_MediaFoundation_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_I_ExtractStream_MediaFoundation_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
      iinitialize_p->initialize (mediafoundation_ui_cb_data);
      Test_I_ExtractStream_MediaFoundation_WxWidgetsIApplication_t* iapplication_2 =
        dynamic_cast<Test_I_ExtractStream_MediaFoundation_WxWidgetsIApplication_t*> (iapplication_p);
      ACE_ASSERT (iapplication_2);
      const Test_I_ExtractStream_MediaFoundation_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
        iapplication_2->getR_2 ();
      ACE_ASSERT (configuration_r.UIState);
      ui_state_p =
        const_cast<Test_I_ExtractStream_MediaFoundation_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  media_framework_e));

      Common_Log_Tools::finalizeLogging ();
      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
      return EXIT_FAILURE;
    }
  } // end SWITCH
#else
//  // *WORKAROUND*: this prevents crashing the wxGTK3 application in Fedora 29
//  GtkCssProvider* css_provider_p = gtk_css_provider_new ();
//  if (!css_provider_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to gtk_css_provider_new(), returning\n")));

//    Common_Log_Tools::finalizeLogging ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // *PORTABILITY*: on Windows, finalize ACE...
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
//    return EXIT_FAILURE;
//  } // end IF
//  GError* error_p = NULL;
  std::string css_profile_path = Common_File_Tools::getWorkingDirectory ();
  css_profile_path += ACE_DIRECTORY_SEPARATOR_STR;
  css_profile_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  css_profile_path += ACE_DIRECTORY_SEPARATOR_STR;
  css_profile_path +=ACE_TEXT_ALWAYS_CHAR (TEST_U_Test_I_ExtractStream_UI_CSS_FILE);
//  if (!gtk_css_provider_load_from_path (css_provider_p,
//                                        css_profile_path.c_str (),
//                                        &error_p))
//  { ACE_ASSERT (error_p);
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gtk_css_provider_load_from_path(\"%s\"): \"%s\", returning\n"),
//                ACE_TEXT (css_profile_path.c_str ()),
//                ACE_TEXT (error_p->message)));
//    g_error_free (error_p); error_p = NULL;

//    Common_Log_Tools::finalizeLogging ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // *PORTABILITY*: on Windows, finalize ACE...
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
//    return EXIT_FAILURE;
//  } // end IF
//  GdkDisplay* display_p = gdk_display_open (ACE_TEXT_ALWAYS_CHAR (":0"));
//  ACE_ASSERT (display_p);
//  gdk_display_manager_set_default_display (gdk_display_manager_get (),
//                                           display_p);
//  GdkScreen* screen_p = gdk_screen_get_default ();
//  ACE_ASSERT (screen_p);
//  gtk_style_context_add_provider_for_screen (screen_p,
//                                             GTK_STYLE_PROVIDER (css_provider_p),
//                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  ACE_NEW_NORETURN (iapplication_p,
                    Test_I_ExtractStream_V4L_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                               argc_in,
                                                               Common_UI_WxWidgets_Tools::convertArgV (argc_in,
                                                                                                       argv_in),
                                                               COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
  Test_I_ExtractStream_V4L_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Test_I_ExtractStream_V4L_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
  // *NOTE*: this sets ui_cb_data.UIState
  iinitialize_p->initialize (ui_cb_data);
  Test_I_ExtractStream_V4L_WxWidgetsIApplication_t* iapplication_2 =
    dynamic_cast<Test_I_ExtractStream_V4L_WxWidgetsIApplication_t*> (iapplication_p);
  ACE_ASSERT (iapplication_2);
  const Test_I_ExtractStream_V4L_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
    iapplication_2->getR_2 ();
  ACE_ASSERT (configuration_r.UIState);
  ui_state_p =
    const_cast<Test_I_ExtractStream_V4L_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;
#endif // ACE_WIN32 || ACE_WIN64
  if (!iapplication_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: %m, aborting\n")));

    Common_Log_Tools::finalizeLogging ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (COM_initialized) Common_Tools::finalizeCOM ();
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
#endif
  ACE_ASSERT (ui_state_p);
#endif // GUI_SUPPORT

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  initialize_signals (signal_set,
                      ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                           false, // do not use networking
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
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
#endif // GUI_SUPPORT
  Test_I_SignalHandler signal_handler;

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false,  // file descriptors
                                        true,   // stack traces
                                        false)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
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

#if defined (GUI_SUPPORT)
  if (!UI_definition_filename.empty ())
  {
#if defined (GTK_USE)
    result_2 =
      gtk_manager_p->initialize (ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
#if defined (GTK_USE)
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), aborting\n")));

      Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
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
#endif // GTK_USE
  } // end IF
#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (program_mode_e,
           source_filename,
           target_filename,
           //display_device_s,
           configuration,
#if defined (GUI_SUPPORT)
           UI_definition_filename,
           ui_cb_data,
#if defined (WXWIDGETS_USE)
           iapplication_p,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
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

  Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
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
