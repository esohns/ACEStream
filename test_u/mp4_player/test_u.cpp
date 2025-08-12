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
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#include "mfapi.h"
#endif // ACE_WIN32 || ACE_WIN64

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

#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_tools.h"
#endif // GTK_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_dev_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_dev_tools.h"

#include "stream_lib_tools.h"

#include "stream_misc_defines.h"

#include "stream_vis_tools.h"

#include "test_u_defines.h"

#include "test_u_eventhandler.h"
#include "test_u_mp4_player_common.h"
#include "test_u_mp4_player_defines.h"
#include "test_u_signalhandler.h"
#include "test_u_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("MP4PlayerStream");

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
//#if defined (CURSES_SUPPORT)
//  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : use curses [")
//            << false
//            << ACE_TEXT_ALWAYS_CHAR ("])")
//            << std::endl;
//#endif // CURSES_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-1          : use GDI renderer [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-2          : use Direct2D renderer [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-3          : use Direct3D renderer [")
            << true
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-1          : use X11 renderer [")
            << true
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : input file [\"")
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  //std::cout << ACE_TEXT_ALWAYS_CHAR ("-g          : OpenGL mode [")
  //          << false
  //          << ACE_TEXT_ALWAYS_CHAR ("]")
  //          << std::endl;
#if defined (GTK_SUPPORT)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-k          : use Gtk [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#endif // GTK_SUPPORT
  std::string path = Common_File_Tools::getTempDirectory ();
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
  struct Common_UI_DisplayDevice display_device_s =
    Common_UI_Tools::getDefaultDisplay ();
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o [STRING] : display device [\"")
            << display_device_s.description
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x          : test device for method support and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-z          : use hardware decoder [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
}

bool
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
                      bool& debug_out,
                      std::string& inputFile_out,
                      bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      enum Stream_MediaFramework_Type& mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                      struct Common_UI_DisplayDevice& displayDevice_out,
                      enum Stream_Visualization_VideoRenderer& renderer_out,
                      bool& traceInformation_out,
                      enum Test_U_ProgramMode& mode_out,
                      bool& useHardwareDecoder_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_process_arguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  debug_out = false;
  inputFile_out.clear ();
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework_out = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  displayDevice_out = Common_UI_Tools::getDefaultDisplay ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D;
#else
  renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_X11;
#endif // ACE_WIN32 || ACE_WIN64
  traceInformation_out = false;
  mode_out = TEST_U_PROGRAMMODE_NORMAL;
  useHardwareDecoder_out = false;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("df:lo:tvz");
//#if defined (CURSES_SUPPORT)
//  options_string += ACE_TEXT_ALWAYS_CHAR ("c");
//#endif // CURSES_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  options_string += ACE_TEXT_ALWAYS_CHAR ("123m");
#else
  options_string += ACE_TEXT_ALWAYS_CHAR ("1");
#endif // ACE_WIN32 || ACE_WIN64
//#if defined (GLUT_SUPPORT)
//  options_string += ACE_TEXT_ALWAYS_CHAR ("g");
//#endif // GLUT_SUPPORT
#if defined (GTK_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("k");
#endif // GTK_SUPPORT
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
      case 'd':
      {
        debug_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case '1':
      {
        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_GDI;
        break;
      }
      case '2':
      {
        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_2D;
        break;
      }
      case '3':
      {
        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D;
        break;
      }
#else
      case '1':
      {
        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_X11;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
//#if defined (CURSES_SUPPORT)
//      case 'c':
//      {
//        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_CURSES;
//        break;
//      }
//#endif // CURSES_SUPPORT
      case 'f':
      {
        inputFile_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      //case 'g':
      //{
      //  renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT;
      //  break;
      //}
#if defined (GTK_SUPPORT)
      case 'k':
      {
        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW;
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
        displayDevice_out =
          Common_UI_Tools::getDisplay (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
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
      case 'z':
      {
        useHardwareDecoder_out = true;
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
do_initialize_directshow (IGraphBuilder*& IGraphBuilder_out,
                          struct _AMMediaType& audioOutputFormat_inout,
                          struct _AMMediaType& videoOutputFormat_inout,
                          HWND& windowHandle_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  // sanity check(s)
  ACE_ASSERT (!IGraphBuilder_out);

  // initialize return value(s)
  ACE_OS::memset (&audioOutputFormat_inout, 0, sizeof (struct _AMMediaType));
  ACE_OS::memset (&videoOutputFormat_inout, 0, sizeof (struct _AMMediaType));
  windowHandle_out = NULL;

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

  // generate default media type: PCM 32bit float @ 48000Hz
  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
  waveformatex_s.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
  waveformatex_s.nChannels = 2;
  waveformatex_s.nSamplesPerSec = 48000;
  waveformatex_s.wBitsPerSample = 32;
  waveformatex_s.nBlockAlign =
    (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
  waveformatex_s.nAvgBytesPerSec =
    (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
  // waveformatex_s.cbSize = 0;
  if (!Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (waveformatex_s,
                                                                 audioOutputFormat_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx(), aborting\n")));
    goto error;
  } // end IF

  // generate default media type: RGB32 640x480 30fps
  videoOutputFormat_inout.majortype = MEDIATYPE_Video;
  videoOutputFormat_inout.subtype = MEDIASUBTYPE_RGB32;
    //STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT;
  videoOutputFormat_inout.bFixedSizeSamples = TRUE;
  videoOutputFormat_inout.bTemporalCompression = FALSE;
  videoOutputFormat_inout.formattype = FORMAT_VideoInfo;
  videoOutputFormat_inout.cbFormat = sizeof (struct tagVIDEOINFOHEADER);
  videoOutputFormat_inout.pbFormat = (BYTE*)CoTaskMemAlloc (videoOutputFormat_inout.cbFormat);
  ACE_ASSERT (videoOutputFormat_inout.pbFormat);
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (videoOutputFormat_inout.pbFormat);
  ACE_OS::memset (video_info_header_p, 0, sizeof (struct tagVIDEOINFOHEADER));
  // *NOTE*: empty --> use entire video
  HRESULT result = SetRectEmpty (&video_info_header_p->rcSource);
  ACE_ASSERT (SUCCEEDED (result));
  result = SetRectEmpty (&video_info_header_p->rcTarget);
  // *NOTE*: empty --> fill entire buffer
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
  video_info_header_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_header_p->bmiHeader.biWidth = 640;
  video_info_header_p->bmiHeader.biHeight = 480;
  video_info_header_p->bmiHeader.biPlanes = 1;
  video_info_header_p->bmiHeader.biBitCount = 32;
  video_info_header_p->bmiHeader.biCompression = BI_RGB;
  video_info_header_p->bmiHeader.biSizeImage =
    DIBSIZE (video_info_header_p->bmiHeader);
  ////video_info_header_p->bmiHeader.biXPelsPerMeter;
  ////video_info_header_p->bmiHeader.biYPelsPerMeter;
  ////video_info_header_p->bmiHeader.biClrUsed;
  ////video_info_header_p->bmiHeader.biClrImportant;
  video_info_header_p->AvgTimePerFrame = 333333; // (1/30) * 10,000,000;
  video_info_header_p->dwBitRate =
    (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
    (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
  videoOutputFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;

  return true;

error:
  Stream_MediaFramework_DirectShow_Tools::free (audioOutputFormat_inout);
  Stream_MediaFramework_DirectShow_Tools::free (videoOutputFormat_inout);
  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
  } // end IF

  return false;
}

void
do_finalize_directshow ()
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

}

bool
do_initialize_mediafoundation (HWND windowHandle_in
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
                               ,IMFMediaSession*& IMFMediaSession_out)
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
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
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

//continue_3:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
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
  // signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */
  // signals_out.sig_del (SIGTSTP);           // 20      /* Stop */

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
do_work (int argc_in,
         ACE_TCHAR* argv_in[],
         bool debug_in,
         const std::string& inputFilePath_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         enum Stream_MediaFramework_Type mediaFramework_in,
#endif // ACE_WIN32 || ACE_WIN64
         struct Common_UI_DisplayDevice& displayDevice_in,
         enum Stream_Visualization_VideoRenderer renderer_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_U_DirectShow_Configuration& directShowConfiguration_in,
         struct Test_U_MediaFoundation_Configuration& mediaFoundationConfiguration_in,
#else
         struct Test_U_MP4Player_Configuration& configuration_in,
#endif // ACE_WIN32 || ACE_WIN64
         bool useHardwareDecoder_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  ACE_Sig_Set handled_signals, ignored_signals, previous_mask;
  Common_SignalActions_t previous_actions_a;
  do_initializeSignals (handled_signals);

  Test_U_SignalHandler signal_handler;
  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // step0a: initialize configuration and stream
  Stream_MediaFramework_DirectSound_Tools::initialize ();

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
                                           false, // use networking ?
                                           false, // use asynch timers ?
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
  allocator_configuration.defaultBufferSize = 32768;
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration codec_configuration;
  codec_configuration.codecId = AV_CODEC_ID_H264;
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration codec_configuration_2; // audio
  codec_configuration_2.codecId = AV_CODEC_ID_AAC;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  codec_configuration.deviceType = AV_HWDEVICE_TYPE_DXVA2;
  //video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_D3D11VA;
  //video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_D3D12VA;
  codec_configuration.format = AV_PIX_FMT_DXVA2_VLD;
#else
  codec_configuration.deviceType = AV_HWDEVICE_TYPE_VAAPI;
  codec_configuration.format = AV_PIX_FMT_VAAPI;
  // video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_VDPAU;
  // video_codec_configuration.format = AV_PIX_FMT_VDPAU;

  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration;
  ALSA_configuration.asynch = false;
#endif // ACE_WIN32 || ACE_WIN64
  codec_configuration.parserFlags = PARSER_FLAG_ONCE | PARSER_FLAG_USE_CODEC_TS;
  //codec_configuration_2.parserFlags = 0;
#else
  struct Stream_AllocatorConfiguration allocator_configuration;
#endif // FFMPEG_SUPPORT

  struct Stream_ModuleConfiguration module_configuration;
  struct Test_U_MP4Player_StreamConfiguration stream_configuration;
  struct Stream_Miscellaneous_DelayConfiguration delay_configuration;
  delay_configuration.averageTokensPerInterval = 1;
  delay_configuration.mode = STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_MP4Player_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  struct Test_U_MP4Player_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2; // converter
  struct Test_U_MP4Player_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2b; // resize
  struct Test_U_MP4Player_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_3; // display
  struct Test_U_MP4Player_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_audio; // decoder

  Test_U_DirectShow_EventHandler_t directshow_ui_event_handler;
  struct Test_U_MP4Player_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Test_U_MP4Player_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_2; // converter
  Test_U_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler;
#else
  struct Test_U_FFMPEG_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_FFMPEG_ModuleHandlerConfiguration modulehandler_configuration_2; // converter
  struct Test_U_FFMPEG_ModuleHandlerConfiguration modulehandler_configuration_2b; // resize
  struct Test_U_FFMPEG_ModuleHandlerConfiguration modulehandler_configuration_audio; // decoder
  Test_U_EventHandler_t ui_event_handler;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
#if defined (FFMPEG_SUPPORT)
      directshow_modulehandler_configuration.codecConfiguration =
        &codec_configuration;
#endif // FFMPEG_SUPPORT
#if defined (_DEBUG)
      directshow_modulehandler_configuration.debug = debug_in;
#endif // _DEBUG
      directshow_modulehandler_configuration.delayConfiguration =
        &delay_configuration;
      directshow_modulehandler_configuration.deviceIdentifier.identifierDiscriminator =
        Stream_Device_Identifier::ID;
      directshow_modulehandler_configuration.deviceIdentifier.identifier._id = 0;
      directshow_modulehandler_configuration.direct3DConfiguration =
        &directShowConfiguration_in.direct3DConfiguration;
      directshow_modulehandler_configuration.fileIdentifier.identifier =
        inputFilePath_in;
      directshow_modulehandler_configuration.streamIndex = -1;
      directshow_modulehandler_configuration.subscriber =
        &directshow_ui_event_handler;

      directshow_modulehandler_configuration_3 =
        directshow_modulehandler_configuration;

      directshow_modulehandler_configuration_audio =
        directshow_modulehandler_configuration;
#if defined (FFMPEG_SUPPORT)
      directshow_modulehandler_configuration_audio.codecConfiguration =
        &codec_configuration_2;
#endif // FFMPEG_SUPPORT

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
#if defined (FFMPEG_SUPPORT)
      mediafoundation_modulehandler_configuration.codecConfiguration =
        &codec_configuration;
#endif // FFMPEG_SUPPORT
#if defined (_DEBUG)
      mediafoundation_modulehandler_configuration.debug = debug_in;
#endif // _DEBUG
      mediafoundation_modulehandler_configuration.delayConfiguration =
        &delay_configuration;
      mediafoundation_modulehandler_configuration.direct3DConfiguration =
        &mediaFoundationConfiguration_in.direct3DConfiguration;
      mediafoundation_modulehandler_configuration.fileIdentifier.identifier =
        inputFilePath_in;
      mediafoundation_modulehandler_configuration.streamIndex = -1;
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
  modulehandler_configuration.ALSAConfiguration = &ALSA_configuration;
  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration.codecConfiguration = &codec_configuration;
#endif // FFMPEG_SUPPORT
#if defined (_DEBUG)
  modulehandler_configuration.debug = debug_in;
#endif // _DEBUG
  modulehandler_configuration.delayConfiguration = &delay_configuration;
  //  modulehandler_configuration.display = displayDevice_in;
  modulehandler_configuration.fileIdentifier.identifier = inputFilePath_in;
  modulehandler_configuration.streamIndex = -1;
  modulehandler_configuration.subscriber = &ui_event_handler;
#endif // ACE_WIN32 || ACE_WIN64

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  stream_configuration.allocatorConfiguration = &allocator_configuration;
  stream_configuration.renderer = renderer_in;
  stream_configuration.useHardwareDecoder = useHardwareDecoder_in;
#if defined (ACE_WIN32) || defined(ACE_WIN64)
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
      stream_configuration.messageAllocator = &directshow_message_allocator;
      stream_configuration.module = &directshow_message_handler;

      directShowConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                 directshow_modulehandler_configuration,
                                                                 stream_configuration);
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING),
                                                                              std::make_pair (&module_configuration,
                                                                                              &directshow_modulehandler_configuration_audio)));
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_FAAD_DEFAULT_NAME_STRING),
                                                                              std::make_pair (&module_configuration,
                                                                                              &directshow_modulehandler_configuration_audio)));
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING),
                                                                              std::make_pair (&module_configuration,
                                                                                              &directshow_modulehandler_configuration_audio)));
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (Stream_Visualization_Tools::rendererToModuleName (renderer_in),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_3)));

      directShowConfiguration_in.signalHandlerConfiguration.stream =
        &directshow_stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //if (bufferSize_in)
      //  mediaFoundationCBData_in.configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
      //      bufferSize_in;
      stream_configuration.messageAllocator = &mediafoundation_message_allocator;
      stream_configuration.module = &mediafoundation_message_handler;

      mediaFoundationConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                      mediafoundation_modulehandler_configuration,
                                                                      stream_configuration);

      mediaFoundationConfiguration_in.signalHandlerConfiguration.stream =
        &mediafoundation_stream;

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

  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &message_handler;
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);

  configuration_in.signalHandlerConfiguration.stream = &stream;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Stream_IStreamControlBase* stream_p = NULL;
//#if defined (CURSES_SUPPORT)
//  Test_U_Curses_Manager_t* curses_manager_p = NULL;
//  struct Common_UI_Curses_Configuration* curses_configuration_p = NULL;
//#endif // CURSES_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_handle = NULL;
  IMFMediaSession* media_session_p = NULL;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (!do_initialize_directshow (directshow_modulehandler_configuration.builder,
                                     directshow_modulehandler_configuration_audio.outputFormat,
                                     directshow_modulehandler_configuration.outputFormat,
                                     directshow_modulehandler_configuration_3.window.win32_hwnd))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::do_initialize_directshow(), returning\n")));
        return;
      } // end IF
      ACE_ASSERT (directshow_modulehandler_configuration_3.window.win32_hwnd);
      directshow_modulehandler_configuration_3.window.type =
        Common_UI_Window::TYPE_WIN32;
      struct _AMMediaType* media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_modulehandler_configuration.outputFormat);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration_2.outputFormat = *media_type_p;
      delete media_type_p; media_type_p = NULL;

      media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_modulehandler_configuration.outputFormat);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration_2b.outputFormat = *media_type_p;
      delete media_type_p; media_type_p = NULL;

      media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_modulehandler_configuration.outputFormat);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration_3.outputFormat = *media_type_p;
      delete media_type_p; media_type_p = NULL;
      directShowConfiguration_in.direct3DConfiguration.presentationParameters.hDeviceWindow =
        directshow_modulehandler_configuration_3.window;
      stream_p = &directshow_stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (!do_initialize_mediafoundation (window_handle
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
                                          ,mediafoundation_modulehandler_configuration.session))
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
  stream_p = &stream;

  modulehandler_configuration.deviceIdentifier.identifier =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_PLAYBACK);
  modulehandler_configuration.outputFormat.video.format = AV_PIX_FMT_RGB24;
  modulehandler_configuration.outputFormat.video.resolution.width = 640;
  modulehandler_configuration.outputFormat.video.resolution.height = 480;
  modulehandler_configuration.outputFormat.video.frameRate.num = 30;
  switch (renderer_in)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
    {
      // *IMPORTANT NOTE*: there does not seem to be a way to feed RGB24 data to
      //                   Xlib; XCreateImage() only 'likes' 32-bit data, regardless
      //                   of what 'depth' values are set (in fact, it requires BGRA
      //                   on little-endian platforms) --> convert
      // modulehandler_configuration_2 = modulehandler_configuration;
      modulehandler_configuration.outputFormat.video.format = AV_PIX_FMT_BGRA;
      break;
    }
//#if defined (GLUT_SUPPORT)
//    case STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT:
//    {
//      modulehandler_configuration_2 = modulehandler_configuration;
//      modulehandler_configuration_2.outputFormat.format.pixelformat =
//        V4L2_PIX_FMT_RGBA32;
//      break;
//    }
//#endif // GLUT_SUPPORT
    default:
      break;
  } // end SWITCH

  modulehandler_configuration_audio = modulehandler_configuration;
  modulehandler_configuration_audio.codecConfiguration = &codec_configuration_2;
  modulehandler_configuration_audio.outputFormat.audio.channels = 2;
  modulehandler_configuration_audio.outputFormat.audio.format =
    AV_SAMPLE_FMT_FLT;
  modulehandler_configuration_audio.outputFormat.audio.sampleRate = 48000;
  modulehandler_configuration_audio.waitForDataOnEnd = true;

  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_audio)));
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_FAAD_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_audio)));
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_SOX_RESAMPLER_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_audio)));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  switch (renderer_in)
  {
//#if defined (CURSES_SUPPORT)
//    case STREAM_VISUALIZATION_VIDEORENDERER_CURSES:
//    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      DestroyWindow (directshow_modulehandler_configuration_3.window);
//      directshow_modulehandler_configuration_3.window = NULL;
//
//      switch (mediaFramework_in)
//      {
//        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//        {
//          curses_configuration_p = &directShowConfiguration_in.cursesConfiguration;
//          directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
//                                                                                 std::make_pair (&module_configuration,
//                                                                                                 &directshow_modulehandler_configuration_2)));
//          directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING),
//                                                                                 std::make_pair (&module_configuration,
//                                                                                                 &directshow_modulehandler_configuration_2b)));
//          break;
//        }
//        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//        {
//          curses_configuration_p = &mediaFoundationConfiguration_in.cursesConfiguration;
//          mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
//                                                                                      std::make_pair (&module_configuration,
//                                                                                                      &mediafoundation_modulehandler_configuration_2)));
//          break;
//        }
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                      mediaFramework_in));
//          return;
//        }
//      } // end SWITCH
//#else
//      curses_configuration_p = &configuration_in.cursesConfiguration;
//#endif // ACE_WIN32 || ACE_WIN64
//      ACE_ASSERT (curses_configuration_p);
//
//      curses_configuration_p->hooks.initHook = curses_init;
//      curses_configuration_p->hooks.finiHook = curses_fini;
//      curses_configuration_p->hooks.inputHook = curses_input;
//      curses_configuration_p->hooks.mainHook = curses_main;
//
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      struct _COORD coord_s;
//      ACE_OS::memset (&coord_s, 0, sizeof (struct _COORD));
//      coord_s.X = TEST_U_CURSES_CONSOLE_FONT_SIZE;
//      coord_s.Y = TEST_U_CURSES_CONSOLE_FONT_SIZE;
//      Common_UI_Tools::setConsoleFontSize (coord_s);
//      coord_s.X = TEST_U_CURSES_CONSOLE_WIDTH;
//      coord_s.Y = TEST_U_CURSES_CONSOLE_HEIGHT;
//      Common_UI_Tools::setConsoleSize (coord_s);
//
//      curses_configuration_p->height = TEST_U_CURSES_CONSOLE_HEIGHT; // lines
//      curses_configuration_p->width = TEST_U_CURSES_CONSOLE_WIDTH;   // columns
//#else
//      Common_UI_Tools::setConsoleFontSize (TEST_U_CURSES_CONSOLE_FONT_FACTOR);
//      Common_UI_Tools::setConsoleSize (TEST_U_CURSES_CONSOLE_WIDTH,
//                                       TEST_U_CURSES_CONSOLE_HEIGHT);
//
//      curses_configuration_p->height = TEST_U_CURSES_CONSOLE_HEIGHT; // lines
//      curses_configuration_p->width = TEST_U_CURSES_CONSOLE_WIDTH;   // columns
////      curses_configuration_p->height = 120; // lines
////      curses_configuration_p->width = 45; // columns
//#endif // ACE_WIN32 || ACE_WIN64
//
//      curses_manager_p = TEST_U_CURSES_MANAGER_SINGLETON::instance ();
//      ACE_ASSERT (curses_manager_p);
//      struct Test_U_CursesState& state_r =
//        const_cast<struct Test_U_CursesState&> (curses_manager_p->getR ());
//      state_r.stream = stream_p;
//
//      if (!curses_manager_p->initialize (*curses_configuration_p))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Common_UI_Curses_Manager_T::initialize(), returning\n")));
//        goto clean;
//      } // end IF
//      if (!curses_manager_p->start (NULL))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Common_UI_Curses_Manager_T::start(), returning\n")));
//        goto clean;
//      } // end IF
//      ACE_OS::sleep (1);
//
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      Common_Image_Resolution_t resolution_s;
//      switch (mediaFramework_in)
//      {
//        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//        {
//          directshow_modulehandler_configuration_3.window_2 = state_r.std_window;
//          ACE_ASSERT (directshow_modulehandler_configuration_3.window_2);
//          resolution_s.cx = getmaxx (state_r.std_window);
//          resolution_s.cy = getmaxy (state_r.std_window);
//          Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_s,
//                                                                 directshow_modulehandler_configuration.outputFormat);
//
//          Stream_MediaFramework_DirectShow_Tools::setFormat (MEDIASUBTYPE_RGB24,
//                                                             directshow_modulehandler_configuration_2.outputFormat);
//          break;
//        }
//        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//        {
//          mediafoundation_modulehandler_configuration.window_2 = state_r.std_window;
//          ACE_ASSERT (mediafoundation_modulehandler_configuration.window_2);
//          Stream_MediaFramework_MediaFoundation_Tools::setResolution (resolution_s,
//                                                                      mediafoundation_modulehandler_configuration.outputFormat);
//
//          Stream_MediaFramework_MediaFoundation_Tools::setFormat (MEDIASUBTYPE_RGB24,
//                                                                  mediafoundation_modulehandler_configuration_2.outputFormat);
//          break;
//        }
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                      mediaFramework_in));
//          return;
//        }
//      } // end SWITCH
//#else
//      modulehandler_configuration.window_2 = state_r.std_window;
//      ACE_ASSERT (modulehandler_configuration.window_2);
//      modulehandler_configuration.outputFormat.format.width =
//        getmaxx (state_r.std_window);
//      modulehandler_configuration.outputFormat.format.height =
//        getmaxy (state_r.std_window);
//      
//      modulehandler_configuration_2.outputFormat.format.pixelformat =
//        V4L2_PIX_FMT_RGB24;
//#endif // ACE_WIN32 || ACE_WIN64
//
//      break;
//    }
//#endif // CURSES_SUPPORT
//#if defined (GLUT_SUPPORT)
//    case STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT:
//    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      DestroyWindow (directshow_modulehandler_configuration_3.window);
//      directshow_modulehandler_configuration_3.window = NULL;
//
//      switch (mediaFramework_in)
//      {
//        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//        { // *TODO*: there is no way to specify BGRA with _AMMediaType !
//          //Stream_MediaFramework_DirectShow_Tools::setFormat (MEDIASUBTYPE_ARGB32,
//          //                                                   directshow_modulehandler_configuration_2.outputFormat);
//          break;
//        }
//        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//        {
//          break;
//        }
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
//                      mediaFramework_in));
//          return;
//        }
//      } // end SWITCH
//#endif // ACE_WIN32 || ACE_WIN64
//
//      break;
//    }
//#endif // GLUT_SUPPORT
#if defined (GTK_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW:
    {
      Common_UI_GTK_Tools::initialize (argc_in, argv_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          Stream_MediaFramework_DirectShow_Tools::setFormat (MEDIASUBTYPE_RGB24,
                                                             directshow_modulehandler_configuration_2.outputFormat);
          directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                                                 std::make_pair (&module_configuration,
                                                                                                 &directshow_modulehandler_configuration_2)));
          directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING),
                                                                                 std::make_pair (&module_configuration,
                                                                                                 &directshow_modulehandler_configuration_2b)));
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                                                      std::make_pair (&module_configuration,
                                                                                                      &mediafoundation_modulehandler_configuration_2)));
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
      modulehandler_configuration_2.outputFormat.video.format =
        AV_PIX_FMT_RGB24;
      configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                                   std::make_pair (&module_configuration,
                                                                                   &modulehandler_configuration_2)));

      configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING),
                                                                   std::make_pair (&module_configuration,
                                                                                   &modulehandler_configuration_2b)));
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
#endif // GTK_SUPPORT
    default:
      break;
  } // end SWITCH

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
      do_finalize_directshow ();
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#endif // ACE_WIN32 || ACE_WIN64
  bool debug_b = false;
  std::string input_file_path;
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  enum Stream_Visualization_VideoRenderer video_renderer_e;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  video_renderer_e = STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D;
#else
  video_renderer_e = STREAM_VISUALIZATION_VIDEORENDERER_X11;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_UI_DisplayDevice display_device_s =
    Common_UI_Tools::getDefaultDisplay ();
  bool trace_information = false;
  enum Test_U_ProgramMode program_mode_e = TEST_U_PROGRAMMODE_NORMAL;
  bool use_hardware_decoder_b = false;

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
                             debug_b,
                             input_file_path,
                             log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                             display_device_s,
                             video_renderer_e,
                             trace_information,
                             program_mode_e,
                             use_hardware_decoder_b))
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
  if (!Common_File_Tools::isReadable (input_file_path))
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
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]),                   // program name
                                     log_file_name,                                // log file name
                                     false,                                        // log to syslog ?
                                     false,                                        // trace messages ?
                                     trace_information,                            // debug messages ?
                                     NULL))                                        // (ui) logger ?
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

  // step1f: handle specific program modes
  switch (program_mode_e)
  {
    case TEST_U_PROGRAMMODE_PRINT_VERSION:
    {
      do_print_version (ACE::basename (argv_in[0]));

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
  struct Test_U_MP4Player_Configuration configuration;
#endif // ACE_WIN32 || ACE_WIN64

//#if defined (GTK_USE)
//  if (video_renderer_e == STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW)
//    Common_UI_GTK_Tools::initialize (argc_in, argv_in);
//#endif // GTK_USE

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (argc_in, argv_in,
           debug_b,
           input_file_path,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
           display_device_s,
           video_renderer_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_configuration,
           mediafoundation_configuration,
#else
           configuration,
#endif // ACE_WIN32 || ACE_WIN64
           use_hardware_decoder_b);
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
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (ACE_Profile_Timer::Rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);

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
} // end main
