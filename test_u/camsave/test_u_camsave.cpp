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

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Synch.h"
#include "ace/Version.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#endif
#include "gtk/gtk.h"

#ifdef LIBACENETWORK_ENABLE_VALGRIND_SUPPORT
#include "valgrind/valgrind.h"
#endif

#include "common_file_tools.h"
#include "common_logger.h"
#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "common_ui_defines.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#ifdef HAVE_CONFIG_H
#include "libACEStream_config.h"
#endif

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "test_u_common.h"
#include "test_u_defines.h"

#include "test_u_camsave_callbacks.h"
#include "test_u_camsave_defines.h"
#include "test_u_camsave_eventhandler.h"
#include "test_u_camsave_module_eventhandler.h"
#include "test_u_camsave_signalhandler.h"
#include "test_u_camsave_stream.h"

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("camsave");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]  : buffer size (byte(s)) [")
            << TEST_U_STREAM_CAMSAVE_DEFAULT_BUFFER_SIZE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : show console [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#else
  std::string device_file = ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY);
  device_file += ACE_DIRECTORY_SEPARATOR_CHAR;
  device_file += ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEFAULT_VIDEO_DEVICE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : device [\"")
            << device_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CAMSAVE_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[STRING]]: target filename [")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CAMSAVE_DEFAULT_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  //std::cout << ACE_TEXT_ALWAYS_CHAR ("-y          : run stress-test [")
  //  << false
  //  << ACE_TEXT_ALWAYS_CHAR ("]")
  //  << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#else
                     std::string& deviceFilename_out,
#endif
                     std::string& targetFileName_out,
                     std::string& UIFile_out,
                     bool& logToFile_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("camsave");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  bufferSize_out = TEST_U_STREAM_CAMSAVE_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#else
  deviceFilename_out = ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY);
  deviceFilename_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  deviceFilename_out += ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEFAULT_VIDEO_DEVICE);
#endif
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CAMSAVE_DEFAULT_OUTPUT_FILE);
  targetFileName_out = path;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CAMSAVE_DEFAULT_GLADE_FILE);
  logToFile_out = false;
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  printVersionAndExit_out = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("b:cf::g::hi:ls:tv"),
#else
                              ACE_TEXT ("b:d:f::g::hi:ls:tv"),
#endif
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'c':
      {
        showConsole_out = true;
        break;
      }
#else
      case 'd':
      {
        deviceFilename_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
#endif
      case 'f':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          targetFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          targetFileName_out.clear ();
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
      //case 'y':
      //{
      //  runStressTest_out = true;
      //  break;
      //}
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
#if defined (DEBUG_DEBUGGER)
  signals_out.sig_del (SIGTRAP);           // 5       /* Trace trap (POSIX) */
#endif
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
do_initialize_mediafoundation (const std::string& deviceName_in,
                               const HWND windowHandle_in,
                               //IGraphBuilder*& IGraphBuilder_out,
                               IMFMediaSession*& IMFMediaSession_out,
                               //IAMBufferNegotiation*& IAMBufferNegotiation_out,
                               //IAMStreamConfig*& IAMStreamConfig_out)
                               bool loadDevice_in,
                               bool coInitialize_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  IMFMediaSource* media_source_p = NULL;
  IMFTopology* topology_p = NULL;

  // sanity check(s)
  ACE_ASSERT (!IMFMediaSession_out);

  if (!coInitialize_in)
    goto continue_;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED    |
                            COINIT_DISABLE_OLE1DDE  |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result))
  {
    // *NOTE*: most probable reason: already initialized (happens in the debugger)
    //         --> continue
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF

continue_:
  result = MFStartup (MF_VERSION,
                      MFSTARTUP_LITE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFStartup(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  Stream_Module_Device_Tools::initialize ();

  if (!loadDevice_in)
    goto continue_2;

  //if (!Stream_Module_Device_Tools::loadDeviceGraph (deviceName_in,
  //                                                  IGraphBuilder_out,
  //                                                  IAMBufferNegotiation_out,
  //                                                  IAMStreamConfig_out))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
  //              ACE_TEXT (deviceName_in.c_str ())));
  //  return false;
  //} // end IF
  //ACE_ASSERT (IGraphBuilder_out);
  //ACE_ASSERT (IAMBufferNegotiation_out);
  //ACE_ASSERT (IAMStreamConfig_out);

  WCHAR* symbolic_link_p = NULL;
  UINT32 symbolic_link_size = 0;
  if (!Stream_Module_Device_Tools::getMediaSource (deviceName_in,
                                                   media_source_p,
                                                   symbolic_link_p,
                                                   symbolic_link_size))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(\"%s\"), aborting\n"),
                ACE_TEXT (deviceName_in.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_source_p);
  ACE_ASSERT (symbolic_link_p);
  ACE_ASSERT (symbolic_link_size);
  CoTaskMemFree (symbolic_link_p);

  if (!Stream_Module_Device_Tools::loadDeviceTopology (deviceName_in,
                                                       media_source_p,
                                                       NULL,
                                                       topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceTopology(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  media_source_p->Release ();
  media_source_p = NULL;

continue_2:
  IMFAttributes* attributes_p = NULL;
  result = MFCreateAttributes (&attributes_p, 4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
  ACE_ASSERT (SUCCEEDED (result));
  //result = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
  //ACE_ASSERT (SUCCEEDED (result));
  result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = MFCreateMediaSession (attributes_p,
                                 &IMFMediaSession_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    attributes_p->Release ();

    goto error;
  } // end IF
  attributes_p->Release ();

  if (topology_p)
  {
    DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);// |
                            //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                            //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
    result = IMFMediaSession_out->SetTopology (topology_flags,
                                               topology_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    topology_p->Release ();
    topology_p = NULL;
  } // end IF

  //if (_DEBUG)
  //{
  //  std::string log_file_name =
  //    Common_File_Tools::getLogDirectory (std::string (),
  //                                        0);
  //  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  //  log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
  //  Stream_Module_Device_Tools::debug (IGraphBuilder_out,
  //                                     log_file_name);
  //} // end IF

  //std::list<std::wstring> filter_pipeline;
  //if (!Stream_Module_Device_Tools::loadRendererGraph (windowHandle_in,
  //                                                    IGraphBuilder_out,
  //                                                    filter_pipeline))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererGraph(), aborting\n")));
  //  goto error;
  //} // end IF

  //IMediaFilter* media_filter_p = NULL;
  //HRESULT result = IGraphBuilder_out->QueryInterface (IID_IMediaFilter,
  //                                                    (void**)&media_filter_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (media_filter_p);
  //result = media_filter_p->SetSyncSource (NULL);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //media_filter_p->Release ();

  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (topology_p)
    topology_p->Release ();
//  if (IAMStreamConfig_out)
//  {
//    IAMStreamConfig_out->Release ();
//    IAMStreamConfig_out = NULL;
//  } // end IF
//  if (IGraphBuilder_out)
//  {
//    IGraphBuilder_out->Release ();
//    IGraphBuilder_out = NULL;
//  } // end IF
  if (IMFMediaSession_out)
  {
    IMFMediaSession_out->Release ();
    IMFMediaSession_out = NULL;
  } // end IF

  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  if (coInitialize_in)
    CoUninitialize ();

  return false;
}

void
do_finalize_mediafoundation (Stream_CamSave_GTK_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_mediafoundation"));

  // sanity check(s)
  ACE_ASSERT (CBData_in.configuration);

  HRESULT result = E_FAIL;
  //if (CBData_in.streamConfiguration)
  //{
  //  CBData_in.streamConfiguration->Release ();
  //  CBData_in.streamConfiguration = NULL;
  //} // end IF
  //if (CBData_in.configuration->moduleHandlerConfiguration.builder)
  //{
  //  CBData_in.configuration->moduleHandlerConfiguration.builder->Release ();
  //  CBData_in.configuration->moduleHandlerConfiguration.builder = NULL;
  //} // end IF
  if (CBData_in.configuration->moduleHandlerConfiguration.session)
  {
    result =
      CBData_in.configuration->moduleHandlerConfiguration.session->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    CBData_in.configuration->moduleHandlerConfiguration.session->Release ();
    CBData_in.configuration->moduleHandlerConfiguration.session = NULL;
  } // end IF

  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  CoUninitialize ();
}
#endif

void
do_work (unsigned int bufferSize_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#else
         const std::string& deviceFilename_in,
#endif
         const std::string& targetFilename_in,
         const std::string& UIDefinitionFilename_in,
         unsigned int statisticReportingInterval_in,
         Stream_CamSave_GTK_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_CamSave_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  // step0a: initialize configuration
  Stream_CamSave_Configuration configuration;
  CBData_in.configuration = &configuration;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_handle = NULL;
  if (configuration.moduleHandlerConfiguration.gdkWindow)
  {
    ACE_ASSERT (gdk_win32_window_is_win32 (configuration.moduleHandlerConfiguration.gdkWindow));
    window_handle =
      //gdk_win32_window_get_impl_hwnd (configuration.moduleHandlerConfiguration.gdkWindow);
      //gdk_win32_drawable_get_handle (GDK_DRAWABLE (configuration.moduleHandlerConfiguration.gdkWindow));
      static_cast<HWND> (GDK_WINDOW_HWND (GDK_DRAWABLE (configuration.moduleHandlerConfiguration.gdkWindow)));
  } // end IF
  IMFMediaSession* media_session_p = NULL;
  //IAMBufferNegotiation* buffer_negotiation_p = NULL;
  bool load_device = UIDefinitionFilename_in.empty ();
  bool initialize_COM = UIDefinitionFilename_in.empty ();
  if (!do_initialize_mediafoundation (configuration.moduleHandlerConfiguration.device,
                                      window_handle,
                                      //configuration.moduleHandlerConfiguration.builder,
                                      media_session_p,
                                      //buffer_negotiation_p,
                                      //CBData_in.streamConfiguration))
                                      load_device,     // load device ?
                                      initialize_COM)) // initialize COM ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::do_initialize_directshow(), returning\n")));
    return;
  } // end IF
  //ACE_ASSERT (configuration.moduleHandlerConfiguration.builder);
  ACE_ASSERT (media_session_p);
  //ACE_ASSERT (buffer_negotiation_p);
  //ACE_ASSERT (CBData_in.streamConfiguration);
  //buffer_negotiation_p->Release ();
#endif

  Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;

  Stream_AllocatorHeap_T<Stream_AllocatorConfiguration> heap_allocator;
  heap_allocator.initialize (configuration.allocatorConfiguration);
  Stream_CamSave_MessageAllocator_t message_allocator (TEST_U_STREAM_CAMSAVE_MAX_MESSAGES, // maximum #buffers
                                                       &heap_allocator,                    // heap allocator handle
                                                       true);                              // block ?
  Stream_CamSave_EventHandler ui_event_handler (&CBData_in);
  Stream_CamSave_Module_EventHandler_Module event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                           NULL,
                                                           true);
  Stream_CamSave_Stream stream;
  Stream_CamSave_Module_EventHandler* event_handler_p =
    dynamic_cast<Stream_CamSave_Module_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_CamSave_Module_EventHandler> failed, returning\n")));
    goto clean;
  } // end IF
  event_handler_p->initialize (&CBData_in.subscribers,
                               &CBData_in.subscribersLock);
  event_handler_p->subscribe (&ui_event_handler);

  // ********************** module configuration data **************************
  configuration.moduleConfiguration.streamConfiguration =
      &configuration.streamConfiguration;
  configuration.moduleHandlerConfiguration.active =
      !UIDefinitionFilename_in.empty ();
  configuration.moduleHandlerConfiguration.hasHeader = true; // write AVI files

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: the media foundation capture source module does not use a worker
  //         thread
  configuration.moduleHandlerConfiguration.active = false;
  configuration.moduleHandlerConfiguration.session = media_session_p;
#else
  configuration.moduleHandlerConfiguration.device = deviceFilename_in;
  // *TODO*: turn these into an option
  configuration.moduleHandlerConfiguration.buffers =
      MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS;
  configuration.moduleHandlerConfiguration.method = V4L2_MEMORY_MMAP;

  configuration.moduleHandlerConfiguration.lock = &CBData_in.lock;
#endif
  configuration.moduleHandlerConfiguration.streamConfiguration =
      &configuration.streamConfiguration;
  if (statisticReportingInterval_in != 0)
    configuration.moduleHandlerConfiguration.statisticCollectionInterval.set (0,
                                                                              MODULE_DEV_CAM_STATISTIC_COLLECTION_INTERVAL * 1000);
  configuration.moduleHandlerConfiguration.targetFileName = targetFilename_in;

  // ********************** stream configuration data **************************
  if (bufferSize_in)
    configuration.streamConfiguration.bufferSize = bufferSize_in;
  configuration.streamConfiguration.messageAllocator = &message_allocator;
  configuration.streamConfiguration.module =
      (!UIDefinitionFilename_in.empty () ? &event_handler
                                         : NULL);
  configuration.streamConfiguration.moduleConfiguration =
      &configuration.moduleConfiguration;
  configuration.streamConfiguration.moduleHandlerConfiguration =
      &configuration.moduleHandlerConfiguration;
  configuration.streamConfiguration.printFinalReport = true;
  configuration.streamConfiguration.statisticReportingInterval =
      statisticReportingInterval_in;

  // step0e: initialize signal handling
  configuration.signalHandlerConfiguration.messageAllocator =
      &message_allocator;
  signalHandler_in.initialize (configuration.signalHandlerConfiguration);
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), returning\n")));
    goto clean;
  } // end IF

  // intialize timers
  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();

  // step0f: (initialize) processing stream

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step1a: start GTK event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
    CBData_in.finalizationHook = idle_finalize_UI_cb;
    CBData_in.initializationHook = idle_initialize_UI_cb;
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<GtkBuilder*> (NULL));
    CBData_in.stream = &stream;
    CBData_in.userData = &CBData_in;

    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->start ();
    ACE_Time_Value one_second (1, 0);
    int result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
    if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      goto clean;
    } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));

      // clean up
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (true);

      goto clean;
    } // end IF
    BOOL was_visible_b = false;
    if (!showConsole_in)
      was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif
  } // end IF
  else
  {
    if (!stream.initialize (configuration.streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));
      goto clean;
    } // end IF

    // *NOTE*: this will block until the file has been copied...
    stream.start ();
//    if (!stream.isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start stream, aborting\n")));

//      // clean up
//      //timer_manager_p->stop ();

//      return;
//    } // end IF
    stream.wait (true, false, false);
  } // end ELSE

  // step3: clean up
  if (!UIDefinitionFilename_in.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->wait ();
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

clean:
  timer_manager_p->stop ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  do_finalize_mediafoundation (CBData_in);
#endif

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
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

  int result = -1;

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
#endif

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("camsave");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  unsigned int buffer_size = TEST_U_STREAM_CAMSAVE_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#else
  std::string device_filename =
      ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY);
  device_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  device_filename += ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEFAULT_VIDEO_DEVICE);
#endif
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CAMSAVE_DEFAULT_OUTPUT_FILE);
  std::string target_filename = path;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  std::string UI_definition_filename = path;
  UI_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CAMSAVE_DEFAULT_GLADE_FILE);
  bool log_to_file = false;
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information = false;
  bool print_version_and_exit = false;
  //bool run_stress_test = false;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#else
                            device_filename,
#endif
                            target_filename,
                            UI_definition_filename,
                            log_to_file,
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit))
  {
    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_U_STREAM_CAMSAVE_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if ((!UI_definition_filename.empty () &&
       !Common_File_Tools::isReadable (UI_definition_filename)))
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
#endif

    return EXIT_FAILURE;
  } // end IF
  //if (run_stress_test)
  //  action_mode = Net_Client_TimeoutHandler::ACTION_STRESS;

  Stream_CamSave_GTK_CBData gtk_cb_user_data;
  gtk_cb_user_data.progressData.GTKState = &gtk_cb_user_data;
  // step1d: initialize logging and/or tracing
  Common_Logger_t logger (&gtk_cb_user_data.logStack,
                          &gtk_cb_user_data.lock);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACESTREAM_PACKAGE_NAME),
                                           ACE::basename (argv_in[0]));
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]),                   // program name
                                        log_file_name,                                // log file name
                                        false,                                        // log to syslog ?
                                        false,                                        // trace messages ?
                                        trace_information,                            // debug messages ?
                                        (UI_definition_filename.empty () ? NULL
                                                                         : &logger))) // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  sigset_t previous_signal_mask;
  result = ACE_OS::sigemptyset (&previous_signal_mask);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sigemptyset(): \"%m\", aborting\n")));

    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (!Common_Tools::preInitializeSignals (signal_set,
                                           true,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::preInitializeSignals(), aborting\n")));

    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  Stream_CamSave_SignalHandler signal_handler;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false,  // file descriptors
                                        true,   // stack traces
                                        false)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  Common_UI_GtkBuilderDefinition ui_definition (argc_in,
                                                argv_in);
  if (!UI_definition_filename.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                              argv_in,
                                                              &gtk_cb_user_data,
                                                              &ui_definition);

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#else
           device_filename,
#endif
           target_filename,
           UI_definition_filename,
           statistic_reporting_interval,
           gtk_cb_user_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  Common_Tools::period2String (working_time,
                               working_time_string);

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

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
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

  Common_Tools::finalizeSignals (signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;
} // end main
