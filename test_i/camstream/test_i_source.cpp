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
#include <limits>
#include <string>

#include <ace/Get_Opt.h>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <ace/Init_ACE.h>
#endif
#include <ace/Log_Msg.h>
#include <ace/Profile_Timer.h>
#include <ace/Sig_Handler.h>
#include <ace/Signal.h>
#include <ace/Synch.h>
#include <ace/Version.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfapi.h>
#include <streams.h>
#endif

#ifdef LIBACESTREAM_ENABLE_VALGRIND_SUPPORT
#include <valgrind/valgrind.h>
#endif

#include "common_file_tools.h"
#include "common_logger.h"
#include "common_tools.h"

#include "common_ui_defines.h"
//#include "common_ui_glade_definition.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#ifdef HAVE_CONFIG_H
#include "libACEStream_config.h"
#endif

#include "test_i_callbacks.h"
#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_common_modules.h"
#include "test_i_source_common.h"
#include "test_i_source_eventhandler.h"
#include "test_i_source_message.h"
#include "test_i_source_session_message.h"
#include "test_i_source_signalhandler.h"
#include "test_i_source_stream.h"

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]  : buffer size (byte(s)) [")
            << TEST_I_DEFAULT_BUFFER_SIZE
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
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e          : Gtk .rc file [\"\"] {\"\": no GUI}")
            << std::endl;
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-h [STRING] : target host [\"")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_HOSTNAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m          : use media foundation [")
            << TEST_I_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o          : use thread pool [")
            << NET_EVENT_USE_THREAD_POOL
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : port number [")
            << TEST_I_DEFAULT_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u          : use UDP [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]  : #dispatch threads [")
            << NET_CLIENT_DEFAULT_NUMBER_OF_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
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
                     std::string& gtkRcFile_out,
                     std::string& gtkGladeFile_out,
                     std::string& hostName_out,
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& useMediaFoundation_out,
#endif
                     bool& useThreadPool_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& useUDP_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  bufferSize_out = TEST_I_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#else
  deviceFilename_out = ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY);
  deviceFilename_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  deviceFilename_out += ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEFAULT_VIDEO_DEVICE);
#endif
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  //gtkRcFile_out = path;
  //gtkRcFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //gtkRcFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  hostName_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_HOSTNAME);
  gtkGladeFile_out = path;
  gtkGladeFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkGladeFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  useMediaFoundation_out =
    TEST_I_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION;
#endif
  useThreadPool_out = NET_EVENT_USE_THREAD_POOL;
  port_out = TEST_I_DEFAULT_PORT;
  useReactor_out = NET_EVENT_USE_REACTOR;
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  useUDP_out = false;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    NET_CLIENT_DEFAULT_NUMBER_OF_DISPATCH_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("b:ce:g::h:lmop:rs:tuvx:"),
#else
                              ACE_TEXT ("b:d:e:g::h:lop:rs:tuvx:"),
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
      case 'e':
      {
        gtkRcFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          gtkGladeFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          gtkGladeFile_out.clear ();
        break;
      }
      case 'h':
      {
        hostName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
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
      case 'o':
      {
        useThreadPool_out = true;
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> port_out;
        break;
      }
      case 'r':
      {
        useReactor_out = true;
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
      case 'u':
      {
        useUDP_out = true;
        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      case 'x':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> numberOfDispatchThreads_out;
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
  struct tagVIDEOINFO* video_info_p = NULL;
  Stream_Module_Device_DirectShow_Graph_t graph_configuration;
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
  if (FAILED (result))
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
                                                    CLSID_VideoInputDeviceCategory,
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

  mediaType_out->majortype = MEDIATYPE_Video;
  //mediaType_out->subtype = MEDIASUBTYPE_RGB32;
  // *NOTE*: apparently, some cameras do not support uncompressed RGB capture
  //         --> add necessary intermediate filters (MFTs) later on
  mediaType_out->subtype = MEDIASUBTYPE_MJPG;
  mediaType_out->bFixedSizeSamples = TRUE;
  mediaType_out->bTemporalCompression = FALSE;
  // *NOTE*: lSampleSize is set after pbFormat (see below)
  //mediaType_out->lSampleSize = video_info_p->bmiHeader.biSizeImage;
  mediaType_out->formattype = FORMAT_VideoInfo;
  mediaType_out->cbFormat = sizeof (struct tagVIDEOINFO);

  if (!mediaType_out->pbFormat)
  {
    mediaType_out->pbFormat =
      static_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tagVIDEOINFO)));
    if (!mediaType_out->pbFormat)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to CoTaskMemAlloc(%u): \"%m\", aborting\n"),
                  sizeof (struct tagVIDEOINFO)));
      goto error;
    } // end IF
    ACE_OS::memset (mediaType_out->pbFormat, 0, sizeof (struct tagVIDEOINFO));
  } // end IF
  ACE_ASSERT (mediaType_out->pbFormat);

  video_info_p =
    reinterpret_cast<struct tagVIDEOINFO*> (mediaType_out->pbFormat);

  BOOL result_2 = SetRectEmpty (&video_info_p->rcSource);
  ACE_ASSERT (result_2);
  video_info_p->rcSource.right = 320;
  video_info_p->rcSource.bottom = 240;
  result_2 = SetRectEmpty (&video_info_p->rcTarget);
  ACE_ASSERT (result_2);
  video_info_p->rcTarget.right = 320;
  video_info_p->rcTarget.bottom = 240;

  // *NOTE*: 320x240 * 4 * 30 * 8
  video_info_p->dwBitRate = 73728000;
  //video_info_p->dwBitErrorRate = 0;
  video_info_p->AvgTimePerFrame = 333333; // --> 30 fps (in 100th ns)

  // *TODO*: make this configurable (and part of a protocol)
  video_info_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_p->bmiHeader.biWidth = 320;
  video_info_p->bmiHeader.biHeight = 240;
  video_info_p->bmiHeader.biPlanes = 1;
  //video_info_p->bmiHeader.biBitCount = 24;
  video_info_p->bmiHeader.biBitCount = 32;
  video_info_p->bmiHeader.biCompression = BI_RGB;
  video_info_p->bmiHeader.biSizeImage =
    GetBitmapSize (&video_info_p->bmiHeader);
  //video_info_p->bmiHeader.biXPelsPerMeter;
  //video_info_p->bmiHeader.biYPelsPerMeter;
  //video_info_p->bmiHeader.biClrUsed;
  //video_info_p->bmiHeader.biClrImportant;

  // *NOTE*: union
  //video_info_p->bmiColors = ;
  //video_info_p->dwBitMasks = {0, 0, 0};
  //video_info_p->TrueColorInfo = ;

  ////////////////////////////////////////

  mediaType_out->lSampleSize = video_info_p->bmiHeader.biSizeImage;

  if (!Stream_Module_Device_Tools::setCaptureFormat (IGraphBuilder_out,
                                                     CLSID_VideoInputDeviceCategory,
                                                     *mediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF

  if (!Stream_Module_Device_Tools::loadVideoRendererGraph (CLSID_VideoInputDeviceCategory,
                                                           *mediaType_out,
                                                           NULL,
                                                           IGraphBuilder_out,
                                                           graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadVideoRendererGraph(), aborting\n")));
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

  IGraphBuilder_out->Release ();
  IGraphBuilder_out = NULL;

  IAMStreamConfig_out->Release ();
  IAMStreamConfig_out = NULL;

  if (mediaType_out)
    Stream_Module_Device_Tools::deleteMediaType (mediaType_out);

  if (coInitialize_in)
    CoUninitialize ();

  return false;
}
bool
do_initialize_mediafoundation (bool coInitialize_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;

  if (!coInitialize_in)
    goto continue_;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED    |
                            COINIT_DISABLE_OLE1DDE  |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result))
  {
    // *NOTE*: most probable reason: already initialized (happens in the
    //         debugger)
    //         --> continue
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF

  result = MFStartup (MF_VERSION,
                      MFSTARTUP_LITE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFStartup(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

continue_:
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
do_finalize_directshow (Test_I_Source_DirectShow_GTK_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  ACE_UNUSED_ARG (CBData_in);

  HRESULT result = E_FAIL;

  if (CBData_in.streamConfiguration)
  {
    CBData_in.streamConfiguration->Release ();
    CBData_in.streamConfiguration = NULL;
  } // end IF
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
#endif
         const std::string& UIDefinitionFilename_in,
         const std::string& hostName_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool useMediaFoundation_in,
#endif
         bool useThreadPool_in,
         unsigned short port_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         bool useUDP_in,
         unsigned int numberOfDispatchThreads_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         Test_I_Source_MediaFoundation_GTK_CBData& mediaFoundationCBData_in,
         Test_I_Source_DirectShow_GTK_CBData& directShowCBData_in,
#else
         Test_I_Source_V4L2_GTK_CBData& v4l2CBData_in,
#endif
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         const sigset_t& previousSignalMask_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  bool result = false;

  // step0a: initialize event dispatch
  struct Common_DispatchThreadData thread_data;
  Stream_Configuration* stream_configuration_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Source_DirectShow_Configuration directshow_configuration;
  Test_I_Source_MediaFoundation_Configuration mediafoundation_configuration;
  if (useMediaFoundation_in)
    stream_configuration_p = &mediafoundation_configuration.streamConfiguration;
  else
    stream_configuration_p = &directshow_configuration.streamConfiguration;
#else
  Test_I_Source_V4L2_Configuration v4l2_configuration;
  stream_configuration_p = &v4l2_configuration.streamConfiguration;
#endif
  ACE_ASSERT (stream_configuration_p);
  if (!Common_Tools::initializeEventDispatch (useReactor_in,
                                              useThreadPool_in,
                                              numberOfDispatchThreads_in,
                                              thread_data.proactorType,
                                              thread_data.reactorType,
                                              stream_configuration_p->serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step0b: initialize configuration and stream
  Test_I_CamStream_Configuration* camstream_configuration_p = NULL;
  Stream_AllocatorConfiguration* allocator_configuration_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    camstream_configuration_p = &mediafoundation_configuration;
    mediafoundation_configuration.userData.configuration =
      &mediafoundation_configuration.connectionConfiguration;
    mediafoundation_configuration.userData.streamConfiguration =
      &mediafoundation_configuration.streamConfiguration;
    mediaFoundationCBData_in.configuration = &mediafoundation_configuration;
    allocator_configuration_p =
      &mediafoundation_configuration.allocatorConfiguration;
  } // end IF
  else
  {
    camstream_configuration_p = &directshow_configuration;
    directshow_configuration.userData.configuration =
      &directshow_configuration.connectionConfiguration;
    directshow_configuration.userData.streamConfiguration =
      &directshow_configuration.streamConfiguration;
    directShowCBData_in.configuration = &directshow_configuration;
    allocator_configuration_p =
      &directshow_configuration.allocatorConfiguration;
  } // end IF
#else
  camstream_configuration_p = &v4l2_configuration;
  v4l2_configuration.userData.configuration =
    &v4l2_configuration.connectionConfiguration;
  v4l2_configuration.userData.streamConfiguration =
    &v4l2_configuration.streamConfiguration;
  v4l2CBData_in.configuration = &v4l2_configuration;
  allocator_configuration_p =
    &v4l2_configuration.allocatorConfiguration;
#endif
  ACE_ASSERT (camstream_configuration_p);
  camstream_configuration_p->protocol = (useUDP_in ? NET_TRANSPORTLAYER_UDP
                                                   : NET_TRANSPORTLAYER_TCP);
  camstream_configuration_p->useReactor = useReactor_in;
  result = false;
  if (useReactor_in)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (useMediaFoundation_in)
    {
      ACE_NEW_NORETURN (mediaFoundationCBData_in.stream,
                        Test_I_Source_MediaFoundation_TCPStream_t ());
      ACE_NEW_NORETURN (mediaFoundationCBData_in.UDPStream,
                        Test_I_Source_MediaFoundation_UDPStream_t ());
      result = (mediaFoundationCBData_in.stream &&
                mediaFoundationCBData_in.UDPStream);
    } // end IF
    else
    {
      ACE_NEW_NORETURN (directShowCBData_in.stream,
                        Test_I_Source_DirectShow_TCPStream_t ());
      ACE_NEW_NORETURN (directShowCBData_in.UDPStream,
                        Test_I_Source_DirectShow_UDPStream_t ());
      result = (directShowCBData_in.stream &&
                directShowCBData_in.UDPStream);
    } // end IF
#else
      ACE_NEW_NORETURN (v4l2CBData_in.stream,
                        Test_I_Source_V4L2_TCPStream_t ());
      ACE_NEW_NORETURN (v4l2CBData_in.UDPStream,
                        Test_I_Source_V4L2_UDPStream_t ());
      result = (v4l2CBData_in.stream &&
                v4l2CBData_in.UDPStream);
#endif
  } // end IF
  else
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (useMediaFoundation_in)
    {
      ACE_NEW_NORETURN (mediaFoundationCBData_in.stream,
                        Test_I_Source_MediaFoundation_AsynchTCPStream_t ());
      ACE_NEW_NORETURN (mediaFoundationCBData_in.UDPStream,
                        Test_I_Source_MediaFoundation_AsynchUDPStream_t ());
      result = (mediaFoundationCBData_in.stream &&
                mediaFoundationCBData_in.UDPStream);
    } // end IF
    else
    {
      ACE_NEW_NORETURN (directShowCBData_in.stream,
                        Test_I_Source_DirectShow_AsynchTCPStream_t ());
      ACE_NEW_NORETURN (directShowCBData_in.UDPStream,
                        Test_I_Source_DirectShow_AsynchUDPStream_t ());
      result = (directShowCBData_in.stream &&
                directShowCBData_in.UDPStream);
    } // end IF
#else
    ACE_NEW_NORETURN (v4l2CBData_in.stream,
                      Test_I_Source_V4L2_AsynchTCPStream_t ());
    ACE_NEW_NORETURN (v4l2CBData_in.UDPStream,
                      Test_I_Source_V4L2_AsynchUDPStream_t ());
    result = (v4l2CBData_in.stream &&
              v4l2CBData_in.UDPStream);
#endif
  } // end ELSE
  if (!result)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: in UI mode, COM has already been initialized for this thread
  // *TODO*: where has that happened ?
  if (useMediaFoundation_in)
    result = do_initialize_mediafoundation (UIDefinitionFilename_in.empty ()); // initialize COM ?
  else
    result = do_initialize_directshow (directshow_configuration.moduleHandlerConfiguration.device,
                                       directshow_configuration.moduleHandlerConfiguration.builder,
                                       directShowCBData_in.streamConfiguration,
                                       directshow_configuration.moduleHandlerConfiguration.format,
                                       UIDefinitionFilename_in.empty ()); // initialize COM ?
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to intialize media framework, returning\n")));
    return;
  } // end IF
  if (!useMediaFoundation_in)
  {
    ACE_ASSERT (directshow_configuration.moduleHandlerConfiguration.builder);
    ACE_ASSERT (directShowCBData_in.streamConfiguration);
  } // end IF
#endif

  ACE_ASSERT (allocator_configuration_p);
  Stream_AllocatorHeap_T<Stream_AllocatorConfiguration> heap_allocator;
  heap_allocator.initialize (*allocator_configuration_p);
  Stream_IAllocator* allocator_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Source_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                            &heap_allocator,     // heap allocator handle
                                                                            true);               // block ?
  Test_I_Source_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                                      &heap_allocator,     // heap allocator handle
                                                                                      true);               // block ?
  if (useMediaFoundation_in)
    allocator_p = &mediafoundation_message_allocator;
  else
    allocator_p = &directshow_message_allocator;
#else
  Test_I_Source_V4L2_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                           &heap_allocator,     // heap allocator handle
                                                           true);               // block ?
  allocator_p = &message_allocator;
#endif
  ACE_ASSERT (allocator_p);

  int result_2 = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Source_DirectShow_EventHandler_t directshow_ui_event_handler (&directShowCBData_in);
  Test_I_Source_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler (&mediaFoundationCBData_in);
  Test_I_Source_DirectShow_EventHandler_Module directshow_event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                                         NULL,
                                                                         true);
  Test_I_Source_MediaFoundation_EventHandler_Module mediafoundation_event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                                                   NULL,
                                                                                   true);
#else
  Test_I_Source_V4L2_EventHandler_t ui_event_handler (&v4l2CBData_in);
  Test_I_Source_V4L2_Module_EventHandler_Module event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                               NULL,
                                                               true);
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Source_DirectShow_EventHandler* directshow_event_handler_p =
    NULL;
  Test_I_Source_MediaFoundation_EventHandler* mediafoundation_event_handler_p =
    NULL;
#else
  Test_I_Source_V4L2_Module_EventHandler* module_event_handler_p = NULL;
#endif
  Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  long timer_id = -1;
  int group_id = -1;
  Net_IConnectionManagerBase* iconnection_manager_p = NULL;
  Test_I_StatisticReportingHandler_t* report_handler_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    mediafoundation_configuration.moduleHandlerConfiguration.connectionManager =
      TEST_I_SOURCE_MEDIAFOUNDATION_CONNECTIONMANAGER_SINGLETON::instance ();
    mediafoundation_configuration.moduleHandlerConfiguration.connectionManager->initialize (std::numeric_limits<unsigned int>::max ());
    mediafoundation_configuration.moduleHandlerConfiguration.connectionManager->set (mediafoundation_configuration.connectionConfiguration,
                                                                                     &mediafoundation_configuration.userData);
    iconnection_manager_p =
      mediafoundation_configuration.moduleHandlerConfiguration.connectionManager;
    report_handler_p =
      mediafoundation_configuration.moduleHandlerConfiguration.connectionManager;
  } // end IF
  else
  {
    directshow_configuration.moduleHandlerConfiguration.connectionManager =
      TEST_I_SOURCE_DIRECTSHOW_CONNECTIONMANAGER_SINGLETON::instance ();
    directshow_configuration.moduleHandlerConfiguration.connectionManager->initialize (std::numeric_limits<unsigned int>::max ());
    directshow_configuration.moduleHandlerConfiguration.connectionManager->set (directshow_configuration.connectionConfiguration,
                                                                                &directshow_configuration.userData);
    iconnection_manager_p =
      directshow_configuration.moduleHandlerConfiguration.connectionManager;
    report_handler_p =
      directshow_configuration.moduleHandlerConfiguration.connectionManager;
  } // end ELSE
#else
  v4l2_configuration.moduleHandlerConfiguration.connectionManager =
    TEST_I_SOURCE_V4L2_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (v4l2_configuration.moduleHandlerConfiguration.connectionManager);
  v4l2_configuration.moduleHandlerConfiguration.connectionManager->initialize (std::numeric_limits<unsigned int>::max ());
  v4l2_configuration.moduleHandlerConfiguration.connectionManager->set (v4l2_configuration.connectionConfiguration,
                                                                        &v4l2_configuration.userData);
  iconnection_manager_p =
    v4l2_configuration.moduleHandlerConfiguration.connectionManager;
  report_handler_p =
    v4l2_configuration.moduleHandlerConfiguration.connectionManager;
#endif
  ACE_ASSERT (iconnection_manager_p);
  ACE_ASSERT (report_handler_p);
  Stream_StatisticHandler_Reactor_t statistic_handler (ACTION_REPORT,
                                                       report_handler_p,
                                                       false);
  //Stream_StatisticHandler_Proactor_t statistic_handler_proactor (ACTION_REPORT,
  //                                                               connection_manager_p,
  //                                                               false);

  ACE_Event_Handler* event_handler_p = NULL;
  Net_SocketHandlerConfiguration* socket_handler_configuration_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Source_DirectShow_SignalHandler_t directshow_signal_handler;
  Test_I_Source_MediaFoundation_SignalHandler_t mediafoundation_signal_handler;

  if (useMediaFoundation_in)
  {
    mediafoundation_event_handler_p =
      dynamic_cast<Test_I_Source_MediaFoundation_EventHandler*> (mediafoundation_event_handler.writer ());
    event_handler_p = mediafoundation_event_handler_p;
  } // end IF
  else
  {
    directshow_event_handler_p =
      dynamic_cast<Test_I_Source_DirectShow_EventHandler*> (directshow_event_handler.writer ());
    event_handler_p = directshow_event_handler_p;
  } // end ELSE
#else
  Test_I_Source_V4L2_SignalHandler_t signal_handler;

  module_event_handler_p =
    dynamic_cast<Test_I_Source_V4L2_Module_EventHandler*> (event_handler.writer ());
  event_handler_p = module_event_handler_p;
#endif
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Test_I_Source_V4L2_Module_EventHandler>, returning\n")));
    goto clean;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
    mediafoundation_event_handler_p->subscribe (&mediafoundation_ui_event_handler);
  else
    directshow_event_handler_p->subscribe (&directshow_ui_event_handler);
#else
  module_event_handler_p->subscribe (&ui_event_handler);
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    // *************************** media foundation ****************************
    mediafoundation_configuration.mediaFoundationConfiguration.controller =
      ((mediafoundation_configuration.protocol == NET_TRANSPORTLAYER_TCP) ? mediaFoundationCBData_in.stream
                                                                          : mediaFoundationCBData_in.UDPStream);
  } // end IF
#endif

  // *********************** socket configuration data *************************
  result_2 =
    camstream_configuration_p->socketConfiguration.address.set (port_in,
                                                                hostName_in.c_str (),
                                                                1,
                                                                ACE_ADDRESS_FAMILY_INET);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));
    goto clean;
  } // end IF
  camstream_configuration_p->socketConfiguration.bufferSize = bufferSize_in;
  camstream_configuration_p->socketConfiguration.useLoopBackDevice =
    camstream_configuration_p->socketConfiguration.address.is_loopback ();
  camstream_configuration_p->socketConfiguration.writeOnly = true;
  // ******************** socket handler configuration data ********************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    mediafoundation_configuration.socketHandlerConfiguration.userData =
      &mediafoundation_configuration.userData;
    mediafoundation_configuration.socketHandlerConfiguration.socketConfiguration =
      &mediafoundation_configuration.socketConfiguration;
    socket_handler_configuration_p =
      &mediafoundation_configuration.socketHandlerConfiguration;
    socket_handler_configuration_p->messageAllocator =
      &mediafoundation_message_allocator;
  } // end IF
  else
  {
    directshow_configuration.socketHandlerConfiguration.userData =
      &directshow_configuration.userData;
    directshow_configuration.socketHandlerConfiguration.socketConfiguration =
      &directshow_configuration.socketConfiguration;
    socket_handler_configuration_p =
      &directshow_configuration.socketHandlerConfiguration;
    socket_handler_configuration_p->messageAllocator =
      &directshow_message_allocator;
  } // end ELSE
#else
  v4l2_configuration.socketHandlerConfiguration.userData =
    &v4l2_configuration.userData;
  v4l2_configuration.socketHandlerConfiguration.socketConfiguration =
    &v4l2_configuration.socketConfiguration;
  socket_handler_configuration_p =
    &v4l2_configuration.socketHandlerConfiguration;
  socket_handler_configuration_p->messageAllocator =
    &message_allocator;
#endif
  ACE_ASSERT (socket_handler_configuration_p);
  socket_handler_configuration_p->PDUSize = bufferSize_in;
  socket_handler_configuration_p->statisticReportingInterval =
    statisticReportingInterval_in;

  // ********************** module configuration data **************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    mediafoundation_configuration.moduleConfiguration.streamConfiguration =
      &mediafoundation_configuration.streamConfiguration;

    mediafoundation_configuration.moduleHandlerConfiguration.configuration =
      &mediafoundation_configuration;
    mediafoundation_configuration.moduleHandlerConfiguration.socketConfiguration =
      &mediafoundation_configuration.socketConfiguration;
    mediafoundation_configuration.moduleHandlerConfiguration.socketHandlerConfiguration =
      &mediafoundation_configuration.socketHandlerConfiguration;
    mediafoundation_configuration.moduleHandlerConfiguration.stream =
      ((mediafoundation_configuration.protocol == NET_TRANSPORTLAYER_TCP) ? mediaFoundationCBData_in.stream
                                                                          : mediaFoundationCBData_in.UDPStream);
    mediafoundation_configuration.moduleHandlerConfiguration.streamConfiguration =
      &mediafoundation_configuration.streamConfiguration;
  } // end IF
  else
  {
    directshow_configuration.moduleConfiguration.streamConfiguration =
      &directshow_configuration.streamConfiguration;

    directshow_configuration.moduleHandlerConfiguration.configuration =
      &directshow_configuration;
    directshow_configuration.moduleHandlerConfiguration.socketConfiguration =
      &directshow_configuration.socketConfiguration;
    directshow_configuration.moduleHandlerConfiguration.socketHandlerConfiguration =
      &directshow_configuration.socketHandlerConfiguration;
    directshow_configuration.moduleHandlerConfiguration.stream =
      ((directshow_configuration.protocol == NET_TRANSPORTLAYER_TCP) ? directShowCBData_in.stream
                                                                     : directShowCBData_in.UDPStream);
    directshow_configuration.moduleHandlerConfiguration.streamConfiguration =
      &directshow_configuration.streamConfiguration;
  } // end ELSE
#else
  v4l2_configuration.moduleConfiguration.streamConfiguration =
    &v4l2_configuration.streamConfiguration;

  v4l2_configuration.moduleHandlerConfiguration.configuration =
    &v4l2_configuration;
  v4l2_configuration.moduleHandlerConfiguration.socketConfiguration =
    &v4l2_configuration.socketConfiguration;
  v4l2_configuration.moduleHandlerConfiguration.socketHandlerConfiguration =
    &v4l2_configuration.socketHandlerConfiguration;
  v4l2_configuration.moduleHandlerConfiguration.stream =
    ((v4l2_configuration.protocol == NET_TRANSPORTLAYER_TCP) ? v4l2CBData_in.stream
                                                             : v4l2CBData_in.UDPStream);

  v4l2_configuration.moduleHandlerConfiguration.device = deviceFilename_in;
  // *TODO*: turn these into an option
  v4l2_configuration.moduleHandlerConfiguration.buffers =
      MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS;
  v4l2_configuration.moduleHandlerConfiguration.format.fmt.pix.pixelformat =
      V4L2_PIX_FMT_RGB24;
  v4l2_configuration.moduleHandlerConfiguration.format.fmt.pix.width = 320;
  v4l2_configuration.moduleHandlerConfiguration.format.fmt.pix.height = 240;
  v4l2_configuration.moduleHandlerConfiguration.frameRate.numerator = 30;
  v4l2_configuration.moduleHandlerConfiguration.frameRate.denominator = 1;
  v4l2_configuration.moduleHandlerConfiguration.lock = &v4l2CBData_in.lock;
  v4l2_configuration.moduleHandlerConfiguration.method = V4L2_MEMORY_MMAP;
  v4l2_configuration.moduleHandlerConfiguration.streamConfiguration =
    &v4l2_configuration.streamConfiguration;
#endif

  // ******************** (sub-)stream configuration data **********************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    if (bufferSize_in)
      mediafoundation_configuration.streamConfiguration.bufferSize =
        bufferSize_in;
    mediafoundation_configuration.streamConfiguration.messageAllocator =
      &mediafoundation_message_allocator;
    if (!UIDefinitionFilename_in.empty ())
      mediafoundation_configuration.streamConfiguration.module =
        &mediafoundation_event_handler;
    mediafoundation_configuration.streamConfiguration.moduleConfiguration =
      &mediafoundation_configuration.moduleConfiguration;
    mediafoundation_configuration.streamConfiguration.mediaFoundationConfiguration =
      &mediafoundation_configuration.mediaFoundationConfiguration;
    mediafoundation_configuration.streamConfiguration.moduleHandlerConfiguration =
      &mediafoundation_configuration.moduleHandlerConfiguration;
    mediafoundation_configuration.streamConfiguration.printFinalReport = true;
    mediafoundation_configuration.streamConfiguration.statisticReportingInterval =
      statisticReportingInterval_in;
  } // end IF
  else
  {
    if (bufferSize_in)
      directshow_configuration.streamConfiguration.bufferSize = bufferSize_in;
    directshow_configuration.streamConfiguration.messageAllocator =
      &directshow_message_allocator;
    if (!UIDefinitionFilename_in.empty ())
      directshow_configuration.streamConfiguration.module =
        &directshow_event_handler;
    directshow_configuration.streamConfiguration.moduleConfiguration =
      &directshow_configuration.moduleConfiguration;
    directshow_configuration.streamConfiguration.moduleHandlerConfiguration =
      &directshow_configuration.moduleHandlerConfiguration;
    directshow_configuration.streamConfiguration.printFinalReport = true;
    directshow_configuration.streamConfiguration.statisticReportingInterval =
      statisticReportingInterval_in;
  } // end ELSE
#else
  if (bufferSize_in)
    v4l2_configuration.streamConfiguration.bufferSize = bufferSize_in;
  v4l2_configuration.streamConfiguration.messageAllocator = &message_allocator;
  if (!UIDefinitionFilename_in.empty ())
    v4l2_configuration.streamConfiguration.module = &event_handler;
  v4l2_configuration.streamConfiguration.moduleConfiguration =
    &v4l2_configuration.moduleConfiguration;
  v4l2_configuration.streamConfiguration.moduleHandlerConfiguration =
    &v4l2_configuration.moduleHandlerConfiguration;
  v4l2_configuration.streamConfiguration.printFinalReport = true;
  v4l2_configuration.streamConfiguration.statisticReportingInterval =
    statisticReportingInterval_in;
#endif

  // step0d: initialize regular (global) statistic reporting
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();
  if (statisticReportingInterval_in)
  {
    event_handler_p = &statistic_handler;
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (event_handler_p,            // event handler
                                       NULL,                       // ACT
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));
      goto clean;
    } // end IF
  } // end IF

  // step0e: initialize signal handling
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    mediafoundation_configuration.signalHandlerConfiguration.useReactor =
      useReactor_in;
    mediafoundation_configuration.signalHandlerConfiguration.stream =
      mediaFoundationCBData_in.stream;
    result =
      mediafoundation_signal_handler.initialize (mediafoundation_configuration.signalHandlerConfiguration);
    event_handler_p = &mediafoundation_signal_handler;
  } // end IF
  else
  {
    directshow_configuration.signalHandlerConfiguration.useReactor =
      useReactor_in;
    directshow_configuration.signalHandlerConfiguration.stream =
      directShowCBData_in.stream;
    result =
      directshow_signal_handler.initialize (directshow_configuration.signalHandlerConfiguration);
    event_handler_p = &directshow_signal_handler;
  } // end IF
#else
  v4l2_configuration.signalHandlerConfiguration.useReactor =
    useReactor_in;
  v4l2_configuration.signalHandlerConfiguration.stream =
    v4l2CBData_in.stream;
  result =
    signal_handler.initialize (v4l2_configuration.signalHandlerConfiguration);
  event_handler_p = &signal_handler;
#endif
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    goto clean;
  } // end IF
  ACE_ASSERT (event_handler_p);
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        event_handler_p,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), aborting\n")));
    goto clean;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistic collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step1a: start GTK event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
    Common_UI_GTKState* gtk_state_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (useMediaFoundation_in)
    {
      gtk_state_p = &mediaFoundationCBData_in;
      gtk_state_p->userData = &mediaFoundationCBData_in;
    } // end IF
    else
    {
      gtk_state_p = &directShowCBData_in;
      gtk_state_p->userData = &directShowCBData_in;
    } // end ELSE
#else
    gtk_state_p = &v4l2CBData_in;
    gtk_state_p->userData = &v4l2CBData_in;
#endif
    ACE_ASSERT (gtk_state_p);
    gtk_state_p->finalizationHook = idle_finalize_source_UI_cb;
    gtk_state_p->initializationHook = idle_initialize_source_UI_cb;
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    gtk_state_p->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<GtkBuilder*> (NULL));

    Test_I_Source_GTK_Manager_t* gtk_manager_p =
      TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    gtk_manager_p->start ();
//    ACE_Time_Value one_second (1, 0);
//    int result = ACE_OS::sleep (one_second);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
    if (!gtk_manager_p->isRunning ())
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
      goto clean;
    } // end IF
    BOOL was_visible_b = false;
    if (!showConsole_in)
      was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif
  } // end IF

  // step1b: initialize worker(s)
  thread_data.numberOfDispatchThreads = numberOfDispatchThreads_in;
  thread_data.useReactor = useReactor_in;
  if (!Common_Tools::startEventDispatch (thread_data,
                                         group_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
    goto clean;
  } // end IF

  if (UIDefinitionFilename_in.empty ())
  {
    result = false;
    Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (useMediaFoundation_in)
    {
      if (mediafoundation_configuration.protocol == NET_TRANSPORTLAYER_TCP)
      {
        stream_p = mediaFoundationCBData_in.stream;
        result =
          mediaFoundationCBData_in.stream->initialize (mediafoundation_configuration.streamConfiguration);
      } // end IF
      else
      {
        stream_p = mediaFoundationCBData_in.UDPStream;
        result =
          mediaFoundationCBData_in.UDPStream->initialize (mediafoundation_configuration.streamConfiguration);
      } // end ELSE
    } // end IF
    else
    {
      if (directshow_configuration.protocol == NET_TRANSPORTLAYER_TCP)
      {
        stream_p = directShowCBData_in.stream;
        result =
          directShowCBData_in.stream->initialize (directshow_configuration.streamConfiguration);
      } // end IF
      else
      {
        stream_p = directShowCBData_in.UDPStream;
        result =
          directShowCBData_in.UDPStream->initialize (directshow_configuration.streamConfiguration);
      } // end ELSE
    } // end ELSE
#else
    if (v4l2_configuration.protocol == NET_TRANSPORTLAYER_TCP)
    {
      stream_p = v4l2CBData_in.stream;
      result =
        v4l2CBData_in.stream->initialize (v4l2_configuration.streamConfiguration);
    } // end IF
    else
    {
      stream_p = v4l2CBData_in.UDPStream;
      result =
        v4l2CBData_in.UDPStream->initialize (v4l2_configuration.streamConfiguration);
    } // end ELSE
#endif
    if (!result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, aborting\n")));

      // clean up
      Common_Tools::finalizeEventDispatch (useReactor_in,
                                           !useReactor_in,
                                           group_id);

      goto clean;
    } // end IF
    ACE_ASSERT (stream_p);

    // *NOTE*: this call blocks until an error occurs
    stream_p->start ();
    //    if (!stream_p->isRunning ())
    //    {
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to start stream, aborting\n")));

    //      // clean up
    //      //timer_manager_p->stop ();

    //      return;
    //    } // end IF
    stream_p->wait (true, false, false);

    // clean up
//    connection_manager_p->stop ();
  } // end IF
  else
  {
    result_2 = TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->wait ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_Base::wait (): \"%m\", continuing\n")));

//    connection_manager_p->abort ();
  } // end ELSE
  iconnection_manager_p->wait ();
  Common_Tools::finalizeEventDispatch (useReactor_in,
                                       !useReactor_in,
                                       group_id);

  // step3: clean up
clean:
//  if (!UIDefinitionFilename_in.empty ())
//    TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->stop ();
  timer_manager_p->stop (true, true);

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

  //result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  //if (result == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
  //              event_handler.name ()));

  Common_Tools::finalizeSignals (signalSet_in,
                                 previousSignalActions_inout,
                                 previousSignalMask_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (useMediaFoundation_in)
  {
    delete mediaFoundationCBData_in.stream;
    delete mediaFoundationCBData_in.UDPStream;

    do_finalize_mediafoundation ();
  } // end IF
  else
  {
    delete directShowCBData_in.stream;
    delete directShowCBData_in.UDPStream;

    do_finalize_directshow (directShowCBData_in);
  } // end ELSE
#else
  delete v4l2CBData_in.stream;
  delete v4l2CBData_in.UDPStream;
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
  Common_Tools::initialize ();

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  unsigned int buffer_size = TEST_I_DEFAULT_BUFFER_SIZE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#else
  std::string device_filename =
      ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEVICE_DIRECTORY);
  device_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  device_filename += ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEFAULT_VIDEO_DEVICE);
#endif
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  std::string gtk_rc_filename;
  //std::string gtk_rc_filename = path;
  //gtk_rc_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //gtk_rc_filename += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  std::string gtk_glade_filename = path;
  gtk_glade_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_glade_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  std::string host_name = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_HOSTNAME);
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool use_mediafoundation =
    TEST_I_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION;
#endif
  bool use_thread_pool = NET_EVENT_USE_THREAD_POOL;
  unsigned short port = TEST_I_DEFAULT_PORT;
  bool use_reactor = NET_EVENT_USE_REACTOR;
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information = false;
  bool use_UDP = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    NET_CLIENT_DEFAULT_NUMBER_OF_DISPATCH_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#else
                            device_filename,
#endif
                            gtk_rc_filename,
                            gtk_glade_filename,
                            host_name,
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            use_mediafoundation,
#endif
                            use_thread_pool,
                            port,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            use_UDP,
                            print_version_and_exit,
                            number_of_dispatch_threads))
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
  if (TEST_I_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if (use_reactor                      &&
      (number_of_dispatch_threads > 1) &&
      !use_thread_pool)
  { // *NOTE*: see also: man (2) select
    // *TODO*: verify this for MS Windows based systems
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
    use_thread_pool = true;
  } // end IF
  if ((!gtk_glade_filename.empty () &&
       !Common_File_Tools::isReadable (gtk_glade_filename))                 ||
      (!gtk_rc_filename.empty () &&
       !Common_File_Tools::isReadable (gtk_rc_filename))                    ||
      (use_thread_pool && !use_reactor)                                     ||
      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool))
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
  if (number_of_dispatch_threads == 0) number_of_dispatch_threads = 1;

  struct Test_I_GTK_CBData* gtk_cb_user_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_Source_DirectShow_GTK_CBData directshow_gtk_cb_user_data;
  struct Test_I_Source_MediaFoundation_GTK_CBData mediafoundation_gtk_cb_user_data;
  if (use_mediafoundation)
  {
    mediafoundation_gtk_cb_user_data.progressData.GTKState =
      &mediafoundation_gtk_cb_user_data;
    mediafoundation_gtk_cb_user_data.useMediaFoundation = use_mediafoundation;
    gtk_cb_user_data_p = &mediafoundation_gtk_cb_user_data;
  } // end IF
  else
  {
    directshow_gtk_cb_user_data.progressData.GTKState =
      &directshow_gtk_cb_user_data;
    directshow_gtk_cb_user_data.useMediaFoundation = use_mediafoundation;
    gtk_cb_user_data_p = &directshow_gtk_cb_user_data;
  } // end ELSE
#else
  Test_I_Source_V4L2_GTK_CBData v4l2_gtk_cb_user_data;
  v4l2_gtk_cb_user_data.progressData.GTKState = &v4l2_gtk_cb_user_data;
  gtk_cb_user_data_p = &v4l2_gtk_cb_user_data;
#endif
  ACE_ASSERT (gtk_cb_user_data_p);
  // step1d: initialize logging and/or tracing
  Common_Logger_t logger (&gtk_cb_user_data_p->logStack,
                          &gtk_cb_user_data_p->logStackLock);
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
                                        (gtk_glade_filename.empty () ? NULL
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
                                           use_reactor,
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
  if (!Common_Tools::setResourceLimits (false, // file descriptors
                                        true,  // stack traces
                                        true)) // pending signals
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
  gtk_cb_user_data_p->RCFiles.push_back (gtk_rc_filename);
  Test_I_Source_GtkBuilderDefinition_t ui_definition (argc_in,
                                                      argv_in);
  if (!gtk_glade_filename.empty ())
    if (!TEST_I_SOURCE_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                                       argv_in,
                                                                       gtk_cb_user_data_p,
                                                                       &ui_definition))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager::initialize(), aborting\n")));

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

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#else
           device_filename,
#endif
           gtk_glade_filename,
           host_name,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           use_mediafoundation,
#endif
           use_thread_pool,
           port,
           use_reactor,
           statistic_reporting_interval,
           use_UDP,
           number_of_dispatch_threads,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           mediafoundation_gtk_cb_user_data,
           directshow_gtk_cb_user_data,
#else
           v4l2_gtk_cb_user_data,
#endif
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           previous_signal_mask);
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

  Common_Tools::finalizeLogging ();
  Common_Tools::finalize ();

  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;
} // end main
