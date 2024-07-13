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
#include "amvideo.h"
#undef NANOSECONDS
#include "reftime.h"
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#else
#include "X11/Xlib.h"

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include <iostream>
#include <string>

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

#include "common_file_tools.h"
#include "common_os_tools.h"
#include "common_process_tools.h"

#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_lib_defines.h"
#include "stream_lib_tools.h"

#include "stream_misc_defines.h"

#include "stream_vis_tools.h"

#include "test_u_defines.h"

#include "test_u_capturewindow_defines.h"
#include "test_u_eventhandler.h"
#include "test_u_signalhandler.h"
#include "test_u_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("CaptureWindowStream");

void
do_print_usage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_print_usage"));

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f          : executable name [\"")
            << ACE_TEXT_ALWAYS_CHAR ("\"])")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i          : (X11) window id [")
            << 0
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m          : use MediaFoundation framework [")
            << (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p          : process id [")
            << 0
            << ACE_TEXT_ALWAYS_CHAR ("])")
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
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                      Window& windowId_out,
#endif // ACE_WIN32 || ACE_WIN64
                      pid_t& processId_out,
                      std::string& executableName_out,
                      bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      enum Stream_MediaFramework_Type mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                      bool& traceInformation_out,
                      enum Test_U_CaptureWindow_ProgramMode& mode_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_process_arguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  windowId_out = 0;
#endif // ACE_WIN32 || ACE_WIN64
  processId_out = 0;
  executableName_out.clear ();
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework_out = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  traceInformation_out = false;
  mode_out = TEST_U_PROGRAMMODE_NORMAL;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("f:lp:tv");
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  options_string += ACE_TEXT_ALWAYS_CHAR ("m");
#else
  options_string += ACE_TEXT_ALWAYS_CHAR ("i:");
#endif // ACE_WIN32 || ACE_WIN64

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT (options_string.c_str ()),
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
      case 'f':
      {
        executableName_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      case 'i':
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();

        converter.str (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
        converter >> windowId_out;
        if (!windowId_out)
        { // try hexadecimal
          converter.str (ACE_TEXT_ALWAYS_CHAR (""));
          converter.clear ();
          converter << std::hex << ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
          converter >> windowId_out;
          converter << std::dec;
        } // end IF
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
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
      case 'p':
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();

        converter.str (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
        converter >> processId_out;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'v':
      {
        mode_out = TEST_U_PROGRAMMODE_PRINT_VERSION;
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
do_initialize_directshow (HWND windowHandle_in,
                          struct _AMMediaType& outputFormat_inout,
                          struct Common_AllocatorConfiguration& allocatorConfiguration_inout)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  // sanity check(s)
  ACE_ASSERT (!GetParent (windowHandle_in));

  struct tagRECT window_size_s;
  BOOL result_2 = GetClientRect (windowHandle_in, &window_size_s);
  ACE_ASSERT (result_2);

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

  ACE_OS::memset (&outputFormat_inout, 0, sizeof (struct _AMMediaType));
  outputFormat_inout.majortype = MEDIATYPE_Video;
  outputFormat_inout.subtype = MEDIASUBTYPE_RGB24;
  outputFormat_inout.bFixedSizeSamples = TRUE;
  outputFormat_inout.bTemporalCompression = FALSE;
  outputFormat_inout.formattype = FORMAT_VideoInfo;
  outputFormat_inout.cbFormat = sizeof (struct tagVIDEOINFOHEADER);
  outputFormat_inout.pbFormat =
    static_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tagVIDEOINFOHEADER)));
  ACE_ASSERT (outputFormat_inout.pbFormat);
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (outputFormat_inout.pbFormat);
  ACE_OS::memset (outputFormat_inout.pbFormat, 0, sizeof (struct tagVIDEOINFOHEADER));
  // *NOTE*: empty --> use entire video
  result_2 = SetRectEmpty (&video_info_header_p->rcSource);
  ACE_ASSERT (result_2);
  result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
  // *NOTE*: empty --> fill entire buffer
  ACE_ASSERT (result_2);
  ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
  video_info_header_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_header_p->bmiHeader.biWidth =
    window_size_s.right - window_size_s.left;
  video_info_header_p->bmiHeader.biHeight =
    window_size_s.bottom - window_size_s.top;
  video_info_header_p->bmiHeader.biPlanes = 1;
  video_info_header_p->bmiHeader.biBitCount = 24;
  video_info_header_p->bmiHeader.biCompression = BI_RGB;
  video_info_header_p->bmiHeader.biSizeImage =
    DIBSIZE (video_info_header_p->bmiHeader);
  ////video_info_header_p->bmiHeader.biXPelsPerMeter;
  ////video_info_header_p->bmiHeader.biYPelsPerMeter;
  ////video_info_header_p->bmiHeader.biClrUsed;
  ////video_info_header_p->bmiHeader.biClrImportant;
  video_info_header_p->AvgTimePerFrame = UNITS / 30;
  video_info_header_p->dwBitRate =
    (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
    (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

  outputFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;

  allocatorConfiguration_inout.defaultBufferSize =
    video_info_header_p->bmiHeader.biSizeImage;

  return true;

//error:
  Stream_MediaFramework_DirectShow_Tools::free (outputFormat_inout);

  return false;
}

void
do_finalize_directshow (struct _AMMediaType& outputFormat_inout)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  Stream_MediaFramework_DirectShow_Tools::free (outputFormat_inout);
}

bool
do_initialize_mediafoundation (HWND windowHandle_in
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
                               , IMFMediaSession*& IMFMediaSession_out)
#else
                              )
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
  IMFTopology* topology_p = NULL;

  // sanity check(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (!IMFMediaSession_out);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  result = MFStartup (MF_VERSION,
                      MFSTARTUP_LITE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFStartup(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION);

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  IMFAttributes* attributes_p = NULL;
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
  //result = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
  //ACE_ASSERT (SUCCEEDED (result));
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
  result = MFCreateMediaSession (attributes_p,
                                 &IMFMediaSession_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    attributes_p->Release (); attributes_p = NULL;
    goto error;
  } // end IF
  attributes_p->Release (); attributes_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  ACE_ASSERT (topology_p);
  DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);// |
                          //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                          //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
  result = IMFMediaSession_out->SetTopology (topology_flags,
                                             topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  topology_p->Release (); topology_p = NULL;

//continue_3:
  return true;

error:
  if (topology_p)
    topology_p->Release ();
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  if (IMFMediaSession_out)
  {
    IMFMediaSession_out->Release (); IMFMediaSession_out = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  return false;
}

void
do_finalize_mediafoundation (IMFMediaSession*& mediaSession_inout)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_mediafoundation"));

  HRESULT result = E_FAIL;

  if (mediaSession_inout)
  {
    result = mediaSession_inout->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_inout->Release (); mediaSession_inout = NULL;
  } // end IF

  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
}
#else
bool
do_initialize (Window windowHandle_in,
               struct Stream_MediaFramework_FFMPEG_VideoMediaType& outputFormat_out,
               struct Common_AllocatorConfiguration& allocatorConfiguration_inout)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize"));

  // intialize return value(s)
  ACE_OS::memset (&outputFormat_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  Display* display_p = XOpenDisplay (NULL);
  ACE_ASSERT (display_p);
  Window window;
  int x, y;
  unsigned int width, height, border_width, depth;
  Status result = XGetGeometry (display_p,
                                windowHandle_in,
                                &window,
                                &x, &y,
                                &width, &height,
                                &border_width,
                                &depth);
  if (unlikely (result == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to XGetGeometry(): \"%s\", aborting\n"),
                ACE_TEXT (Common_UI_Tools::toString (*display_p, result).c_str ())));
    result = XCloseDisplay (display_p); display_p = NULL;
    ACE_ASSERT (result == Success);
    return false;
  } // end IF
  result = XCloseDisplay (display_p); display_p = NULL;
  ACE_ASSERT (result == Success);

  // *NOTE*: Gtk 2 expects RGB24
  // *NOTE*: "...CAIRO_FORMAT_ARGB32: each pixel is a 32-bit quantity, with
  //         alpha in the upper 8 bits, then red, then green, then blue. The
  //         32-bit quantities are stored native-endian. ..."
  // *NOTE*: X11 expects RGB32
  // *TODO*: auto-determine color depth of selected (default) screen (i.e.
  //         'Display' ":0")
  outputFormat_out.format = AV_PIX_FMT_RGB24;
  outputFormat_out.resolution.width = width;
  outputFormat_out.resolution.height = height;
  outputFormat_out.codec = AV_CODEC_ID_NONE;
  outputFormat_out.frameRate.num = 30;
  outputFormat_out.frameRate.den = 1;

  allocatorConfiguration_inout.defaultBufferSize =
    av_image_get_buffer_size (outputFormat_out.format,
                              width, height,
                              1);

  return true;

//error:
  return false;
}

void
do_finalize ()
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize"));
}
#endif // ACE_WIN32 || ACE_WIN64

void
do_initializeSignals (ACE_Sig_Set& signals_out)
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

  // *PORTABILITY*: on Windows most signals are not defined,
  // and ACE_Sig_Set::fill_set() doesn't really work as specified
  // --> add valid signals (see <signal.h>)...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signals_out.sig_add (SIGINT);            // 2       /* interrupt */
  signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
  signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
  //  signals_out.sig_add(SIGSEGV);          // 11      /* segment violation */
  signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
  signals_out.sig_add (SIGBREAK);        // 21      /* Ctrl-Break sequence */
//  ignoredSignals_out.sig_add (SIGBREAK); // 21      /* Ctrl-Break sequence */
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
  // *NOTE*: cannot handle some signals --> registration fails for these...
  signals_out.sig_del (SIGKILL);           // 9       /* Kill signal */
  // ---------------------------------------------------------------------------
  // *NOTE* core dump on SIGSEGV
  signals_out.sig_del (SIGSEGV);           // 11      /* Segmentation fault: Invalid memory reference */
  // *NOTE* don't care about SIGPIPE
  signals_out.sig_del (SIGPIPE);           // 12      /* Broken pipe: write to pipe with no readers */
  signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */

  // *IMPORTANT NOTE*: "...NPTL makes internal use of the first two real-time
  //                   signals (see also signal(7)); these signals cannot be
  //                   used in applications. ..." (see 'man 7 pthreads')
  // --> on POSIX platforms, make sure that ACE_SIGRTMIN == 34
//  for (int i = ACE_SIGRTMIN;
//       i <= ACE_SIGRTMAX;
//       i++)
//    signals_out.sig_del (i);
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         HWND windowHandle_in,
         enum Stream_MediaFramework_Type mediaFramework_in,
         struct Test_U_DirectShow_Configuration& directShowConfiguration_in,
         struct Test_U_MediaFoundation_Configuration& mediaFoundationConfiguration_in
#else
         int argc_in, ACE_TCHAR** argv_in,
         Window windowHandle_in,
         struct Test_U_CaptureWindow_Configuration& configuration_in
#endif // ACE_WIN32 || ACE_WIN64
         )
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Common_UI_GTK_Tools::initialize (argc_in, argv_in);
#endif // ACE_WIN32 || ACE_WIN64

  ACE_Sig_Set handled_signals, ignored_signals, previous_mask;
  Common_SignalActions_t previous_actions_a;
  do_initializeSignals (handled_signals);

  Test_U_SignalHandler signal_handler;
  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      result =
        signal_handler.initialize (directShowConfiguration_in.signalHandlerConfiguration);
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      result =
        signal_handler.initialize (mediaFoundationConfiguration_in.signalHandlerConfiguration);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  result =
    signal_handler.initialize (configuration_in.signalHandlerConfiguration);
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Test_U_SignalHandler::initialize(): \"%m\", returning\n")));
    return;
  } // end IF
  if (!Common_Signal_Tools::preInitialize (handled_signals,
                                           COMMON_SIGNAL_DISPATCH_SIGNAL,
                                           false,
                                           false,
                                           previous_actions_a,
                                           previous_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(): \"%m\", returning\n")));
    return;
  } // end IF
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                        handled_signals,
                                        ignored_signals,
                                        &signal_handler,
                                        previous_actions_a))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(): \"%m\", returning\n")));
    return;
  } // end IF

  // ********************** module configuration data **************************
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration allocator_configuration;
  //allocator_configuration.defaultBufferSize = 524288;
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration codec_configuration;
  codec_configuration.codecId = AV_CODEC_ID_H264;
#else
  struct Stream_AllocatorConfiguration allocator_configuration;
#endif // FFMPEG_SUPPORT
  struct Stream_ModuleConfiguration module_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2; // converter/resize --> encoder
  struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_3; // display
  struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_4; // converter --> display
  struct Test_U_CaptureWindow_DirectShow_StreamConfiguration directshow_stream_configuration;
  Test_U_DirectShow_EventHandler_t directshow_ui_event_handler;
  struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_2; // converter
  struct Test_U_CaptureWindow_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration;
  Test_U_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler;
#else
  struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration modulehandler_configuration_2; // converter
  struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration modulehandler_configuration_3; // converter_2
  struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration modulehandler_configuration_4; // display
  Test_U_EventHandler_t ui_event_handler;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
      directshow_modulehandler_configuration.direct3DConfiguration =
        &directShowConfiguration_in.direct3DConfiguration;
#if defined (FFMPEG_SUPPORT)
      directshow_modulehandler_configuration.codecConfiguration = &codec_configuration;
#endif // FFMPEG_SUPPORT
      directshow_modulehandler_configuration.fileFormat = ACE_TEXT_ALWAYS_CHAR ("mp4");
      directshow_modulehandler_configuration.window = windowHandle_in;

      //if (statisticReportingInterval_in)
      //{
      //  directshow_modulehandler_configuration.statisticCollectionInterval.set (0,
      //                                                                          STREAM_DEV_CAM_STATISTIC_COLLECTION_INTERVAL * 1000);
      //  directshow_modulehandler_configuration.statisticReportingInterval =
      //    statisticReportingInterval_in;
      //} // end IF
      directshow_modulehandler_configuration.subscriber =
        &directshow_ui_event_handler;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
      mediafoundation_modulehandler_configuration.direct3DConfiguration =
        &mediaFoundationConfiguration_in.direct3DConfiguration;
      //mediafoundation_modulehandler_configuration.lock = &state_r.subscribersLock;

      //if (statisticReportingInterval_in)
      //{
      //  mediafoundation_modulehandler_configuration.statisticCollectionInterval.set (0,
      //                                                                               STREAM_DEV_CAM_STATISTIC_COLLECTION_INTERVAL * 1000);
      //  mediafoundation_modulehandler_configuration.statisticReportingInterval =
      //    statisticReportingInterval_in;
      //} // end IF
      mediafoundation_modulehandler_configuration.subscriber =
        &mediafoundation_ui_event_handler;
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
  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
  modulehandler_configuration.codecId = AV_CODEC_ID_H264;
  modulehandler_configuration.fileFormat = ACE_TEXT_ALWAYS_CHAR ("mp4");
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.window = windowHandle_in;

  struct Test_U_CaptureWindow_StreamConfiguration stream_configuration;
#endif // ACE_WIN32 || ACE_WIN64

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                     &heap_allocator,     // heap allocator handle
                                                                     true);               // block ?
  Test_U_DirectShow_Stream directshow_stream;
  Test_U_DirectShow_MessageHandler_Module directshow_message_handler (&directshow_stream,
                                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  Test_U_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                               &heap_allocator,     // heap allocator handle
                                                                               true);               // block ?
  Test_U_MediaFoundation_Stream mediafoundation_stream;
  Test_U_MediaFoundation_MessageHandler_Module mediafoundation_message_handler (&mediafoundation_stream,
                                                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directShowConfiguration_in.signalHandlerConfiguration.stream =
        &directshow_stream;

      directshow_stream_configuration.allocatorConfiguration = &allocator_configuration;
      directshow_stream_configuration.messageAllocator =
          &directshow_message_allocator;
      directshow_stream_configuration.module =
        &directshow_message_handler;
      //directshow_stream_configuration.renderer = STREAM_VISUALIZATION_VIDEORENDERER_GDI;
      directshow_stream_configuration.renderer = STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D;

      directShowConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                 directshow_modulehandler_configuration,
                                                                 directshow_stream_configuration);

      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_2)));
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_2)));

      directshow_modulehandler_configuration_3 = directshow_modulehandler_configuration;
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (Stream_Visualization_Tools::rendererToModuleName (directshow_stream_configuration.renderer),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_3)));
      directshow_modulehandler_configuration_4 = directshow_modulehandler_configuration;
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("LibAV_Converter_2"),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_4)));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //if (bufferSize_in)
      //  mediaFoundationCBData_in.configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
      //      bufferSize_in;
      mediafoundation_stream_configuration.messageAllocator =
          &mediafoundation_message_allocator;
      mediafoundation_stream_configuration.module =
          &mediafoundation_message_handler;
      //mediaFoundationConfiguration_in.streamConfiguration.configuration_.renderer =
      //  renderer_in;

      mediaFoundationConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                      mediafoundation_modulehandler_configuration,
                                                                      mediafoundation_stream_configuration);
      //mediafoundation_stream_iterator =
      //  mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      //ACE_ASSERT (mediafoundation_stream_iterator != mediaFoundationConfiguration_in.streamConfiguration.end ());
      //mediafoundation_stream_iterator_2 =
      //  mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING));
      //ACE_ASSERT (mediafoundation_stream_iterator_2 != mediaFoundationConfiguration_in.streamConfiguration.end ());
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
  Test_U_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?
  Test_U_Stream stream;
  Test_U_MessageHandler_Module message_handler (&stream,
                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  configuration_in.signalHandlerConfiguration.stream = &stream;

  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &message_handler;
  stream_configuration.renderer = STREAM_VISUALIZATION_VIDEORENDERER_X11;
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);

  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Stream_IStreamControlBase* stream_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IMFMediaSession* media_session_p = NULL;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (!do_initialize_directshow (windowHandle_in,
                                     directshow_stream_configuration.format,
                                     allocator_configuration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::do_initialize_directshow(), returning\n")));
        return;
      } // end IF
      struct _AMMediaType* media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_stream_configuration.format);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration.outputFormat = *media_type_p;
      delete media_type_p; media_type_p = NULL;

      // *NOTE*: need to set this for RGB-capture formats ONLY !
      directshow_modulehandler_configuration_2.flipImage =
        Stream_MediaFramework_DirectShow_Tools::isMediaTypeBottomUp (directshow_stream_configuration.format);

      media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_stream_configuration.format);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration_2.outputFormat = *media_type_p;
      delete media_type_p; media_type_p = NULL;
      directshow_modulehandler_configuration_2.outputFormat.subtype =
        MEDIASUBTYPE_NV12; // *NOTE*: required by H264
        //MEDIASUBTYPE_IMC1; // *NOTE*: maps to AV_PIX_FMT_YUV420P, required for H263
      //Common_Image_Resolution_t resolution_s;
      //resolution_s.cx = 704;
      //resolution_s.cy = 576;
      //Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_s,
      //                                                       directshow_modulehandler_configuration_2.outputFormat);

      media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_stream_configuration.format);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration_3.outputFormat = *media_type_p;
      delete media_type_p; media_type_p = NULL;
      directshow_modulehandler_configuration_3.window = NULL;

      directshow_modulehandler_configuration_4.flipImage = true;
      media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_stream_configuration.format);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration_4.outputFormat = *media_type_p;
      delete media_type_p; media_type_p = NULL;
      Stream_MediaFramework_DirectShow_Tools::setFormat (MEDIASUBTYPE_RGB32,
                                                         directshow_modulehandler_configuration_4.outputFormat);

      stream_p = &directshow_stream;

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (!do_initialize_mediafoundation (windowHandle_in
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
                                          , mediafoundation_modulehandler_configuration.session))
#else
                                         ))
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::do_initialize_mediafoundation(), returning\n")));
        return;
      } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT (mediafoundation_modulehandler_configuration.session);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

      stream_p = &mediafoundation_stream;

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
  if (!do_initialize (windowHandle_in,
                      stream_configuration.format,
                      allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::do_initialize(), returning\n")));
    return;
  } // end IF
  stream_p = &stream;

  switch (stream_configuration.renderer)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
    {
      modulehandler_configuration.outputFormat =
        stream_configuration.format;

      // *IMPORTANT NOTE*: there does not seem to be a way to feed RGB24 data to
      //                   Xlib; XCreateImage() only 'likes' 32-bit data, regardless
      //                   of what 'depth' values are set (in fact, it requires BGRA
      //                   on little-endian platforms) --> convert
      modulehandler_configuration_2 = modulehandler_configuration;
      modulehandler_configuration_2.outputFormat.format =
        AV_PIX_FMT_BGRA;

      modulehandler_configuration_3 = modulehandler_configuration;
      modulehandler_configuration_3.outputFormat.format =
        AV_PIX_FMT_NV12; // *NOTE*: required by H264

      modulehandler_configuration_4 = modulehandler_configuration;
      modulehandler_configuration_4.window = 0;
      break;
    }
    default:
      break;
  } // end SWITCH
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_2)));
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("LibAV_Converter_2"),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_3)));
  configuration_in.streamConfiguration.insert (std::make_pair (Stream_Visualization_Tools::rendererToModuleName (stream_configuration.renderer),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_4)));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  // intialize timers
  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

  // step0f: (initialize) processing stream

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (!directshow_stream.initialize (directShowConfiguration_in.streamConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, returning\n")));
        goto clean;
      } // end IF
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (!mediafoundation_stream.initialize (mediaFoundationConfiguration_in.streamConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, returning\n")));
        goto clean;
      } // end IF
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
  if (!stream.initialize (configuration_in.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, returning\n")));
    goto clean;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  stream_p->start ();
  stream_p->wait (true,
                  false,
                  false);

  // step3: clean up
clean:
  timer_manager_p->stop ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      do_finalize_directshow (directshow_stream_configuration.format);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      do_finalize_mediafoundation (mediafoundation_modulehandler_configuration.session);
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
  do_finalize ();

  stream.remove (&message_handler,
                 true,
                 true);
#endif // ACE_WIN32 || ACE_WIN64
  Common_Signal_Tools::finalize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                 previous_actions_a,
                                 previous_mask);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

COMMON_DEFINE_PRINTVERSION_FUNCTION(do_print_version,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

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
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  // initialize framework(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_Tools::initialize (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
#endif // ACE_WIN32 || ACE_WIN64

  // step1a set defaults
  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();
  pid_t process_id = 0;
  std::string executable_name_string;
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  bool trace_information = false;
  enum Test_U_CaptureWindow_ProgramMode program_mode_e =
    TEST_U_PROGRAMMODE_NORMAL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::vector<HWND> window_handles_a;
  HWND window_handle_h = NULL;
#else
  Window window_handle_h = 0;
#endif // ACE_WIN32 || ACE_WIN64

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                             window_handle_h,
#endif // ACE_WIN32 || ACE_WIN64
                             process_id,
                             executable_name_string,
                             log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                             trace_information,
                             program_mode_e))
  {
    do_print_usage (ACE::basename (argv_in[0]));
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
                ACE_TEXT ("limiting the number of message buffers could (!) lead to a deadlock --> ensure the streaming elements are sufficiently efficient in this regard\n")));
  if (window_handle_h)
    goto continue_;
  if (!executable_name_string.empty ())
  {
    process_id = Common_Process_Tools::id (executable_name_string);
    if (!process_id)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Process_Tools::id(\"%s\"), aborting\n"),
                  ACE_TEXT (executable_name_string.c_str ())));
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
  } // end IF
  if (unlikely (!process_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid process id, aborting\n")));
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  window_handles_a = Common_Process_Tools::window (process_id);
  ACE_ASSERT (!window_handles_a.empty ());
  window_handle_h = window_handles_a[1]; // *TODO*: how to choose the right one ?
#else
  window_handle_h = Common_Process_Tools::window (process_id);
#endif
  if (unlikely (!window_handle_h))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Process_Tools::window(\"%d\"), aborting\n"),
                process_id));
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
continue_:
  if (false)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_print_usage (ACE::basename (argv_in[0]));
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

  // step1d: initialize logging and/or tracing
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
                                            NULL))                                        // (ui) logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

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

  // step1f: handle specific program modes
  switch (program_mode_e)
  {
    case TEST_U_PROGRAMMODE_PRINT_VERSION:
    {
      do_print_version (ACE::basename (argv_in[0]));

      Common_Log_Tools::finalizeLogging ();
      Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_SUCCESS;
    }
    case TEST_U_PROGRAMMODE_NORMAL:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown program mode (was: %d), aborting\n"),
                  program_mode_e));

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Visualization_Tools::initialize (STREAM_VIS_FRAMEWORK_DEFAULT);

  struct Test_U_DirectShow_Configuration directshow_configuration;
  struct Test_U_MediaFoundation_Configuration mediafoundation_configuration;
#else
  struct Test_U_CaptureWindow_Configuration configuration;
#endif // ACE_WIN32 || ACE_WIN64

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           window_handle_h,
           media_framework_e,
           directshow_configuration,
           mediafoundation_configuration
#else
           argc_in, argv_in,
           window_handle_h,
           configuration
#endif // ACE_WIN32 || ACE_WIN64
          );
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

    Common_Log_Tools::finalizeLogging ();
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

  Common_Log_Tools::finalizeLogging ();
  Common_Tools::finalize ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: on Windows, finalize ACE...
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
