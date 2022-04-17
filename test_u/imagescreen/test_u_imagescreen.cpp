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

#include "ace/config-lite.h"
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

#include "common_ui_tools.h"
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"

#include "test_u_imagescreen_gtk_callbacks.h"
#elif defined (WXWIDGETS_USE)
#include "common_ui_wxwidgets_application.h"
#include "common_ui_wxwidgets_tools.h"
#endif

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_vis_tools.h"

#include "test_u_common.h"
#include "test_u_defines.h"

#include "test_u_imagescreen_defines.h"
#include "test_u_imagescreen_eventhandler.h"
#if defined (GTK_USE)
#include "test_u_imagescreen_gtk_callbacks.h"
#elif defined (WXWIDGETS_USE)
#include "test_u_imagescreen_ui.h"
#endif
#include "test_u_imagescreen_signalhandler.h"
#include "test_u_imagescreen_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("ImageScreenStream");
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
const char toplevel_widget_classname_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME);
const char toplevel_widget_name_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME);
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

int
dirent_selector_cb (const dirent* dirEntry_in)
{
  // *IMPORTANT NOTE*: select all files

  std::string filename (ACE_TEXT_ALWAYS_CHAR (dirEntry_in->d_name));
  std::string::size_type position =
      filename.find_last_of ('.', std::string::npos);
  if ((position == 0) || ((position == 1) && filename[0] == '.')) // filter '.' and '..'
    return 0;
  filename.erase (0, position + 1);
  if (!ACE_OS::strncmp (filename.c_str (),
                        ACE_TEXT_ALWAYS_CHAR ("jpg"),
                        ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR ("jpg"))))
    return 1;
  else if (!ACE_OS::strncmp (filename.c_str (),
                             ACE_TEXT_ALWAYS_CHAR ("png"),
                             ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR ("png"))))
    return 1;

  return 0;
}

//////////////////////////////////////////

void
do_print_usage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_print_usage"));

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
  std::string image_file_path = path_root;
  image_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  image_file_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_DATA_SUBDIRECTORY);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [PATH]   : image path [")
            << image_file_path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f          : fullscreen [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (GUI_SUPPORT)
  std::string ui_definition_file_path = path_root;
  ui_definition_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  ui_definition_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file_path += ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g {[PATH]} : ui definition file [")
            << ui_definition_file_path
            << ACE_TEXT_ALWAYS_CHAR ("]")
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
}

bool
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
                      std::string& ImageFilePath_out,
                      bool& fullscreen_out,
                      bool& logToFile_out,
                      bool& traceInformation_out
#if defined (GUI_SUPPORT)
                      , std::string& UIDefinitionFilePath_out
#endif // GUI_SUPPORT
                      )
{
  STREAM_TRACE (ACE_TEXT ("::do_process_arguments"));

  std::string path_root =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  ImageFilePath_out = path_root;
  ImageFilePath_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ImageFilePath_out +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_DATA_SUBDIRECTORY);
  fullscreen_out = false;
  logToFile_out = false;
  traceInformation_out = false;
#if defined (GUI_SUPPORT)
  UIDefinitionFilePath_out = path_root;
  UIDefinitionFilePath_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitionFilePath_out +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UIDefinitionFilePath_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitionFilePath_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
#if defined (GUI_SUPPORT)
                               ACE_TEXT ("d:fg::lt"),
#else
                               ACE_TEXT ("d:flt"),
#endif // GUI_SUPPORT
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
        ImageFilePath_out =
          ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'f':
      {
        fullscreen_out = true;
        break;
      }
#if defined (GUI_SUPPORT)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UIDefinitionFilePath_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIDefinitionFilePath_out.clear ();
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
do_initialize_signals (ACE_Sig_Set& signals_out,
                       ACE_Sig_Set& ignoredSignals_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_signals"));

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
  signals_out.sig_add (SIGBREAK);        // 21      /* Ctrl-Break sequence */
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
#endif
}

void
do_work (int argc_in,
         ACE_TCHAR* argv_in[],
         const std::string& imageFilePath_in,
         bool fullscreen_in,
#if defined (GUI_SUPPORT)
         const std::string& UIDefinitionFilePath_in,
#endif // GUI_SUPPORT
         /////////////////////////////////
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_ImageScreen_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  struct Stream_ImageScreen_Configuration configuration;
  configuration.delayConfiguration.interval = ACE_Time_Value (5, 0);
  configuration.delayConfiguration.mode =
    STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //configuration.direct3DConfiguration.presentationParameters.BackBufferFormat =
  //  D3DFMT_X8R8G8B8;
#endif // ACE_WIN32 || ACE_WIN64
  struct Stream_ImageScreen_UI_CBData ui_cb_data;

  // initialize stream
  struct Stream_AllocatorConfiguration allocator_configuration;
  struct Stream_ModuleConfiguration module_configuration;
  struct Stream_ImageScreen_ModuleHandlerConfiguration modulehandler_configuration;
  struct Stream_ImageScreen_StreamConfiguration stream_configuration;

//  Stream_ImageScreen_StreamConfiguration_t::ITERATOR_T stream_configuration_iterator;
  modulehandler_configuration.allocatorConfiguration =
    &allocator_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration.codecId = AV_CODEC_ID_MJPEG;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  modulehandler_configuration.delayConfiguration =
    &configuration.delayConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration.display = Common_UI_Tools::getDefaultDisplay ();
#else
  modulehandler_configuration.display =
      Common_UI_Tools::getLogicalDisplay (ACE_TEXT_ALWAYS_CHAR (""));
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration.direct3DConfiguration = &configuration.direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  modulehandler_configuration.fileIdentifier.identifier = imageFilePath_in;
  modulehandler_configuration.fileIdentifier.identifierDiscriminator =
      Common_File_Identifier::DIRECTORY;
  modulehandler_configuration.fileIdentifier.selector =
      dirent_selector_cb;
  modulehandler_configuration.fullScreen = fullscreen_in;
  // X11 requires RGB32
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
  video_info_header_p->bmiHeader.biWidth = 640;
  video_info_header_p->bmiHeader.biHeight = 480;
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
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration.outputFormat.format = AV_PIX_FMT_RGBA;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  modulehandler_configuration.slurpFiles = true;

  Stream_ImageScreen_EventHandler_t ui_event_handler (
                                                      &ui_cb_data
#if defined (WXWIDGETS_USE)
                                                      ,iapplication_in
#endif
                                                     );
  modulehandler_configuration.subscriber = &ui_event_handler;

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF

  Stream_ImageScreen_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                           &heap_allocator,     // heap allocator handle
                                                           true);               // block ?
  Stream_ImageScreen_Stream stream;
  Stream_ImageScreen_MessageHandler_Module message_handler (&stream,
                                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  stream_configuration.messageAllocator = &message_allocator;
#if defined (GUI_SUPPORT)
  stream_configuration.module = &message_handler;
#endif // GUI_SUPPORT

  // X11 requires RGB32
//  configuration.streamConfiguration.configuration_.format.format =
//      AV_PIX_FMT_RGB32;
//  configuration.streamConfiguration.configuration_.renderer =
//      STREAM_VISUALIZATION_VIDEORENDERER_X11;

  configuration.streamConfiguration.initialize (module_configuration,
                                                modulehandler_configuration,
                                                stream_configuration);
//  configuration.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (Stream_Visualization_Tools::rendererToModuleName (renderer_in).c_str ()),
//                                                            std::make_pair (module_configuration,
//                                                                            modulehandler_configuration)));
//  stream_configuration_iterator =
//    configuration.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (stream_configuration_iterator != configuration.streamConfiguration.end ());

  // initialize UI
#if defined (GTK_USE)
  Common_UI_GTK_Configuration_t gtk_configuration;
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  gtk_configuration.argc = argc_in;
  gtk_configuration.argv = argv_in;
  gtk_configuration.CBData = &ui_cb_data;
  gtk_configuration.eventHooks.finiHook = idle_finalize_UI_cb;
  gtk_configuration.eventHooks.initHook = idle_initialize_UI_cb;
  gtk_configuration.definition = &gtk_ui_definition;

  ui_cb_data.UIState = &state_r;
  ui_cb_data.progressData.state = &state_r;

  state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (UIDefinitionFilePath_in, static_cast<GtkBuilder*> (NULL));

  bool result = gtk_manager_p->initialize (gtk_configuration);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), returning\n")));
    return;
  } // end IF
#endif // GTK_USE
  ui_cb_data.configuration = &configuration;
  ui_cb_data.stream = &stream;

  // initialize signal handling
  signalHandler_in.initialize (configuration.signalHandlerConfiguration);
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    return;
  } // end IF

  // initialize timer manager
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (configuration.timerConfiguration);
  timer_manager_p->start (NULL);

  // start UI
#if defined (GTK_USE)
  gtk_manager_p->start (NULL);
  ACE_Time_Value timeout (0,
                          COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
  int result_3 = ACE_OS::sleep (timeout);
  if (result_3 == -1)
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
  gtk_manager_p->wait (false);
#endif // GTK_USE

  timer_manager_p->stop ();
}

COMMON_DEFINE_PRINTVERSION_FUNCTION(do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

  int result = EXIT_FAILURE, result_2 = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool COM_initialized = false;
#endif // ACE_WIN32 || ACE_WIN64

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

  MagickWandGenesis ();
  Common_Tools::initialize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  COM_initialized = Common_Tools::initializeCOM ();
  Stream_Visualization_Tools::initialize (STREAM_VIS_FRAMEWORK_DEFAULT);
#endif // ACE_WIN32 || ACE_WIN64

  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  Stream_ImageScreen_SignalHandler signal_handler;

  ACE_High_Res_Timer timer;
  ACE_Time_Value working_time;
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_Time_Value user_time, system_time;

  // step1a set defaults
  std::string path_root = Common_File_Tools::getWorkingDirectory ();
  bool fullscreen_b = false;
  bool log_to_file = false;
  std::string log_file_name;
  bool trace_information = false;
#if defined (GUI_SUPPORT)
  std::string ui_definition_file_path = path_root;
  ui_definition_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  ui_definition_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file_path += ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT
  std::string image_file_path = path_root;
  image_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  image_file_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_DATA_SUBDIRECTORY);

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
                             image_file_path,
                             fullscreen_b,
                             log_to_file,
                             trace_information
#if defined (GUI_SUPPORT)
                             , ui_definition_file_path
#endif // GUI_SUPPORT
                             ))
  {
    do_print_usage (ACE::basename (argv_in[0]));
    goto clean;
  } // end IF

  if (
#if defined (GUI_SUPPORT)
      (!ui_definition_file_path.empty () && !Common_File_Tools::isReadable (ui_definition_file_path)) ||
#endif // GUI_SUPPORT
      (!Common_File_Tools::isDirectory (image_file_path) || !Common_File_Tools::isReadable (image_file_path)))
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

  // step1d: pre-initialize signal handling
  do_initialize_signals (signal_set,
                         ignored_signal_set);
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                           false, // do not use networking
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));
    goto clean;
  } // end IF

  timer.start ();
  // step2: do actual work
  do_work (argc_in,
           argv_in,
           image_file_path,
           fullscreen_b,
#if defined (GUI_SUPPORT)
           ui_definition_file_path,
#endif // GUI_SUPPORT
           ///////////////////////////////
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
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

  MagickWandTerminus ();

  result = EXIT_SUCCESS;

clean:
  Common_Signal_Tools::finalize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (COM_initialized) Common_Tools::finalizeCOM ();
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return result;
} // end main
