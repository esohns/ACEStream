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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#define INITGUID
#endif // ACE_WIN32 || ACE_WIN64

#include <iostream>
#include <string>

#if defined (VALGRIND_SUPPORT)
#include "valgrind/valgrind.h"
#endif // VALGRIND_SUPPORT

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
#include "common_os_tools.h"

#include "common_error_tools.h"

#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#include "common_ui_tools.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

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
#include "test_u_tools.h"

#if defined (GTK_SUPPORT)
#include "test_u_audioeffect_callbacks.h"
#endif // GTK_SUPPORT
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

  std::string configuration_path =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_U_SUBDIRECTORY),
                                                      true); // configuration-

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : capture device [\"")
            << Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                                SND_PCM_STREAM_CAPTURE)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e[[STRING]]: effect [\"")
            << ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_SOX_DEFAULT_EFFECT_NAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> default}")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : source filename")
            << std::endl;
  std::string path = configuration_path;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#if defined (GTK_USE)
  UI_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GLADE_FILE);
#elif defined (WXWIDGETS_USE)
  UI_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_WXWIDGETS_XRC_FILE);
#endif // GTK_USE || WXWIDGETS_USE
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_SUPPORT)
  std::string UI_style_file = path;
  UI_style_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_style_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_DEFAULT_GTK_CSS_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i[[STRING]]: UI CSS file [\"")
            << UI_style_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> default}")
            << std::endl;
#endif // GTK_SUPPORT
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
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o[[STRING]]: target filename [")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("] {\"\" --> do not save}")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [STRING] : playback device [\"")
            << Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                                SND_PCM_STREAM_PLAYBACK)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
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
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-w          : use pipewire [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x          : use framework source [") // ? (directshow|mediafoundation) capture : WASAPI|waveIn
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-y          : use framework renderer [") // ? (directshow|mediafoundation) renderer : WASAPI|waveOut
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#else
                     std::string& captureDeviceIdentifier_out,
                     bool& captureDeviceIdentifierSet_out,
                     std::string& effect_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& sourceFileName_out,
                     std::string& UIFile_out,
#if defined (GTK_SUPPORT)
                     std::string& UICSSFile_out,
#endif // GTK_SUPPORT
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     enum Stream_MediaFramework_Type& mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& targetFileName_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                     std::string& playbackDeviceIdentifier_out,
#endif // ACE_WIN32 || ACE_WIN64
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& mute_out,
                     bool& printVersionAndExit_out
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                     ,bool& usePipewire_out
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     ,bool& useFrameworkSource_out,
                     bool& useFrameworkRenderer_out)
#else
                     )
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_U_SUBDIRECTORY),
                                                      true);

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#else
  captureDeviceIdentifier_out =
      Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                       SND_PCM_STREAM_CAPTURE);
  captureDeviceIdentifierSet_out = false;
  effect_out.clear ();
#endif // ACE_WIN32 || ACE_WIN64
  sourceFileName_out.clear ();
  UIFile_out = configuration_path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out +=
#if defined (GTK_USE)
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GLADE_FILE);
#elif defined (WXWIDGETS_USE)
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_WXWIDGETS_XRC_FILE);
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_SUPPORT)
  UICSSFile_out.clear ();
#endif // GTK_SUPPORT
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework_out = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
  targetFileName_out = path;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  playbackDeviceIdentifier_out =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_PLAYBACK);
#endif // ACE_WIN32 || ACE_WIN64
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  mute_out = false;
  printVersionAndExit_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  usePipewire_out = false;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  useFrameworkSource_out = false;
  useFrameworkRenderer_out = false;
#endif // ACE_WIN32 || ACE_WIN64

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("flo::s:tuv");
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  options_string += ACE_TEXT_ALWAYS_CHAR ("g::");
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("i::");
#endif // GTK_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  options_string += ACE_TEXT_ALWAYS_CHAR ("cmxy");
#else
  options_string += ACE_TEXT_ALWAYS_CHAR ("d:e::p:w");
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
        captureDeviceIdentifier_out =
            ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        captureDeviceIdentifierSet_out = true;
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
        sourceFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_SUPPORT)
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
#endif // GTK_SUPPORT
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
      case 'o':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          targetFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          targetFileName_out.clear ();
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      case 'p':
      {
        playbackDeviceIdentifier_out =
          ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
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
#else
      case 'w':
      {
        usePipewire_out = true;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'x':
      {
        useFrameworkSource_out = true;
        break;
      }
      case 'y':
      {
        useFrameworkRenderer_out = true;
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
  // *NOTE* core dump on SIGSEGV
  signals_out.sig_del (SIGSEGV);           // 11      /* Segmentation fault: Invalid memory reference */
  // *NOTE* don't care about SIGPIPE
  signals_out.sig_del (SIGPIPE);           // 12      /* Broken pipe: write to pipe with no readers */

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
do_initialize_directshow (const struct Stream_Device_Identifier& deviceIdentifier_in,
                          const struct Test_U_AudioEffect_DirectShow_Configuration& configuration_in,
                          IGraphBuilder*& IGraphBuilder_out,
                          IAMStreamConfig*& IAMStreamConfig_out,
                          struct _AMMediaType& captureMediaType_out,
                          bool useDirectShowSource_in,
                          bool useDirectShowDestination_in,
                          bool mute_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  IMediaFilter* media_filter_p = NULL;
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  Test_U_AudioEffect_DirectShowFilter_t* filter_p = NULL;
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
  IBaseFilter* filter_2 = NULL;
  std::wstring filter_name = STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L;
  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));

  // sanity check(s)
  ACE_ASSERT (!IGraphBuilder_out);
  ACE_ASSERT (!IAMStreamConfig_out);

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (captureMediaType_out);

  waveformatex_s.wFormatTag = STREAM_DEV_MIC_DEFAULT_FORMAT;
  waveformatex_s.nChannels = STREAM_DEV_MIC_DEFAULT_CHANNELS;
  waveformatex_s.nSamplesPerSec = STREAM_DEV_MIC_DEFAULT_SAMPLE_RATE;
  waveformatex_s.wBitsPerSample = STREAM_DEV_MIC_DEFAULT_BITS_PER_SAMPLE;
  waveformatex_s.nBlockAlign =
    (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
  waveformatex_s.nAvgBytesPerSec =
    (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
  //waveformatex_s.cbSize = 0;
  if (!Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (waveformatex_s,
                                                                 captureMediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx(), aborting\n")));
    goto error;
  } // end IF

  if (!useDirectShowSource_in)
    goto continue_3;

  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (deviceIdentifier_in,
                                                        CLSID_AudioInputDeviceCategory,
                                                        IGraphBuilder_out,
                                                        buffer_negotiation_p,
                                                        IAMStreamConfig_out,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (deviceIdentifier_in.identifier._string)));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (IAMStreamConfig_out);

  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  goto continue_3;

//continue_2:
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
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
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
  Common_IInitialize_T<struct Test_U_AudioEffect_DirectShow_FilterConfiguration>* iinitialize_p = filter_p;
  if (!iinitialize_p->initialize (configuration_in.filterConfiguration))
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
  filter_p = NULL;
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
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

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
  if (!useDirectShowDestination_in)
    goto continue_5;

  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  union Stream_MediaFramework_DirectSound_AudioEffectOptions effect_options;
  if (!Stream_Module_Decoder_Tools::loadAudioRendererGraph ((useDirectShowSource_in ? CLSID_AudioInputDeviceCategory : GUID_NULL),
                                                            captureMediaType_out,
                                                            media_type_s,
                                                            false,
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

  if (!Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder_out,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n")));
    goto error;
  } // end IF
  Stream_MediaFramework_DirectShow_Tools::clear (graph_configuration);

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

continue_5:
  return true;

error:
  Stream_MediaFramework_DirectShow_Tools::clear (graph_configuration);
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

  return false;
}

bool
do_initialize_mediafoundation (const struct Stream_Device_Identifier& deviceIdentifier_in,
                               struct Test_U_AudioEffect_MediaFoundation_Configuration& configuration_in,
                               IMFMediaSession*& session_out,
                               IMFMediaType*& captureMediaType_out,
                               bool initializeMediaFoundation_in,
                               bool useMediaFoundationSource_in,
                               bool mute_in,
                               Test_U_AudioEffect_MediaFoundation_Stream& stream_in,
                               bool makeSession_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;
  struct tWAVEFORMATEX* waveformatex_p = NULL;
  IMFAudioMediaType* media_type_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  IMFTopology* topology_p = NULL;
  TOPOID sample_grabber_id = 0, renderer_id = 0;
  IMFAttributes* attributes_p = NULL;
  std::string effect_options; // *TODO*

  // sanity check(s)
  ACE_ASSERT (deviceIdentifier_in.identifierDiscriminator == Stream_Device_Identifier::GUID);

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
  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
  //Stream_Module_Device_Tools::initialize (true);

  // initialize return value(s)
  if (captureMediaType_out)
  {
    captureMediaType_out->Release (); captureMediaType_out = NULL;
  } // end IF

  waveformatex_p =
    Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat (deviceIdentifier_in.identifier._guid);
  if (unlikely (!waveformatex_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectSound_Tools::getDeviceDriverFormat(%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceIdentifier_in.identifier._guid).c_str ())));
    goto error;
  } // end IF
  result = MFCreateAudioMediaType (waveformatex_p,
                                   &media_type_p);
  if (FAILED (result) || !media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaTypeFromRepresentation(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
    goto error;
  } // end IF
  CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
  captureMediaType_out = media_type_p;
  ACE_ASSERT (!configuration_in.mediaFoundationConfiguration.mediaType);
  configuration_in.mediaFoundationConfiguration.mediaType =
    Stream_MediaFramework_MediaFoundation_Tools::copy (captureMediaType_out);
  ACE_ASSERT (configuration_in.mediaFoundationConfiguration.mediaType);

  if (!makeSession_in)
    goto continue_4;

  if (!useMediaFoundationSource_in)
  {
    Test_U_AudioEffect_MediaFoundation_Target* writer_p =
      &const_cast<Test_U_AudioEffect_MediaFoundation_Target&> (stream_in.getR_4 ());
    if (!writer_p->initialize (configuration_in.mediaFoundationConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::initialize(), aborting\n")));
      goto error;
    } // end IF
    result = stream_in.QueryInterface (IID_PPV_ARGS (&media_source_p));
    if (FAILED (result) || !media_source_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Test_U_AudioEffect_MediaFoundation_Stream::QueryInterface(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF

  if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (deviceIdentifier_in,
                                                                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                media_source_p,
                                                                NULL,
                                                                topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceIdentifier_in.identifier._guid).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_source_p);
  ACE_ASSERT (topology_p);
  media_source_p->Release (); media_source_p = NULL;

  if (!useMediaFoundationSource_in)
    goto continue_3;

  if (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                              captureMediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF

continue_3:
  if (!Stream_Module_Decoder_Tools::loadAudioRendererTopology (deviceIdentifier_in.identifier._guid,
                                                               MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                               useMediaFoundationSource_in,
                                                               captureMediaType_out,
                                                               NULL,
                                                               NULL,
                                                               (mute_in ? -1 : 0),
                                                               GUID_NULL,
                                                               effect_options,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::loadAudioRendererTopology(), aborting\n")));
    goto error;
  } // end IF

  result = MFCreateAttributes (&attributes_p, 4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
  ACE_ASSERT (SUCCEEDED (result));
  //result_2 = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
  //ACE_ASSERT (SUCCEEDED (result_2));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  if (session_out)
  {
    session_out->Release (); session_out = NULL;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateMediaSession (attributes_p,
                                 &session_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (session_out);
  attributes_p->Release (); attributes_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 session_out,
                                                                 false, // is partial ?
                                                                 true)) // wait for completion ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

continue_4:
  return true;

error:
  if (captureMediaType_out)
  {
    captureMediaType_out->Release (); captureMediaType_out = NULL;
  } // end IF
  if (media_source_p)
    media_source_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (attributes_p)
    attributes_p->Release ();
  if (session_out)
  {
    session_out->Release (); session_out = NULL;
  } // end IF

  if (initializeMediaFoundation_in)
  {
    result = MFShutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF

  return false;
}

void
do_finalize_directshow (struct Test_U_AudioEffect_DirectShow_UI_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  ACE_UNUSED_ARG (CBData_in);

  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T iterator =
    CBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->streamConfiguration.end ());

  if ((*iterator).second.second->builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF
}

void
do_finalize_mediafoundation ()
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_mediafoundation"));

  HRESULT result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
}
#endif // ACE_WIN32 || ACE_WIN64

void
do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#else
         const std::string& captureDeviceIdentifier_in,
         const std::string& playbackDeviceIdentifier_in,
         const std::string& effectName_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& sourceFilename_in,
         const std::string& UIDefinitionFile_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         enum Stream_MediaFramework_Type mediaFramework_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& targetFilename_in,
         unsigned int statisticReportingInterval_in,
         bool mute_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
         bool usePipewire_in,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool useFrameworkSource_in,
         bool useFrameworkRenderer_in,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_U_AudioEffect_DirectShow_UI_CBData& directShowCBData_in,
         struct Test_U_AudioEffect_MediaFoundation_UI_CBData& mediaFoundationCBData_in,
#else
         struct Test_U_AudioEffect_UI_CBData& CBData_in,
#endif // ACE_WIN32 || ACE_WIN64
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

  struct Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration spectrumanalyzer_configuration;
  struct Stream_AllocatorConfiguration allocator_configuration;
  struct Common_AllocatorConfiguration* allocator_configuration_p = NULL;
  Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Common_IAsynchTask* itask_p = NULL;
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  struct Stream_SessionManager_Configuration session_manager_configuration_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_Stream directshow_stream;
  Test_U_AudioEffect_MediaFoundation_Stream mediafoundation_stream;

  Test_U_AudioEffect_DirectShow_SessionData directshow_session_data;
  Test_U_AudioEffect_MediaFoundation_SessionData mediafoundation_session_data;

  Test_U_DirectShow_SessionManager_t* directshow_session_manager_p =
    Test_U_DirectShow_SessionManager_t::SINGLETON_T::instance ();
  directshow_session_manager_p->setR (directshow_session_data,
                                      directshow_stream.id ());
  Test_U_MediaFoundation_SessionManager_t* mediafoundation_session_manager_p =
    Test_U_MediaFoundation_SessionManager_t::SINGLETON_T::instance ();
  mediafoundation_session_manager_p->setR (mediafoundation_session_data,
                                           mediafoundation_stream.id ());

  Test_U_AudioEffect_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                                 &heap_allocator,     // heap allocator handle
                                                                                 true);               // block ?
  Test_U_AudioEffect_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                                           &heap_allocator,     // heap allocator handle
                                                                                           true);               // block ?
#else
  Test_U_AudioEffect_SessionData session_data;

  Test_U_SessionManager_t* session_manager_p =
    Test_U_SessionManager_t::SINGLETON_T::instance ();
  session_manager_p->setR (session_data);

  Test_U_AudioEffect_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                           &heap_allocator,     // heap allocator handle
                                                           true);               // block ?
#endif // ACE_WIN32 || ACE_WIN64
  bool result = false;
  Stream_IStream_t* istream_p = NULL;
  Stream_IStreamControlBase* istream_control_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2; // directshow target module
  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_3; // renderer module
  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_4; // file writer module
  struct Test_U_AudioEffect_DirectShow_StreamConfiguration directshow_stream_configuration;
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_2; // mediafoundation target target module
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_3; // renderer module
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_4; // file writer module
  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_5; // resampler module
  struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration;
  std::string renderer_modulename_string;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      istream_p = &directshow_stream;
      istream_control_p = &directshow_stream;
      directshow_stream_configuration.capturer =
        (useFrameworkSource_in ? STREAM_DEVICE_CAPTURER_DIRECTSHOW
                               : STREAM_DEVICE_CAPTURER_WASAPI);
                               //: STREAM_DEVICE_CAPTURER_WAVEIN);
      directshow_stream_configuration.renderer =
        (useFrameworkRenderer_in ? STREAM_DEVICE_RENDERER_DIRECTSHOW
                                 : STREAM_DEVICE_RENDERER_WASAPI);
                                 //: STREAM_DEVICE_RENDERER_WAVEOUT);
#if defined (GTK_USE)
      directshow_stream_configuration.UIFramework = COMMON_UI_FRAMEWORK_GTK;
#endif // GTK_USE
      if (showConsole_in)
      {
        directshow_stream_configuration.sourceType = AUDIOEFFECT_SOURCE_DEVICE;
        directshow_stream_configuration.UIFramework = COMMON_UI_FRAMEWORK_CONSOLE;
      } // end IF

#if defined (GTKGL_SUPPORT)
      directShowCBData_in.OpenGLInstructions = &directshow_stream.instructions_;
      directShowCBData_in.OpenGLInstructionsLock =
        &directshow_stream.instructionsLock_;
#endif // GTKGL_SUPPORT
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      istream_p = &mediafoundation_stream;
      istream_control_p = &mediafoundation_stream;
      mediafoundation_stream_configuration.capturer =
        (useFrameworkSource_in ? STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION
                               : STREAM_DEVICE_CAPTURER_WASAPI);
      mediafoundation_stream_configuration.renderer =
        (useFrameworkRenderer_in ? STREAM_DEVICE_RENDERER_MEDIAFOUNDATION
                                 : STREAM_DEV_AUDIO_DEFAULT_RENDERER);
#if defined (GTKGL_SUPPORT)
      mediaFoundationCBData_in.OpenGLInstructions =
        &mediafoundation_stream.instructions_;
      mediaFoundationCBData_in.OpenGLInstructionsLock =
        &mediafoundation_stream.instructionsLock_;
#endif // GTKGL_SUPPORT
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
#if defined (GTKGL_SUPPORT)
  CBData_in.OpenGLInstructions = &stream.instructions_;
  CBData_in.OpenGLInstructionsLock = &stream.instructionsLock_;
#endif // GTKGL_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  session_manager_configuration_s.stream = istream_control_p;

  ACE_Time_Value one_second (1, 0);
#if defined (GTK_SUPPORT)
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  int result_2 = -1;
#endif // GTK_SUPPORT
//  const Stream_Module_t* module_p = NULL;
//  Test_U_AudioEffect_IDispatch_t* idispatch_p = NULL;
  struct Stream_ModuleConfiguration module_configuration;

  // step0a: initialize configuration
  allocator_configuration_p = &allocator_configuration;
  ACE_ASSERT (allocator_configuration_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_EventHandler directshow_ui_event_handler (
                                                                          &directShowCBData_in
                                                                         );
  Test_U_AudioEffect_DirectShow_Module_EventHandler_Module directshow_event_handler (istream_p,
                                                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_AudioEffect_MediaFoundation_EventHandler mediafoundation_ui_event_handler (
                                                                                    &mediaFoundationCBData_in
                                                                                   );
  Test_U_AudioEffect_MediaFoundation_Module_EventHandler_Module mediafoundation_event_handler (istream_p,
                                                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
#else
  session_manager_p->initialize (session_manager_configuration_s);

  Test_U_AudioEffect_EventHandler ui_event_handler (
                                                    &CBData_in
                                                   );
  Test_U_AudioEffect_Module_EventHandler_Module event_handler (istream_p,
                                                               ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_iterator;
  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration; // capture
  ALSA_configuration.asynch = STREAM_LIB_ALSA_CAPTURE_DEFAULT_ASYNCH;
  if (Common_Error_Tools::inDebugSession ()) // gdb seems not to play too well with signals
    ALSA_configuration.asynch = false;
  ALSA_configuration.bufferSize = STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_SIZE;
  ALSA_configuration.bufferTime = STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_TIME;
  ALSA_configuration.periods = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIODS;
  ALSA_configuration.periodSize = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_SIZE;
  ALSA_configuration.periodTime = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_TIME;
  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration_2; // playback
  if (Common_Error_Tools::inDebugSession ()) // gdb seems not to play too well with signals
    ALSA_configuration_2.asynch = false;
  ALSA_configuration_2.rateResample = true;
  struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration modulehandler_configuration_2; // renderer module
  struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration modulehandler_configuration_3; // file writer module
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
    {
      directshow_session_manager_p->initialize (session_manager_configuration_s);

      directshow_modulehandler_configuration.allocatorConfiguration =
        allocator_configuration_p;
      directshow_modulehandler_configuration.delayConfiguration =
        &directShowConfiguration_in.delayConfiguration;
      //directShowConfiguration_in.delayConfiguration.averageTokensPerInterval =
      //  static_cast<ACE_UINT64> ((static_cast<float> (4 * 44100) * static_cast<float> (STREAM_MISC_DEFAULT_DELAY_AUDIO_INTERVAL_US)) / 1000000.0F);
      //directShowConfiguration_in.delayConfiguration.mode =
      //  STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES;
      //directShowConfiguration_in.delayConfiguration.interval =
      //  ACE_Time_Value (0, STREAM_MISC_DEFAULT_DELAY_AUDIO_INTERVAL_US);
      switch (directshow_stream_configuration.capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          directshow_modulehandler_configuration.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          directshow_modulehandler_configuration.deviceIdentifier.identifier._id =
            0; // *TODO*: -1 means WAVE_MAPPER; 0 may not be the default device id
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          directshow_modulehandler_configuration.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          directshow_modulehandler_configuration.deviceIdentifier.identifier._guid =
            Stream_MediaFramework_DirectSound_Tools::getDefaultDevice (true); // capture
          break;
        }
        case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
        {
          directshow_modulehandler_configuration.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          directshow_modulehandler_configuration.deviceIdentifier.identifier._id =
            0; // *TODO*: -1 means WAVE_MAPPER; 0 may not be the default device id
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer type (was: %d), returning\n"),
                      directshow_stream_configuration.capturer));
          goto error;
        }
      } // end SWITCH
      directshow_modulehandler_configuration.filterConfiguration =
        &directShowConfiguration_in.filterConfiguration;
      directshow_modulehandler_configuration.generatorConfiguration =
        &directShowConfiguration_in.generatorConfiguration;
      directshow_modulehandler_configuration.messageAllocator =
        &directshow_message_allocator;
#if defined (SOX_SUPPORT)
      directshow_modulehandler_configuration.manageSoX = true;
#endif // SOX_SUPPORT
      directshow_modulehandler_configuration.mute = mute_in;
      directshow_modulehandler_configuration.printProgressDot =
        UIDefinitionFile_in.empty ();
      directshow_modulehandler_configuration.statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      directshow_modulehandler_configuration.subscriber =
        &directshow_ui_event_handler;
      if (!sourceFilename_in.empty ())
        directshow_modulehandler_configuration.fileIdentifier.identifier =
          sourceFilename_in;
      directshow_modulehandler_configuration.spectrumAnalyzerConfiguration =
        &spectrumanalyzer_configuration;

      directShowConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                 directshow_modulehandler_configuration,
                                                                 directshow_stream_configuration);

      directshow_modulehandler_iterator =
        directShowConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directShowConfiguration_in.streamConfiguration.end ());

      directshow_modulehandler_configuration_2 =
        directshow_modulehandler_configuration;
      //directshow_modulehandler_configuration_2.deviceIdentifier.identifierDiscriminator =
      //  Stream_Device_Identifier::ID;
      //directshow_modulehandler_configuration_2.deviceIdentifier.identifier._id =
      //  (mute_in ? -1 : 0);
      //directshow_modulehandler_configuration_2.passData = false;
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_2)));

      directshow_modulehandler_configuration_3 =
        directshow_modulehandler_configuration;
      switch (directshow_stream_configuration.renderer)
      {
        case STREAM_DEVICE_RENDERER_WAVEOUT:
        {
          directshow_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          directshow_modulehandler_configuration_3.deviceIdentifier.identifier._id =
            (mute_in ? -1 : 0); // *TODO*: -1 means WAVE_MAPPER
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING);
          break;
        }
        case STREAM_DEVICE_RENDERER_WASAPI:
        {
          directshow_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          directshow_modulehandler_configuration_3.deviceIdentifier.identifier._guid =
            (mute_in ? GUID_NULL
                     : Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (0,
                                                                                               false)); // playback

          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING);
          break;
        }
        case STREAM_DEVICE_RENDERER_DIRECTSHOW:
        {
          directshow_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          directshow_modulehandler_configuration_3.deviceIdentifier.identifier._id =
            (mute_in ? -1 : 0); // *TODO*: -1 means WAVE_MAPPER
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown renderer type (was: %d), returning\n"),
                      directshow_stream_configuration.renderer));
          goto error;
        }
      } // end SWITCH
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (renderer_modulename_string,
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_3)));

      directshow_modulehandler_configuration_4 =
        directshow_modulehandler_configuration;
      directshow_modulehandler_configuration_4.fileIdentifier.clear ();
      if (!targetFilename_in.empty ())
        directshow_modulehandler_configuration_4.fileIdentifier.identifier =
          targetFilename_in;
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_4)));

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_session_manager_p->initialize (session_manager_configuration_s);

      mediafoundation_modulehandler_configuration.allocatorConfiguration =
        allocator_configuration_p;
      //mediaFoundationConfiguration_in.delayConfiguration.averageTokensPerInterval =
      //  8;
      //mediaFoundationConfiguration_in.delayConfiguration.mode =
      //  STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES;
      //mediaFoundationConfiguration_in.delayConfiguration.interval =
      //  ACE_Time_Value (0, (1000000.0F / (float)44100));
      mediafoundation_modulehandler_configuration.delayConfiguration =
        &mediaFoundationConfiguration_in.delayConfiguration;
      switch (mediafoundation_stream_configuration.capturer)
      {
        case STREAM_DEVICE_CAPTURER_WAVEIN:
        {
          mediafoundation_modulehandler_configuration.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          mediafoundation_modulehandler_configuration.deviceIdentifier.identifier._id =
            0; // *TODO*: 0 may not be the default device id
          break;
        }
        case STREAM_DEVICE_CAPTURER_WASAPI:
        {
          mediafoundation_modulehandler_configuration.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          mediafoundation_modulehandler_configuration.deviceIdentifier.identifier._guid =
            Stream_MediaFramework_DirectSound_Tools::getDefaultDevice (true); // capture
          break;
        }
        case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
        {
          mediafoundation_modulehandler_configuration.deviceIdentifier =
            Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown capturer type (was: %d), returning\n"),
                      mediafoundation_stream_configuration.capturer));
          goto error;
        }
      } // end SWITCH
      mediafoundation_modulehandler_configuration.generatorConfiguration =
        &mediaFoundationConfiguration_in.generatorConfiguration;
      mediafoundation_modulehandler_configuration.mediaFoundationConfiguration =
        &mediaFoundationConfiguration_in.mediaFoundationConfiguration;
#if defined (SOX_SUPPORT)
      mediafoundation_modulehandler_configuration.manageSoX = true;
#endif // SOX_SUPPORT
      mediafoundation_modulehandler_configuration.mute = mute_in;
      mediafoundation_modulehandler_configuration.printProgressDot =
        UIDefinitionFile_in.empty ();
      mediafoundation_modulehandler_configuration.statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      mediafoundation_modulehandler_configuration.subscriber =
        &mediafoundation_ui_event_handler;
      if (!sourceFilename_in.empty ())
        mediafoundation_modulehandler_configuration.fileIdentifier.identifier =
          sourceFilename_in;
      mediafoundation_modulehandler_configuration.spectrumAnalyzerConfiguration =
        &spectrumanalyzer_configuration;

      mediaFoundationConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                      mediafoundation_modulehandler_configuration,
                                                                      mediafoundation_stream_configuration);
      mediafoundation_modulehandler_iterator =
        mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediaFoundationConfiguration_in.streamConfiguration.end ());

      mediafoundation_modulehandler_configuration_2 =
        mediafoundation_modulehandler_configuration;
      //mediafoundation_modulehandler_configuration_2.deviceIdentifier.identifierDiscriminator =
      //  Stream_Device_Identifier::ID;
      //mediafoundation_modulehandler_configuration_2.deviceIdentifier.identifier._id =
      //  (mute_in ? -1 : 0);
      //mediafoundation_modulehandler_configuration_2.passData = false;
      mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING),
                                                                                  std::make_pair (&module_configuration,
                                                                                                  &mediafoundation_modulehandler_configuration_2)));

      mediafoundation_modulehandler_configuration_3 =
        mediafoundation_modulehandler_configuration;
      switch (mediafoundation_stream_configuration.renderer)
      {
        case STREAM_DEVICE_RENDERER_WAVEOUT:
        {
          mediafoundation_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::ID;
          mediafoundation_modulehandler_configuration_3.deviceIdentifier.identifier._id =
            (mute_in ? -1 : 0); // *TODO*: -1 means WAVE_MAPPER
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING);
          break;
        }
        case STREAM_DEVICE_RENDERER_WASAPI:
        {
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING);
          // *WARNING*: falls through !
        }
        case STREAM_DEVICE_RENDERER_MEDIAFOUNDATION:
        {
          mediafoundation_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          mediafoundation_modulehandler_configuration_3.deviceIdentifier.identifier._guid =
            (mute_in ? GUID_NULL
                     : Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (0,
                                                                                               false)); // playback
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown renderer type (was: %d), returning\n"),
                      mediafoundation_stream_configuration.renderer));
          goto error;
        }
      } // end SWITCH
      mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (renderer_modulename_string,
                                                                                  std::make_pair (&module_configuration,
                                                                                                  &mediafoundation_modulehandler_configuration_3)));

      mediafoundation_modulehandler_configuration_4 =
        mediafoundation_modulehandler_configuration;
      mediafoundation_modulehandler_configuration_4.fileIdentifier.clear ();
      if (!targetFilename_in.empty ())
        mediafoundation_modulehandler_configuration_4.fileIdentifier.identifier =
          targetFilename_in;
      mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING),
                                                                                  std::make_pair (&module_configuration,
                                                                                                  &mediafoundation_modulehandler_configuration_4)));

      mediafoundation_modulehandler_configuration_5 =
        mediafoundation_modulehandler_configuration;
      mediafoundation_modulehandler_configuration_5.outputFormat = NULL;
      mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING),
                                                                                  std::make_pair (&module_configuration,
                                                                                                  &mediafoundation_modulehandler_configuration_5)));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      goto error;
    }
  } // end SWITCH
#else
  modulehandler_configuration.allocatorConfiguration =
    allocator_configuration_p;
  modulehandler_configuration.ALSAConfiguration = &ALSA_configuration;
  stream_configuration.allocatorConfiguration = allocator_configuration_p;
//  modulehandler_configuration.concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  modulehandler_configuration.delayConfiguration =
    &configuration_in.delayConfiguration;
  //configuration_in.delayConfiguration.averageTokensPerInterval =
  //  8;
//  configuration_in.delayConfiguration.mode =
//    STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES;
  //configuration_in.delayConfiguration.interval =
  //  ACE_Time_Value (0, (1.0F / (float)44100) * 1000000.0F);
  modulehandler_configuration.deviceIdentifier.identifier =
    captureDeviceIdentifier_in;
  modulehandler_configuration.effect = effectName_in;
  modulehandler_configuration.generatorConfiguration =
    &configuration_in.generatorConfiguration;
  modulehandler_configuration.messageAllocator = &message_allocator;
  modulehandler_configuration.mute = mute_in;
  modulehandler_configuration.printProgressDot = UIDefinitionFile_in.empty ();
  modulehandler_configuration.statisticReportingInterval =
    ACE_Time_Value (statisticReportingInterval_in, 0);
  modulehandler_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
  modulehandler_configuration.subscriber = &ui_event_handler;
  if (!sourceFilename_in.empty ())
    modulehandler_configuration.fileIdentifier.identifier =
      sourceFilename_in;
  modulehandler_configuration.spectrumAnalyzerConfiguration =
    &spectrumanalyzer_configuration;

  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  modulehandler_iterator =
      configuration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR(""));
  ACE_ASSERT(modulehandler_iterator != configuration_in.streamConfiguration.end());

  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.ALSAConfiguration = &ALSA_configuration_2;
  //Stream_MediaFramework_ALSA_Tools::listCards ();
  modulehandler_configuration_2.deviceIdentifier.identifier =
    playbackDeviceIdentifier_in;
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_2)));

  modulehandler_configuration_3 = modulehandler_configuration;
  modulehandler_configuration_3.fileIdentifier.clear ();
  if (!targetFilename_in.empty ())
    modulehandler_configuration_3.fileIdentifier.identifier =
      targetFilename_in;
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_WAV_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_3)));
#endif // ACE_WIN32 || ACE_WIN64

  // ********************** stream configuration data **************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_stream_configuration.allocatorConfiguration =
        &allocator_configuration;
      directshow_stream_configuration.messageAllocator =
        &directshow_message_allocator;
      directshow_stream_configuration.module =
        (!UIDefinitionFile_in.empty () ? &directshow_event_handler
                                       : NULL);
      directshow_stream_configuration.printFinalReport = true;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_stream_configuration.allocatorConfiguration =
        &allocator_configuration;
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
      goto error;
    }
  } // end SWITCH
#else
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
      (!UIDefinitionFile_in.empty () ? &event_handler
                                     : NULL);
#if defined (GTK_USE)
  stream_configuration.UIFramework = COMMON_UI_FRAMEWORK_GTK;
#endif // GTK_USE
  if (usePipewire_in)
  {
    modulehandler_configuration.concurrency =
      STREAM_HEADMODULECONCURRENCY_ACTIVE;

    stream_configuration.capturer = STREAM_DEVICE_CAPTURER_PIPEWIRE;
    // *TODO*: remove these temporary settings
    stream_configuration.format.format = SND_PCM_FORMAT_FLOAT_LE;
    stream_configuration.sourceType = AUDIOEFFECT_SOURCE_DEVICE;
    stream_configuration.UIFramework = COMMON_UI_FRAMEWORK_CONSOLE;

    configuration_in.signalHandlerConfiguration.stream = &stream;
  } // end IF
  stream_configuration.printFinalReport = true;
#endif // ACE_WIN32 || ACE_WIN64

  // intialize timers
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  timer_configuration.taskType =
    ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_WASAPI_CAPTURE_DEFAULT_TASKNAME);
#endif // ACE_WIN32 || ACE_WIN64
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
        do_initialize_directshow ((*directshow_modulehandler_iterator).second.second->deviceIdentifier,
                                  *directShowCBData_in.configuration,
                                  (*directshow_modulehandler_iterator).second.second->builder,
                                  directShowCBData_in.streamConfiguration,
                                  directshow_stream_configuration.format,
                                  useFrameworkSource_in, // use DirectShow source ? : WASAPI
                                  directshow_stream_configuration.renderer == STREAM_DEVICE_RENDERER_DIRECTSHOW,
                                  mute_in);
      if (!result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to do_initialize_directshow(), returning\n")));
        goto error;
      } // end IF
      if (useFrameworkSource_in) // use DirectShow source ?
      {
        ACE_ASSERT ((*directshow_modulehandler_iterator).second.second->builder);
        ACE_ASSERT (directShowCBData_in.streamConfiguration);
      } // end IF
      if (directshow_stream_configuration.renderer == STREAM_DEVICE_RENDERER_DIRECTSHOW)
      {
        ACE_ASSERT ((*directshow_modulehandler_iterator).second.second->builder);
      } // end IF
      Stream_MediaFramework_DirectShow_Tools::copy (directshow_stream_configuration.format,
                                                    (*directshow_modulehandler_iterator).second.second->outputFormat);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      result =
        do_initialize_mediafoundation ((*mediafoundation_modulehandler_iterator).second.second->deviceIdentifier,
                                       *mediaFoundationCBData_in.configuration,
                                       (*mediafoundation_modulehandler_iterator).second.second->session,
                                       mediafoundation_stream_configuration.format,
                                       true, // initialize MediaFoundation framework ?
                                       useFrameworkSource_in, // use MediaFoundation source ? : WASAPI
                                       mute_in,
                                       mediafoundation_stream,
                                       UIDefinitionFile_in.empty ()); // make session ?
      (*mediafoundation_modulehandler_iterator).second.second->outputFormat =
        Stream_MediaFramework_MediaFoundation_Tools::copy (mediafoundation_stream_configuration.format);
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second->outputFormat);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      goto error;
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
      signalHandler_in.initialize (directShowConfiguration_in.signalHandlerConfiguration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      signalHandler_in.initialize (mediaFoundationConfiguration_in.signalHandlerConfiguration);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      goto error;
    }
  } // end SWITCH
#else
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

  // step1a: start ui event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    //Common_UI_GTK_Manager_t* gtk_manager_p =
    //  COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
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
        goto error;
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
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
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
        goto error;
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
  } // end ELSE

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
      result =
        directshow_stream.remove (&directshow_event_handler,
                                  true,   // lock ?
                                  false); // reset ?
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      result =
        mediafoundation_stream.remove (&mediafoundation_event_handler,
                                       true,   // lock ?
                                       false); // reset ?
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      goto error;
    }
  } // end SWITCH
#else
  result = stream.remove (&event_handler,
                          true,   // lock ?
                          false); // reset ?
#endif // ACE_WIN32 || ACE_WIN64
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base::remove(), continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

error:
  if (!UIDefinitionFile_in.empty () && itask_p)
    itask_p->stop (true,  // wait ?
                   true); // high priority ?
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result = -1;
#endif // ACE_WIN32 || ACE_WIN64

  // step0: initialize
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: on Windows, initialize ACE...
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
  Common_Tools::initialize (true,   // COM ?
                            false); // RNG ?
#else
#if defined (LIBPIPEWIRE_SUPPORT)
  pw_init (&argc_in, &argv_in);
#endif // LIBPIPEWIRE_SUPPORT
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  Common_File_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (argv_in[0]));

  std::string configuration_path =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_U_SUBDIRECTORY),
                                                      true);

  // step1a set defaults
  //unsigned int buffer_size = TEST_U_STREAM_AUDIOEFFECT_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#else
  std::string capture_device_identifier_string =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_CAPTURE);
  bool capture_device_identifier_set_b = false;
  std::string effect_name;
#endif // ACE_WIN32 || ACE_WIN64
  std::string path;
  std::string source_filename;
  path = configuration_path;
  std::string UI_definition_file = path;
  UI_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#if defined (GTK_USE)
  UI_definition_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_GLADE_FILE);
#elif defined (WXWIDGETS_USE)
  UI_definition_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_WXWIDGETS_XRC_FILE);
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_USE)
  std::string UI_CSS_file;
#endif // GTK_USE
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
  std::string target_filename = path;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  std::string playback_device_identifier_string =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_PLAYBACK);
#endif // ACE_WIN32 || ACE_WIN64
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool mute = false;
  bool print_version_and_exit = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  bool use_pipewire_b = false;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool use_framework_source = false;
  bool use_framework_renderer = false;
#endif // ACE_WIN32 || ACE_WIN64

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#else
                            capture_device_identifier_string,
                            capture_device_identifier_set_b,
                            effect_name,
#endif // ACE_WIN32 || ACE_WIN64
                            source_filename,
                            UI_definition_file,
#if defined (GTK_SUPPORT)
                            UI_CSS_file,
#endif // GTK_SUPPORT
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                            target_filename,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                            playback_device_identifier_string,
#endif // ACE_WIN32 || ACE_WIN64
                            statistic_reporting_interval,
                            trace_information,
                            mute,
                            print_version_and_exit
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                            ,use_pipewire_b
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            ,use_framework_source,
                            use_framework_renderer))
#else
                            ))
#endif // ACE_WIN32 || ACE_WIN64
  {
    do_printUsage (Common_File_Tools::executable);

    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
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
  if ((!UI_definition_file.empty () &&
       !Common_File_Tools::isReadable (UI_definition_file))
#if defined (GTK_USE)
      || (!UI_CSS_file.empty () &&
          !Common_File_Tools::isReadable (UI_CSS_file)))
#else
     )
#endif // GTK_USE
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));

    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  //if (run_stress_test)
  //  action_mode = Net_Client_TimeoutHandler::ACTION_STRESS;

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
  directshow_configuration.generatorConfiguration.amplitude = 1.0;
  directshow_configuration.generatorConfiguration.frequency =
    TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_FREQUENCY_D;
#if defined (LIBNOISE_SUPPORT)
  directshow_configuration.generatorConfiguration.step =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_STEP;
  directshow_configuration.generatorConfiguration.x =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_X;
  directshow_configuration.generatorConfiguration.y =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Y;
  directshow_configuration.generatorConfiguration.z =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Z;
#endif // LIBNOISE_SUPPORT
  directshow_configuration.generatorConfiguration.type =
    TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_TYPE;
  struct Test_U_AudioEffect_DirectShow_UI_CBData directshow_ui_cb_data;

  struct Test_U_AudioEffect_MediaFoundation_Configuration mediafoundation_configuration;
  mediafoundation_configuration.generatorConfiguration.amplitude = 1.0;
  mediafoundation_configuration.generatorConfiguration.frequency =
    TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_FREQUENCY_D;
#if defined (LIBNOISE_SUPPORT)
  mediafoundation_configuration.generatorConfiguration.step =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_STEP;
  mediafoundation_configuration.generatorConfiguration.x =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_X;
  mediafoundation_configuration.generatorConfiguration.y =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Y;
  mediafoundation_configuration.generatorConfiguration.z =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Z;
#endif // LIBNOISE_SUPPORT
  mediafoundation_configuration.generatorConfiguration.type =
    TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_TYPE;
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
#if GTK_CHECK_VERSION (3,0,0)
      if (!UI_CSS_file.empty ())
        directshow_configuration.GTKConfiguration.CSSProviders[UI_CSS_file] =
          NULL;
#endif // GTK_CHECK_VERSION (3,0,0)
      gtk_configuration_p = &directshow_configuration.GTKConfiguration;
#endif // GTK_SUPPORT
      cb_data_base_p = &directshow_ui_cb_data;
      cb_data_base_p->mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data.configuration = &mediafoundation_configuration;
#if defined (GTK_SUPPORT)
      mediafoundation_ui_cb_data.progressData.state = &state_r;

      mediafoundation_configuration.GTKConfiguration.argc = argc_in;
      mediafoundation_configuration.GTKConfiguration.argv = argv_in;
      mediafoundation_configuration.GTKConfiguration.CBData = &mediafoundation_ui_cb_data;
      mediafoundation_configuration.GTKConfiguration.eventHooks.finiHook =
          idle_finalize_UI_cb;
      mediafoundation_configuration.GTKConfiguration.eventHooks.initHook =
          idle_initialize_UI_cb;
      mediafoundation_configuration.GTKConfiguration.definition = &gtk_ui_definition;
#if GTK_CHECK_VERSION (3,0,0)
      if (!UI_CSS_file.empty ())
        mediafoundation_configuration.GTKConfiguration.CSSProviders[UI_CSS_file] =
          NULL;
#endif // GTK_CHECK_VERSION (3,0,0)
      gtk_configuration_p = &mediafoundation_ui_cb_data.configuration->GTKConfiguration;
#endif // GTK_SUPPORT
      cb_data_base_p = &mediafoundation_ui_cb_data;
      cb_data_base_p->mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  media_framework_e));

      Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *PORTABILITY*: on Windows, finalize ACE...
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
  configuration.generatorConfiguration.amplitude = 1.0;
  configuration.generatorConfiguration.frequency =
    TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_FREQUENCY_D;
#if defined (LIBNOISE_SUPPORT)
  configuration.generatorConfiguration.step =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_STEP;
  configuration.generatorConfiguration.x =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_X;
  configuration.generatorConfiguration.y =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Y;
  configuration.generatorConfiguration.z =
    STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Z;
#endif // LIBNOISE_SUPPORT
  configuration.generatorConfiguration.type =
    TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_TYPE;
  struct Test_U_AudioEffect_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
  ui_cb_data.switchCaptureDevice = !capture_device_identifier_set_b;

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

#if defined (GTK_SUPPORT)
  cb_data_base_p->UIState = &state_r;
#endif // GTK_SUPPORT

  // step1d: initialize logging and/or tracing
//  Common_Logger_Queue_t logger;
// logger.initialize (&state_r.logQueue,
//                    &state_r.logQueueLock);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        Common_File_Tools::executable);
  if (!Common_Log_Tools::initialize (Common_File_Tools::executable,            // program name
                                     log_file_name,                            // log file name
                                     false,                                    // log to syslog ?
                                     false,                                    // trace messages ?
                                     trace_information,                        // debug messages ?
                                     NULL))                                    // (ui-) logger ?
//                                            (UI_definition_file.empty () ? NULL
//                                                                         : &logger))) // (ui-) logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (signal_set,
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GTK_SUPPORT)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_SUPPORT
  Test_U_AudioEffect_SignalHandler signal_handler;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
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

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
#if defined (GTK_SUPPORT)
  ACE_ASSERT (gtk_configuration_p);
  if (!UI_definition_file.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (*gtk_configuration_p);
#endif // GTK_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#else
           capture_device_identifier_string,
           playback_device_identifier_string,
           effect_name,
#endif // ACE_WIN32 || ACE_WIN64
           source_filename,
           UI_definition_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
           target_filename,
           statistic_reporting_interval,
           mute,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
           use_pipewire_b,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           use_framework_source,
           use_framework_renderer,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_ui_cb_data,
           mediafoundation_ui_cb_data,
#else
           ui_cb_data,
#endif // ACE_WIN32 || ACE_WIN64
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
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (LIBPIPEWIRE_SUPPORT)
  pw_deinit ();
#endif // LIBPIPEWIRE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: on Windows, finalize ACE...
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
