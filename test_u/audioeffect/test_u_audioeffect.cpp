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
#include "streams.h"
#endif

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

#ifdef LIBACENETWORK_ENABLE_VALGRIND_SUPPORT
#include "valgrind/valgrind.h"
#endif

#include "common_file_tools.h"
#include "common_logger.h"
#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "common_ui_defines.h"
//#include "common_ui_glade_definition.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_allocatorheap.h"
//#include "stream_control_message.h"
#include "stream_macros.h"

#include "stream_dev_tools.h"

#ifdef HAVE_CONFIG_H
#include "libACEStream_config.h"
#endif

#include "test_u_common.h"
#include "test_u_defines.h"

#include "test_u_audioeffect_callbacks.h"
#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_defines.h"
#include "test_u_audioeffect_eventhandler.h"
#include "test_u_audioeffect_module_eventhandler.h"
#include "test_u_audioeffect_signalhandler.h"
#include "test_u_audioeffect_stream.h"

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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("audioeffect");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]  : buffer size (byte(s)) [")
            << TEST_U_STREAM_AUDIOEFFECT_DEFAULT_BUFFER_SIZE
            << ACE_TEXT ("])")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : show console [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : device [\"")
            << ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e [STRING] : effect [\"")
            << ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_SOX_DEFAULT_EFFECT_NAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[STRING]]: target filename [")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
  std::string UI_style_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_DEFAULT_GTK_CSS_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i[[STRING]]: UI CSS file [\"")
            << UI_style_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m          : use media foundation [")
            << TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif
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
                     std::string& effect_out,
#endif
                     std::string& targetFileName_out,
                     std::string& UIFile_out,
                     std::string& UICSSFile_out,
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& useMediaFoundation_out,
#endif
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out)
                     //bool& runStressTest_out)
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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("audioeffect");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  bufferSize_out = TEST_U_STREAM_AUDIOEFFECT_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#else
//  deviceFilename_out = ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY);
//  deviceFilename_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  deviceFilename_out =
      ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME);
  effect_out.clear ();
#endif
  path = Common_File_Tools::getTempDirectory ();
  targetFileName_out = path;
  targetFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  targetFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
  UIFile_out = configuration_path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_GLADE_FILE);
  UICSSFile_out = configuration_path;
  UICSSFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UICSSFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  UICSSFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UICSSFile_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_DEFAULT_GTK_CSS_FILE);
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  useMediaFoundation_out =
    TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION;
#endif
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  printVersionAndExit_out = false;
  //runStressTest_out = false;

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("b:cf::g::i::lms:tvx"),
#else
                              ACE_TEXT ("b:d:e::f::g::hi:ls:tvx"),
#endif
                              1,                          // skip command name
                              1,                          // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS,  // ordering
                              0);                         // for now, don't use long options
  int result = argument_parser.long_option (ACE_TEXT ("sync"),
                                            'x',
                                            ACE_Get_Opt::NO_ARG);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Get_Opt::long_option(): \"%m\", aborting\n")));
    return false;
  } // end IF

  int option = 0;
  std::stringstream converter;
  while ((option = argument_parser ()) != EOF)
  {
    switch (option)
    {
      case 'b':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argument_parser.opt_arg ();
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
        deviceFilename_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
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
#endif
      case 'f':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          targetFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          targetFileName_out.clear ();
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
      case 'i':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UICSSFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UICSSFile_out.clear ();
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'm':
      {
        useMediaFoundation_out = true;
        break;
      }
#endif
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
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      case 'x':
        break;
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

  signals_out.sig_del (SIGIO);             // 29      /* I/O now possible */

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
do_initialize_directshow (const std::string& deviceName_in,
                          IGraphBuilder*& IGraphBuilder_out,
                          IAMStreamConfig*& IAMStreamConfig_out,
                          struct _AMMediaType*& mediaType_out,
                          bool coInitialize_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  struct tWAVEFORMATEX* waveformatex_p = NULL;
  std::list<std::wstring> filter_pipeline;
  IMediaFilter* media_filter_p = NULL;

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF

continue_:
  Stream_Module_Device_Tools::initialize ();

  if (!Stream_Module_Device_Tools::loadDeviceGraph (deviceName_in,
                                                    CLSID_AudioInputDeviceCategory,
                                                    IGraphBuilder_out,
                                                    buffer_negotiation_p,
                                                    IAMStreamConfig_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (deviceName_in.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (IAMStreamConfig_out);

  buffer_negotiation_p->Release ();
  buffer_negotiation_p = NULL;

  if (!mediaType_out)
  {
    mediaType_out =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    if (!mediaType_out)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to CoTaskMemAlloc(%u): \"%m\", aborting\n"),
                  sizeof (struct _AMMediaType)));
      goto error;
    } // end IF
    ACE_OS::memset (mediaType_out, 0, sizeof (struct _AMMediaType));
  } // end IF
  ACE_ASSERT (mediaType_out);

  //mediaType_out->majortype = MEDIATYPE_Audio;
  //mediaType_out->subtype = MEDIASUBTYPE_PCM;
  //mediaType_out->bFixedSizeSamples = TRUE;
  //mediaType_out->bTemporalCompression = FALSE;
  //// *NOTE*: lSampleSize is set after pbFormat (see below)
  ////mediaType_out->lSampleSize = waveformatex_p->;
  //mediaType_out->formattype = FORMAT_WaveFormatEx;
  //mediaType_out->cbFormat = sizeof (struct tWAVEFORMATEX);

  //if (!mediaType_out->pbFormat)
  //{
  //  mediaType_out->pbFormat =
  //    static_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tWAVEFORMATEX)));
  //  if (!mediaType_out->pbFormat)
  //  {
  //    ACE_DEBUG ((LM_CRITICAL,
  //                ACE_TEXT ("failed to CoTaskMemAlloc(%u): \"%m\", aborting\n"),
  //                sizeof (struct tWAVEFORMATEX)));
  //    goto error;
  //  } // end IF
  //  ACE_OS::memset (mediaType_out->pbFormat, 0, sizeof (struct tWAVEFORMATEX));
  //} // end IF
  //ACE_ASSERT (mediaType_out->pbFormat);
  //waveformatex_p =
  //  reinterpret_cast<struct tWAVEFORMATEX*> (mediaType_out->pbFormat);

  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
  waveformatex_p = &waveformatex_s;

  waveformatex_p->wFormatTag = WAVE_FORMAT_PCM;
  waveformatex_p->nChannels = 2; // stereo
  waveformatex_p->nSamplesPerSec = 44100;
  waveformatex_p->nAvgBytesPerSec = 176400; // 44100 * (16 / 8) * 2
  waveformatex_p->nBlockAlign = 4; // (16 / 8) * 2
  waveformatex_p->wBitsPerSample = 16;
  waveformatex_p->cbSize = 0;

  result = CreateAudioMediaType (waveformatex_p,
                                 mediaType_out,
                                 TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateAudioMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  ////////////////////////////////////////

  //mediaType_out->lSampleSize = waveformatex_p->nAvgBytesPerSec;

  if (!Stream_Module_Device_Tools::setCaptureFormat (IGraphBuilder_out,
                                                     CLSID_AudioInputDeviceCategory,
                                                     *mediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF

  union Stream_Decoder_DirectShow_AudioEffectOptions effect_options;
  if (!Stream_Module_Device_Tools::loadAudioRendererGraph (*mediaType_out,
                                                           0,
                                                           IGraphBuilder_out,
                                                           GUID_NULL,
                                                           effect_options,
                                                           filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadAudioRendererGraph(), aborting\n")));
    goto error;
  } // end IF

  result = IGraphBuilder_out->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release ();

  return true;

error:
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  if (media_filter_p)
    media_filter_p->Release ();

  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release ();
    IGraphBuilder_out = NULL;
  } // end IF
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release ();
    IAMStreamConfig_out = NULL;
  } // end IF

  if (mediaType_out)
    Stream_Module_Device_Tools::deleteMediaType (mediaType_out);

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

continue_2:
  Stream_Module_Device_Tools::initialize ();

  return true;

error:
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
do_finalize_directshow (Test_U_AudioEffect_DirectShow_GTK_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  ACE_UNUSED_ARG (CBData_in);

  HRESULT result = E_FAIL;

  if (CBData_in.configuration->moduleHandlerConfiguration.builder)
  {
    CBData_in.configuration->moduleHandlerConfiguration.builder->Release ();
    CBData_in.configuration->moduleHandlerConfiguration.builder = NULL;
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
         const std::string& effectName_in,
#endif
         const std::string& targetFilename_in,
         const std::string& UIDefinitionFile_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool useMediaFoundation_in,
#endif
         unsigned int statisticReportingInterval_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        Test_U_AudioEffect_DirectShow_GTK_CBData& directShowCBData_in,
        Test_U_AudioEffect_MediaFoundation_GTK_CBData& mediaFoundationCBData_in,
#else
         Test_U_AudioEffect_GTK_CBData& CBData_in,
#endif
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_AudioEffect_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  Stream_AllocatorConfiguration* allocator_configuration_p = NULL;
  Common_TimerConfiguration timer_configuration;
  Test_U_AudioEffect_Configuration configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Common_ITask_t* itask_p = NULL;
  Stream_AllocatorHeap_T<Stream_AllocatorConfiguration> heap_allocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_U_STREAM_AUDIOEFFECT_MAX_MESSAGES, // maximum #buffers
                                                                                 &heap_allocator,                        // heap allocator handle
                                                                                 true);                                  // block ?
  Test_U_AudioEffect_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_U_STREAM_AUDIOEFFECT_MAX_MESSAGES, // maximum #buffers
                                                                                           &heap_allocator,                        // heap allocator handle
                                                                                           true);                                  // block ?
#else
  Test_U_AudioEffect_MessageAllocator_t message_allocator (TEST_U_STREAM_AUDIOEFFECT_MAX_MESSAGES, // maximum #buffers
                                                           &heap_allocator,                        // heap allocator handle
                                                           true);                                  // block ?
#endif
  bool result = false;
  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_Stream directshow_stream;
  Test_U_AudioEffect_MediaFoundation_Stream mediafoundation_stream;
  if (useMediaFoundation_in)
    stream_p = &mediafoundation_stream;
  else
    stream_p = &directshow_stream;
#else
  Test_U_AudioEffect_Stream stream;
  stream_p = &stream;
#endif
  ACE_Time_Value one_second (1, 0);
  int result_2 = -1;
  const Stream_Module_t* module_p = NULL;
  Test_U_AudioEffect_IDispatch_t* idispatch_p = NULL;

  // step0a: initialize configuration
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_Configuration directshow_configuration;
  directShowCBData_in.configuration = &directshow_configuration;
  Test_U_AudioEffect_MediaFoundation_Configuration mediafoundation_configuration;
  mediaFoundationCBData_in.configuration = &mediafoundation_configuration;
  allocator_configuration_p =
    (useMediaFoundation_in ? &mediafoundation_configuration.allocatorConfiguration
                           : &directshow_configuration.allocatorConfiguration);
#else
  CBData_in.configuration = &configuration;
  allocator_configuration_p = &configuration.allocatorConfiguration;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_EventHandler directshow_ui_event_handler (&directShowCBData_in);
  Test_U_AudioEffect_DirectShow_Module_EventHandler_Module directshow_event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                                                      NULL,
                                                                                      true);
  Test_U_AudioEffect_MediaFoundation_EventHandler mediafoundation_ui_event_handler (&mediaFoundationCBData_in);
  Test_U_AudioEffect_MediaFoundation_Module_EventHandler_Module mediafoundation_event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                                                               NULL,
                                                                                               true);
  if (useMediaFoundation_in)
  {
    Test_U_AudioEffect_MediaFoundation_Module_EventHandler* event_handler_p =
      dynamic_cast<Test_U_AudioEffect_MediaFoundation_Module_EventHandler*> (mediafoundation_event_handler.writer ());
    if (!event_handler_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("dynamic_cast<Test_U_AudioEffect_MediaFoundation_Module_EventHandler> failed, returning\n")));
      goto error;
    } // end IF
    event_handler_p->initialize (&mediaFoundationCBData_in.subscribers,
                                 &mediaFoundationCBData_in.subscribersLock);
    event_handler_p->subscribe (&mediafoundation_ui_event_handler);
  } // end IF
  else
  {
    Test_U_AudioEffect_DirectShow_Module_EventHandler* event_handler_p =
      dynamic_cast<Test_U_AudioEffect_DirectShow_Module_EventHandler*> (directshow_event_handler.writer ());
    if (!event_handler_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("dynamic_cast<Test_U_AudioEffect_DirectShow_Module_EventHandler> failed, returning\n")));
      goto error;
    } // end IF
    event_handler_p->initialize (&directShowCBData_in.subscribers,
                                 &directShowCBData_in.subscribersLock);
    event_handler_p->subscribe (&directshow_ui_event_handler);
  } // end ELSE
#else
  Test_U_AudioEffect_EventHandler ui_event_handler (&CBData_in);
  Test_U_AudioEffect_Module_EventHandler_Module event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                               NULL,
                                                               true);
  Test_U_AudioEffect_Module_EventHandler* event_handler_p =
    dynamic_cast<Test_U_AudioEffect_Module_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_AudioEffect_Module_EventHandler> failed, aborting\n")));
    goto error;
  } // end IF
  event_handler_p->initialize (&CBData_in.subscribers,
                               &CBData_in.subscribersLock);
  event_handler_p->subscribe (&ui_event_handler);
#endif

  ACE_ASSERT (allocator_configuration_p);
  heap_allocator.initialize (*allocator_configuration_p);

  // ********************** module configuration data **************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    mediafoundation_configuration.moduleHandlerConfiguration.audioOutput =
      1;
    mediafoundation_configuration.moduleHandlerConfiguration.cairoSurfaceLock =
      &mediaFoundationCBData_in.cairoSurfaceLock;
    mediafoundation_configuration.moduleHandlerConfiguration.printProgressDot =
      UIDefinitionFile_in.empty ();
    mediafoundation_configuration.moduleHandlerConfiguration.streamConfiguration =
      &mediafoundation_configuration.streamConfiguration;
    mediafoundation_configuration.moduleHandlerConfiguration.targetFileName =
        (targetFilename_in.empty () ? Common_File_Tools::getTempDirectory ()
                                    : targetFilename_in);
  } // end IF
  else
  {
    directshow_configuration.moduleHandlerConfiguration.active = true;
    directshow_configuration.moduleHandlerConfiguration.audioOutput =
      1;
    directshow_configuration.moduleHandlerConfiguration.cairoSurfaceLock =
      &directShowCBData_in.cairoSurfaceLock;
    //directshow_configuration.moduleHandlerConfiguration.format =
    //  (struct _AMMediaType*)CoTaskMemAlloc (sizeof (struct _AMMediaType));
    //ACE_ASSERT (directshow_configuration.moduleHandlerConfiguration.format);
    //struct tWAVEFORMATEX waveformatex_s;
    //ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
    //waveformatex_s.cbSize = sizeof (struct tWAVEFORMATEX);
    ////waveformatex_s.nAvgBytesPerSec = ;
    ////waveformatex_s.nBlockAlign = ;
    ////waveformatex_s.nChannels = 1;
    ////waveformatex_s.nSamplesPerSec = 44100;
    ////waveformatex_s.wBitsPerSample = 16;
    //waveformatex_s.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    //HRESULT result =
    //  CreateAudioMediaType (&waveformatex_s,
    //                        directshow_configuration.moduleHandlerConfiguration.format,
    //                        TRUE);
    //ACE_ASSERT (SUCCEEDED (result));
    directshow_configuration.moduleHandlerConfiguration.printProgressDot =
      UIDefinitionFile_in.empty ();
    directshow_configuration.moduleHandlerConfiguration.streamConfiguration =
      &directshow_configuration.streamConfiguration;
    directshow_configuration.moduleHandlerConfiguration.targetFileName =
        (targetFilename_in.empty () ? Common_File_Tools::getTempDirectory ()
                                    : targetFilename_in);
  } // end ELSE
#else
//  configuration.moduleHandlerConfiguration.device =
//    device_in;
  configuration.moduleHandlerConfiguration.effect =
      effectName_in;
  configuration.moduleHandlerConfiguration.format =
      &configuration.ALSAConfiguration;
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
  configuration.moduleHandlerConfiguration.cairoSurfaceLock =
      &CBData_in.cairoSurfaceLock;
#else
  configuration.moduleHandlerConfiguration.pixelBufferLock =
      &CBData_in.pixelBufferLock;
#endif
  configuration.moduleHandlerConfiguration.messageAllocator =
      &message_allocator;
  configuration.moduleHandlerConfiguration.printProgressDot =
      UIDefinitionFile_in.empty ();
  configuration.moduleHandlerConfiguration.streamConfiguration =
      &configuration.streamConfiguration;
  configuration.moduleHandlerConfiguration.targetFileName =
      (targetFilename_in.empty () ? Common_File_Tools::getTempDirectory ()
                                  : targetFilename_in);
#endif

  // ********************** stream configuration data **************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    if (bufferSize_in)
      mediafoundation_configuration.streamConfiguration.bufferSize =
        bufferSize_in;
    mediafoundation_configuration.streamConfiguration.messageAllocator =
      &mediafoundation_message_allocator;
    mediafoundation_configuration.streamConfiguration.module =
      (!UIDefinitionFile_in.empty () ? &mediafoundation_event_handler
                                     : NULL);
    mediafoundation_configuration.streamConfiguration.moduleConfiguration =
      &mediafoundation_configuration.moduleConfiguration;
    mediafoundation_configuration.moduleConfiguration.streamConfiguration =
      &mediafoundation_configuration.streamConfiguration;
    mediafoundation_configuration.streamConfiguration.moduleHandlerConfiguration =
      &mediafoundation_configuration.moduleHandlerConfiguration;
    mediafoundation_configuration.streamConfiguration.printFinalReport = true;
    mediafoundation_configuration.streamConfiguration.statisticReportingInterval =
        statisticReportingInterval_in;
  } // end IF
  else
  {
    if (bufferSize_in)
      directshow_configuration.streamConfiguration.bufferSize =
        bufferSize_in;
    directshow_configuration.streamConfiguration.messageAllocator =
      &directshow_message_allocator;
    directshow_configuration.streamConfiguration.module =
      (!UIDefinitionFile_in.empty () ? &directshow_event_handler
                                     : NULL);
    directshow_configuration.streamConfiguration.moduleConfiguration =
      &directshow_configuration.moduleConfiguration;
    directshow_configuration.moduleConfiguration.streamConfiguration =
      &directshow_configuration.streamConfiguration;
    directshow_configuration.streamConfiguration.moduleHandlerConfiguration =
      &directshow_configuration.moduleHandlerConfiguration;
    directshow_configuration.streamConfiguration.printFinalReport = true;
    directshow_configuration.streamConfiguration.statisticReportingInterval =
        statisticReportingInterval_in;
  } // end ELSE
#else
  if (bufferSize_in)
    configuration.streamConfiguration.bufferSize = bufferSize_in;
  configuration.streamConfiguration.messageAllocator = &message_allocator;
  configuration.streamConfiguration.module =
    (!UIDefinitionFile_in.empty () ? &event_handler
                                   : NULL);
  configuration.streamConfiguration.moduleConfiguration =
    &configuration.moduleConfiguration;
  configuration.moduleConfiguration.streamConfiguration =
    &configuration.streamConfiguration;
  configuration.streamConfiguration.moduleHandlerConfiguration =
    &configuration.moduleHandlerConfiguration;
  configuration.streamConfiguration.printFinalReport = true;
  configuration.streamConfiguration.statisticReportingInterval =
      statisticReportingInterval_in;
#endif

  // intialize timers
  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();

  #if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: in UI mode, COM has already been initialized for this thread
  // *TODO*: where has that happened ?
  if (useMediaFoundation_in)
    result =
      do_initialize_mediafoundation (true, // initialize COM ?
                                     true);
  else
    result =
      do_initialize_directshow (directshow_configuration.moduleHandlerConfiguration.device,
                                directshow_configuration.moduleHandlerConfiguration.builder,
                                directShowCBData_in.streamConfiguration,
                                directshow_configuration.moduleHandlerConfiguration.format,
                                true); // initialize COM ?
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to intialize media framework, returning\n")));
    goto error;
  } // end IF
  if (!useMediaFoundation_in)
  {
    ACE_ASSERT (directshow_configuration.moduleHandlerConfiguration.builder);
    ACE_ASSERT (directShowCBData_in.streamConfiguration);
  } // end IF
#endif

  // step0b: (initialize) processing stream
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
    result =
      mediafoundation_stream.initialize (mediafoundation_configuration.streamConfiguration);
  else
    result =
      directshow_stream.initialize (directshow_configuration.streamConfiguration);
#else
  result = stream.initialize (configuration.streamConfiguration);
#endif
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto error;
  } // end IF

  // ********************** module configuration data (part 2) *****************
  module_p =
    stream_p->find (ACE_TEXT_ALWAYS_CHAR ("SpectrumAnalyzer"));
  ACE_ASSERT (module_p);
  idispatch_p =
    dynamic_cast<Test_U_AudioEffect_IDispatch_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (idispatch_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
    mediafoundation_configuration.moduleHandlerConfiguration.dispatch =
      idispatch_p;
  else
    directshow_configuration.moduleHandlerConfiguration.dispatch =
      idispatch_p;
#else
  configuration.moduleHandlerConfiguration.dispatch = idispatch_p;
#endif

  // step0e: initialize signal handling
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    mediafoundation_configuration.signalHandlerConfiguration.messageAllocator =
      &mediafoundation_message_allocator;
    signalHandler_in.initialize (mediafoundation_configuration.signalHandlerConfiguration);
  } // end IF
  else
  {
    directshow_configuration.signalHandlerConfiguration.messageAllocator =
      &directshow_message_allocator;
    signalHandler_in.initialize (directshow_configuration.signalHandlerConfiguration);
  } // end ELSE
#else
  configuration.signalHandlerConfiguration.messageAllocator =
    &message_allocator;
  signalHandler_in.initialize (configuration.signalHandlerConfiguration);
#endif
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), aborting\n")));
    goto error;
  } // end IF

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step1a: start GTK event loop ?
  itask_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (useMediaFoundation_in)
    {
      mediaFoundationCBData_in.finalizationHook = idle_finalize_UI_cb;
      mediaFoundationCBData_in.initializationHook = idle_initialize_UI_cb;
      //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
      mediaFoundationCBData_in.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
        std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
      mediaFoundationCBData_in.stream = &mediafoundation_stream;
      mediaFoundationCBData_in.userData = &mediaFoundationCBData_in;
    } // end IF
    else
    {
      directShowCBData_in.finalizationHook = idle_finalize_UI_cb;
      directShowCBData_in.initializationHook = idle_initialize_UI_cb;
      //directShowCBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
      directShowCBData_in.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
        std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
      directShowCBData_in.stream = &directshow_stream;
      directShowCBData_in.userData = &directShowCBData_in;
    } // end ELSE
#else
    CBData_in.finalizationHook = idle_finalize_UI_cb;
    CBData_in.initializationHook = idle_initialize_UI_cb;
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
    CBData_in.stream = &stream;
    CBData_in.userData = &CBData_in;
#endif

    itask_p->start ();
    result_2 = ACE_OS::sleep (one_second);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
    if (!itask_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, aborting\n")));
      goto error;
    } // end IF

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
#endif
  } // end IF
  else
  {
    ACE_ASSERT (stream_p);

    // *NOTE*: this will block until the file has been copied...
    stream_p->start ();
//    if (!stream.isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start stream, aborting\n")));
//      goto error;
//    } // end IF
    stream_p->wait (true, false, false);
  } // end ELSE

  // step3: clean up
  if (!UIDefinitionFile_in.empty ())
    itask_p->wait ();
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
  if (useMediaFoundation_in)
    result = mediafoundation_stream.remove (&mediafoundation_event_handler);
  else
    result = directshow_stream.remove (&directshow_event_handler);
#else
  result = stream.remove (&event_handler);
#endif
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base::remove(), continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

error:
  if (!UIDefinitionFile_in.empty () && itask_p)
    itask_p->stop (true); // wait for completion ?
  timer_manager_p->stop ();
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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("audioeffect");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  unsigned int buffer_size = TEST_U_STREAM_AUDIOEFFECT_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#else
  std::string device_filename =
//    ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY);
//  device_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
//  device_filename +=
      ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME);
  std::string effect_name;
#endif
  std::string path = Common_File_Tools::getTempDirectory ();
  std::string target_filename = path;
  target_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  target_filename += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE);
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  std::string UI_definition_file = path;
  UI_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  UI_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_GLADE_FILE);
  std::string UI_CSS_file = path;
  UI_CSS_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_CSS_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CONFIGURATION_DIRECTORY);
  UI_CSS_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_CSS_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_DEFAULT_GTK_CSS_FILE);
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool use_mediafoundation =
    TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION;
#endif
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
                            effect_name,
#endif
                            target_filename,
                            UI_definition_file,
                            UI_CSS_file,
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            use_mediafoundation,
#endif
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit))//,
                            //run_stress_test))
  {
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
  if (TEST_U_STREAM_AUDIOEFFECT_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if ((!UI_definition_file.empty () &&
       !Common_File_Tools::isReadable (UI_definition_file)) ||
      (!UI_CSS_file.empty () &&
       !Common_File_Tools::isReadable (UI_CSS_file)))
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

  Test_U_AudioEffect_GTK_CBData* gtk_cb_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData directshow_gtk_cb_data;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData mediafoundation_gtk_cb_data;
  if (use_mediafoundation)
  {
    mediafoundation_gtk_cb_data.progressData.GTKState =
      &mediafoundation_gtk_cb_data;
    gtk_cb_data_p = &mediafoundation_gtk_cb_data;
    gtk_cb_data_p->useMediaFoundation = true;
  } // end IF
  else
  {
    directshow_gtk_cb_data.progressData.GTKState =
      &directshow_gtk_cb_data;
    gtk_cb_data_p = &directshow_gtk_cb_data;
  } // end ELSE
#else
  Test_U_AudioEffect_GTK_CBData gtk_cb_data;
  gtk_cb_data.progressData.GTKState = &gtk_cb_data;
  gtk_cb_data_p = &gtk_cb_data;
#endif
  ACE_ASSERT (gtk_cb_data_p);
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
  if (!UI_CSS_file.empty ())
    gtk_cb_data_p->CSSProviders[UI_CSS_file] = NULL;
#endif
  // step1d: initialize logging and/or tracing
  Common_Logger_t logger (&gtk_cb_data_p->logStack,
                          &gtk_cb_data_p->logStackLock);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACESTREAM_PACKAGE_NAME),
                                           ACE::basename (argv_in[0]));
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]),               // program name
                                        log_file_name,                            // log file name
                                        false,                                    // log to syslog ?
                                        false,                                    // trace messages ?
                                        trace_information,                        // debug messages ?
                                        (UI_definition_file.empty () ? NULL
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
  Test_U_AudioEffect_SignalHandler signal_handler;

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
  if (!UI_definition_file.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                              argv_in,
                                                              gtk_cb_data_p,
                                                              &ui_definition);

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#else
           device_filename,
           effect_name,
#endif
           target_filename,
           UI_definition_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           use_mediafoundation,
#endif
           statistic_reporting_interval,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_gtk_cb_data,
           mediafoundation_gtk_cb_data,
#else
           gtk_cb_data,
#endif
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
