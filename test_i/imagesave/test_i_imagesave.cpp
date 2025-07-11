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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define INITGUID // *NOTE*: this exports DEFINE_GUIDs (see stream_misc_common.h)
#include "mfapi.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "libavutil/pixfmt.h"
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT

#if defined (GTK_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#else
#include "gdk/gdk.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

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

#include "common_logger_queue.h"
#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#include "common_ui_defines.h"
#include "common_ui_tools.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "common_ui_wxwidgets_application.h"
#include "common_ui_wxwidgets_tools.h"
#endif // WXWIDGETS_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#include "stream_lib_tools.h"

#include "stream_misc_defines.h"

#include "stream_vis_tools.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_imagesave_defines.h"
#include "test_i_imagesave_eventhandler.h"
#if defined (GTK_SUPPORT)
#include "test_i_imagesave_gtk_callbacks.h"
#endif // GTK_SUPPORT
//#if defined (WXWIDGETS_SUPPORT)
//#include "test_i_imagesave_ui.h"
//#endif // WXWIDGETS_SUPPORT
#include "test_i_imagesave_signalhandler.h"
#include "test_i_imagesave_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("ImageSaveStream");
#if defined (WXWIDGETS_SUPPORT)
const char toplevel_widget_classname_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME);
const char toplevel_widget_name_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME);
#endif // WXWIDGETS_SUPPORT

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : show console [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string path = Common_File_Tools::getTempDirectory ();
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : source file [")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o[[STRING]]: target filename [")
            << path
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
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#endif // ACE_WIN32 || ACE_WIN64
                     bool& debug_out,
                     std::string& sourceFileName_out,
                     std::string& UIFile_out,
                     bool& logToFile_out,
                     std::string& targetFileName_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#endif // ACE_WIN32 || ACE_WIN64
  debug_out = false;
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  sourceFileName_out = path;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
  logToFile_out = false;
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  targetFileName_out = path;
  traceInformation_out = false;
  printVersionAndExit_out = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("cd:f:g::hlo::tvx"),
#else
                              ACE_TEXT ("d:f:g::hlo::tvx"),
#endif // ACE_WIN32 || ACE_WIN64
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'c':
      {
        showConsole_out = true;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      case 'd':
      {
        debug_out = true;
        break;
      }
      case 'f':
      { ACE_ASSERT (argumentParser.opt_arg ());
        sourceFileName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'o':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          targetFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          targetFileName_out.clear ();
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
                    argumentParser.last_option ()));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    argumentParser.long_option ()));
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
  // ---------------------------------------------------------------------------
  if (!allowUserRuntimeConnect_in)
  {
    signals_out.sig_del (SIGUSR1);         // 10      /* User-defined signal 1 */
    ignoredSignals_out.sig_add (SIGUSR1);  // 10      /* User-defined signal 1 */
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#endif // ACE_WIN32 || ACE_WIN64
         bool debug_in,
         const std::string& sourceFilename_in,
         const std::string& targetFilename_in,
         struct Test_I_ImageSave_Configuration& configuration_in,
         const std::string& UIDefinitionFilename_in,
         struct Test_I_ImageSave_UI_CBData& CBData_in,
#if defined (WXWIDGETS_USE)
         Common_UI_wxWidgets_IApplicationBase_t* iapplication_in,
#endif // WXWIDGETS_USE
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_I_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  //CBData_in.UIState->gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
  //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
  state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (UIDefinitionFilename_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE

  // ********************** module configuration data **************************
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration allocator_configuration;
  allocator_configuration.defaultBufferSize = 524288;
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration codec_configuration;
  codec_configuration.codecId = AV_CODEC_ID_H264;
  codec_configuration.parserFlags = PARSER_FLAG_ONCE | PARSER_FLAG_USE_CODEC_TS;
  //codec_configuration.useParser = false;
#else
  struct Stream_AllocatorConfiguration allocator_configuration;
#endif // FFMPEG_SUPPORT

  struct Stream_ModuleConfiguration module_configuration;
  struct Test_I_ImageSave_ModuleHandlerConfiguration modulehandler_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_ImageSave_ModuleHandlerConfiguration modulehandler_configuration_2; // display
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_I_ImageSave_StreamConfiguration stream_configuration;
  Test_I_EventHandler_t ui_event_handler (&CBData_in
#if defined (GTK_USE)
                                          );
#elif defined (QT_USE)
                                          );
#elif defined (WXWIDGETS_USE)
                                          ,iapplication_in);
#else
                                          );
#endif

  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
#if defined (_DEBUG)
  modulehandler_configuration.debug = debug_in;
#endif // _DEBUG
  modulehandler_configuration.concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration.codecConfiguration = &codec_configuration;
#endif // FFMPEG_SUPPORT
//#if defined (_DEBUG)
//  modulehandler_configuration.debug = true;
//#endif // _DEBUG
  modulehandler_configuration.defragmentMode = STREAM_DEFRAGMENT_CLONE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration.direct3DConfiguration = &configuration_in.direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  modulehandler_configuration.fileIdentifier.identifier = sourceFilename_in;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  modulehandler_configuration.lock = &state_r.subscribersLock;
#endif // GTK_USE || WXWIDGETS_USE
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.subscribers = &CBData_in.subscribers;
  modulehandler_configuration.targetFileName = targetFilename_in;

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
#if defined (FFMPEG_SUPPORT)
                         struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration> heap_allocator;
#else
                         struct Stream_AllocatorConfiguration> heap_allocator;
#endif // FFMPEG_SUPPORT
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF

  Test_I_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?
  Test_I_Stream stream;
  Test_I_MessageHandler_Module message_handler (&stream,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
    (!UIDefinitionFilename_in.empty () ? &message_handler
                                       : NULL);
  stream_configuration.saveOnly = UIDefinitionFilename_in.empty ();

  stream_configuration.allocatorConfiguration = &allocator_configuration;

  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration_2 = modulehandler_configuration;
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_2)));
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_handle = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  //if ((*iterator).second.second->window)
  //{
  //  ACE_ASSERT (gdk_win32_window_is_win32 ((*iterator).second.second->window));
  //  window_handle =
  //    //gdk_win32_window_get_impl_hwnd (configuration.moduleHandlerConfiguration.window);
  //    //gdk_win32_drawable_get_handle (GDK_DRAWABLE (configuration.moduleHandlerConfiguration.window));
  //    static_cast<HWND> (GDK_WINDOW_HWND ((*iterator).second.second->window));
  //} // end IF
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration.outputFormat.video.frameRate.num =
    STREAM_DEV_CAM_DEFAULT_CAPTURE_FRAME_RATE;
#if defined (GTK_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration.outputFormat.video.format = AV_PIX_FMT_RGB24;
#else
  modulehandler_configuration.outputFormat.video.format = AV_PIX_FMT_BGRA;
#endif // ACE_WIN32 || ACE_WIN64
#else
  modulehandler_configuration.outputFormat.video.format = AV_PIX_FMT_BGR32;
#endif // GTK_USE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration.outputFormat.video.resolution.cx = 1920;
  modulehandler_configuration.outputFormat.video.resolution.cy = 1080;
#else
  modulehandler_configuration.outputFormat.video.resolution.width = 1920;
  modulehandler_configuration.outputFormat.video.resolution.height = 1080;
#endif // ACE_WIN32 || ACE_WIN64
#endif // FFMPEG_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration_2.outputFormat =
    modulehandler_configuration.outputFormat;
#endif // ACE_WIN32 || ACE_WIN64
  configuration_in.streamConfiguration.configuration_->format =
    modulehandler_configuration.outputFormat;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // struct _D3DDISPLAYMODE display_mode_s =
  //   Stream_MediaFramework_DirectDraw_Tools::getDisplayMode(directShowConfiguration_in.direct3DConfiguration.adapter,
  //                                                           STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT,
  //                                                           resolution_s);
  ACE_ASSERT (!configuration_in.direct3DConfiguration.presentationParameters.hDeviceWindow);
  //directShowConfiguration_in.direct3DConfiguration.focusWindow =
  //    GetConsoleWindow ();
  configuration_in.direct3DConfiguration.presentationParameters.BackBufferWidth =
    modulehandler_configuration.outputFormat.video.resolution.cx;
  configuration_in.direct3DConfiguration.presentationParameters.BackBufferHeight =
    modulehandler_configuration.outputFormat.video.resolution.cy;
  configuration_in.direct3DConfiguration.presentationParameters.hDeviceWindow =
    GetConsoleWindow ();
  IDirect3DDeviceManager9* direct3D_manager_p = NULL;
  UINT reset_token = 0;
  if (!Stream_MediaFramework_DirectDraw_Tools::getDevice (configuration_in.direct3DConfiguration,
                                                          direct3D_manager_p,
                                                          reset_token))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectDraw_Tools::getDevice(), continuing\n")));
  else
  {
    ACE_ASSERT (configuration_in.direct3DConfiguration.handle);
    ACE_ASSERT (direct3D_manager_p);
    ACE_ASSERT (reset_token);
    direct3D_manager_p->Release (); direct3D_manager_p = NULL;
    reset_token = 0;
  } // end ELSE
#endif // ACE_WIN32 || ACE_WIN64

  struct Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
#if defined (GTK_USE)
  int result = -1;
#endif // GTK_USE

  // step0e: initialize signal handling
  //configuration_in.signalHandlerConfiguration.messageAllocator =
  //  &message_allocator;
  signalHandler_in.initialize (configuration_in.signalHandlerConfiguration);
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

  // intialize timers
  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

  // step0f: (initialize) processing stream

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step1a: start UI event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
    CBData_in.stream = &stream;
#if defined (GTK_USE)
    CBData_in.UIState = &state_r;
    CBData_in.progressData.state = &state_r;
#elif defined (WXWIDGETS_USE)
    struct Common_UI_wxWidgets_State& state_r =
      const_cast<struct Common_UI_wxWidgets_State&> (iapplication_in->getR ());
    state_r.resources[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<wxObject*> (NULL));
#endif

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
    gtk_manager_p->wait (false);
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
    Stream_IStreamControlBase* stream_p = NULL;
#if defined (GTK_USE)
    Common_UI_GTK_Tools::initialize (CBData_in.configuration->GTKConfiguration.argc,
                                     CBData_in.configuration->GTKConfiguration.argv);
#endif // GTK_USE

    if (!stream.initialize (configuration_in.streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));
      goto clean;
    } // end IF
    stream_p = &stream;
    ACE_ASSERT (stream_p);
    // *NOTE*: this will block until the file has been copied...
    stream_p->start ();
//    if (!stream_p->isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start stream, aborting\n")));
//      //timer_manager_p->stop ();
//      return;
//    } // end IF
    stream_p->wait (true, false, false);
  } // end ELSE

  // step3: clean up
clean:
#if defined (GTK_USE)
  gtk_manager_p->stop (true,   // wait ?
                       false);
#endif // GTK_USE
  timer_manager_p->stop ();

  stream.remove (&message_handler,
                 true,             // lock ?
                 true);            // reset ?

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

COMMON_DEFINE_PRINTVERSION_FUNCTION (do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE (programName_in, ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

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

  // initialize framework(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (true,   // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  Common_UI_Tools::initialize ();
#if defined (WXWIDGETS_USE)
  if (!Common_UI_WxWidgets_Tools::initialize (argc_in,
                                              argv_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_WxWidgets_Tools::initialize(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
#endif // WXWIDGETS_USE

  // step1a set defaults
  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#endif // ACE_WIN32 || ACE_WIN64
  bool debug_b = false;
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::string source_filename = path;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string UI_definition_filename = path;
  UI_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
  bool log_to_file = false;
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::string target_filename = path;
  bool trace_information = false;
  bool print_version_and_exit = false;
#if defined (GTK_USE)
  bool result_2 = false;
#endif // GTK_USE

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#endif // ACE_WIN32 || ACE_WIN64
                            debug_b,
                            source_filename,
                            UI_definition_filename,
                            log_to_file,
                            target_filename,
                            trace_information,
                            print_version_and_exit))
  {
    do_printUsage (ACE::basename (argv_in[0]));
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
  if (!Common_File_Tools::isReadable (source_filename)
      || (!UI_definition_filename.empty () &&
          !Common_File_Tools::isReadable (UI_definition_filename))
     )
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));
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
#if defined (GTK_USE)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);
#endif // GTK_USE
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                          ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]),                   // program name
                                     log_file_name,                                // log file name
                                     false,                                        // log to syslog ?
                                     false,                                        // trace messages ?
                                     trace_information,                            // debug messages ?
#if defined (GTK_USE)
                                     NULL))                                        // (ui) logger ?
//                                            (UI_definition_filename.empty () ? NULL
//                                                                             : &logger))) // (ui) logger ?
#elif defined (QT_USE)
                                     NULL))                                        // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                     NULL))                                        // (ui) logger ?
#else
                                     NULL))                                        // (ui) logger ?
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Visualization_Tools::initialize (STREAM_VIS_FRAMEWORK_DEFAULT);
#endif // ACE_WIN32 || ACE_WIN64

  struct Test_I_UI_CBData* ui_cb_data_p = NULL;
  struct Test_I_ImageSave_Configuration configuration;
  struct Test_I_ImageSave_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
  ui_cb_data_p = &ui_cb_data;

#if defined (GTK_USE)
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
#endif // GTK_USE
  ACE_ASSERT (ui_cb_data_p);

  // step1h: initialize UI framework
  struct Common_UI_State* ui_state_p = NULL;
#if defined (GTK_USE)
  ui_state_p = &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  Common_UI_wxWidgets_IApplicationBase_t* iapplication_p = NULL;
  ACE_NEW_NORETURN (iapplication_p,
                    Test_I_ImageSave_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                                      argc_in,
                                                                      Common_UI_WxWidgets_Tools::convertArgV (argc_in, argv_in),
                                                                      COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
  Test_I_ImageSave_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Test_I_ImageSave_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
  iinitialize_p->initialize (directshow_ui_cb_data);
  Test_I_ImageSave_WxWidgetsIApplication_t* iapplication_2 =
    dynamic_cast<Test_I_ImageSave_WxWidgetsIApplication_t*> (iapplication_p);
  ACE_ASSERT (iapplication_2);
  Test_I_ImageSave_WxWidgetsApplication_t::STATE_T& state_r =
    const_cast<Test_I_ImageSave_WxWidgetsApplication_t::STATE_T&> (iapplication_2->getR ());
  Test_I_ImageSave_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
    const_cast<Test_I_ImageSave_WxWidgetsApplication_t::CONFIGURATION_T&> (iapplication_2->getR_2 ());
  configuration_r.UIState = &state_r;
  ACE_ASSERT (configuration_r.UIState);
  ui_state_p =
    const_cast<Test_I_ImageSave_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;
  if (!iapplication_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: %m, aborting\n")));

    Common_Log_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
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

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (true, // handle SIGUSR1/SIGBREAK
                              // iff regular reporting
                              // is off
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                           false, // using networking ?
                                           false, // using asynch timers ?
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalize ();
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
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
  Test_I_SignalHandler signal_handler;

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_OS_Tools::setResourceLimits (false,  // file descriptors
                                           true,   // stack traces
                                           false)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
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

  if (!UI_definition_filename.empty ())
  {
#if defined (GTK_USE)
    result_2 = gtk_manager_p->initialize (ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
#if defined (GTK_USE)
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), aborting\n")));

      Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                     previous_signal_actions,
                                     previous_signal_mask);
      Common_Log_Tools::finalize ();
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

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#endif // ACE_WIN32 || ACE_WIN64
           debug_b,
           source_filename,
           target_filename,
           configuration,
           UI_definition_filename,
           ui_cb_data,
#if defined (WXWIDGETS_USE)
           iapplication_p,
#endif // WXWIDGETS_USE
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
  Common_Log_Tools::finalize ();
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
