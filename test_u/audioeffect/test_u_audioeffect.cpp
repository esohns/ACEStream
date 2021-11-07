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
#include "mtype.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#define INITGUID
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/High_Res_Timer.h"
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_tools.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_tools.h"

#include "stream_dev_directshow_tools.h"
#include "stream_dev_tools.h"

#include "stream_lib_directsound_common.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_guids.h"
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_defines.h"

#include "test_u_common.h"
#include "test_u_defines.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_u_audioeffect_callbacks.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_common_modules.h"
#include "test_u_audioeffect_defines.h"
#include "test_u_audioeffect_eventhandler.h"
#include "test_u_audioeffect_module_eventhandler.h"
#include "test_u_audioeffect_signalhandler.h"
#include "test_u_audioeffect_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("AudioEffectStream");

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();

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
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : device [\"")
            << ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e[[STRING]]: effect [\"")
            << ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_SOX_DEFAULT_EFFECT_NAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> default}")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[STRING]]: target filename [")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("] {\"\" --> default}")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#if defined (GTK_USE)
#if GTK_CHECK_VERSION(3,0,0)
  UI_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GTK3_GLADE_FILE);
#else
  UI_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GTK2_GLADE_FILE);
#endif // GTK_CHECK_VERSION (3,0,0)
#elif defined (WXWIDGETS_USE)
  UI_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_WXWIDGETS_XRC_FILE);
#endif // GTK_USE || WXWIDGETS_USE
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
#if defined (GTK_USE)
  std::string UI_style_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_DEFAULT_GTK_CSS_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i[[STRING]]: UI CSS file [\"")
            << UI_style_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> default}")
            << std::endl;
#endif // GTK_USE
#endif // GUI_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m          : use media foundation [")
            << (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u          : mute [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x          : use framework source [") // ? (directshow/mediafoundation) source : wavein
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#else
                     std::string& deviceIdentifier_out,
                     std::string& effect_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& targetFileName_out,
#if defined (GUI_SUPPORT)
                     std::string& UIFile_out,
#if defined (GTK_USE)
                     std::string& UICSSFile_out,
#endif // GTK_USE
#endif // GUI_SUPPORT
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     enum Stream_MediaFramework_Type& mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& mute_out,
                     bool& printVersionAndExit_out
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     ,bool& useFrameworkSource_out)
#else
                     )
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  bufferSize_out = TEST_U_STREAM_AUDIOEFFECT_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#else
//  deviceIdentifier_out = ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
//  deviceIdentifier_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  deviceIdentifier_out =
      ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME);
  effect_out.clear ();
#endif // ACE_WIN32 || ACE_WIN64
  path = Common_File_Tools::getTempDirectory ();
  targetFileName_out.clear ();
#if defined (GUI_SUPPORT)
  UIFile_out = configuration_path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out +=
#if defined (GTK_USE)
#if GTK_CHECK_VERSION (3,0,0)
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GTK3_GLADE_FILE);
#else
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GTK2_GLADE_FILE);
#endif
#elif defined (WXWIDGETS_USE)
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_WXWIDGETS_XRC_FILE);
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_USE)
  UICSSFile_out.clear ();
#endif // GTK_USE
#endif // GUI_SUPPORT
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework_out = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  mute_out = false;
  printVersionAndExit_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  useFrameworkSource_out = false;
#endif // ACE_WIN32 || ACE_WIN64

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("f::ls:tuv");
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  options_string += ACE_TEXT_ALWAYS_CHAR ("g::i::");
#elif defined (WXWIDGETS_USE)
  options_string += ACE_TEXT_ALWAYS_CHAR ("g::");
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  options_string += ACE_TEXT_ALWAYS_CHAR ("cmx");
#else
  options_string += ACE_TEXT_ALWAYS_CHAR ("d:e::");
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
                               ACE_TEXT_CHAR_TO_TCHAR (options_string.c_str ()),
                               1,                          // skip command name
                               1,                          // report parsing errors
                               ACE_Get_Opt::PERMUTE_ARGS,  // ordering
                               0);                         // for now, don't use long options
  //int result = argument_parser.long_option (ACE_TEXT ("sync"),
  //                                          'x',
  //                                          ACE_Get_Opt::NO_ARG);
  //if (result == -1)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Get_Opt::long_option(): \"%m\", aborting\n")));
  //  return false;
  //} // end IF

  int option = 0;
  std::stringstream converter;
  while ((option = argument_parser ()) != EOF)
  {
    switch (option)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'c':
      {
        showConsole_out = true;
        break;
      }
#else
      case 'd':
      {
        deviceIdentifier_out =
            ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'e':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          effect_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        else
          effect_out =
            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_SOX_DEFAULT_EFFECT_NAME);
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      case 'f':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          targetFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
        {
          targetFileName_out = path;
          targetFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
          targetFileName_out +=
            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
        } // end ELSE
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
#if defined (GTK_USE)
      case 'i':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UICSSFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
        {
          UICSSFile_out = configuration_path;
          UICSSFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
          UICSSFile_out += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
          UICSSFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
          UICSSFile_out +=
            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_DEFAULT_GTK_CSS_FILE);
        } // end ELSE
        break;
      }
#endif // GTK_USE
#endif // GUI_SUPPORT
      case 'l':
      {
        logToFile_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'm':
      {
        mediaFramework_out = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      case 's':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argument_parser.opt_arg ();
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
        mute_out = true;
        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'x':
      {
        useFrameworkSource_out = true;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
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
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("found long option \"%s\", continuing\n"),
                    argument_parser.long_option ()));
        break;
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
do_initializeSignals (ACE_Sig_Set& signals_out,
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
  signals_out.sig_add (SIGBREAK);          // 21      /* Ctrl-Break sequence */
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

//  signals_out.sig_del (SIGIO);             // 29      /* I/O now possible */
  // remove realtime-signals (don't need 'em)

#if defined (VALGRIND_SUPPORT)
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif // VALGRIND_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
do_initialize_directshow (const std::string& deviceIdentifier_in,
                          const struct Test_U_AudioEffect_DirectShow_Configuration& configuration_in,
                          IGraphBuilder*& IGraphBuilder_out,
                          IAMStreamConfig*& IAMStreamConfig_out,
                          struct _AMMediaType& captureMediaType_out,
                          bool coInitialize_in,
                          bool useDirectShowSource_in,
                          bool mute_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  struct tWAVEFORMATEX* waveformatex_p = NULL;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  IMediaFilter* media_filter_p = NULL;
  Test_U_AudioEffect_DirectShowFilter_t* filter_p = NULL;
  IBaseFilter* filter_2 = NULL;
  std::wstring filter_name = STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L;

  // sanity check(s)
  ACE_ASSERT (!IGraphBuilder_out);
  ACE_ASSERT (!IAMStreamConfig_out);

  if (!coInitialize_in)
    goto continue_;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED    |
                            COINIT_DISABLE_OLE1DDE  |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result)) // RPC_E_CHANGED_MODE : 0x80010106L
  {
    // *NOTE*: most probable reason: already initialized (happens in the
    //         debugger)
    //         --> continue
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF

continue_:
  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
  //Stream_Module_Device_Tools::initialize (true);

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (captureMediaType_out);

  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
  waveformatex_p = &waveformatex_s;

  waveformatex_p->wFormatTag = WAVE_FORMAT_PCM;
  waveformatex_p->nChannels = STREAM_DEV_MIC_DEFAULT_CHANNELS; // stereo
  waveformatex_p->nSamplesPerSec = STREAM_DEV_MIC_DEFAULT_SAMPLE_RATE;
  waveformatex_p->wBitsPerSample = STREAM_DEV_MIC_DEFAULT_BITS_PER_SAMPLE;
  waveformatex_p->nBlockAlign =
    (waveformatex_p->nChannels * (waveformatex_p->wBitsPerSample / 8));
  waveformatex_p->nAvgBytesPerSec =
    (waveformatex_p->nSamplesPerSec * waveformatex_p->nBlockAlign);
  waveformatex_p->cbSize = 0;

  result = CreateAudioMediaType (waveformatex_p,
                                 &captureMediaType_out,
                                 TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateAudioMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  if (!useDirectShowSource_in)
    goto continue_2;

  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (deviceIdentifier_in,
                                                        CLSID_AudioInputDeviceCategory,
                                                        IGraphBuilder_out,
                                                        buffer_negotiation_p,
                                                        IAMStreamConfig_out,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (deviceIdentifier_in.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (IAMStreamConfig_out);

  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  goto continue_3;

continue_2:
  result = CoCreateInstance (CLSID_FilterGraph, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&IGraphBuilder_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
  ACE_NEW_NORETURN (filter_p,
                    Test_U_AudioEffect_DirectShowFilter_t ());
  if (!filter_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    goto error;
  } // end IF

  ACE_ASSERT (!configuration_in.filterConfiguration.pinConfiguration->format);
  configuration_in.filterConfiguration.pinConfiguration->format =
    &captureMediaType_out;
  if (!filter_p->initialize (configuration_in.filterConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Source_Filter_T::initialize(), aborting\n")));
    delete filter_p; filter_p = NULL;
    goto error;
  } // end IF
  result =
    filter_p->NonDelegatingQueryInterface (IID_PPV_ARGS (&filter_2));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to NonDelegatingQueryInterface(IID_IBaseFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    delete filter_p; filter_p = NULL;
    goto error;
  } // end IF
  // *WARNING*: invokes IBaseFilter::GetBuffer
  result = IGraphBuilder_out->AddFilter (filter_2,
                                         filter_name.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    filter_2->Release (); filter_2 = NULL;
    delete filter_p; filter_p = NULL;
    goto error;
  } // end IF
  filter_2->Release (); filter_2 = NULL;
  graph_layout.push_back (filter_name);

continue_3:
  if (!useDirectShowSource_in)
    goto continue_4;

  if (!Stream_Device_DirectShow_Tools::setCaptureFormat (IGraphBuilder_out,
                                                         CLSID_AudioInputDeviceCategory,
                                                         captureMediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF

continue_4:
  union Stream_MediaFramework_DirectSound_AudioEffectOptions effect_options;
  if (!Stream_Module_Decoder_Tools::loadAudioRendererGraph ((useDirectShowSource_in ? CLSID_AudioInputDeviceCategory : GUID_NULL),
                                                            captureMediaType_out,
                                                            (mute_in ? -1 : 0),
                                                            IGraphBuilder_out,
                                                            GUID_NULL,
                                                            effect_options,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::loadAudioRendererGraph(), aborting\n")));
    goto error;
  } // end IF

  result = IGraphBuilder_out->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release (); media_filter_p = NULL;

  return true;

error:
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  if (media_filter_p)
    media_filter_p->Release ();

  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
  } // end IF
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
  } // end IF
  Stream_MediaFramework_DirectShow_Tools::free (captureMediaType_out);

  if (coInitialize_in)
    CoUninitialize ();

  return false;
}
bool
do_initialize_mediafoundation (bool coInitialize_in,
                               bool initializeMediaFoundation_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;

  if (!coInitialize_in)
    goto continue_;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED    |
                            COINIT_DISABLE_OLE1DDE  |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result)) // RPC_E_CHANGED_MODE : 0x80010106L
  {
    // *NOTE*: most probable reason: already initialized (happens in the
    //         debugger)
    //         --> continue
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF

continue_:
  if (!initializeMediaFoundation_in)
    goto continue_2;

  result = MFStartup (MF_VERSION,
                      MFSTARTUP_LITE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFStartup(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

continue_2:
  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION);
  //Stream_Module_Device_Tools::initialize (true);

  return true;

error:
  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  if (coInitialize_in)
    CoUninitialize ();

  return false;
}
void
do_finalize_directshow (struct Test_U_AudioEffect_DirectShow_UI_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  ACE_UNUSED_ARG (CBData_in);

  HRESULT result = E_FAIL;

  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T iterator =
    CBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->streamConfiguration.end ());

  if ((*iterator).second.second->builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF

  CoUninitialize ();
}
void
do_finalize_mediafoundation ()
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_mediafoundation"));

  HRESULT result = E_FAIL;

  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  CoUninitialize ();
}
#endif // ACE_WIN32 || ACE_WIN64

void
do_work (unsigned int bufferSize_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#else
         const std::string& deviceIdentifier_in,
         const std::string& effectName_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& targetFilename_in,
#if defined (GUI_SUPPORT)
         const std::string& UIDefinitionFile_in,
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         enum Stream_MediaFramework_Type mediaFramework_in,
#endif // ACE_WIN32 || ACE_WIN64
         unsigned int statisticReportingInterval_in,
         bool mute_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool useFrameworkSource_in,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_U_AudioEffect_DirectShow_UI_CBData& directShowCBData_in,
         struct Test_U_AudioEffect_MediaFoundation_UI_CBData& mediaFoundationCBData_in,
#else
         struct Test_U_AudioEffect_UI_CBData& CBData_in,
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_U_AudioEffect_DirectShow_Configuration& directShowConfiguration_in,
         struct Test_U_AudioEffect_MediaFoundation_Configuration& mediaFoundationConfiguration_in,
#else
         struct Test_U_AudioEffect_Configuration& configuration_in,
#endif // ACE_WIN32 || ACE_WIN64
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_AudioEffect_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  struct Stream_AllocatorConfiguration allocator_configuration;
  struct Common_AllocatorConfiguration* allocator_configuration_p = NULL;
  Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Common_IAsynchTask* itask_p = NULL;
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                                 &heap_allocator,     // heap allocator handle
                                                                                 true);               // block ?
  Test_U_AudioEffect_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                                           &heap_allocator,     // heap allocator handle
                                                                                           true);               // block ?
#else
  Test_U_AudioEffect_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                           &heap_allocator,     // heap allocator handle
                                                           true);               // block ?
#endif // ACE_WIN32 || ACE_WIN64
  bool result = false;
  Stream_IStream_t* istream_p = NULL;
  Stream_IStreamControlBase* istream_control_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  struct Test_U_AudioEffect_DirectShow_StreamConfiguration directshow_stream_configuration;
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration;
  Test_U_AudioEffect_DirectShow_Stream directshow_stream;
  Test_U_AudioEffect_MediaFoundation_Stream mediafoundation_stream;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      istream_p = &directshow_stream;
      istream_control_p = &directshow_stream;
      directshow_stream_configuration.useFrameworkSource =
        useFrameworkSource_in;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      istream_p = &mediafoundation_stream;
      istream_control_p = &mediafoundation_stream;
      mediafoundation_stream_configuration.useFrameworkSource =
        useFrameworkSource_in;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  Test_U_AudioEffect_ALSA_Stream stream;
  istream_p = &stream;
  istream_control_p = &stream;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Time_Value one_second (1, 0);
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#if defined (GTKGL_SUPPORT)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTKGL_SUPPORT
  int result_2 = -1;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
//  const Stream_Module_t* module_p = NULL;
//  Test_U_AudioEffect_IDispatch_t* idispatch_p = NULL;
  struct Stream_ModuleConfiguration module_configuration;

  // step0a: initialize configuration
  allocator_configuration_p = &allocator_configuration;
  ACE_ASSERT (allocator_configuration_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_EventHandler directshow_ui_event_handler (
#if defined (GUI_SUPPORT)
                                                                          &directShowCBData_in
#endif // GUI_SUPPORT
                                                                         );
  Test_U_AudioEffect_DirectShow_Module_EventHandler_Module directshow_event_handler (istream_p,
                                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_AudioEffect_MediaFoundation_EventHandler mediafoundation_ui_event_handler (
#if defined (GUI_SUPPORT)
                                                                                    &mediaFoundationCBData_in
#endif // GUI_SUPPORT
                                                                                   );
  Test_U_AudioEffect_MediaFoundation_Module_EventHandler_Module mediafoundation_event_handler (istream_p,
                                                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
#else
  Test_U_AudioEffect_EventHandler ui_event_handler (
#if defined (GUI_SUPPORT)
                                                    &CBData_in
#endif // GUI_SUPPORT
                                                   );
  Test_U_AudioEffect_Module_EventHandler_Module event_handler (istream_p,
                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_iterator;
  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration;
  ALSA_configuration.asynch = STREAM_LIB_ALSA_CAPTURE_DEFAULT_ASYNCH;
  ALSA_configuration.bufferSize = STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_SIZE;
  ALSA_configuration.bufferTime = STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_TIME;
  ALSA_configuration.periods = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIODS;
  ALSA_configuration.periodSize = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_SIZE;
  ALSA_configuration.periodTime = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_TIME;
  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration_2;
//  ALSA_configuration_2.asynch = false;
  struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration modulehandler_configuration_2;
  struct Test_U_AudioEffect_ALSA_StreamConfiguration stream_configuration;
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (allocator_configuration_p);
  if (!heap_allocator.initialize (*allocator_configuration_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to intialize heap allocator, returning\n")));
    goto error;
  } // end IF
  // ********************** module configuration data **************************

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directShowCBData_in.configuration);
      directshow_modulehandler_configuration.allocatorConfiguration =
        allocator_configuration_p;
      directshow_modulehandler_configuration.filterConfiguration =
        &directShowCBData_in.configuration->filterConfiguration;
      directshow_modulehandler_configuration.messageAllocator =
        &directshow_message_allocator;
      directshow_modulehandler_configuration.mute = mute_in;

      directshow_modulehandler_configuration.audioOutput = 1;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
      directshow_modulehandler_configuration.surfaceLock =
        &directShowCBData_in.surfaceLock;

#if defined (GTKGL_SUPPORT)
      directshow_modulehandler_configuration.OpenGLInstructions =
        &directShowCBData_in.OpenGLInstructions;
      directshow_modulehandler_configuration.OpenGLInstructionsLock =
        &state_r.lock;
#endif // GTKGL_SUPPORT
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

      directshow_modulehandler_configuration.printProgressDot =
        UIDefinitionFile_in.empty ();
      directshow_modulehandler_configuration.statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      directshow_modulehandler_configuration.subscriber =
        &directshow_ui_event_handler;
      directshow_modulehandler_configuration.targetFileName =
          (targetFilename_in.empty () ? Common_File_Tools::getTempDirectory ()
                                      : targetFilename_in);

      directShowCBData_in.configuration->streamConfiguration.initialize (module_configuration,
                                                                         directshow_modulehandler_configuration,
                                                                         directshow_stream_configuration);

      directshow_modulehandler_iterator =
        directShowConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directShowConfiguration_in.streamConfiguration.end ());

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediaFoundationCBData_in.configuration);
      mediafoundation_modulehandler_configuration.allocatorConfiguration =
        allocator_configuration_p;
      mediafoundation_modulehandler_configuration.mute = mute_in;

      mediafoundation_modulehandler_configuration.audioOutput = 1;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
      mediafoundation_modulehandler_configuration.surfaceLock =
        &mediaFoundationCBData_in.surfaceLock;

#if defined (GTKGL_SUPPORT)
      mediafoundation_modulehandler_configuration.OpenGLInstructions =
        &mediaFoundationCBData_in.OpenGLInstructions;
      mediafoundation_modulehandler_configuration.OpenGLInstructionsLock =
        &state_r.lock;
#endif // GTKGL_SUPPORT
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

      mediafoundation_modulehandler_configuration.printProgressDot =
        UIDefinitionFile_in.empty ();
      mediafoundation_modulehandler_configuration.statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      mediafoundation_modulehandler_configuration.subscriber =
        &mediafoundation_ui_event_handler;
      mediafoundation_modulehandler_configuration.targetFileName =
          (targetFilename_in.empty () ? Common_File_Tools::getTempDirectory ()
                                      : targetFilename_in);

      mediaFoundationCBData_in.configuration->streamConfiguration.initialize (module_configuration,
                                                                              mediafoundation_modulehandler_configuration,
                                                                              mediafoundation_stream_configuration);
      mediafoundation_modulehandler_iterator =
        mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediaFoundationConfiguration_in.streamConfiguration.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  modulehandler_configuration.allocatorConfiguration =
    allocator_configuration_p;
  modulehandler_configuration.ALSAConfiguration = &ALSA_configuration;
  stream_configuration.allocatorConfiguration = allocator_configuration_p;
  modulehandler_configuration.deviceIdentifier.identifier = deviceIdentifier_in;
  modulehandler_configuration.effect = effectName_in;
  modulehandler_configuration.messageAllocator = &message_allocator;
  modulehandler_configuration.mute = mute_in;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  modulehandler_configuration.surfaceLock = &CBData_in.surfaceLock;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
  modulehandler_configuration.OpenGLInstructions =
      &CBData_in.OpenGLInstructions;
  modulehandler_configuration.OpenGLInstructionsLock = &state_r.lock;
#endif // GTKGL_SUPPORT
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
  modulehandler_configuration.printProgressDot = UIDefinitionFile_in.empty ();
  modulehandler_configuration.statisticReportingInterval =
      ACE_Time_Value (statisticReportingInterval_in, 0);
  modulehandler_configuration.streamConfiguration =
      &configuration_in.streamConfiguration;
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.targetFileName =
      (targetFilename_in.empty () ? Common_File_Tools::getTempDirectory ()
                                  : targetFilename_in);

  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  modulehandler_iterator =
      configuration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR(""));
  ACE_ASSERT(modulehandler_iterator != configuration_in.streamConfiguration.end());

  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.ALSAConfiguration = &ALSA_configuration_2;
  modulehandler_configuration_2.deviceIdentifier.identifier =
    ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEVICE_PLAYBACK_PREFIX);
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_2)));
#endif // ACE_WIN32 || ACE_WIN64

  // ********************** stream configuration data **************************
  if (bufferSize_in)
    allocator_configuration.defaultBufferSize = bufferSize_in;
#if defined (ACE_WIN32) || defined (ACE_WIN64)

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_stream_configuration.allocatorConfiguration = &allocator_configuration;
      directshow_stream_configuration.messageAllocator = &directshow_message_allocator;
      directshow_stream_configuration.module =
        (!UIDefinitionFile_in.empty () ? &directshow_event_handler
                                       : NULL);
      directshow_stream_configuration.printFinalReport = true;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_stream_configuration.messageAllocator =
        &mediafoundation_message_allocator;
      mediafoundation_stream_configuration.module =
        (!UIDefinitionFile_in.empty () ? &mediafoundation_event_handler
                                       : NULL);
      mediafoundation_stream_configuration.printFinalReport = true;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
      (!UIDefinitionFile_in.empty () ? &event_handler
                                     : NULL);
  stream_configuration.printFinalReport = true;
#endif // ACE_WIN32 || ACE_WIN64

  // intialize timers
  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: in UI mode, COM has already been initialized for this thread
  // *TODO*: where has that happened ?
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result =
        do_initialize_directshow ((*directshow_modulehandler_iterator).second.second->deviceIdentifier.identifier._string,
                                  *directShowCBData_in.configuration,
                                  (*directshow_modulehandler_iterator).second.second->builder,
                                  directShowCBData_in.streamConfiguration,
                                  directshow_stream_configuration.format,
                                  true,  // initialize COM ?
                                  useFrameworkSource_in, // use DirectShow source ? : WaveIn
                                  mute_in);
      if (!result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to do_initialize_directshow(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT ((*directshow_modulehandler_iterator).second.second->builder);
      if (false) // use DirectShow source ?
      {
        ACE_ASSERT (directShowCBData_in.streamConfiguration);
      } // end IF
      Stream_MediaFramework_DirectShow_Tools::copy (directshow_stream_configuration.format,
                                                    (*directshow_modulehandler_iterator).second.second->outputFormat);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      result =
        do_initialize_mediafoundation (true, // initialize COM ?
                                       true);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  result = true;
#endif // ACE_WIN32 || ACE_WIN64
  if (!result)
  {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to intialize media framework, returning\n")));
      goto error;
  } // end IF

  // step0e: initialize signal handling
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directShowConfiguration_in.signalHandlerConfiguration.messageAllocator =
        &directshow_message_allocator;
      signalHandler_in.initialize (directShowConfiguration_in.signalHandlerConfiguration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediaFoundationConfiguration_in.signalHandlerConfiguration.messageAllocator =
        &mediafoundation_message_allocator;
      signalHandler_in.initialize (mediaFoundationConfiguration_in.signalHandlerConfiguration);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  configuration_in.signalHandlerConfiguration.messageAllocator =
    &message_allocator;
  signalHandler_in.initialize (configuration_in.signalHandlerConfiguration);
#endif // ACE_WIN32 || ACE_WIN64
  if (!Common_Signal_Tools::initialize (((COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR) ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                                                                          : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), aborting\n")));
    goto error;
  } // end IF

#if defined (GUI_SUPPORT)
  // step1a: start ui event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (mediaFramework_in)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
#if defined (GTK_USE)
        //directShowCBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
        //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
        state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
          std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE
        directShowCBData_in.stream = &directshow_stream;
        //directShowCBData_in.userData = &directShowCBData_in;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
#if defined (GTK_USE)
        //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
        //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
        state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
          std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE
        mediaFoundationCBData_in.stream = &mediafoundation_stream;
        //mediaFoundationCBData_in.UIState.userData = &mediaFoundationCBData_in;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    mediaFramework_in));
        return;
      }
    } // end SWITCH
#else
#if defined (GTK_USE)
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
    CBData_in.UIState = &state_r;
#endif // GTK_USE
    CBData_in.stream = &stream;
    //CBData_in.userData = &CBData_in;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_USE)
    itask_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (itask_p);
    itask_p->start (NULL);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION * 1000);
    result_2 = ACE_OS::sleep (timeout);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!itask_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, aborting\n")));
      goto error;
    } // end IF
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), aborting\n")));
      goto error;
    } // end IF
    BOOL was_visible_b = false;
    if (!showConsole_in)
      was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64
    itask_p->wait (false);
  } // end IF
  else
  {
#endif // GUI_SUPPORT
    // pre-initialize processing stream
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    bool success_b = false;
    switch (mediaFramework_in)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        success_b =
          directshow_stream.initialize (directShowConfiguration_in.streamConfiguration);
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        success_b =
          mediafoundation_stream.initialize (mediaFoundationConfiguration_in.streamConfiguration);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    mediaFramework_in));
        return;
      }
    } // end SWITCH
    if (!success_b)
#else
    if (!stream.initialize (configuration_in.streamConfiguration))
#endif // ACE_WIN32 || ACE_WIN64
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize processing stream, aborting\n")));
      goto error;
    } // end IF

    ACE_ASSERT (istream_control_p);
    istream_control_p->start ();
//    if (!istream_control_p->isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start stream, aborting\n")));
//      goto error;
//    } // end IF
    istream_control_p->wait (true,
                             false,
                             false);
#if defined (GUI_SUPPORT)
  } // end ELSE
#endif // GUI_SUPPORT

  // step3: clean up
  //		{ // synch access
  //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //					 iterator != CBData_in.event_source_ids.end();
  //					 iterator++)
  //				g_source_remove(*iterator);
  //		} // end lock scope
  timer_manager_p->stop ();

  //  { // synch access
  //    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //		for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //				 iterator != CBData_in.event_source_ids.end();
  //				 iterator++)
  //			g_source_remove(*iterator);
  //	} // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result = directshow_stream.remove (&directshow_event_handler);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      result = mediafoundation_stream.remove (&mediafoundation_event_handler);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  result = stream.remove (&event_handler);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base::remove(), continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

error:
#if defined (GUI_SUPPORT)
  if (!UIDefinitionFile_in.empty () && itask_p)
    itask_p->stop (true,  // wait ?
                   true); // high priority ?
#endif // GUI_SUPPORT
  timer_manager_p->stop ();
}

void
do_printVersion (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printVersion"));

  std::ostringstream converter;

  // compiler version string...
  converter << ACE::compiler_major_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::compiler_minor_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::compiler_beta_version ();

  std::cout << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" compiled on ")
            << ACE::compiler_name ()
            << ACE_TEXT_ALWAYS_CHAR (" ")
            << converter.str ()
            << std::endl << std::endl;

  std::cout << ACE_TEXT_ALWAYS_CHAR ("libraries: ")
            << std::endl
#if defined (HAVE_CONFIG_H)
            << ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME)
            << ACE_TEXT_ALWAYS_CHAR (": ")
            << ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION)
            << std::endl
#endif // HAVE_CONFIG_H
            ;

  converter.str ("");
  // ACE version string...
  converter << ACE::major_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::minor_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::beta_version ();
  // *NOTE*: cannot use ACE_VERSION, as it doesn't contain the (potential) beta
  // version number... Need this, as the library soname is compared to this
  // string
  std::cout << ACE_TEXT_ALWAYS_CHAR ("ACE: ")
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
#endif // ACE_WIN32 || ACE_WIN64

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  unsigned int buffer_size = TEST_U_STREAM_AUDIOEFFECT_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#else
  std::string device_identifier_string =
//    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
//  device_identifier_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
//  device_identifier_string +=
      ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME);
  std::string effect_name;
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = Common_File_Tools::getTempDirectory ();
  std::string target_filename;
#if defined (GUI_SUPPORT)
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string UI_definition_file = path;
  UI_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_file +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UI_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#if defined (GTK_USE)
#if GTK_CHECK_VERSION(3,0,0)
  UI_definition_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GTK3_GLADE_FILE);
#else
  UI_definition_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GTK2_GLADE_FILE);
#endif // GTK_CHECK_VERSION(3,0,0)
#elif defined (WXWIDGETS_USE)
  UI_definition_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_WXWIDGETS_XRC_FILE);
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_USE)
  std::string UI_CSS_file;
#endif // GTK_USE
#endif // GUI_SUPPORT
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool mute = false;
  bool print_version_and_exit = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool use_framework_source = false;
#endif // ACE_WIN32 || ACE_WIN64

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#else
                            device_identifier_string,
                            effect_name,
#endif // ACE_WIN32 || ACE_WIN64
                            target_filename,
#if defined (GUI_SUPPORT)
                            UI_definition_file,
#if defined (GTK_USE)
                            UI_CSS_file,
#endif // GTK_USE
#endif // GUI_SUPPORT
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                            statistic_reporting_interval,
                            trace_information,
                            mute,
                            print_version_and_exit
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            ,use_framework_source))
#else
                            ))
#endif // ACE_WIN32 || ACE_WIN64
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
  if (TEST_U_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
#if defined (GUI_SUPPORT)
  if ((!UI_definition_file.empty () &&
       !Common_File_Tools::isReadable (UI_definition_file))
#if defined (GTK_USE)
      || (!UI_CSS_file.empty () &&
          !Common_File_Tools::isReadable (UI_CSS_file)))
#else
     )
#endif // GTK_USE
#endif // GUI_SUPPORT
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
  //if (run_stress_test)
  //  action_mode = Net_Client_TimeoutHandler::ACTION_STRESS;

#if defined (GUI_SUPPORT)
  Common_UI_Tools::initialize ();
#if defined (GTK_SUPPORT)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
#if defined (GTKGL_SUPPORT)
  struct Common_UI_GTK_GLConfiguration* gtk_configuration_p = NULL;
#else
  struct Common_UI_GTK_Configuration* gtk_configuration_p = NULL;
#endif // GTKGL_SUPPORT
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_SUPPORT
  struct Test_U_AudioEffect_UI_CBDataBase* cb_data_base_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_Configuration directshow_configuration;
  directshow_configuration.filterConfiguration.pinConfiguration =
    &directshow_configuration.pinConfiguration;
  struct Test_U_AudioEffect_DirectShow_UI_CBData directshow_ui_cb_data;
  struct Test_U_AudioEffect_MediaFoundation_Configuration mediafoundation_configuration;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData mediafoundation_ui_cb_data;
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data.configuration = &directshow_configuration;
#if defined (GTK_SUPPORT)
      directshow_ui_cb_data.progressData.state = &state_r;
      directshow_configuration.GTKConfiguration.argc = argc_in;
      directshow_configuration.GTKConfiguration.argv = argv_in;
      directshow_configuration.GTKConfiguration.CBData = &directshow_ui_cb_data;
      directshow_configuration.GTKConfiguration.eventHooks.finiHook =
          idle_finalize_UI_cb;
      directshow_configuration.GTKConfiguration.eventHooks.initHook =
          idle_initialize_UI_cb;
      directshow_configuration.GTKConfiguration.definition = &gtk_ui_definition;
#if GTK_CHECK_VERSION(3,0,0)
      if (!UI_CSS_file.empty ())
        directshow_ui_cb_data.configuration->GTKConfiguration.CSSProviders[UI_CSS_file] =
          NULL;
#endif // GTK_CHECK_VERSION(3,0,0)
      gtk_configuration_p = &directshow_configuration.GTKConfiguration;
#endif // GTK_SUPPORT
      cb_data_base_p = &directshow_ui_cb_data;
      cb_data_base_p->mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
#if defined (GTK_SUPPORT)
      mediafoundation_ui_cb_data.progressData.state = &state_r;

      mediafoundation_ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.CBData = &mediafoundation_ui_cb_data;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
          idle_finalize_UI_cb;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
          idle_initialize_UI_cb;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
#if GTK_CHECK_VERSION(3,0,0)
      if (!UI_CSS_file.empty ())
        mediafoundation_ui_cb_data.configuration->GTKConfiguration.CSSProviders[UI_CSS_file] =
          NULL;
#endif // GTK_CHECK_VERSION(3,0,0)
      gtk_configuration_p = &mediafoundation_ui_cb_data.configuration->GTKConfiguration;
#endif // GTK_SUPPORT
      cb_data_base_p = &mediafoundation_ui_cb_data;
      cb_data_base_p->mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
      mediafoundation_ui_cb_data.configuration = &mediafoundation_configuration;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  media_framework_e));

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
#else
  struct Test_U_AudioEffect_Configuration configuration;
  struct Test_U_AudioEffect_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;

#if defined (GTK_SUPPORT)
  ui_cb_data.progressData.state = &state_r;

  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
#if GTK_CHECK_VERSION(3,0,0)
  if (!UI_CSS_file.empty ())
    ui_cb_data.configuration->GTKConfiguration.CSSProviders[UI_CSS_file] = NULL;
#endif // GTK_CHECK_VERSION(3,0,0)
  gtk_configuration_p = &ui_cb_data.configuration->GTKConfiguration;
#endif // GTK_SUPPORT
  cb_data_base_p = &ui_cb_data;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (cb_data_base_p);
#endif // GUI_SUPPORT

  Common_MessageStack_t* logstack_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  cb_data_base_p->UIState = &state_r;
  logstack_p = &state_r.logStack;
  lock_p = &state_r.logStackLock;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

  // step1d: initialize logging and/or tracing
  Common_Logger_t logger (logstack_p,
                          lock_p);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),               // program name
                                            log_file_name,                            // log file name
                                            false,                                    // log to syslog ?
                                            false,                                    // trace messages ?
                                            trace_information,                        // debug messages ?
                                            (UI_definition_file.empty () ? NULL
                                                                         : &logger))) // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

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
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  sigset_t previous_signal_mask;
  result = ACE_OS::sigemptyset (&previous_signal_mask);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sigemptyset(): \"%m\", aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  lock_2 = &state_r.subscribersLock;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
  Test_U_AudioEffect_SignalHandler signal_handler (lock_2);

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
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
  if (!Common_Tools::setResourceLimits (false,  // file descriptors
                                        true,   // stack traces
                                        false)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
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
  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
#if defined (GTK_SUPPORT)
  ACE_ASSERT (gtk_configuration_p);
  if (!UI_definition_file.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (*gtk_configuration_p);
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#else
           device_identifier_string,
           effect_name,
#endif // ACE_WIN32 || ACE_WIN64
           target_filename,
#if defined (GUI_SUPPORT)
           UI_definition_file,
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
           statistic_reporting_interval,
           mute,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           use_framework_source,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_ui_cb_data,
           mediafoundation_ui_cb_data,
#else
           ui_cb_data,
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_configuration,
           mediafoundation_configuration,
#else
           configuration,
#endif // ACE_WIN32 || ACE_WIN64
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
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"\n"),
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
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
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
                                 signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
