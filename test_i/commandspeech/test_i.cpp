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
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
#include "mtype.h"
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#define INITGUID
#endif // ACE_WIN32 || ACE_WIN64

#if defined (SOX_SUPPORT)
#include "sox.h"
#endif // SOX_SUPPORT

#include <iostream>
#include <string>

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

#include "common_input_manager.h"
#include "common_input_tools.h"

#include "common_logger_queue.h"
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
#include "stream_lib_directsound_common.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_guids.h"
#include "stream_lib_tools.h"

#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#include "stream_dev_tools.h"

#include "stream_dec_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_defines.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT
#include "test_i_session_message.h"
#include "test_i_commandspeech_common.h"
#include "test_i_commandspeech_defines.h"
#include "test_i_modules.h"
#include "test_i_eventhandler.h"
#include "test_i_signalhandler.h"
#include "test_i_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("CommandSpeechStream");

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
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  std::string voice = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_VOICE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [STRING] : voice [\"")
            << voice
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  std::string voice_directory = path;
  voice_directory += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  voice_directory += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_VOICE_DIRECTORY);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [STRING] : voice directory [\"")
            << voice_directory
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [INTEGER]: device id [")
            << 0
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : device [\"")
            << Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                                SND_PCM_STREAM_PLAYBACK)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : text file {\"\" --> stdin}")
            << std::endl;
#if defined (GTK_SUPPORT) || defined (WXWIDGETS_SUPPORT)
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
#endif // GTK_SUPPORT || WXWIDGETS_SUPPORT
#if defined (GTK_SUPPORT)
  std::string UI_style_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_CSS_FILE);
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
  path += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o[[STRING]]: target filename {\"\" --> ")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
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
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
                     std::string& voice_out,
                     std::string& voiceDirectory_out,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     unsigned int& deviceIdentifier_out,
#else
                     std::string& deviceIdentifier_out,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
                     std::string& sourceFileName_out,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
                     std::string& UIFile_out,
#if defined (GTK_SUPPORT)
                     std::string& UICSSFile_out,
#endif // GTK_SUPPORT
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     enum Stream_MediaFramework_Type& mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& targetFileName_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& mute_out,
                     bool& printVersionAndExit_out
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     ,bool& useFrameworkSource_out,
                     bool& useFrameworkRenderer_out
#endif // ACE_WIN32 || ACE_WIN64
                     )
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  // initialize results
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  voice_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_VOICE);
  voiceDirectory_out = configuration_path;
  voiceDirectory_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  voiceDirectory_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_VOICE_DIRECTORY);
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  deviceIdentifier_out = 0;
#else
  deviceIdentifier_out =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_PLAYBACK);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  sourceFileName_out.clear ();
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
  UIFile_out = configuration_path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
#if defined (GTK_SUPPORT)
  UICSSFile_out.clear ();
#endif // GTK_SUPPORT
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework_out = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  targetFileName_out.clear ();
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  mute_out = false;
  printVersionAndExit_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  useFrameworkSource_out = false;
  useFrameworkRenderer_out = false;
#endif // ACE_WIN32 || ACE_WIN64

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("d:f:lo::s:tuv");
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("b:c:");
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (GTK_SUPPORT) || defined (WXWIDGETS_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("g::");
#endif // GTK_SUPPORT || WXWIDGETS_SUPPORT
#if defined (GTK_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("i::");
#endif // GTK_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  options_string += ACE_TEXT_ALWAYS_CHAR ("mxy");
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
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
      case 'b':
      {
        voice_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'c':
      {
        voiceDirectory_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
      case 'd':
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        converter >> deviceIdentifier_out;
#else
        deviceIdentifier_out =
          ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
#endif // ACE_WIN32 || ACE_WIN64
        break;
      }
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
      case 'f':
      {
        sourceFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (GTK_SUPPORT) || defined (WXWIDGETS_SUPPORT)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
#endif // GTK_SUPPORT || WXWIDGETS_SUPPORT
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
          UICSSFile_out +=
            ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
          UICSSFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
          UICSSFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_CSS_FILE);
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
        {
          std::string path = Common_File_Tools::getTempDirectory ();
          path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
          path += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
          targetFileName_out = path;
        } // end ELSE
        break;
      }
      case 's':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
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
  result = signals_out.sig_add (SIGINT);            // 2       /* interrupt */
  ACE_ASSERT (result == 0);
  result = signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
  ACE_ASSERT (result == 0);
  result = signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
  ACE_ASSERT (result == 0);
  //result = signals_out.sig_add (SIGSEGV);           // 11      /* segment violation */
  //ACE_ASSERT (result == 0);
  result = signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
  ACE_ASSERT (result == 0);
  result = signals_out.sig_add (SIGBREAK);          // 21      /* Ctrl-Break sequence */
  ACE_ASSERT (result == 0);
  result = signals_out.sig_add (SIGABRT);           // 22      /* abnormal termination triggered by abort call */
  ACE_ASSERT (result == 0);
  result = signals_out.sig_add (SIGABRT_COMPAT);    // 6       /* SIGABRT compatible with other platforms, same as SIGABRT */
  ACE_ASSERT (result == 0);
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
do_initialize_directshow (const struct Stream_Device_Identifier& deviceIdentifier_in,
                          const struct Test_I_DirectShow_Configuration& configuration_in,
                          IGraphBuilder*& IGraphBuilder_out,
                          IAMStreamConfig*& IAMStreamConfig_out,
                          struct _AMMediaType& targetMediaType_out,
                          bool useFrameworkRenderer_in,
                          bool mute_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  IMediaFilter* media_filter_p = NULL;
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  Test_I_DirectShowFilter_t* filter_p = NULL;
  IBaseFilter* filter_2 = NULL;
  std::wstring filter_name = STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L;
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));

  // sanity check(s)
  ACE_ASSERT (!IGraphBuilder_out);
  ACE_ASSERT (!IAMStreamConfig_out);

#if defined (SOX_SUPPORT)
  int result_2 = sox_init ();
  if (unlikely (result_2 != SOX_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_init(): \"%s\", aborting\n"),
                ACE_TEXT (sox_strerror (result_2))));
    return false;
  } // end IF
#endif // SOX_SUPPORT

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (targetMediaType_out);

  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
  waveformatex_s.wFormatTag = WAVE_FORMAT_PCM;
  waveformatex_s.nChannels = 1;
  waveformatex_s.nSamplesPerSec = 16000;
  waveformatex_s.wBitsPerSample = 16;
  waveformatex_s.nBlockAlign =
    (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
  waveformatex_s.nAvgBytesPerSec =
    (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
  // waveformatex_s.cbSize = 0;
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  result = CreateAudioMediaType (&waveformatex_s,
                                 &targetMediaType_out,
                                 TRUE); // set format ?
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateAudioMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

  if (!useFrameworkRenderer_in)
    goto continue_2;

  if (unlikely (!Stream_Device_DirectShow_Tools::loadDeviceGraph (deviceIdentifier_in,
                                                                  CLSID_AudioInputDeviceCategory,
                                                                  IGraphBuilder_out,
                                                                  buffer_negotiation_p,
                                                                  IAMStreamConfig_out,
                                                                  graph_layout)))
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

continue_2:
  result = CoCreateInstance (CLSID_FilterGraph, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&IGraphBuilder_out));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  ACE_NEW_NORETURN (filter_p,
                    Test_I_DirectShowFilter_t ());
  if (unlikely (!filter_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (!configuration_in.filterConfiguration.pinConfiguration->format);
  configuration_in.filterConfiguration.pinConfiguration->format =
    &targetMediaType_out;
  if (unlikely (!filter_p->initialize (configuration_in.filterConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Source_Filter_T::initialize(), aborting\n")));
    delete filter_p; filter_p = NULL;
    goto error;
  } // end IF
  result =
    filter_p->NonDelegatingQueryInterface (IID_PPV_ARGS (&filter_2));
  if (unlikely (FAILED (result)))
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
  if (unlikely (FAILED (result)))
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
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  union Stream_MediaFramework_DirectSound_AudioEffectOptions effect_options;
  if (unlikely (!Stream_Module_Decoder_Tools::loadAudioRendererGraph ((useFrameworkRenderer_in ? CLSID_AudioInputDeviceCategory : GUID_NULL),
                                                                      targetMediaType_out,
                                                                      media_type_s,
                                                                      false,
                                                                      (mute_in ? -1 : 0),
                                                                      IGraphBuilder_out,
                                                                      GUID_NULL,
                                                                      effect_options,
                                                                      graph_configuration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::loadAudioRendererGraph(), aborting\n")));
    goto error;
  } // end IF

  if (unlikely (!Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder_out,
                                                                  graph_configuration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n")));
    goto error;
  } // end IF
  Stream_MediaFramework_DirectShow_Tools::clear (graph_configuration);

  result = IGraphBuilder_out->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result = media_filter_p->SetSyncSource (NULL);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release (); media_filter_p = NULL;

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
  Stream_MediaFramework_DirectShow_Tools::free (targetMediaType_out);

  return false;
}

bool
do_initialize_mediafoundation (const struct Stream_Device_Identifier& deviceIdentifier_in,
                               struct Test_I_MediaFoundation_Configuration& configuration_in,
                               IMFMediaSession*& session_out,
                               IMFMediaType*& targetMediaType_out,
                               bool initializeMediaFoundation_in,
                               bool useFrameworkRenderer_in,
                               bool mute_in,
                               Test_I_MediaFoundation_Stream& stream_in,
                               bool makeSession_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;
  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  IMFTopology* topology_p = NULL;
  TOPOID sample_grabber_id = 0, renderer_id = 0;
  IMFAttributes* attributes_p = NULL;
  std::string effect_options; // *TODO*
  UINT32 channel_mask_i = (SPEAKER_FRONT_LEFT |
                           SPEAKER_FRONT_RIGHT);

#if defined (SOX_SUPPORT)
  int result_2 = sox_init ();
  if (unlikely (result_2 != SOX_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_init(): \"%s\", aborting\n"),
                ACE_TEXT (sox_strerror (result_2))));
    return false;
  } // end IF
#endif // SOX_SUPPORT

  if (!initializeMediaFoundation_in)
    goto continue_2;

  result = MFStartup (MF_VERSION,
                      MFSTARTUP_LITE);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFStartup(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

continue_2:
  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION);

  // initialize return value(s)
  if (unlikely (targetMediaType_out))
  {
    targetMediaType_out->Release (); targetMediaType_out = NULL;
  } // end IF

  struct tWAVEFORMATEX* waveformatex_p =
    Stream_MediaFramework_DirectSound_Tools::getAudioEngineMixFormat (deviceIdentifier_in);
  targetMediaType_out =
    Stream_MediaFramework_MediaFoundation_Tools::to (*waveformatex_p);
  ACE_ASSERT (targetMediaType_out);
  CoTaskMemFree (waveformatex_p);
  // *TODO*: remove ASAP
  //result = targetMediaType_out->SetGUID (MF_MT_SUBTYPE,
  //                                       MFAudioFormat_Float);
  //ACE_ASSERT (SUCCEEDED (result));
  //result = targetMediaType_out->SetUINT32 (MF_MT_AUDIO_CHANNEL_MASK,
  //                                         channel_mask_i);
  //ACE_ASSERT (SUCCEEDED (result));
  //result =
  //  targetMediaType_out->DeleteItem (MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
  //ACE_ASSERT (SUCCEEDED (result));
  //result = targetMediaType_out->DeleteItem (MF_MT_AUDIO_PREFER_WAVEFORMATEX);
  //ACE_ASSERT (SUCCEEDED (result));

  ACE_ASSERT (!configuration_in.mediaFoundationConfiguration.mediaType);
  configuration_in.mediaFoundationConfiguration.mediaType =
    Stream_MediaFramework_MediaFoundation_Tools::copy (targetMediaType_out);
  ACE_ASSERT (configuration_in.mediaFoundationConfiguration.mediaType);

  if (!makeSession_in)
    goto continue_4;

  Test_I_MediaFoundation_Target* writer_p =
    &const_cast<Test_I_MediaFoundation_Target&> (stream_in.getR_4 ());
  if (!writer_p->initialize (configuration_in.mediaFoundationConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::initialize(), aborting\n")));
    goto error;
  } // end IF
  result = stream_in.QueryInterface (IID_PPV_ARGS (&media_source_p));
  if (unlikely (FAILED (result) || !media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Test_I_MediaFoundation_Stream::QueryInterface(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  if (unlikely (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (deviceIdentifier_in,
                                                                          MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                          media_source_p,
                                                                          NULL,
                                                                          topology_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceIdentifier_in.identifier._guid).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_source_p);
  ACE_ASSERT (topology_p);
  media_source_p->Release (); media_source_p = NULL;

  ACE_ASSERT (deviceIdentifier_in.identifierDiscriminator == Stream_Device_Identifier::GUID);
  int device_id =
    Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (deviceIdentifier_in.identifier._guid);
  if (unlikely (!Stream_Module_Decoder_Tools::loadAudioRendererTopology (deviceIdentifier_in.identifier._guid,
                                                                         MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                         useFrameworkRenderer_in,
                                                                         targetMediaType_out,
                                                                         NULL,
                                                                         NULL,
                                                                         (mute_in ? -1 : device_id),
                                                                         GUID_NULL,
                                                                         effect_options,
                                                                         topology_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::loadAudioRendererTopology(), aborting\n")));
    goto error;
  } // end IF

  result = MFCreateAttributes (&attributes_p, 4);
  if (unlikely (FAILED (result)))
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
  if (unlikely (session_out))
  {
    session_out->Release (); session_out = NULL;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateMediaSession (attributes_p,
                                 &session_out);
  if (unlikely (FAILED (result)))
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
  if (unlikely (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                           session_out,
                                                                           false,  // is partial ?
                                                                           true))) // wait for completion ?
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
  if (targetMediaType_out)
  {
    targetMediaType_out->Release (); targetMediaType_out = NULL;
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
    if (unlikely (FAILED (result)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF

  return false;
}

void
do_finalize_directshow (struct Test_I_DirectShow_UI_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  // sanity check(s)
  ACE_ASSERT (CBData_in.configuration);

  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T iterator =
    CBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->streamConfiguration.end ());

  if ((*iterator).second.second->builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF

#if defined (SOX_SUPPORT)
  int result_2 = sox_quit ();
  if (unlikely (result_2 != SOX_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_quit(): \"%s\", continuing\n"),
                ACE_TEXT (sox_strerror (result_2))));
#endif // SOX_SUPPORT
}

void
do_finalize_mediafoundation ()
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_mediafoundation"));

  HRESULT result = MFShutdown ();
  if (unlikely (FAILED (result)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

#if defined (SOX_SUPPORT)
  int result_2 = sox_quit ();
  if (unlikely (result_2 != SOX_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_quit(): \"%s\", continuing\n"),
                ACE_TEXT (sox_strerror (result_2))));
#endif // SOX_SUPPORT
}
#endif // ACE_WIN32 || ACE_WIN64

void
do_work (
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
         const std::string& voice_in,
         const std::string& voiceDirectory_in,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         unsigned int deviceId_in,
#else
         const std::string& deviceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
         const std::string& sourceFileName_in,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
         const std::string& UIDefinitionFile_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         enum Stream_MediaFramework_Type mediaFramework_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& targetFileName_in,
         unsigned int statisticReportingInterval_in,
         bool mute_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool useFrameworkRenderer_in,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_I_DirectShow_UI_CBData& directShowCBData_in,
         struct Test_I_MediaFoundation_UI_CBData& mediaFoundationCBData_in,
#else
         struct Test_I_ALSA_UI_CBData& CBData_in,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_I_DirectShow_Configuration& directShowConfiguration_in,
         struct Test_I_MediaFoundation_Configuration& mediaFoundationConfiguration_in,
#else
         struct Test_I_ALSA_Configuration& configuration_in,
#endif // ACE_WIN32 || ACE_WIN64
         ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         ACE_Sig_Set& previousSignalMask_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  struct Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration spectrumanalyzer_configuration;
  struct Stream_AllocatorConfiguration allocator_configuration;
  struct Stream_AllocatorConfiguration* allocator_configuration_p = NULL;
  Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Common_IAsynchTask* itask_p = NULL;
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Stream_AllocatorConfiguration> heap_allocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                     &heap_allocator,     // heap allocator handle
                                                                     true);               // block ?
  Test_I_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                               &heap_allocator,     // heap allocator handle
                                                                               true);               // block ?
#else
  Test_I_ALSA_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                    &heap_allocator,     // heap allocator handle
                                                    true);               // block ?
#endif // ACE_WIN32 || ACE_WIN64
  bool result = false;
  Stream_IStream_t* istream_p = NULL;
  Stream_IStreamControlBase* istream_control_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_CommandSpeech_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  struct Test_I_CommandSpeech_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2; // directshow target module
  struct Test_I_CommandSpeech_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_3; // renderer module
  struct Test_I_CommandSpeech_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_4; // file writer module
  struct Test_I_DirectShow_StreamConfiguration directshow_stream_configuration;
  struct Test_I_CommandSpeech_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Test_I_CommandSpeech_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_2; // mediafoundation target target module
  struct Test_I_CommandSpeech_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_3; // renderer module
  struct Test_I_CommandSpeech_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_4; // file writer module
  struct Test_I_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration;
  Test_I_DirectShow_Stream directshow_stream;
  Test_I_MediaFoundation_Stream mediafoundation_stream;
  std::string renderer_modulename_string;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      istream_p = &directshow_stream;
      istream_control_p = &directshow_stream;
      directshow_stream_configuration.renderer =
        (useFrameworkRenderer_in ? STREAM_DEVICE_RENDERER_DIRECTSHOW
                                 : STREAM_DEVICE_RENDERER_WAVEOUT);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      istream_p = &mediafoundation_stream;
      istream_control_p = &mediafoundation_stream;
      mediafoundation_stream_configuration.renderer =
        (useFrameworkRenderer_in ? STREAM_DEVICE_RENDERER_MEDIAFOUNDATION
                                 : STREAM_DEV_AUDIO_DEFAULT_RENDERER);
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
  Test_I_ALSA_Stream stream;
  istream_p = &stream;
  istream_control_p = &stream;
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (UIDefinitionFile_in.empty () &&
                !Common_Input_Tools::initialize (true))) // keep line mode
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Input_Tools::initialize(), returning\n")));
    return;
  } // end IF

  Test_I_InputManager_t* input_manager_p =
    Test_I_InputManager_t::SINGLETON_T::instance ();
  ACE_ASSERT (input_manager_p);

  ACE_Time_Value one_second (1, 0);
#if defined (GTK_SUPPORT)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
//#if defined (GTKGL_SUPPORT)
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
//#endif // GTKGL_SUPPORT
  int result_2 = -1;
#endif // GTK_SUPPORT
  //  const Stream_Module_t* module_p = NULL;
  //  Test_I_IDispatch_t* idispatch_p = NULL;
  struct Stream_ModuleConfiguration module_configuration;

  // step0a: initialize configuration
  allocator_configuration_p = &allocator_configuration;
  ACE_ASSERT (allocator_configuration_p);

//  struct Stream_Input_ModuleHandlerConfiguration modulehandler_configuration_i; // input-
//  struct Stream_Configuration stream_configuration_2; // input-
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_DirectShow_EventHandler_t directshow_ui_event_handler (
                                                                (UIDefinitionFile_in.empty () ? NULL : &directShowCBData_in)
                                                                );
  Test_I_DirectShow_MessageHandler_Module directshow_event_handler (istream_p,
                                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
//  Test_I_InputHandler_t directshow_input_handler (
//                                                  (UIDefinitionFile_in.empty () ? NULL : &directShowCBData_in)
//                                                 );
  Test_I_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler (
                                                                          (UIDefinitionFile_in.empty () ? NULL : &mediaFoundationCBData_in)
                                                                          );
  Test_I_MediaFoundation_MessageHandler_Module mediafoundation_event_handler (istream_p,
                                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
//  Test_I_InputHandler_t mediafoundation_input_handler (
//                                                       (UIDefinitionFile_in.empty () ? NULL : &mediaFoundationCBData_in)
//                                                      );
  Test_I_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;
  Test_I_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
#else
  Test_I_ALSA_EventHandler_t event_handler (
                                            (UIDefinitionFile_in.empty () ? NULL : &CBData_in)
                                           );
  Test_I_ALSA_MessageHandler_Module event_handler_module (istream_p,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_InputHandler_t input_handler;
  Test_I_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_iterator;
  struct Test_I_ALSA_StreamConfiguration stream_configuration;
  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration; // playback
  ALSA_configuration.asynch = false;
//  ALSA_configuration.mode = SND_PCM_NONBLOCK;
//  ALSA_configuration.rateResample = true;
  struct Test_I_CommandSpeech_ALSA_ModuleHandlerConfiguration modulehandler_configuration;
  ALSA_configuration.format = &modulehandler_configuration.outputFormat;
  struct Test_I_CommandSpeech_ALSA_ModuleHandlerConfiguration modulehandler_configuration_2; // renderer module
  struct Test_I_CommandSpeech_ALSA_ModuleHandlerConfiguration modulehandler_configuration_3; // file writer module
#endif // ACE_WIN32 || ACE_WIN64
//  Test_I_InputMessageHandler_Module input_handler_module (istream_2,
//                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_SignalHandler signal_handler;
  struct Common_EventDispatchState dispatch_state_s;

  ACE_ASSERT (allocator_configuration_p);
  if (unlikely (!heap_allocator.initialize (*allocator_configuration_p)))
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
      directshow_modulehandler_configuration.spectrumAnalyzerConfiguration =
        &spectrumanalyzer_configuration;
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
      directshow_modulehandler_configuration.fileIdentifier.identifier =
        sourceFileName_in;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
      directshow_modulehandler_configuration.allocatorConfiguration =
        allocator_configuration_p;
      directshow_modulehandler_configuration.filterConfiguration =
        &directShowConfiguration_in.filterConfiguration;
      directshow_modulehandler_configuration.messageAllocator =
        &directshow_message_allocator;
      directshow_modulehandler_configuration.mute = mute_in;
      directshow_modulehandler_configuration.queue =
        &const_cast<Stream_MessageQueue_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Test_I_DirectShow_SessionMessage_t>&> (directshow_stream.getR_3 ());
      directshow_modulehandler_configuration.statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      directshow_modulehandler_configuration.subscriber =
        &directshow_ui_event_handler;
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
      directshow_modulehandler_configuration.voice =
        voice_in;
      directshow_modulehandler_configuration.voiceDirectory =
        voiceDirectory_in;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT

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
          renderer_modulename_string =
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING);
          // *WARNING*: falls through !
        }
        case STREAM_DEVICE_RENDERER_DIRECTSHOW:
        {
          directshow_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
            Stream_Device_Identifier::GUID;
          directshow_modulehandler_configuration_3.deviceIdentifier.identifier._guid =
            (mute_in ? GUID_NULL
                     : Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (0,
                                                                                               false)); // playback
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
      directshow_modulehandler_configuration_4.fileIdentifier.identifier =
        targetFileName_in;
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_4)));

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_configuration.spectrumAnalyzerConfiguration =
        &spectrumanalyzer_configuration;
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
      mediafoundation_modulehandler_configuration.fileIdentifier.identifier =
        sourceFileName_in;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
      mediafoundation_modulehandler_configuration.allocatorConfiguration =
        allocator_configuration_p;
      mediafoundation_modulehandler_configuration.mediaFoundationConfiguration =
        &mediaFoundationConfiguration_in.mediaFoundationConfiguration;
      mediafoundation_modulehandler_configuration.mute = mute_in;
      mediafoundation_modulehandler_configuration.queue =
        &const_cast<Stream_MessageQueue_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Test_I_MediaFoundation_SessionMessage_t>&> (mediafoundation_stream.getR_3 ());
      mediafoundation_modulehandler_configuration.statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      mediafoundation_modulehandler_configuration.subscriber =
        &mediafoundation_ui_event_handler;
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
      mediafoundation_modulehandler_configuration.voice =
        voice_in;
      mediafoundation_modulehandler_configuration.voiceDirectory =
        voiceDirectory_in;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT

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
      mediafoundation_modulehandler_configuration_4.fileIdentifier.identifier =
        targetFileName_in;
      mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING),
                                                                                  std::make_pair (&module_configuration,
                                                                                                  &mediafoundation_modulehandler_configuration_4)));

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
  modulehandler_configuration.spectrumAnalyzerConfiguration =
    &spectrumanalyzer_configuration;
  modulehandler_configuration.allocatorConfiguration =
    allocator_configuration_p;
  modulehandler_configuration.ALSAConfiguration = &ALSA_configuration;
#if defined (_DEBUG)
  modulehandler_configuration.debug = true;
#endif // _DEBUG
  modulehandler_configuration.queue =
    &const_cast<Stream_MessageQueue_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Test_I_ALSA_SessionMessage_t>&> (stream.getR_3 ());
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  modulehandler_configuration.voice = voice_in;
  modulehandler_configuration.voiceDirectory = voiceDirectory_in;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
  //modulehandler_configuration.spectrumAnalyzerResolution = 512;

  stream_configuration.allocatorConfiguration = allocator_configuration_p;
  if (unlikely (!Stream_MediaFramework_ALSA_Tools::getDefaultFormat (deviceIdentifier_in,
                                                                     false, // playback
                                                                     modulehandler_configuration.outputFormat)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_ALSA_Tools::getDefaultFormat(\"%s\"), returning\n"),
                ACE_TEXT (deviceIdentifier_in.c_str ())));
    goto error;
  } // end IF

#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  modulehandler_configuration.fileIdentifier.identifier = sourceFileName_in;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
  modulehandler_configuration.messageAllocator = &message_allocator;
  modulehandler_configuration.statisticReportingInterval =
    ACE_Time_Value (statisticReportingInterval_in, 0);
  modulehandler_configuration.subscriber = &event_handler;

  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  modulehandler_iterator =
    configuration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != configuration_in.streamConfiguration.end ());

  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.ALSAConfiguration = &ALSA_configuration;
  modulehandler_configuration_2.deviceIdentifier.identifier =
    deviceIdentifier_in;
  modulehandler_configuration_2.mute = mute_in;
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_2)));

  modulehandler_configuration_3 = modulehandler_configuration;
  modulehandler_configuration_3.fileIdentifier.identifier = targetFileName_in;
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
        &directshow_event_handler;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_stream_configuration.allocatorConfiguration =
        &allocator_configuration;
      mediafoundation_stream_configuration.messageAllocator =
        &mediafoundation_message_allocator;
      mediafoundation_stream_configuration.module =
        &mediafoundation_event_handler;
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
  stream_configuration.allocatorConfiguration =
    &allocator_configuration;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &event_handler_module;
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
                                  directshow_modulehandler_configuration.outputFormat,
                                  useFrameworkRenderer_in, // use DirectShow renderer ?
                                  mute_in);
      if (unlikely (!result))
        break;
      ACE_ASSERT ((*directshow_modulehandler_iterator).second.second->builder);
      if (useFrameworkRenderer_in) // use DirectShow renderer ?
      { ACE_ASSERT (directShowCBData_in.streamConfiguration);
      } // end IF
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      result =
        do_initialize_mediafoundation ((*mediafoundation_modulehandler_iterator).second.second->deviceIdentifier,
                                       *mediaFoundationCBData_in.configuration,
                                       (*mediafoundation_modulehandler_iterator).second.second->session,
                                       mediafoundation_modulehandler_configuration.outputFormat,
                                       true, // initialize MediaFoundation framework ?
                                       useFrameworkRenderer_in, // use MediaFoundation renderer ?
                                       mute_in,
                                       mediafoundation_stream,
                                       UIDefinitionFile_in.empty ()); // make session ?
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
#if defined (SOX_SUPPORT)
  result_2 = sox_init ();
  if (unlikely (result_2 != SOX_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_init(): \"%s\", returning\n"),
                ACE_TEXT (sox_strerror (result_2))));
    goto error;
  } // end IF
#endif // SOX_SUPPORT

  result = true;
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!result))
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
      dispatch_state_s.configuration =
        &directShowConfiguration_in.dispatchConfiguration;

      directShowConfiguration_in.signalHandlerConfiguration.dispatchState =
        &dispatch_state_s;
      directShowConfiguration_in.signalHandlerConfiguration.stream =
        istream_control_p;
      directShowConfiguration_in.signalHandlerConfiguration.stopEventDispatchOnShutdown =
        false; // the input manager manages event handling
      signal_handler.initialize (directShowConfiguration_in.signalHandlerConfiguration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      dispatch_state_s.configuration =
        &mediaFoundationConfiguration_in.dispatchConfiguration;

      mediaFoundationConfiguration_in.signalHandlerConfiguration.dispatchState =
        &dispatch_state_s;
      mediaFoundationConfiguration_in.signalHandlerConfiguration.stream =
        istream_control_p;
      mediaFoundationConfiguration_in.signalHandlerConfiguration.stopEventDispatchOnShutdown =
        false; // the input manager manages event handling
      signal_handler.initialize (mediaFoundationConfiguration_in.signalHandlerConfiguration);
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
  dispatch_state_s.configuration = &configuration_in.dispatchConfiguration;

  configuration_in.signalHandlerConfiguration.dispatchState =
    &dispatch_state_s;
  configuration_in.signalHandlerConfiguration.stream =
    istream_control_p;
  configuration_in.signalHandlerConfiguration.stopEventDispatchOnShutdown =
    false; // the input manager manages event handling
  signal_handler.initialize (configuration_in.signalHandlerConfiguration);
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                                  signalSet_in,
                                                  ignoredSignalSet_in,
                                                  &signal_handler,
                                                  previousSignalActions_inout)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), aborting\n")));
    goto error;
  } // end IF

  // step0f: initialize input handling
//  modulehandler_configuration_i.concurrency =
//    STREAM_HEADMODULECONCURRENCY_ACTIVE;
//  modulehandler_configuration_i.queue = &stream_2_r.queue_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directShowConfiguration_in.inputConfiguration.lineMode = true;
      directShowConfiguration_in.inputConfiguration.manager =
        input_manager_p;
      directShowConfiguration_in.inputConfiguration.messageAllocator =
        &directshow_message_allocator;
      directShowConfiguration_in.inputConfiguration.queue =
        &const_cast<Stream_MessageQueue_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Test_I_DirectShow_SessionMessage_t>&> (directshow_stream.getR_3 ());

      //modulehandler_configuration_i.subscriber = &directshow_input_handler;

      //stream_configuration_2 = directshow_stream_configuration;
      //stream_configuration_2.module = &input_handler_module;
      //stream_configuration_2.moduleBranch.clear ();
      //directShowConfiguration_in.streamConfiguration_2.initialize (module_configuration,
      //                                                             modulehandler_configuration_i,
      //                                                             stream_configuration_2);
      directShowConfiguration_in.inputManagerConfiguration.eventDispatchState =
        &dispatch_state_s;
      //directShowConfiguration_in.inputManagerConfiguration.streamConfiguration =
      //  &directShowConfiguration_in.streamConfiguration_2;
      if (unlikely (!input_manager_p->initialize (directShowConfiguration_in.inputManagerConfiguration)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Input_Manager_T::initialize(), returning\n")));
        goto error;
      } // end IF

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediaFoundationConfiguration_in.inputConfiguration.lineMode = true;
      mediaFoundationConfiguration_in.inputConfiguration.manager =
        input_manager_p;
        mediaFoundationConfiguration_in.inputConfiguration.messageAllocator =
        &mediafoundation_message_allocator;
      mediaFoundationConfiguration_in.inputConfiguration.queue =
        &const_cast<Stream_MessageQueue_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Test_I_MediaFoundation_SessionMessage_t>&> (mediafoundation_stream.getR_3 ());

      //modulehandler_configuration_i.subscriber = &mediafoundation_input_handler;

      //stream_configuration_2 = mediafoundation_stream_configuration;
      //stream_configuration_2.module = &input_handler_module;
      //stream_configuration_2.moduleBranch.clear ();
      //mediaFoundationConfiguration_in.streamConfiguration_2.initialize (module_configuration,
      //                                                                  modulehandler_configuration_i,
      //                                                                  stream_configuration_2);
      mediaFoundationConfiguration_in.inputManagerConfiguration.eventDispatchState =
        &dispatch_state_s;
      //mediaFoundationConfiguration_in.inputManagerConfiguration.streamConfiguration =
      //  &mediaFoundationConfiguration_in.streamConfiguration_2;
      if (unlikely (!input_manager_p->initialize (mediaFoundationConfiguration_in.inputManagerConfiguration)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Input_Manager_T::initialize(), returning\n")));
        goto error;
      } // end IF

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
  configuration_in.inputConfiguration.lineMode = true;
  configuration_in.inputConfiguration.manager = input_manager_p;
  configuration_in.inputConfiguration.messageAllocator = &message_allocator;
  configuration_in.inputConfiguration.queue =
    &const_cast<Stream_MessageQueue_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Test_I_ALSA_SessionMessage_t>&> (stream.getR_3 ());

//  modulehandler_configuration_i.subscriber = &input_handler;

//  stream_configuration_2 = stream_configuration;
//  stream_configuration_2.module = &input_handler_module;
//  stream_configuration_2.moduleBranch.clear ();
//  configuration_in.streamConfiguration_2.initialize (module_configuration,
//                                                     modulehandler_configuration_i,
//                                                     stream_configuration_2);
  configuration_in.inputManagerConfiguration.eventDispatchState =
    &dispatch_state_s;
//  configuration_in.inputManagerConfiguration.streamConfiguration =
//    &configuration_in.streamConfiguration_2;
  if (unlikely (!input_manager_p->initialize (configuration_in.inputManagerConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Input_Manager_T::initialize(), returning\n")));
    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // step1a: start ui event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_SUPPORT)
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_SUPPORT
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
        //directShowCBData_in.UIState = &state_r;
#endif // GTK_USE
        directShowCBData_in.stream = &directshow_stream;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
#if defined (GTK_USE)
        //mediaFoundationCBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
        //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
        state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
          std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
        //mediaFoundationCBData_in.UIState = &state_r;
#endif // GTK_USE
        mediaFoundationCBData_in.stream = &mediafoundation_stream;
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
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (unlikely (!window_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      goto error;
    } // end IF
    BOOL was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_USE)
    itask_p = gtk_manager_p;
    ACE_ASSERT (itask_p);
    itask_p->start (NULL);
    //ACE_Time_Value timeout (0,
    //                        COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
    //result_2 = ACE_OS::sleep (timeout);
    //if (unlikely (result_2 == -1))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
    //              &timeout));
    if (unlikely (!itask_p->isRunning ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, aborting\n")));
      goto error;
    } // end IF
#endif // GTK_USE
    ACE_ASSERT (itask_p);
    itask_p->wait (false);
  } // end IF
  else
  {
    // pre-initialize processing stream
    if (unlikely (!input_manager_p->start ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Input_Manager_T::start(), returning\n")));
      goto error;
    } // end IF
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
    if (unlikely (!success_b))
#else
    if (unlikely (!stream.initialize (configuration_in.streamConfiguration)))
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
    istream_control_p->wait (true,   // wait for thread(s) ?
                             false,  // wait for upstream ?
                             false); // wait for downstream ?
  } // end ELSE

  // step3: clean up
  input_manager_p->stop (true,   // wait for completion ?
                         false); // N/A
//  result = stream_2_r.remove (&input_handler_module,
//                              true,   // lock ?
//                              false); // reset ?
//  if (unlikely (!result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Base_T::remove(\"%s\"), aborting\n"),
//                input_handler_module.name ()));
//    goto error;
//  } // end IF
  timer_manager_p->stop (true,   // wait ?
                         false); // N/A
  if (UIDefinitionFile_in.empty ())
    Common_Input_Tools::finalize ();
  Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                 previousSignalActions_inout,
                                 previousSignalMask_in);

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
  result = stream.remove (&event_handler_module,
                          true,   // lock ?
                          false); // reset ?

#if defined (SOX_SUPPORT)
  result_2 = sox_quit ();
  if (unlikely (result_2 != SOX_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_quit(): \"%s\", returning\n"),
                ACE_TEXT (sox_strerror (result_2))));
    goto error;
  } // end IF
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base::remove(), continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

error:
  if (input_manager_p)
    input_manager_p->stop (true,   // wait ?
                           false); // N/A
//  stream_2_r.remove (&input_handler_module,
//                     true,   // lock ?
//                     false); // reset ?
  if (!UIDefinitionFile_in.empty () && itask_p)
    itask_p->stop (true,  // wait ?
                   true); // high priority ?
  if (timer_manager_p)
    timer_manager_p->stop ();
  Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                 previousSignalActions_inout,
                                 previousSignalMask_in);
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: on Windows, initialize ACE...
  result = ACE::init ();
  if (unlikely (result == -1))
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
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  // step1a set defaults
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  std::string voice_string = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_VOICE);
  std::string voice_directory = path;
  voice_directory += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  voice_directory += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_VOICE_DIRECTORY);
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  unsigned int device_id = 0;
#else
  std::string device_identifier_string =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_PLAYBACK);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
  std::string source_filename;
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
  std::string UI_definition_file = path;
  UI_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
#if defined (GTK_SUPPORT)
  std::string UI_CSS_file;
#endif // GTK_SUPPORT
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  std::string target_filename;
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool mute = false;
  bool print_version_and_exit = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool use_framework_source = false;
  bool use_framework_renderer = false;
#endif // ACE_WIN32 || ACE_WIN64

  ACE_High_Res_Timer timer;
  ACE_Time_Value working_time;
  ACE_Time_Value user_time;
  ACE_Time_Value system_time;

  Common_Logger_Queue_t logger;
  std::string log_file_name;
  ACE_Sig_Set signal_set (false); // fill ?
  ACE_Sig_Set ignored_signal_set (false); // fill ?
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_DirectShow_Configuration directshow_configuration;
  struct Test_I_MediaFoundation_Configuration mediafoundation_configuration;
  struct Test_I_DirectShow_UI_CBData directshow_ui_cb_data;
  struct Test_I_MediaFoundation_UI_CBData mediafoundation_ui_cb_data;
#else
  struct Test_I_ALSA_Configuration configuration;
  struct Test_I_ALSA_UI_CBData ui_cb_data;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_SUPPORT)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
#if defined (GTKGL_SUPPORT)
  struct Common_UI_GTK_GLConfiguration* gtk_configuration_p = NULL;
#else
  struct Common_UI_GTK_Configuration* gtk_configuration_p = NULL;
#endif // GTKGL_SUPPORT
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
  Common_UI_GTK_State_t* state_p = NULL;
#endif // GTK_SUPPORT
  struct Test_I_UI_CBData* cb_data_base_p = NULL;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
                            voice_string,
                            voice_directory,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            device_id,
#else
                            device_identifier_string,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
                            source_filename,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
                            UI_definition_file,
#if defined (GTK_SUPPORT)
                            UI_CSS_file,
#endif // GTK_SUPPORT
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                            target_filename,
                            statistic_reporting_interval,
                            trace_information,
                            mute,
                            print_version_and_exit
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            ,use_framework_source,
                            use_framework_renderer))
#else
                            ))
#endif // ACE_WIN32 || ACE_WIN64
  {
    do_printUsage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
    goto error;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_I_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if (
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
      !Common_File_Tools::isDirectory (voice_directory)
      || (!source_filename.empty () &&
          !Common_File_Tools::isReadable (source_filename)) 
#else
      false
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
      || (!UI_definition_file.empty () &&
          !Common_File_Tools::isReadable (UI_definition_file))
#if defined (GTK_SUPPORT)
      || (!UI_CSS_file.empty () &&
          !Common_File_Tools::isReadable (UI_CSS_file))
#endif // GTK_SUPPORT
      )
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));
    goto error;
  } // end IF

  Common_UI_Tools::initialize ();
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
#else
#endif // GTKGL_SUPPORT
  gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  state_p = &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  directshow_configuration.filterConfiguration.pinConfiguration =
    &directshow_configuration.pinConfiguration;
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data.configuration = &directshow_configuration;
#if defined (GTK_SUPPORT)
      directshow_ui_cb_data.progressData.state = state_p;

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
        directshow_configuration.GTKConfiguration.CSSProviders[UI_CSS_file] =
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
      mediafoundation_ui_cb_data.configuration = &mediafoundation_configuration;
#if defined (GTK_SUPPORT)
      mediafoundation_ui_cb_data.progressData.state = state_p;

      mediafoundation_configuration.GTKConfiguration.argc = argc_in;
      mediafoundation_configuration.GTKConfiguration.argv = argv_in;
      mediafoundation_configuration.GTKConfiguration.CBData =
        &mediafoundation_ui_cb_data;
      mediafoundation_configuration.GTKConfiguration.eventHooks.finiHook =
        idle_finalize_UI_cb;
      mediafoundation_configuration.GTKConfiguration.eventHooks.initHook =
        idle_initialize_UI_cb;
      mediafoundation_configuration.GTKConfiguration.definition =
        &gtk_ui_definition;
#if GTK_CHECK_VERSION(3,0,0)
      if (!UI_CSS_file.empty ())
        mediafoundation_configuration.GTKConfiguration.CSSProviders[UI_CSS_file] =
          NULL;
#endif // GTK_CHECK_VERSION(3,0,0)
      gtk_configuration_p = &mediafoundation_configuration.GTKConfiguration;
#endif // GTK_SUPPORT
      cb_data_base_p = &mediafoundation_ui_cb_data;
      cb_data_base_p->mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  media_framework_e));
      goto error;
    }
  } // end SWITCH
#else
  ui_cb_data.configuration = &configuration;

#if defined (GTK_SUPPORT)
  ui_cb_data.progressData.state = state_p;

  configuration.GTKConfiguration.argc = argc_in;
  configuration.GTKConfiguration.argv = argv_in;
  configuration.GTKConfiguration.CBData = &ui_cb_data;
  configuration.GTKConfiguration.eventHooks.finiHook =
    idle_finalize_UI_cb;
  configuration.GTKConfiguration.eventHooks.initHook =
    idle_initialize_UI_cb;
  configuration.GTKConfiguration.definition = &gtk_ui_definition;
#if GTK_CHECK_VERSION(3,0,0)
  if (!UI_CSS_file.empty ())
    configuration.GTKConfiguration.CSSProviders[UI_CSS_file] = NULL;
#endif // GTK_CHECK_VERSION(3,0,0)
  gtk_configuration_p = &configuration.GTKConfiguration;
#endif // GTK_SUPPORT
  cb_data_base_p = &ui_cb_data;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (cb_data_base_p);

#if defined (GTK_SUPPORT)
  cb_data_base_p->UIState = state_p;
#endif // GTK_SUPPORT

  // step1d: initialize logging and/or tracing
  if (unlikely (!logger.initialize (&state_p->logQueue,
                                    &state_p->logQueueLock)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Logger_T::initialize(), aborting\n")));
    goto error;
  } // end IF
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
  if (unlikely (!Common_Log_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])), // program name
                                               log_file_name,                                     // log file name
                                               false,                                             // log to syslog ?
                                               false,                                             // trace messages ?
                                               trace_information,                                 // debug messages ?
                                               NULL)))                                            // ui logger ?
//                                                      (UI_definition_file.empty () ? NULL
//                                                                                   : &logger))))         // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));
    goto error;
  } // end IF

  // step1e: pre-initialize signal handling
  do_initializeSignals (signal_set,
                        ignored_signal_set);
  if (unlikely (!Common_Signal_Tools::preInitialize (signal_set,
                                                     COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                                     false, // using networking ?
                                                     false, // using asynch timers ?
                                                     previous_signal_actions,
                                                     previous_signal_mask)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));
    goto error;
  } // end IF

  // step1f: handle specific program modes
  if (unlikely (print_version_and_exit))
  {
    do_printVersion (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined(ACE_WIN32) || defined(ACE_WIN64)
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
  if (unlikely (!Common_OS_Tools::setResourceLimits (false,   // file descriptors
                                                     true,    // stack traces
                                                     false))) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));
    goto error;
  } // end IF

  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
#if defined (GTK_SUPPORT)
  ACE_ASSERT (gtk_manager_p);
  ACE_ASSERT (gtk_configuration_p);
  if (!UI_definition_file.empty () &&
      unlikely (!gtk_manager_p->initialize (*gtk_configuration_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), aborting\n")));
    goto error;
  } // end IF
#endif // GTK_SUPPORT

  timer.start ();
  // step2: do actual work
  do_work (
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
           voice_string,
           voice_directory,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           device_id,
#else
           device_identifier_string,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
           source_filename,
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
           UI_definition_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
           target_filename,
           statistic_reporting_interval,
           mute,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
           previous_signal_mask);
  timer.stop ();

  timer.elapsed_time (working_time);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"\n"),
              ACE_TEXT (Common_Timer_Tools::periodToString (working_time).c_str ())));

  // stop profile timer...
  process_profile.stop ();

  // only process profile left to do...
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  result = process_profile.elapsed_time (elapsed_time);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  user_time.set (elapsed_rusage.ru_utime);
  system_time.set (elapsed_rusage.ru_stime);

  // debug info
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

  //Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
  //                               previous_signal_actions,
  //                               previous_signal_mask);
  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();
#if defined(ACE_WIN32) || defined(ACE_WIN64)
  // *PORTABILITY*: on Windows, finalize ACE...
  result = ACE::fini ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;

error:
  //Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
  //                               previous_signal_actions,
  //                               previous_signal_mask);
  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();
#if defined(ACE_WIN32) || defined(ACE_WIN64)
  // *PORTABILITY*: on Windows, finalize ACE...
  result = ACE::fini ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_FAILURE;
} // end main
