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
#else
#include "linux/videodev2.h"
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

#include "common_tools.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

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

#include "test_u_defines.h"

#include "test_u_camerascreen_defines.h"
#include "test_u_camerascreen_eventhandler.h"
#include "test_u_camerascreen_signalhandler.h"
#include "test_u_camerascreen_stream.h"

#if defined (CURSES_SUPPORT)
#include "test_u_camerascreen_curses.h"
#endif // CURSES_SUPPORT

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("CameraScreenStream");

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
#if defined (CURSES_SUPPORT)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : use curses [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#endif // CURSES_SUPPORT
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
  struct Stream_Device_Identifier device_identifier;
  std::string device_identifier_string;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      device_identifier =
        Stream_Device_DirectShow_Tools::getDefaultCaptureDevice (CLSID_VideoInputDeviceCategory);
      ACE_ASSERT (device_identifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
      device_identifier_string =
        Stream_Device_DirectShow_Tools::devicePathToString (device_identifier.identifier._string);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // *TODO*
      ACE_ASSERT (false);
      ACE_NOTSUP;
      ACE_NOTREACHED (return;)
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  STREAM_LIB_DEFAULT_MEDIAFRAMEWORK));
      return;
    }
  } // end SWITCH
#else
  device_identifier_string =
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
  device_identifier_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  device_identifier_string +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : device [\"")
            << device_identifier_string
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g          : OpenGL mode [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x          : test device for method support and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
                      struct Stream_Device_Identifier& deviceIdentifier_out,
                      bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      enum Stream_MediaFramework_Type& mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                      struct Common_UI_DisplayDevice& displayDevice_out,
                      enum Stream_Visualization_VideoRenderer& renderer_out,
                      bool& traceInformation_out,
                      enum Stream_CameraScreen_ProgramMode& mode_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_process_arguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_out)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      deviceIdentifier_out =
        Stream_Device_DirectShow_Tools::getDefaultCaptureDevice (CLSID_VideoInputDeviceCategory);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // *TODO*
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_out));
      return false;
    }
  } // end SWITCH
#else
  deviceIdentifier_out.identifier =
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
  deviceIdentifier_out.identifier += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  deviceIdentifier_out.identifier +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
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
  mode_out = STREAM_CAMERASCREEN_PROGRAMMODE_NORMAL;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("d:glo:tvx");
#if defined (CURSES_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("c");
#endif // CURSES_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  options_string += ACE_TEXT_ALWAYS_CHAR ("123m");
#else
  options_string += ACE_TEXT_ALWAYS_CHAR ("1");
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
#if defined (CURSES_SUPPORT)
      case 'c':
      {
        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_CURSES;
        break;
      }
#endif // CURSES_SUPPORT
      case 'd':
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        deviceIdentifier_out.identifierDiscriminator =
          Stream_Device_Identifier::STRING;
        ACE_OS::strcpy (deviceIdentifier_out.identifier._string,
                        ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
#else
        deviceIdentifier_out.identifier =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
#endif // ACE_WIN32 || ACE_WIN64
        break;
      }
      case 'g':
      {
        renderer_out = STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT;
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
        mode_out = STREAM_CAMERASCREEN_PROGRAMMODE_PRINT_VERSION;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      case 'x':
      {
        mode_out = STREAM_CAMERASCREEN_PROGRAMMODE_TEST_METHODS;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
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
do_initialize_directshow (const struct Stream_Device_Identifier& deviceIdentifier_in,
                          bool hasUI_in,
                          IGraphBuilder*& IGraphBuilder_out,
                          IAMStreamConfig*& IAMStreamConfig_out,
                          struct _AMMediaType& captureFormat_inout,
                          struct _AMMediaType& outputFormat_inout,
                          HWND& windowHandle_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  BOOL result_2 = false;
  IMediaFilter* media_filter_p = NULL;
  struct _AMMediaType* media_type_p = NULL;

  // sanity check(s)
  ACE_ASSERT (!IGraphBuilder_out);
  ACE_ASSERT (!IAMStreamConfig_out);

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (deviceIdentifier_in,
                                                        CLSID_VideoInputDeviceCategory,
                                                        IGraphBuilder_out,
                                                        buffer_negotiation_p,
                                                        IAMStreamConfig_out,
                                                        graph_layout))
  {
    ACE_ASSERT (deviceIdentifier_in.identifierDiscriminator == Stream_Device_Identifier::STRING);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (deviceIdentifier_in.identifier._string)));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (IAMStreamConfig_out);
  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  if (!Stream_Device_DirectShow_Tools::getCaptureFormat (IGraphBuilder_out,
                                                         CLSID_VideoInputDeviceCategory,
                                                         captureFormat_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::getCaptureFormat(CLSID_VideoInputDeviceCategory), aborting\n")));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": default capture format: %s\n"),
              ACE_TEXT (Stream_Device_DirectShow_Tools::devicePathToString (deviceIdentifier_in.identifier._string).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (captureFormat_inout, true).c_str ())));
  media_type_p =
    Stream_MediaFramework_DirectShow_Tools::copy (captureFormat_inout);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  outputFormat_inout = *media_type_p;
  delete media_type_p; media_type_p = NULL;

  // *NOTE*: the default (sample grabber-) output format is RGB32
  ACE_ASSERT (InlineIsEqualGUID (outputFormat_inout.majortype, MEDIATYPE_Video));
  outputFormat_inout.subtype =
    STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT;
  outputFormat_inout.bFixedSizeSamples = TRUE;
  outputFormat_inout.bTemporalCompression = FALSE;
  if (InlineIsEqualGUID (outputFormat_inout.formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (outputFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    struct tagVIDEOINFOHEADER* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (outputFormat_inout.pbFormat);
    // *NOTE*: empty --> use entire video
    result_2 = SetRectEmpty (&video_info_header_p->rcSource);
    ACE_ASSERT (SUCCEEDED (result_2));
    result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
    // *NOTE*: empty --> fill entire buffer
    ACE_ASSERT (SUCCEEDED (result_2));
    ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
    ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
    video_info_header_p->bmiHeader.biBitCount = 32;
    video_info_header_p->bmiHeader.biCompression = BI_RGB;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    ////video_info_header_p->bmiHeader.biXPelsPerMeter;
    ////video_info_header_p->bmiHeader.biYPelsPerMeter;
    ////video_info_header_p->bmiHeader.biClrUsed;
    ////video_info_header_p->bmiHeader.biClrImportant;
    ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
      (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

    outputFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else if (InlineIsEqualGUID (outputFormat_inout.formattype, FORMAT_VideoInfo2))
  {
    ACE_ASSERT (outputFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
    struct tagVIDEOINFOHEADER2* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (outputFormat_inout.pbFormat);
    ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
    video_info_header_p->bmiHeader.biBitCount = 32;
    video_info_header_p->bmiHeader.biCompression = BI_RGB;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    ////video_info_header_p->bmiHeader.biXPelsPerMeter;
    ////video_info_header_p->bmiHeader.biYPelsPerMeter;
    ////video_info_header_p->bmiHeader.biClrUsed;
    ////video_info_header_p->bmiHeader.biClrImportant;
    ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
      (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

    outputFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (outputFormat_inout.formattype).c_str ())));
    goto error;
  } // end ELSE

  IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
  IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;

//  DWORD window_style = (WS_CAPTION     |
//                        WS_MAXIMIZEBOX |
//                        WS_MINIMIZEBOX |
//                        //WS_OVERLAPPED     |
//                        WS_SIZEBOX     |
//                        WS_SYSMENU     |
//                        WS_VISIBLE);
//  DWORD window_style_ex = (WS_EX_APPWINDOW     |
//                           WS_EX_RIGHTSCROLLBAR// |
//                           /*WS_EX_WINDOWEDGE*/);
//  windowHandle_out =
//    CreateWindowEx (window_style_ex,                                  // dwExStyle
//#if defined (UNICODE)
//                    ACE_TEXT_ALWAYS_WCHAR ("EDIT"),                   // lpClassName
//#else
//                    ACE_TEXT_ALWAYS_CHAR ("EDIT"),                    // lpClassName
//#endif // UNICODE
//                    NULL,                                             // lpWindowName
//                    window_style,                                     // dwStyle
//                    CW_USEDEFAULT, CW_USEDEFAULT,                     // x,y
//                    STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH,        // width
//                    STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT,       // height
//                    //parent_window_handle,                           // hWndParent
//                    NULL,                                             // hWndParent
//                    NULL,                                             // hMenu
//                    GetModuleHandle (NULL),                           // hInstance
//                    NULL);                                            // lpParam
//  if (unlikely (!windowHandle_out))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to CreateWindow(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
//    goto error;
//  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("opened display window (size: %dx%d, handle: 0x%@)\n"),
//              STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH, STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT,
//              windowHandle_out));

  return true;

error:
  if (media_filter_p)
    media_filter_p->Release ();
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  Stream_MediaFramework_DirectShow_Tools::free (outputFormat_inout);
  Stream_MediaFramework_DirectShow_Tools::free (captureFormat_inout);
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
  } // end IF
  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
  } // end IF

  return false;
}

void
do_finalize_directshow (IAMStreamConfig*& streamConfiguration_inout)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  if (streamConfiguration_inout)
  {
    streamConfiguration_inout->Release (); streamConfiguration_inout = NULL;
  } // end IF
}

bool
do_initialize_mediafoundation (const struct Stream_Device_Identifier& deviceIdentifier_in,
                               HWND windowHandle_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                               IMFMediaSession*& IMFMediaSession_out,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                               bool loadDevice_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  IMFTopology* topology_p = NULL;

  // sanity check(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (!IMFMediaSession_out);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

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

  if (!loadDevice_in)
    goto continue_2;

  //if (!Stream_Device_Tools::loadDeviceGraph (deviceName_in,
  //                                                  IGraphBuilder_out,
  //                                                  IAMBufferNegotiation_out,
  //                                                  IAMStreamConfig_out))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Device_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
  //              ACE_TEXT (deviceName_in.c_str ())));
  //  return false;
  //} // end IF
  //ACE_ASSERT (IGraphBuilder_out);
  //ACE_ASSERT (IAMBufferNegotiation_out);
  //ACE_ASSERT (IAMStreamConfig_out);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  ACE_ASSERT (deviceIdentifier_in.identifierDiscriminator == Stream_Device_Identifier::GUID);
  if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (deviceIdentifier_in.identifier._guid,
                                                                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                    media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(\"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in.identifier._guid).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_source_p);

  if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (deviceIdentifier_in,
                                                                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                media_source_p,
                                                                NULL,
                                                                topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  media_source_p->Release (); media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

continue_2:
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (IMFMediaSession_out)
  {
    IMFMediaSession_out->Release (); IMFMediaSession_out = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

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
do_initialize_v4l (const std::string& deviceIdentifier_in,
                   struct Stream_Device_Identifier& deviceIdentifier_out,
                   struct Stream_MediaFramework_V4L_MediaType& captureFormat_out,
                   struct Stream_MediaFramework_V4L_MediaType& outputFormat_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_v4l"));

  // intialize return value(s)
  ACE_OS::memset (&captureFormat_out, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
//  ACE_OS::memset (&outputFormat_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  // sanity check(s)
  ACE_ASSERT (!deviceIdentifier_in.empty ());

  // *NOTE*: use O_NONBLOCK with a reactor (v4l2_select()) or proactor
  //         (v4l2_poll()) for asynchronous operation
  // *TODO*: support O_NONBLOCK
  int open_mode =
      ((STREAM_LIB_V4L_DEFAULT_IO_METHOD == V4L2_MEMORY_MMAP) ? O_RDWR
                                                              : O_RDONLY);
  int result = -1;
  deviceIdentifier_out.fileDescriptor =
      v4l2_open (deviceIdentifier_in.c_str (),
                 open_mode);
  if (unlikely (deviceIdentifier_out.fileDescriptor == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", aborting\n"),
                ACE_TEXT (deviceIdentifier_in.c_str ()),
                open_mode));
    return false;
  } // end IF

  Stream_Device_Tools::getDefaultCaptureFormat (deviceIdentifier_in,
                                                captureFormat_out);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\" (%d): default capture format: \"%s\" (%d), resolution: %ux%u, framerate: %u/%u\n"),
              ACE_TEXT (deviceIdentifier_in.c_str ()), deviceIdentifier_out.fileDescriptor,
              ACE_TEXT (Stream_Device_Tools::formatToString (deviceIdentifier_out.fileDescriptor, captureFormat_out.format.pixelformat).c_str ()), captureFormat_out.format.pixelformat,
              captureFormat_out.format.width, captureFormat_out.format.height,
              captureFormat_out.frameRate.numerator, captureFormat_out.frameRate.denominator));
//  if (!Stream_MediaFramework_Tools::isRGB (captureFormat_out.format.pixelformat))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("\"%s\" (%d): setting RGB24 capture format\n"),
//                ACE_TEXT (deviceIdentifier_in.c_str ()), deviceIdentifier_out.fileDescriptor));
//    Common_Image_Resolution_t resolution_s;
//    resolution_s.height = captureFormat_out.format.height;
//    resolution_s.width = captureFormat_out.format.width;
//    struct v4l2_pix_format format_s =
//        Stream_Device_Tools::getVideoCaptureFormat (deviceIdentifier_out.fileDescriptor,
//                                                    V4L2_PIX_FMT_RGB24,
//                                                    resolution_s,
//                                                    captureFormat_out.frameRate);
//    ACE_ASSERT (format_s.pixelformat == V4L2_PIX_FMT_RGB24);
//    if (!Stream_Device_Tools::setFormat (deviceIdentifier_out.fileDescriptor,
//                                         format_s))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_Device_Tools::setFormat(), aborting\n")));
//      return false;
//    } // end IF
//    captureFormat_out.format = format_s;
//  } // end IF

  // *NOTE*: Gtk 2 expects RGB24
  // *NOTE*: "...CAIRO_FORMAT_ARGB32: each pixel is a 32-bit quantity, with
  //         alpha in the upper 8 bits, then red, then green, then blue. The
  //         32-bit quantities are stored native-endian. ..."
  // *NOTE*: X11 expects RGB32
  // *TODO*: auto-determine color depth of selected (default) screen (i.e.
  //         'Display' ":0")
  outputFormat_out = captureFormat_out;
  if (!Stream_MediaFramework_Tools::isRGB (captureFormat_out.format.pixelformat))
    outputFormat_out.format.pixelformat = V4L2_PIX_FMT_RGB32;

  return true;

//error:
  if (deviceIdentifier_out.fileDescriptor != -1)
  {
    result = v4l2_close (deviceIdentifier_out.fileDescriptor);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  deviceIdentifier_out.fileDescriptor));
    deviceIdentifier_out.fileDescriptor = -1;
  } // end IF

  return false;
}

void
do_finalize_v4l (struct Stream_Device_Identifier& deviceIdentifier_inout)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_v4l"));

  int result = -1;

  if (deviceIdentifier_inout.fileDescriptor != -1)
  {
    result = v4l2_close (deviceIdentifier_inout.fileDescriptor);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  deviceIdentifier_inout.fileDescriptor));
    deviceIdentifier_inout.fileDescriptor = -1;
  } // end IF
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
do_work (struct Stream_Device_Identifier& deviceIdentifier_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         enum Stream_MediaFramework_Type mediaFramework_in,
#endif // ACE_WIN32 || ACE_WIN64
         struct Common_UI_DisplayDevice& displayDevice_in,
         enum Stream_Visualization_VideoRenderer renderer_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Stream_CameraScreen_DirectShow_Configuration& directShowConfiguration_in,
         struct Stream_CameraScreen_MediaFoundation_Configuration& mediaFoundationConfiguration_in
#else
         struct Stream_CameraScreen_Configuration& configuration_in
#endif // ACE_WIN32 || ACE_WIN64
         )
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  ACE_Sig_Set handled_signals, ignored_signals, previous_mask;
  Common_SignalActions_t previous_actions_a;
  do_initializeSignals (handled_signals);

  Test_U_SignalHandler signal_handler;
  if (!signal_handler.initialize (configuration_in.signalHandlerConfiguration))
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
  struct Stream_AllocatorConfiguration allocator_configuration;
  struct Stream_ModuleConfiguration module_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2; // converter
  struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_3; // display
  struct Stream_CameraScreen_DirectShow_StreamConfiguration directshow_stream_configuration;
  Stream_CameraScreen_DirectShow_EventHandler_t directshow_ui_event_handler;
  struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Stream_CameraScreen_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_2; // converter
  struct Stream_CameraScreen_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration;
  Stream_CameraScreen_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler;
#else
  struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration modulehandler_configuration;
  struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration modulehandler_configuration_2; // converter
  Stream_CameraScreen_EventHandler_t ui_event_handler;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //Stream_CameraScreen_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  //Stream_CameraScreen_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator_2;
  //Stream_CameraScreen_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  //Stream_CameraScreen_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator_2;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
      directshow_modulehandler_configuration.deviceIdentifier =
        deviceIdentifier_in;
      directshow_modulehandler_configuration.direct3DConfiguration =
        &directShowConfiguration_in.direct3DConfiguration;
      //directshow_modulehandler_configuration.lock = &state_r.subscribersLock;

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
      mediafoundation_modulehandler_configuration.deviceIdentifier =
        deviceIdentifier_in;
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
//  Stream_CameraScreen_StreamConfiguration_t::ITERATOR_T v4l_stream_iterator;
//  Stream_CameraScreen_StreamConfiguration_t::ITERATOR_T v4l_stream_iterator_2;
  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
  modulehandler_configuration.buffers = STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS;
  modulehandler_configuration.deviceIdentifier = deviceIdentifier_in;
//  modulehandler_configuration.display = displayDevice_in;
//  // *TODO*: turn these into an option
//  modulehandler_configuration.method = STREAM_DEV_CAM_V4L_DEFAULT_IO_METHOD;
  Stream_Device_Tools::getDefaultCaptureFormat (deviceIdentifier_in.identifier,
                                                modulehandler_configuration.outputFormat);
  modulehandler_configuration.subscriber = &ui_event_handler;

  struct Stream_CameraScreen_V4L_StreamConfiguration stream_configuration;
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
  Stream_CameraScreen_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                                  &heap_allocator,     // heap allocator handle
                                                                                  true);               // block ?
  Stream_CameraScreen_DirectShow_Stream directshow_stream;
  Stream_CameraScreen_DirectShow_MessageHandler_Module directshow_message_handler (&directshow_stream,
                                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  Stream_CameraScreen_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                                            &heap_allocator,     // heap allocator handle
                                                                                            true);               // block ?
  Stream_CameraScreen_MediaFoundation_Stream mediafoundation_stream;
  Stream_CameraScreen_MediaFoundation_MessageHandler_Module mediafoundation_message_handler (&mediafoundation_stream,
                                                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //if (bufferSize_in)
      //  directShowCBData_in.configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
      //      bufferSize_in;
      directshow_stream_configuration.allocatorConfiguration = &allocator_configuration;
      directshow_stream_configuration.messageAllocator =
          &directshow_message_allocator;
      directshow_stream_configuration.module =
        &directshow_message_handler;
      directshow_stream_configuration.renderer = renderer_in;

      directShowConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                 directshow_modulehandler_configuration,
                                                                 directshow_stream_configuration);
      directshow_modulehandler_configuration_3 = directshow_modulehandler_configuration;
      directshow_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
        Stream_Device_Identifier::STRING;
      ACE_OS::strcpy (directshow_modulehandler_configuration_3.deviceIdentifier.identifier._string,
                      displayDevice_in.device.c_str ());
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (Stream_Visualization_Tools::rendererToModuleName (renderer_in),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_3)));
      //directshow_stream_iterator =
      //  directShowConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      //ACE_ASSERT (directshow_stream_iterator != directShowConfiguration_in.streamConfiguration.end ());
      //directshow_stream_iterator_2 =
      //  directShowConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING));
      //ACE_ASSERT (directshow_stream_iterator_2 != directShowConfiguration_in.streamConfiguration.end ());
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
  Stream_CameraScreen_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                            &heap_allocator,     // heap allocator handle
                                                            true);               // block ?
  Stream_CameraScreen_Stream stream;
  Stream_CameraScreen_MessageHandler_Module message_handler (&stream,
                                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  configuration_in.signalHandlerConfiguration.stream = &stream;

  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &message_handler;
  stream_configuration.renderer = renderer_in;
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
#if defined (CURSES_SUPPORT)
  Test_U_Curses_Manager_t* curses_manager_p = NULL;
  struct Common_UI_Curses_Configuration* curses_configuration_p = NULL;
#endif // CURSES_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_handle = NULL;
  //IAMBufferNegotiation* buffer_negotiation_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;
  IMFMediaSession* media_session_p = NULL;
  bool load_device = true;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      struct _AMMediaType* media_type_p = NULL;
      if (!do_initialize_directshow (deviceIdentifier_in,
                                     false,                             // has UI ?
                                     directshow_modulehandler_configuration.builder,
                                     stream_config_p,
                                     directshow_stream_configuration.format,
                                     directshow_modulehandler_configuration.outputFormat,
                                     directshow_modulehandler_configuration_3.window))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::do_initialize_directshow(), returning\n")));
        return;
      } // end IF
      //ACE_ASSERT (stream_config_p);
      //ACE_ASSERT (directshow_modulehandler_configuration_3.window);
      //directShowCBData_in.streamConfiguration = stream_config_p;
      media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_modulehandler_configuration.outputFormat);
      ACE_ASSERT (media_type_p);
      directshow_modulehandler_configuration_2.outputFormat = *media_type_p;
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
      if (!do_initialize_mediafoundation (deviceIdentifier_in,
                                          window_handle,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                          mediafoundation_modulehandler_configuration.session,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                          load_device)) // load device ?
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::do_initialize_mediafoundation(), returning\n")));
        return;
      } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT (mediafoundation_modulehandler_configuration.session);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
  if (!do_initialize_v4l (deviceIdentifier_in.identifier,
                          modulehandler_configuration.deviceIdentifier,
                          stream_configuration.format,
                          modulehandler_configuration.outputFormat))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::do_initialize_v4l(), returning\n")));
    return;
  } // end IF
  stream_p = &stream;

  switch (renderer_in)
  {
    case STREAM_VISUALIZATION_VIDEORENDERER_X11:
    {
      // *IMPORTANT NOTE*: there does not seem to be a way to feed RGB24 data to
      //                   Xlib; XCreateImage() only 'likes' 32-bit data, regardless
      //                   of what 'depth' values are set (in fact, it requires BGRA
      //                   on little-endian platforms) --> convert
      modulehandler_configuration_2 = modulehandler_configuration;
      modulehandler_configuration_2.outputFormat.format.pixelformat =
        V4L2_PIX_FMT_BGRA32;
      break;
    }
#if defined (GLUT_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT:
    {
      modulehandler_configuration_2 = modulehandler_configuration;
      modulehandler_configuration_2.outputFormat.format.pixelformat =
        V4L2_PIX_FMT_RGBA32;
      break;
    }
#endif // GLUT_SUPPORT
    default:
      break;
  } // end SWITCH
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_2)));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  switch (renderer_in)
  {
#if defined (CURSES_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_CURSES:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      DestroyWindow (directshow_modulehandler_configuration_3.window);
      directshow_modulehandler_configuration_3.window = NULL;

      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          curses_configuration_p = &directShowConfiguration_in.cursesConfiguration;
          directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                                                 std::make_pair (&module_configuration,
                                                                                                 &directshow_modulehandler_configuration_2)));
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          curses_configuration_p = &mediaFoundationConfiguration_in.cursesConfiguration;
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
      curses_configuration_p = &configuration_in.cursesConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (curses_configuration_p);

      curses_configuration_p->hooks.initHook = curses_init;
      curses_configuration_p->hooks.finiHook = curses_fini;
      curses_configuration_p->hooks.inputHook = curses_input;
      curses_configuration_p->hooks.mainHook = curses_main;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _COORD coord_s;
      ACE_OS::memset (&coord_s, 0, sizeof (struct _COORD));
      coord_s.X = TEST_U_CURSES_CONSOLE_FONT_SIZE;
      coord_s.Y = TEST_U_CURSES_CONSOLE_FONT_SIZE;
      Common_UI_Tools::setConsoleFontSize (coord_s);
      coord_s.X = TEST_U_CURSES_CONSOLE_WIDTH;
      coord_s.Y = TEST_U_CURSES_CONSOLE_HEIGHT;
      Common_UI_Tools::setConsoleSize (coord_s);

      curses_configuration_p->height = TEST_U_CURSES_CONSOLE_HEIGHT; // lines
      curses_configuration_p->width = TEST_U_CURSES_CONSOLE_WIDTH;   // columns
#else
      Common_UI_Tools::setConsoleFontSize (TEST_U_CURSES_CONSOLE_FONT_FACTOR);
      Common_UI_Tools::setConsoleSize (TEST_U_CURSES_CONSOLE_WIDTH,
                                       TEST_U_CURSES_CONSOLE_HEIGHT);

      curses_configuration_p->height = TEST_U_CURSES_CONSOLE_HEIGHT; // lines
      curses_configuration_p->width = TEST_U_CURSES_CONSOLE_WIDTH; // columns
//      curses_configuration_p->height = 120; // lines
//      curses_configuration_p->width = 45; // columns
#endif // ACE_WIN32 || ACE_WIN64

      curses_manager_p = TEST_U_CURSES_MANAGER_SINGLETON::instance ();
      ACE_ASSERT (curses_manager_p);
      struct Test_U_CursesState& state_r =
        const_cast<struct Test_U_CursesState&> (curses_manager_p->getR ());
      state_r.stream = stream_p;

      if (!curses_manager_p->initialize (*curses_configuration_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_Curses_Manager_T::initialize(), returning\n")));
        goto clean;
      } // end IF
      if (!curses_manager_p->start (NULL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_Curses_Manager_T::start(), returning\n")));
        goto clean;
      } // end IF
      ACE_OS::sleep (1);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Common_Image_Resolution_t resolution_s;
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          directshow_modulehandler_configuration_3.window_2 = state_r.std_window;
          ACE_ASSERT (directshow_modulehandler_configuration_3.window_2);
          resolution_s.cx = getmaxx (state_r.std_window);
          resolution_s.cy = getmaxy (state_r.std_window);
          Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_s,
                                                                 directshow_modulehandler_configuration.outputFormat);
          Stream_MediaFramework_DirectShow_Tools::setFormat (MEDIASUBTYPE_RGB24,
                                                             directshow_modulehandler_configuration_2.outputFormat);
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          mediafoundation_modulehandler_configuration.window_2 = state_r.std_window;
          ACE_ASSERT (mediafoundation_modulehandler_configuration.window_2);
          Stream_MediaFramework_MediaFoundation_Tools::setResolution (resolution_s,
                                                                      mediafoundation_modulehandler_configuration.outputFormat);
          Stream_MediaFramework_MediaFoundation_Tools::setFormat (MEDIASUBTYPE_RGB24,
                                                                  mediafoundation_modulehandler_configuration_2.outputFormat);
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
      modulehandler_configuration.window_2 = state_r.std_window;
      ACE_ASSERT (modulehandler_configuration.window_2);
      modulehandler_configuration.outputFormat.format.width =
        getmaxx (state_r.std_window);
      modulehandler_configuration.outputFormat.format.height =
        getmaxy (state_r.std_window);
      modulehandler_configuration_2.outputFormat.format.pixelformat =
        V4L2_PIX_FMT_RGB24;
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
#endif // CURSES_SUPPORT
#if defined (GLUT_SUPPORT)
    case STREAM_VISUALIZATION_VIDEORENDERER_OPENGL_GLUT:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      DestroyWindow (directshow_modulehandler_configuration_3.window);
      directshow_modulehandler_configuration_3.window = NULL;

      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        { // *TODO*: there is no way to specify BGRA with _AMMediaType !
          //Stream_MediaFramework_DirectShow_Tools::setFormat (MEDIASUBTYPE_ARGB32,
          //                                                   directshow_modulehandler_configuration_2.outputFormat);
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
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
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
#endif // GLUT_SUPPORT
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
      //do_finalize_directshow (directShowCBData_in.streamConfiguration);
      IAMStreamConfig* dummy_p = NULL;
      do_finalize_directshow (dummy_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Stream_CameraScreen_MediaFoundation_StreamConfiguration_t::ITERATOR_T iterator =
      mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator != mediaFoundationConfiguration_in.streamConfiguration.end ());
      do_finalize_mediafoundation ((*iterator).second.second->session);
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
  do_finalize_v4l (deviceIdentifier_in);

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
void
do_test_methods (const std::string& deviceIdentifier_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_test_methods"));

  enum v4l2_memory method_e = V4L2_MEMORY_MMAP;

  int open_mode_i =
      ((method_e == V4L2_MEMORY_MMAP) ? O_RDWR : O_RDONLY);
  int fd = v4l2_open (ACE_TEXT_ALWAYS_CHAR (deviceIdentifier_in.c_str ()),
                      open_mode_i);
  if (unlikely (fd == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", returning\n"),
                ACE_TEXT (deviceIdentifier_in.c_str ()),
                open_mode_i));
    return;
  } // end IF

  struct v4l2_requestbuffers request_buffers_s;
  ACE_OS::memset (&request_buffers_s, 0, sizeof (struct v4l2_requestbuffers));
  request_buffers_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request_buffers_s.memory = method_e;
  request_buffers_s.count = 1;

  int result = v4l2_ioctl (fd,
                           VIDIOC_REQBUFS,
                           &request_buffers_s);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error == EINVAL)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: V4L2_MEMORY_MMAP not supported, continuing\n"),
                  ACE_TEXT (deviceIdentifier_in.c_str ())));
      goto next;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(VIDIOC_REQBUFS): \"%m\", returning\n")));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s: supports V4L2_MEMORY_MMAP\n"),
              ACE_TEXT (deviceIdentifier_in.c_str ())));

next:
  method_e = V4L2_MEMORY_USERPTR;
  ACE_OS::memset (&request_buffers_s, 0, sizeof (struct v4l2_requestbuffers));
  request_buffers_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request_buffers_s.memory = method_e;
  request_buffers_s.count = 1;

  result = v4l2_ioctl (fd,
                       VIDIOC_REQBUFS,
                       &request_buffers_s);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error == EINVAL)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: V4L2_MEMORY_USERPTR not supported, continuing\n"),
                  ACE_TEXT (deviceIdentifier_in.c_str ())));
      goto next_2;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(VIDIOC_REQBUFS): \"%m\", returning\n")));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s: supports V4L2_MEMORY_USERPTR\n"),
              ACE_TEXT (deviceIdentifier_in.c_str ())));

next_2:
  method_e = V4L2_MEMORY_DMABUF;
  ACE_OS::memset (&request_buffers_s, 0, sizeof (struct v4l2_requestbuffers));
  request_buffers_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request_buffers_s.memory = method_e;
  request_buffers_s.count = 1;

  result = v4l2_ioctl (fd,
                       VIDIOC_REQBUFS,
                       &request_buffers_s);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error == EINVAL)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: V4L2_MEMORY_DMABUF not supported, continuing\n"),
                  ACE_TEXT (deviceIdentifier_in.c_str ())));
      goto next_3;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_ioctl(VIDIOC_REQBUFS): \"%m\", returning\n")));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s: supports V4L2_MEMORY_DMABUF\n"),
              ACE_TEXT (deviceIdentifier_in.c_str ())));

next_3:
error:
  result = v4l2_close (fd);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                fd));
}
#endif // ACE_WIN32 || ACE_WIN64

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
  struct Stream_Device_Identifier device_identifier;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      device_identifier =
        Stream_Device_DirectShow_Tools::getDefaultCaptureDevice (CLSID_VideoInputDeviceCategory);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // *TODO*
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  STREAM_LIB_DEFAULT_MEDIAFRAMEWORK));
      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
      return EXIT_FAILURE;
    }
  } // end SWITCH
  bool show_console = false;
#else
  device_identifier.identifier =
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
  device_identifier.identifier += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  device_identifier.identifier +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
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
  enum Stream_CameraScreen_ProgramMode program_mode_e =
      STREAM_CAMERASCREEN_PROGRAMMODE_NORMAL;
  //bool run_stress_test = false;

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
                             device_identifier,
                             log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                             display_device_s,
                             video_renderer_e,
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
    case STREAM_CAMERASCREEN_PROGRAMMODE_PRINT_VERSION:
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case STREAM_CAMERASCREEN_PROGRAMMODE_TEST_METHODS:
    {
      do_test_methods (device_identifier.identifier);

      Common_Log_Tools::finalizeLogging ();
      Common_Tools::finalize ();

      return EXIT_SUCCESS;
    }
#endif // ACE_WIN32 || ACE_WIN64
    case STREAM_CAMERASCREEN_PROGRAMMODE_NORMAL:
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

  struct Stream_CameraScreen_DirectShow_Configuration directshow_configuration;
  struct Stream_CameraScreen_MediaFoundation_Configuration mediafoundation_configuration;
#else
  struct Stream_CameraScreen_Configuration configuration;
#endif // ACE_WIN32 || ACE_WIN64

//#if defined (GTK_USE)
//  if (video_renderer_e == STREAM_VISUALIZATION_VIDEORENDERER_GTK_WINDOW)
//    Common_UI_GTK_Tools::initialize (argc_in, argv_in);
//#endif // GTK_USE

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (device_identifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
           display_device_s,
           video_renderer_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_configuration,
           mediafoundation_configuration
#else
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
