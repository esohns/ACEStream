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
#else
#include "libv4l2.h"
#if defined (LIBCAMERA_SUPPORT)
#undef emit
#undef slots
#include "libcamera/libcamera.h"
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "gtk/gtk.h"
#elif defined (WXWIDGETS_USE)
#include "gdk/gdk.h"
#include "gtk/gtk.h"
#endif
#endif // GUI_SUPPORT

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

#include "common_image_defines.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_defines.h"
#include "common_ui_tools.h"
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#elif defined (WXWIDGETS_USE)
#include "common_ui_wxwidgets_application.h"
#include "common_ui_wxwidgets_tools.h"
#endif
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#include "stream_dev_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#include "stream_dev_vfw_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_dev_tools.h"

#include "stream_lib_tools.h"

#include "stream_misc_defines.h"

#include "stream_vis_tools.h"

#include "test_u_common.h"
#include "test_u_defines.h"

#include "test_u_camsave_defines.h"
#include "test_u_camsave_eventhandler.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_camsave_gtk_callbacks.h"
#elif defined (WXWIDGETS_USE)
#include "test_u_camsave_ui.h"
#endif
#endif // GUI_SUPPORT
#include "test_u_camsave_signalhandler.h"
#include "test_u_camsave_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("CamSaveStream");
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
const char toplevel_widget_classname_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME);
const char toplevel_widget_name_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME);
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-2          : use Direct2D renderer [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-3          : use Direct3D renderer [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : show console [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : use libcamera [")
            << false
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
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[STRING]]: target filename [")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
#endif // GUI_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m          : use MediaFoundation framework [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_UI_DisplayDevice display_device_s =
    Common_UI_Tools::getDefaultDisplay ();
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o [STRING] : display device [\"")
            << display_device_s.description
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-w          : use Video for Windows API [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x          : test device for method support and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     struct Stream_Device_Identifier& deviceIdentifier_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#else
                     bool& useLibCamera_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& targetFileName_out,
#if defined (GUI_SUPPORT)
                     std::string& UIFile_out,
#endif // GUI_SUPPORT
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     enum Stream_Device_Capturer& capturer_out,
#endif // ACE_WIN32 || ACE_WIN64
                     struct Common_UI_DisplayDevice& displayDevice_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     enum Stream_Camsave_ProgramMode& mode_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  deviceIdentifier_out.clear ();
  showConsole_out = false;
#else
  useLibCamera_out = false;
  deviceIdentifier_out.identifier =
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
  deviceIdentifier_out.identifier += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  deviceIdentifier_out.identifier +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  targetFileName_out = path;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  capturer_out = STREAM_DEVICE_CAPTURER_DIRECTSHOW;
#endif // ACE_WIN32 || ACE_WIN64
  displayDevice_out = Common_UI_Tools::getDefaultDisplay ();
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  mode_out = STREAM_CAMSAVE_PROGRAMMODE_NORMAL;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("cd:f::g::hlmo:s:tvw"),
#else
                              ACE_TEXT ("cd:f::g::hlo:s:tvx"),
#endif // ACE_WIN32 || ACE_WIN64
#else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("cd:f::hlmo:s:tvw"),
#else
                              ACE_TEXT ("cd:f::hlo:s:tvx"),
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT
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
      case 'c':
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        showConsole_out = true;
#else
        useLibCamera_out = true;
#endif // ACE_WIN32 || ACE_WIN64
        break;
      }
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
      case 'f':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          targetFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          targetFileName_out.clear ();
        break;
      }
#if defined (GUI_SUPPORT)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
#endif // GUI_SUPPORT
      case 'l':
      {
        logToFile_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'm':
      {
        capturer_out = STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      case 'o':
      {
        displayDevice_out =
          Common_UI_Tools::getDisplay (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
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
        mode_out = STREAM_CAMSAVE_PROGRAMMODE_PRINT_VERSION;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'w':
      {
        capturer_out = STREAM_DEVICE_CAPTURER_VFW;
        break;
      }
#else
      case 'x':
      {
        mode_out = STREAM_CAMSAVE_PROGRAMMODE_TEST_METHODS;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      // case 'y':
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
  // ---------------------------------------------------------------------------
  if (!allowUserRuntimeConnect_in)
  {
    signals_out.sig_del (SIGUSR1);         // 10      /* User-defined signal 1 */
    ignoredSignals_out.sig_add (SIGUSR1);  // 10      /* User-defined signal 1 */
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
do_initialize_directshow (const struct Stream_Device_Identifier& deviceIdentifier_in,
                          enum Stream_Device_Capturer capturer_in,
                          enum Stream_Visualization_VideoRenderer renderer_in,
                          bool hasUI_in,
                          IGraphBuilder*& IGraphBuilder_out,
                          IAMStreamConfig*& IAMStreamConfig_out,
                          struct _AMMediaType& captureFormat_inout,
                          struct _AMMediaType& displayFormat_inout, // display format
                          struct _AMMediaType& saveFormat_inout) // save format
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

  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_VFW:
    {
      if (unlikely (!Stream_Device_VideoForWindows_Tools::getCaptureFormat (deviceIdentifier_in,
                                                                            captureFormat_inout)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_VideoForWindows_Tools::getCaptureFormat(%u), aborting\n"),
                    ACE_TEXT (deviceIdentifier_in.identifier._id)));
        goto error;
      } // end IF
      ACE_ASSERT (InlineIsEqualGUID (captureFormat_inout.formattype, FORMAT_VideoInfo));
      ACE_ASSERT (captureFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
      struct tagVIDEOINFOHEADER* video_info_header_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER*> (captureFormat_inout.pbFormat);
      video_info_header_p->AvgTimePerFrame =
        /*UNITS*/ 10000000 / STREAM_DEV_CAM_DEFAULT_CAPTURE_FRAME_RATE;
      video_info_header_p->dwBitRate =
        (video_info_header_p->bmiHeader.biSizeImage * 8) *                    // bits / frame
         (UNITS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
      goto continue_;
    }
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
      break;
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown capturer API (was: %d), aborting\n"),
                  capturer_in));
      return false;
    }
  } // end SWITCH

  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (deviceIdentifier_in,
                                                        CLSID_VideoInputDeviceCategory,
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
continue_:
  media_type_p =
    Stream_MediaFramework_DirectShow_Tools::copy (captureFormat_inout);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  saveFormat_inout = *media_type_p;
  delete media_type_p; media_type_p = NULL;

  // *NOTE*: the default save format is ARGB32
  ACE_ASSERT (InlineIsEqualGUID (saveFormat_inout.majortype, MEDIATYPE_Video));
  saveFormat_inout.subtype = MEDIASUBTYPE_RGB32;
  saveFormat_inout.bFixedSizeSamples = TRUE;
  saveFormat_inout.bTemporalCompression = FALSE;
  if (InlineIsEqualGUID (saveFormat_inout.formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (saveFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    struct tagVIDEOINFOHEADER* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (saveFormat_inout.pbFormat);
    // *NOTE*: empty --> use entire video
    result_2 = SetRectEmpty (&video_info_header_p->rcSource);
    ACE_ASSERT (result_2);
    result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
    // *NOTE*: empty --> fill entire buffer
    ACE_ASSERT (result_2);
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
      (UNITS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
    saveFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else if (InlineIsEqualGUID (saveFormat_inout.formattype, FORMAT_VideoInfo2))
  {
    ACE_ASSERT (saveFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
    struct tagVIDEOINFOHEADER2* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (saveFormat_inout.pbFormat);
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
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                   // bits / frame
      (UNITS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
    saveFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (saveFormat_inout.formattype).c_str ())));
    goto error;
  } // end ELSE

  media_type_p =
    Stream_MediaFramework_DirectShow_Tools::copy (saveFormat_inout);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  displayFormat_inout = *media_type_p;
  delete media_type_p; media_type_p = NULL;

  ACE_ASSERT (InlineIsEqualGUID (displayFormat_inout.majortype, MEDIATYPE_Video));
  displayFormat_inout.bFixedSizeSamples = TRUE;
  displayFormat_inout.bTemporalCompression = FALSE;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header_2 = NULL;
  switch (renderer_in)
  {
#if defined (GTK_USE)
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF:
    case STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO:
    {
      // *NOTE*: gdk_pixbuf_get_from_window() returns three channels (see also:
      //         stream_vis_gtk_pixbuf.inl:236), BUT
      //         "...CAIRO_FORMAT_ARGB32: each pixel is a 32-bit quantity, with
      //         alpha in the upper 8 bits, then red, then green, then blue. The
      //         32-bit quantities are stored native-endian. ..."
      // *TODO*: determine color depth of selected (default) screen (i.e.'Display'
      //         ":0")
      displayFormat_inout.subtype = MEDIASUBTYPE_RGB24;
      //displayFormat_inout.subtype = MEDIASUBTYPE_RGB32;
      if (InlineIsEqualGUID (displayFormat_inout.formattype, FORMAT_VideoInfo))
      { ACE_ASSERT (displayFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (displayFormat_inout.pbFormat);
        // *NOTE*: empty --> use entire video
        result_2 = SetRectEmpty (&video_info_header_p->rcSource);
        ACE_ASSERT (result_2);
        result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
        // *NOTE*: empty --> fill entire buffer
        ACE_ASSERT (result_2);
        ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
        ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
        ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
        video_info_header_p->bmiHeader.biBitCount = 24;
        //video_info_header_p->bmiHeader.biBitCount = 32;
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
        displayFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
      } // end IF
      else if (InlineIsEqualGUID (displayFormat_inout.formattype, FORMAT_VideoInfo2))
      {
        ACE_ASSERT (displayFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
        video_info_header_2 =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (displayFormat_inout.pbFormat);
        ACE_ASSERT (video_info_header_2->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
        ACE_ASSERT (video_info_header_2->bmiHeader.biPlanes == 1);
        video_info_header_2->bmiHeader.biBitCount = 24;
        video_info_header_2->bmiHeader.biCompression = BI_RGB;
        video_info_header_2->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_2->bmiHeader);
        ////video_info_header_2->bmiHeader.biXPelsPerMeter;
        ////video_info_header_2->bmiHeader.biYPelsPerMeter;
        ////video_info_header_2->bmiHeader.biClrUsed;
        ////video_info_header_2->bmiHeader.biClrImportant;
        ACE_ASSERT (video_info_header_2->AvgTimePerFrame);
        video_info_header_2->dwBitRate =
          (video_info_header_2->bmiHeader.biSizeImage * 8) *                         // bits / frame
          (NANOSECONDS / static_cast<DWORD> (video_info_header_2->AvgTimePerFrame)); // fps
        displayFormat_inout.lSampleSize = video_info_header_2->bmiHeader.biSizeImage;
      } // end IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (displayFormat_inout.formattype).c_str ())));
        goto error;
      } // end ELSE

      break;
    }
#endif // GTK_USE
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D:
    case STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D_11:
    {
      displayFormat_inout.subtype = MEDIASUBTYPE_RGB32;
      if (InlineIsEqualGUID (displayFormat_inout.formattype, FORMAT_VideoInfo))
      { ACE_ASSERT (displayFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (displayFormat_inout.pbFormat);
        // *NOTE*: empty --> use entire video
        result_2 = SetRectEmpty (&video_info_header_p->rcSource);
        ACE_ASSERT (result_2);
        result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
        // *NOTE*: empty --> fill entire buffer
        ACE_ASSERT (result_2);
        ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
        ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
        ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
        video_info_header_p->bmiHeader.biBitCount = 32;
        video_info_header_p->bmiHeader.biCompression = BI_RGB;
        video_info_header_p->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_p->bmiHeader);
        ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
        video_info_header_p->dwBitRate =
          (video_info_header_p->bmiHeader.biSizeImage * 8) *                         // bits / frame
          (NANOSECONDS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
        displayFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
      } // end IF
      else if (InlineIsEqualGUID (displayFormat_inout.formattype, FORMAT_VideoInfo2))
      {
        ACE_ASSERT (displayFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
        video_info_header_2 =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (displayFormat_inout.pbFormat);
        ACE_ASSERT (video_info_header_2->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
        ACE_ASSERT (video_info_header_2->bmiHeader.biPlanes == 1);
        video_info_header_2->bmiHeader.biBitCount = 32;
        video_info_header_2->bmiHeader.biCompression = BI_RGB;
        video_info_header_2->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_2->bmiHeader);
        ACE_ASSERT (video_info_header_2->AvgTimePerFrame);
        video_info_header_2->dwBitRate =
          (video_info_header_2->bmiHeader.biSizeImage * 8) *                         // bits / frame
          (NANOSECONDS / static_cast<DWORD> (video_info_header_2->AvgTimePerFrame)); // fps
        displayFormat_inout.lSampleSize = video_info_header_2->bmiHeader.biSizeImage;
      } // end IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (displayFormat_inout.formattype).c_str ())));
        goto error;
      } // end ELSE

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown renderer API (was: %d), aborting\n"),
                  renderer_in));
      return false;
    }
  } // end SWITCH

  if (hasUI_in)
  {
    if (IAMStreamConfig_out)
    {
      IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
    } // end IF
    if (IGraphBuilder_out)
    {
      IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
    } // end IF
    return true;
  } // end IF

  if (!Stream_Module_Decoder_Tools::loadVideoRendererGraph (CLSID_VideoInputDeviceCategory,
                                                            captureFormat_inout,
                                                            saveFormat_inout, // directshow output format
                                                            NULL,
                                                            IGraphBuilder_out,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::loadVideoRendererGraph(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
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
  if (media_filter_p)
    media_filter_p->Release ();
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  Stream_MediaFramework_DirectShow_Tools::free (saveFormat_inout);
  Stream_MediaFramework_DirectShow_Tools::free (displayFormat_inout);
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
                               IMFMediaType*& captureFormat_out
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                               ,IMFMediaSession*& IMFMediaSession_out
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                               //,bool loadDevice_in
                              )
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

  //if (!loadDevice_in)
  //  goto continue_2;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
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
  if (!Stream_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                              captureFormat_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getCaptureFormat(\"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in.identifier._guid).c_str ())));
    goto error;
  } // end IF
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

//continue_2:
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
  ACE_ASSERT (IMFMediaSession_out);
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    topology_p->Release (); topology_p = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

//continue_3:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (captureFormat_out)
  {
    captureFormat_out->Release (); captureFormat_out = NULL;
  } // end IF
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
#if defined (LIBCAMERA_SUPPORT)
bool
do_initialize_libcamera (struct Stream_Device_Identifier& deviceIdentifier_out,
                         struct Stream_MediaFramework_LibCamera_MediaType& captureFormat_out,
                         struct Stream_MediaFramework_LibCamera_MediaType& outputFormat_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_libcamera"));

  int result = -1;

  Stream_Device_List_t devices_a;
  libcamera::Camera* camera_p = NULL;
  libcamera::CameraManager* camera_manager_p = NULL;
  ACE_NEW_NORETURN (camera_manager_p,
                    libcamera::CameraManager ());
  ACE_ASSERT (camera_manager_p);
  result = camera_manager_p->start();
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to libcamera::CameraManager::start(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  devices_a =
    Stream_Device_Tools::getVideoCaptureDevices (camera_manager_p);
  if (devices_a.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("found no video capture devices, aborting\n")));
    result = -1;
    goto error;
  } // end IF
  camera_p = Stream_Device_Tools::getCamera (camera_manager_p,
                                             devices_a.front ().identifier).get ();
  ACE_ASSERT (camera_p);
  captureFormat_out =
      Stream_Device_Tools::defaultCaptureFormat (camera_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": default capture format: \"%s\", resolution: %ux%u, framerate: %u/%u\n"),
              ACE_TEXT (devices_a.front ().identifier.c_str ()),
              ACE_TEXT (captureFormat_out.format.toString ().c_str ()),
              captureFormat_out.resolution.width, captureFormat_out.resolution.height,
              captureFormat_out.frameRateNumerator, captureFormat_out.frameRateDenominator));
  // *NOTE*: Gtk 2 expects RGB24
  // *NOTE*: "...CAIRO_FORMAT_ARGB32: each pixel is a 32-bit quantity, with
  //         alpha in the upper 8 bits, then red, then green, then blue. The
  //         32-bit quantities are stored native-endian. ..."
  // *TODO*: determine color depth of selected (default) screen (i.e.'Display'
  //         ":0")
  outputFormat_out.format =
    libcamera::PixelFormat (FOURCC ('R', 'G', 'B', 'A'), 0);
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (GTK2_USE)
  outputFormat_out.format =
    libcamera::PixelFormat (FOURCC ('r', 'a', 'w', ' '), 0);
#endif // GTK2_USE
#endif // GTK_USE
#endif // GUI_SUPPORT
  outputFormat_out.frameRateNumerator = 30;
  outputFormat_out.frameRateDenominator = 1;
  outputFormat_out.resolution = captureFormat_out.resolution;

error:
  camera_manager_p->stop ();
  delete camera_manager_p; camera_manager_p = NULL;

  return (result == 0);
}
#endif // LIBCAMERA_SUPPORT

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
  outputFormat_out = captureFormat_out;
  // *NOTE*: Gtk 2 expects RGB24
  // *NOTE*: "...CAIRO_FORMAT_ARGB32: each pixel is a 32-bit quantity, with
  //         alpha in the upper 8 bits, then red, then green, then blue. The
  //         32-bit quantities are stored native-endian. ..."
  // *TODO*: determine color depth of selected (default) screen (i.e.'Display'
  //         ":0")
  outputFormat_out.format.pixelformat = V4L2_PIX_FMT_RGB32;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (GTK2_USE)
  outputFormat_out.format.pixelformat = V4L2_PIX_FMT_RGB24;
#endif // GTK2_USE
#endif // GTK_USE
#endif // GUI_SUPPORT

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
do_work (const struct Stream_Device_Identifier& deviceIdentifier_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#else
         bool useLibCamera_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& targetFilename_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         //enum Stream_MediaFramework_Type mediaFramework_in,
         enum Stream_Device_Capturer capturer_in,
#endif // ACE_WIN32 || ACE_WIN64
         const struct Common_UI_DisplayDevice& displayDevice_in,
         unsigned int statisticReportingInterval_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Stream_CamSave_DirectShow_Configuration& directShowConfiguration_in,
         struct Stream_CamSave_MediaFoundation_Configuration& mediaFoundationConfiguration_in,
#else
         struct Stream_CamSave_Configuration& configuration_in,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GUI_SUPPORT)
         const std::string& UIDefinitionFilename_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Stream_CamSave_DirectShow_UI_CBData& directShowCBData_in,
         struct Stream_CamSave_MediaFoundation_UI_CBData& mediaFoundationCBData_in,
#else
         struct Stream_CamSave_V4L_UI_CBData& v4l_CBData_in,
#if defined (LIBCAMERA_SUPPORT)
         struct Stream_CamSave_LibCamera_UI_CBData& libCamera_CBData_in,
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#if defined (WXWIDGETS_USE)
         Common_UI_wxWidgets_IApplicationBase_t* iapplication_in,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_CamSave_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (GUI_SUPPORT)
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
#endif // GUI_SUPPORT

  // ********************** module configuration data **************************
  struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration allocator_configuration;
  //if (bufferSize_in)
  //  allocator_configuration.defaultBufferSize = bufferSize_in;

  struct Stream_ModuleConfiguration module_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;   // source
  struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2; // resize
  struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_3; // renderer
  struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_4; // converter_2
  struct Stream_CamSave_DirectShow_StreamConfiguration directshow_stream_configuration;
#if defined (GUI_SUPPORT)
  Stream_CamSave_DirectShow_EventHandler_t directshow_ui_event_handler (&directShowCBData_in
#if defined (GTK_USE)
                                                                       );
#elif defined (WXWIDGETS_USE)
                                                                        ,iapplication_in);
#else
                                                                       );
#endif
#else
  Stream_CamSave_DirectShow_EventHandler_t directshow_ui_event_handler;
#endif // GUI_SUPPORT
  struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Stream_CamSave_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration;
#if defined (GUI_SUPPORT)
  Stream_CamSave_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler (&mediaFoundationCBData_in
#if defined (GTK_USE)
                                                                                 );
#elif defined (WXWIDGETS_USE)
                                                                                  ,iapplication_in);
#else
                                                                                 );
#endif
#else
  Stream_CamSave_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler;
#endif // GUI_SUPPORT
#else
  struct Stream_CamSave_V4L_ModuleHandlerConfiguration v4l_modulehandler_configuration;
  struct Stream_CamSave_V4L_ModuleHandlerConfiguration v4l_resize_modulehandler_configuration;
  struct Stream_CamSave_V4L_ModuleHandlerConfiguration v4l_renderer_modulehandler_configuration;
  struct Stream_CamSave_V4L_ModuleHandlerConfiguration v4l_converter_2_modulehandler_configuration; // save to file

  struct Stream_CamSave_V4L_StreamConfiguration v4l_stream_configuration;
  Stream_CamSave_V4L_EventHandler_t v4l_ui_event_handler (
#if defined (GUI_SUPPORT)
                                                          &v4l_CBData_in
#if defined (GTK_USE)
#elif defined (WXWIDGETS_USE)
                                                          ,iapplication_in
#endif
#endif // GUI_SUPPORT
                                                          );
#if defined (LIBCAMERA_SUPPORT)
  struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration libcamera_modulehandler_configuration;
  struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration libcamera_converter_2_modulehandler_configuration; // save to file
  struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration libcamera_display_modulehandler_configuration;

  struct Stream_CamSave_LibCamera_StreamConfiguration libcamera_stream_configuration;
  Stream_CamSave_LibCamera_EventHandler_t libcamera_ui_event_handler (
#if defined (GUI_SUPPORT)
                                                                      &libCamera_CBData_in
#if defined (GTK_USE)
#elif defined (WXWIDGETS_USE)
                                                                      ,iapplication_in
#endif
#endif // GUI_SUPPORT
                                                                     );
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator;
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator_2; // resize
  //Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator_3; // renderer
  Stream_CamSave_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_stream_iterator_4; // converter_2
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_stream_iterator_2;
  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_VFW:
    {
      //directshow_modulehandler_configuration.preview = true;
      // *WARNING*: falls through !
    }
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
    {
      directshow_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
      directshow_modulehandler_configuration.deviceIdentifier =
        deviceIdentifier_in;
      directshow_modulehandler_configuration.direct3DConfiguration =
        &directShowConfiguration_in.direct3DConfiguration;
      directshow_modulehandler_configuration.lock = &state_r.subscribersLock;

      if (statisticReportingInterval_in)
      {
        directshow_modulehandler_configuration.statisticCollectionInterval.set (0,
                                                                                STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
        directshow_modulehandler_configuration.statisticReportingInterval =
          statisticReportingInterval_in;
      } // end IF
      directshow_modulehandler_configuration.subscriber =
        &directshow_ui_event_handler;
      directshow_modulehandler_configuration.subscribers =
        &directShowCBData_in.subscribers;
      directshow_modulehandler_configuration.targetFileName = targetFilename_in;
      break;
    }
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
      mediafoundation_modulehandler_configuration.deviceIdentifier =
        deviceIdentifier_in;
      mediafoundation_modulehandler_configuration.direct3DConfiguration =
        &mediaFoundationConfiguration_in.direct3DConfiguration;
      mediafoundation_modulehandler_configuration.lock = &state_r.subscribersLock;

      if (statisticReportingInterval_in)
      {
        mediafoundation_modulehandler_configuration.statisticCollectionInterval.set (0,
                                                                                     STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
        mediafoundation_modulehandler_configuration.statisticReportingInterval =
          statisticReportingInterval_in;
      } // end IF
      mediafoundation_modulehandler_configuration.subscriber =
        &mediafoundation_ui_event_handler;
      mediafoundation_modulehandler_configuration.subscribers =
        &mediaFoundationCBData_in.subscribers;
      mediafoundation_modulehandler_configuration.targetFileName =
        targetFilename_in;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                  capturer_in));
      return;
    }
  } // end SWITCH
#else
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T v4l_stream_iterator;
  Stream_CamSave_V4L_StreamConfiguration_t::ITERATOR_T v4l_stream_iterator_2;

  if (useLibCamera_in)
  {
#if defined (LIBCAMERA_SUPPORT)
    libcamera_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
    libcamera_modulehandler_configuration.deviceIdentifier = deviceIdentifier_in;
    libcamera::Camera* camera_p = NULL;
    libcamera::CameraManager* camera_manager_p = NULL;
    ACE_NEW_NORETURN (camera_manager_p,
                      libcamera::CameraManager ());
    ACE_ASSERT (camera_manager_p);
    int result = camera_manager_p->start();
    if (result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to libcamera::CameraManager::start(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    camera_p =
        Stream_Device_Tools::getCamera (camera_manager_p,
                                        deviceIdentifier_in.identifier).get ();
    ACE_ASSERT (camera_p);
    libcamera_modulehandler_configuration.outputFormat =
        Stream_Device_Tools::defaultCaptureFormat (camera_p);
    libcamera_modulehandler_configuration.outputFormat.format =
        libcamera::PixelFormat (FOURCC ('R', 'G', 'B', 'A'));
#if defined (GUI_SUPPORT)
  libcamera_modulehandler_configuration.subscriber = &libcamera_ui_event_handler;
#endif // GUI_SUPPORT
    libcamera_modulehandler_configuration.targetFileName = targetFilename_in;
error:
    camera_manager_p->stop ();
    delete camera_manager_p; camera_manager_p = NULL;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("useLibCamera_in specified, but LIBCAMERA_SUPPORT not set, aborting\n")));
    return;
#endif // LIBCAMERA_SUPPORT
  } // end IF
  else
  {
    v4l_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;
    v4l_modulehandler_configuration.buffers =
      STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS;
    v4l_modulehandler_configuration.deviceIdentifier = deviceIdentifier_in;
    Stream_Device_Tools::getDefaultCaptureFormat (deviceIdentifier_in.identifier,
                                                  v4l_modulehandler_configuration.outputFormat);
    v4l_modulehandler_configuration.outputFormat.format.pixelformat =
        V4L2_PIX_FMT_RGB32;
    if (statisticReportingInterval_in)
    {
      v4l_modulehandler_configuration.statisticCollectionInterval.set (0,
                                                                       STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
      v4l_modulehandler_configuration.statisticReportingInterval =
        statisticReportingInterval_in;
    } // end IF
#if defined (GUI_SUPPORT)
  v4l_modulehandler_configuration.subscriber = &v4l_ui_event_handler;
#endif // GUI_SUPPORT
    v4l_modulehandler_configuration.targetFileName = targetFilename_in;
  } // end ELSE
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
  Stream_CamSave_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                             &heap_allocator,     // heap allocator handle
                                                                             true);               // block ?
  Stream_CamSave_DirectShow_MessageHandler_Module directshow_message_handler (NULL,
                                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Stream_CamSave_DirectShow_Stream directshow_stream;

  Stream_CamSave_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                                       &heap_allocator,     // heap allocator handle
                                                                                       true);               // block ?
  Stream_CamSave_MediaFoundation_MessageHandler_Module mediafoundation_message_handler (NULL,
                                                                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Stream_CamSave_MediaFoundation_Stream mediafoundation_stream;
  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_VFW:
    {
      // *WARNING*: falls through !
    }
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
    {
      directshow_stream_configuration.messageAllocator =
        &directshow_message_allocator;
#if defined (GUI_SUPPORT)
      directshow_stream_configuration.module =
          (!UIDefinitionFilename_in.empty () ? &directshow_message_handler
                                             : NULL);
#endif // GUI_SUPPORT

      directshow_stream_configuration.allocatorConfiguration = &allocator_configuration;
      directshow_stream_configuration.capturer = capturer_in;
      directshow_stream_configuration.renderer =
        STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO;
        //STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D_11;

      directShowConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                 directshow_modulehandler_configuration,
                                                                 directshow_stream_configuration);
      directshow_stream_iterator =
        directShowConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directShowConfiguration_in.streamConfiguration.end ());

#if defined (FFMPEG_SUPPORT)
      directshow_modulehandler_configuration_2 = directshow_modulehandler_configuration;
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_2)));
      directshow_stream_iterator_2 =
        directShowConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_stream_iterator_2 != directShowConfiguration_in.streamConfiguration.end ());
#endif // FFMPEG_SUPPORT

      directshow_modulehandler_configuration_3 = directshow_modulehandler_configuration;
      directshow_modulehandler_configuration_3.deviceIdentifier.identifierDiscriminator =
        Stream_Device_Identifier::STRING;
      ACE_OS::strcpy (directshow_modulehandler_configuration_3.deviceIdentifier.identifier._string,
                      displayDevice_in.device.c_str ());
      directshow_modulehandler_configuration_3.shaderFile =
        Common_File_Tools::getWorkingDirectory ();
      directshow_modulehandler_configuration_3.shaderFile +=
        ACE_DIRECTORY_SEPARATOR_STR;
      directshow_modulehandler_configuration_3.shaderFile +=
        ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
      directshow_modulehandler_configuration_3.shaderFile +=
        ACE_DIRECTORY_SEPARATOR_STR;
      directshow_modulehandler_configuration_3.shaderFile +=
        ACE_TEXT_ALWAYS_CHAR ("shaders.hlsl");
      ACE_ASSERT (Common_File_Tools::isReadable (directshow_modulehandler_configuration_3.shaderFile));
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (Stream_Visualization_Tools::rendererToModuleName (directshow_stream_configuration.renderer),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_3)));
      //directshow_stream_iterator_3 =
      //  directShowConfiguration_in.streamConfiguration.find (Stream_Visualization_Tools::rendererToModuleName (STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO));
      //ACE_ASSERT (directshow_stream_iterator_3 != directShowConfiguration_in.streamConfiguration.end ());

#if defined (FFMPEG_SUPPORT)
      directshow_modulehandler_configuration_4 = directshow_modulehandler_configuration;
      directShowConfiguration_in.streamConfiguration.insert (std::make_pair (std::string (std::string (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)) + ACE_TEXT_ALWAYS_CHAR ("_2")),
                                                                             std::make_pair (&module_configuration,
                                                                                             &directshow_modulehandler_configuration_4)));
      directshow_stream_iterator_4 =
        directShowConfiguration_in.streamConfiguration.find (std::string (std::string (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)) + ACE_TEXT_ALWAYS_CHAR ("_2")));
      ACE_ASSERT (directshow_stream_iterator_4 != directShowConfiguration_in.streamConfiguration.end ());
#endif // FFMPEG_SUPPORT
      break;
    }
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    {
      mediafoundation_stream_configuration.messageAllocator =
          &mediafoundation_message_allocator;
#if defined (GUI_SUPPORT)
      mediafoundation_stream_configuration.module =
          (!UIDefinitionFilename_in.empty () ? &mediafoundation_message_handler
                                             : NULL);
#endif // GUI_SUPPORT
      //mediaFoundationConfiguration_in.streamConfiguration.configuration_.renderer =
      //  renderer_in;
      mediafoundation_stream_configuration.allocatorConfiguration = &allocator_configuration;
      mediafoundation_stream_configuration.capturer = capturer_in;

      mediaFoundationConfiguration_in.streamConfiguration.initialize (module_configuration,
                                                                      mediafoundation_modulehandler_configuration,
                                                                      mediafoundation_stream_configuration);
      mediafoundation_stream_iterator =
        mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediaFoundationConfiguration_in.streamConfiguration.end ());

      mediaFoundationConfiguration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING),
                                                                                  std::make_pair (&module_configuration,
                                                                                                  &mediafoundation_modulehandler_configuration)));

      mediafoundation_stream_iterator_2 =
        mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_stream_iterator_2 != mediaFoundationConfiguration_in.streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                  capturer_in));
      return;
    }
  } // end SWITCH
#else
#if defined (LIBCAMERA_SUPPORT)
  Stream_CamSave_LibCamera_MessageAllocator_t libcamera_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                                           &heap_allocator,     // heap allocator handle
                                                                           true);               // block ?
  Stream_CamSave_LibCamera_MessageHandler_Module libcamera_message_handler (NULL,
                                                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Stream_CamSave_LibCamera_Stream libcamera_stream;
#endif // LIBCAMERA_SUPPORT
  Stream_CamSave_V4L_MessageAllocator_t v4l_message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                               &heap_allocator,     // heap allocator handle
                                                               true);               // block ?
  Stream_CamSave_V4L_MessageHandler_Module v4l_message_handler (NULL,
                                                                ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Stream_CamSave_V4L_Stream v4l_stream;

  //if (bufferSize_in)
  //  CBData_in.configuration->streamConfiguration.allocatorConfiguration_.defaultBufferSize =
  //      bufferSize_in;
#if defined (LIBCAMERA_SUPPORT)
  libcamera_stream_configuration.allocatorConfiguration = &allocator_configuration;
  libcamera_stream_configuration.messageAllocator = &libcamera_message_allocator;
#endif // LIBCAMERA_SUPPORT
  v4l_stream_configuration.allocatorConfiguration = &allocator_configuration;
  v4l_stream_configuration.messageAllocator = &v4l_message_allocator;
#if defined (GUI_SUPPORT)
#if defined (LIBCAMERA_SUPPORT)
  libcamera_stream_configuration.module =
      (!UIDefinitionFilename_in.empty () ? &libcamera_message_handler
                                         : NULL);
#endif // LIBCAMERA_SUPPORT
  v4l_stream_configuration.module =
      (!UIDefinitionFilename_in.empty () ? &v4l_message_handler
                                         : NULL);
#endif // GUI_SUPPORT

  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_handle = NULL;
  //if ((*iterator).second.second->window)
  //{
  //  ACE_ASSERT (gdk_win32_window_is_win32 ((*iterator).second.second->window));
  //  window_handle =
  //    //gdk_win32_window_get_impl_hwnd (configuration.moduleHandlerConfiguration.window);
  //    //gdk_win32_drawable_get_handle (GDK_DRAWABLE (configuration.moduleHandlerConfiguration.window));
  //    static_cast<HWND> (GDK_WINDOW_HWND ((*iterator).second.second->window));
  //} // end IF
  //IAMBufferNegotiation* buffer_negotiation_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;
  IMFMediaSession* media_session_p = NULL;
  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_VFW:
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
    {
      struct _AMMediaType* media_type_p = NULL;
      struct _AMMediaType& media_type_r =
#if defined (FFMPEG_SUPPORT)
        (*directshow_stream_iterator_4).second.second->outputFormat;
#else
        (*directshow_stream_iterator).second.second->outputFormat;
#endif // FFMPEG_SUPPORT
      struct _AMMediaType& media_type_2 =
        (*directshow_stream_iterator).second.second->outputFormat;
      if (!do_initialize_directshow (deviceIdentifier_in,
                                     capturer_in,
                                     directshow_stream_configuration.renderer,
                                     !UIDefinitionFilename_in.empty (), // has UI ?
                                     directshow_modulehandler_configuration.builder,
                                     stream_config_p,
                                     directshow_stream_configuration.format, // capture format
                                     media_type_2,  // --> converter (display format)
                                     media_type_r)) // --> converter_2 (save format); also: directshow output format
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::do_initialize_directshow(), returning\n")));
        return;
      } // end IF
      if (UIDefinitionFilename_in.empty ())
      { ACE_ASSERT (stream_config_p);
        directShowCBData_in.streamConfiguration = stream_config_p;
      } // end IF
      media_type_p =
        Stream_MediaFramework_DirectShow_Tools::copy ((*directshow_stream_iterator).second.second->outputFormat);
      ACE_ASSERT (media_type_p);
#if defined (FFMPEG_SUPPORT)
      (*directshow_stream_iterator_2).second.second->outputFormat = *media_type_p;
#endif // FFMPEG_SUPPORT
      delete media_type_p; media_type_p = NULL;
      break;
    }
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    {
      if (!do_initialize_mediafoundation (deviceIdentifier_in,
                                          window_handle,
                                          mediafoundation_stream_configuration.format
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                          ,(*mediafoundation_stream_iterator).second.second->session
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                         ))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::do_initialize_mediafoundation(), returning\n")));
        return;
      } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT ((*mediafoundation_stream_iterator).second.second->session);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                  capturer_in));
      return;
    }
  } // end SWITCH
#else
  if (useLibCamera_in)
  {
#if defined (LIBCAMERA_SUPPORT)
    if (!do_initialize_libcamera (libcamera_modulehandler_configuration.deviceIdentifier,
                                  libcamera_stream_configuration.format,
                                  libcamera_modulehandler_configuration.outputFormat))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::do_initialize_libcamera(), returning\n")));
      return;
    } // end IF
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("useLibCamera_in specified, but LIBCAMERA_SUPPORT not set, aborting\n")));
    return;
#endif // LIBCAMERA_SUPPORT
  }
  else
    if (!do_initialize_v4l (deviceIdentifier_in.identifier,
                            v4l_modulehandler_configuration.deviceIdentifier,
                            v4l_stream_configuration.format,
                            v4l_modulehandler_configuration.outputFormat))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::do_initialize_v4l(), returning\n")));
      return;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *TODO*: X11 window crashes for 24 bit depths... (BadIDChoice)
//  v4l_modulehandler_configuration.outputFormat.format = AV_PIX_FMT_RGB32;
  configuration_in.v4l_streamConfiguration.initialize (module_configuration,
                                                       v4l_modulehandler_configuration,
                                                       v4l_stream_configuration);
#if defined (LIBCAMERA_SUPPORT)
  configuration_in.libCamera_streamConfiguration.initialize (module_configuration,
                                                             libcamera_modulehandler_configuration,
                                                             libcamera_stream_configuration);
#endif // LIBCAMERA_SUPPORT

  v4l_resize_modulehandler_configuration = v4l_modulehandler_configuration;
  configuration_in.v4l_streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING),
                                                                   std::make_pair (&module_configuration,
                                                                                   &v4l_resize_modulehandler_configuration)));

  v4l_renderer_modulehandler_configuration = v4l_modulehandler_configuration;
  v4l_renderer_modulehandler_configuration.deviceIdentifier.identifier =
      displayDevice_in.device;
  // *TODO*: X11 window crashes for 24 bit depths... (BadIDChoice)
//  v4l_renderer_modulehandler_configuration.outputFormat.format = AV_PIX_FMT_RGB32;
  configuration_in.v4l_streamConfiguration.insert (std::make_pair (Stream_Visualization_Tools::rendererToModuleName (STREAM_VISUALIZATION_VIDEORENDERER_GTK_CAIRO),
                                                                   std::make_pair (&module_configuration,
                                                                                   &v4l_renderer_modulehandler_configuration)));

  // *NOTE*: apparently, Windows Media Player supports only RGB 5:5:5 16bpp AVI
  //         content (see also avienc.c:448)
  v4l_converter_2_modulehandler_configuration = v4l_modulehandler_configuration;
  v4l_converter_2_modulehandler_configuration.outputFormat.format.pixelformat =
      V4L2_PIX_FMT_BGR24;
  configuration_in.v4l_streamConfiguration.insert (std::make_pair (std::string (std::string (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)) + ACE_TEXT_ALWAYS_CHAR ("_2")),
                                                                   std::make_pair (&module_configuration,
                                                                                   &v4l_converter_2_modulehandler_configuration)));
#if defined (LIBCAMERA_SUPPORT)
  libcamera_display_modulehandler_configuration = libcamera_modulehandler_configuration;
  libcamera_display_modulehandler_configuration.display = displayDevice_in;
  configuration_in.libCamera_streamConfiguration.insert (std::make_pair (Stream_Visualization_Tools::rendererToModuleName (STREAM_VISUALIZATION_VIDEORENDERER_GTK_PIXBUF),
                                                                         std::make_pair (&module_configuration,
                                                                                         &libcamera_display_modulehandler_configuration)));

  libcamera_converter_2_modulehandler_configuration =
      libcamera_modulehandler_configuration;
  libcamera_converter_2_modulehandler_configuration.outputFormat.format =
      libcamera::PixelFormat (FOURCC ('R','G', 'B', '5'), 0);
  configuration_in.libCamera_streamConfiguration.insert (std::make_pair (std::string (std::string (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)) + ACE_TEXT_ALWAYS_CHAR ("_2")),
                                                                         std::make_pair (&module_configuration,
                                                                                         &libcamera_converter_2_modulehandler_configuration)));
#endif // LIBCAMERA_SUPPORT
  v4l_stream_iterator =
    configuration_in.v4l_streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (v4l_stream_iterator != configuration_in.v4l_streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Image_Resolution_t resolution_s;
  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_VFW:
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
    {
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (directshow_stream_configuration.format);
      stream_p = &directshow_stream;
      break;
    }
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    {
      resolution_s =
        Stream_MediaFramework_MediaFoundation_Tools::toResolution (mediafoundation_stream_configuration.format);
      stream_p = &mediafoundation_stream;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                  capturer_in));
      return;
    }
  } // end SWITCH
  //struct _D3DDISPLAYMODE display_mode_s =
  //  Stream_MediaFramework_DirectDraw_Tools::getDisplayMode (directShowConfiguration_in.direct3DConfiguration.adapter,
  //                                                          STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT,
  //                                                          resolution_s);
  ACE_ASSERT (!directShowConfiguration_in.direct3DConfiguration.presentationParameters.hDeviceWindow);
  //directShowConfiguration_in.direct3DConfiguration.focusWindow =
  //    GetConsoleWindow ();
  directShowConfiguration_in.direct3DConfiguration.presentationParameters.BackBufferWidth =
      resolution_s.cx;
  directShowConfiguration_in.direct3DConfiguration.presentationParameters.BackBufferHeight =
      resolution_s.cy;
  directShowConfiguration_in.direct3DConfiguration.presentationParameters.hDeviceWindow =
    GetConsoleWindow ();
  IDirect3DDeviceManager9* direct3D_manager_p = NULL;
  UINT reset_token = 0;
  if (!Stream_MediaFramework_DirectDraw_Tools::getDevice (directShowConfiguration_in.direct3DConfiguration,
                                                          direct3D_manager_p,
                                                          reset_token))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectDraw_Tools::getDevice(), continuing\n")));
  else
  {
    ACE_ASSERT (directShowConfiguration_in.direct3DConfiguration.handle);
    ACE_ASSERT (direct3D_manager_p);
    ACE_ASSERT (reset_token);
    direct3D_manager_p->Release (); direct3D_manager_p = NULL;
    reset_token = 0;
  } // end ELSE
#else
  if (useLibCamera_in)
#if defined (LIBCAMERA_SUPPORT)
    stream_p = &libcamera_stream;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("useLibCamera_in specified, but LIBCAMERA_SUPPORT not set, continuing\n")));
#endif // LIBCAMERA_SUPPORT
  else
    stream_p = &v4l_stream;
#endif // ACE_WIN32 || ACE_WIN64

  struct Common_TimerConfiguration timer_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  int result = -1;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step0e: initialize signal handling
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_VFW:
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
    {
      directShowConfiguration_in.signalHandlerConfiguration.messageAllocator =
        &directshow_message_allocator;
      signalHandler_in.initialize (directShowConfiguration_in.signalHandlerConfiguration);
      break;
    }
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    {
      mediaFoundationConfiguration_in.signalHandlerConfiguration.messageAllocator =
        &mediafoundation_message_allocator;
      signalHandler_in.initialize (mediaFoundationConfiguration_in.signalHandlerConfiguration);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                  capturer_in));
      return;
    }
  } // end SWITCH
#else
  configuration_in.signalHandlerConfiguration.messageAllocator =
    &v4l_message_allocator;
  signalHandler_in.initialize (configuration_in.signalHandlerConfiguration);
#endif // ACE_WIN32 || ACE_WIN64
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

#if defined (GUI_SUPPORT)
  // step1a: start UI event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (capturer_in)
    {
      case STREAM_DEVICE_CAPTURER_VFW:
      case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
      {
        directShowCBData_in.stream = &directshow_stream;
#if defined (GTK_USE)
        directShowCBData_in.UIState = &state_r;
        directShowCBData_in.progressData.state = &state_r;
#elif defined (WXWIDGETS_USE)
        struct Common_UI_wxWidgets_State& state_r =
          const_cast<struct Common_UI_wxWidgets_State&> (iapplication_in->getR ());
        state_r.resources[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
          std::make_pair (UIDefinitionFilename_in, static_cast<wxObject*> (NULL));
#endif
        break;
      }
      case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
      {
        mediaFoundationCBData_in.stream = &mediafoundation_stream;
#if defined (GTK_USE)
        mediaFoundationCBData_in.UIState = &state_r;
        mediaFoundationCBData_in.progressData.state = &state_r;
#elif defined (WXWIDGETS_USE)
        struct Common_UI_wxWidgets_State& state_r =
          const_cast<struct Common_UI_wxWidgets_State&> (iapplication_in->getR ());
        state_r.resources[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
          std::make_pair (UIDefinitionFilename_in, static_cast<wxObject*> (NULL));
#endif
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                    capturer_in));
        return;
      }
    } // end SWITCH
#else
#if defined (LIBCAMERA_SUPPORT)
    libCamera_CBData_in.stream = &libcamera_stream;
#endif // LIBCAMERA_SUPPORT
    v4l_CBData_in.stream = &v4l_stream;
#if defined (GTK_USE)
#if defined (LIBCAMERA_SUPPORT)
    libCamera_CBData_in.UIState = &state_r;
    libCamera_CBData_in.progressData.state = &state_r;
#endif // LIBCAMERA_SUPPORT
    v4l_CBData_in.UIState = &state_r;
    v4l_CBData_in.progressData.state = &state_r;
//    CBData_in.pixelBufferLock = &state_r.lock;
#elif defined (WXWIDGETS_USE)
    struct Common_UI_wxWidgets_State& state_r =
      const_cast<struct Common_UI_wxWidgets_State&> (iapplication_in->getR ());
    state_r.resources[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<wxObject*> (NULL));
#endif // GTK_USE
#endif // ACE_WIN32 || ACE_WIN64

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

    ACE_ASSERT (stream_p);
    stream_p->wait (true,
                    false,
                    false);
  } // end IF
  else
  {
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (capturer_in)
    {
      case STREAM_DEVICE_CAPTURER_VFW:
      case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
      {
        Common_UI_GTK_Tools::initialize (directShowCBData_in.configuration->GTKConfiguration.argc,
                                         directShowCBData_in.configuration->GTKConfiguration.argv);

        if (!directshow_stream.initialize (directShowConfiguration_in.streamConfiguration))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize stream, returning\n")));
          goto clean;
        } // end IF
        break;
      }
      case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
      {
        Common_UI_GTK_Tools::initialize (mediaFoundationCBData_in.configuration->GTKConfiguration.argc,
                                         mediaFoundationCBData_in.configuration->GTKConfiguration.argv);

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
                    ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                    capturer_in));
        return;
      }
    } // end SWITCH
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    if (useLibCamera_in)
#if defined (LIBCAMERA_SUPPORT)
      Common_UI_GTK_Tools::initialize (libCamera_CBData_in.configuration->GTKConfiguration.argc,
                                       libCamera_CBData_in.configuration->GTKConfiguration.argv);
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("useLibCamera_in specified, but LIBCAMERA_SUPPORT not set, continuing\n")));
#endif // LIBCAMERA_SUPPORT
    else
      Common_UI_GTK_Tools::initialize (v4l_CBData_in.configuration->GTKConfiguration.argc,
                                       v4l_CBData_in.configuration->GTKConfiguration.argv);
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (LIBCAMERA_SUPPORT)
    if (!libcamera_stream.initialize (configuration_in.libCamera_streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));
      goto clean;
    } // end IF
#endif // LIBCAMERA_SUPPORT
    if (!v4l_stream.initialize (configuration_in.v4l_streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));
      goto clean;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64
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
    stream_p->wait (true,
                    false,
                    false);
#if defined (GUI_SUPPORT)
  } // end ELSE
#endif // GUI_SUPPORT

  // step3: clean up
clean:
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  gtk_manager_p->stop (true, true);
#endif // GTK_USE
#endif // GUI_SUPPORT
  timer_manager_p->stop ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (capturer_in)
  {
    case STREAM_DEVICE_CAPTURER_VFW:
    case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      do_finalize_directshow (directShowCBData_in.streamConfiguration);
#elif defined (WXWIDGETS_USE)
      do_finalize_directshow (stream_config_p);
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
      break;
    case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
    {
      Stream_CamSave_MediaFoundation_StreamConfiguration_t::ITERATOR_T iterator =
      mediaFoundationConfiguration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator != mediaFoundationConfiguration_in.streamConfiguration.end ());
      do_finalize_mediafoundation ((*iterator).second.second->session);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown capturer API (was: %d), returning\n"),
                  capturer_in));
      return;
    }
  } // end SWITCH
#else
  if (useLibCamera_in) ;
//    do_libcamera_finalize ();
  else
    do_finalize_v4l (v4l_modulehandler_configuration.deviceIdentifier);
#endif // ACE_WIN32 || ACE_WIN64

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

COMMON_DEFINE_PRINTVERSION_FUNCTION(do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
void
do_testMethods (const std::string& deviceIdentifier_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_testMethods"));

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

  // initialize framework(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (true,   // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
  if (!Common_UI_WxWidgets_Tools::initialize (argc_in,
                                              argv_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_WxWidgets_Tools::initialize(), aborting\n")));

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
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_Tools::initialize (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
#endif // ACE_WIN32 || ACE_WIN64

  // step1a set defaults
  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();
  struct Stream_Device_Identifier device_identifier;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#else
  bool use_libcamera = false;
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = Common_File_Tools::getTempDirectory ();
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  std::string target_filename = path;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  std::string UI_definition_filename = path;
  UI_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_Device_Capturer capturer_e =
    STREAM_DEVICE_CAPTURER_DIRECTSHOW;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_UI_DisplayDevice display_device_s =
    Common_UI_Tools::getDefaultDisplay ();
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  enum Stream_Camsave_ProgramMode program_mode_e =
      STREAM_CAMSAVE_PROGRAMMODE_NORMAL;
  //bool run_stress_test = false;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  bool result_2 = false;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            device_identifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#else
                            use_libcamera,
#endif // ACE_WIN32 || ACE_WIN64
                            target_filename,
#if defined (GUI_SUPPORT)
                            UI_definition_filename,
#endif // GUI_SUPPORT
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            capturer_e,
#endif // ACE_WIN32 || ACE_WIN64
                            display_device_s,
                            statistic_reporting_interval,
                            trace_information,
                            program_mode_e))
  {
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

  // step1a set defaults (II)

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
  if (device_identifier.empty ())
    switch (capturer_e)
    {
      case STREAM_DEVICE_CAPTURER_VFW:
      {
        device_identifier =
          Stream_Device_VideoForWindows_Tools::getDefaultCaptureDevice ();
        break;
      }
      case STREAM_DEVICE_CAPTURER_DIRECTSHOW:
      {
        media_framework_e = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
        device_identifier =
          Stream_Device_DirectShow_Tools::getDefaultCaptureDevice (CLSID_VideoInputDeviceCategory);
        break;
      }
      case STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION:
      {
        media_framework_e = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
        device_identifier =
          Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown capturer API (was: %d), aborting\n"),
                    capturer_e));

        Common_Tools::finalize ();
        // *PORTABILITY*: on Windows, finalize ACE...
        result = ACE::fini ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
        return EXIT_FAILURE;
      }
    } // end SWITCH
#else
  device_identifier.identifier =
      Stream_Device_Tools::getDefaultVideoCaptureDevice (use_libcamera);
#endif // ACE_WIN32 || ACE_WIN64

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_U_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to a deadlock --> ensure the streaming elements are sufficiently efficient in this regard\n")));
  if (
#if defined (GUI_SUPPORT)
      (!UI_definition_filename.empty () &&
       !Common_File_Tools::isReadable (UI_definition_filename)) ||
#endif // GUI_SUPPORT
      device_identifier.empty ()
     )
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

  // step1d: initialize logging and/or tracing
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_MessageStack_t* logstack_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  logstack_p = &state_r.logStack;
  lock_p = &state_r.logStackLock;

  Common_Logger_t logger (logstack_p,
                          lock_p);
#endif // GTK_USE
#endif // GUI_SUPPORT
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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                            (UI_definition_filename.empty () ? NULL
                                                                             : &logger))) // (ui) logger ?
#elif defined (QT_USE)
                                            NULL))                                        // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                            NULL))                                        // (ui) logger ?
#else
                                            NULL))                                        // (ui) logger ?
#endif
#else
                                            NULL))                                        // (ui) logger ?
#endif // GUI_SUPPORT
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
    case STREAM_CAMSAVE_PROGRAMMODE_PRINT_VERSION:
    {
      do_printVersion (ACE::basename (argv_in[0]));

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
    case STREAM_CAMSAVE_PROGRAMMODE_TEST_METHODS:
    {
      do_testMethods (device_identifier.identifier);

      Common_Log_Tools::finalizeLogging ();
      Common_Tools::finalize ();

      return EXIT_SUCCESS;
    }
#endif // ACE_WIN32 || ACE_WIN64
    case STREAM_CAMSAVE_PROGRAMMODE_NORMAL:
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
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
//  struct Stream_CamSave_UI_CBData* ui_cb_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_CamSave_DirectShow_Configuration directshow_configuration;
  struct Stream_CamSave_DirectShow_UI_CBData directshow_ui_cb_data;
  struct Stream_CamSave_MediaFoundation_Configuration mediafoundation_configuration;
  struct Stream_CamSave_MediaFoundation_UI_CBData mediafoundation_ui_cb_data;
  Stream_MediaFramework_Tools::initialize (media_framework_e);
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data.configuration = &directshow_configuration;
      directshow_ui_cb_data.mediaFramework = media_framework_e;
//      ui_cb_data_p = &directshow_ui_cb_data;

      directshow_ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
      directshow_ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
      directshow_ui_cb_data.configuration->GTKConfiguration.CBData = &directshow_ui_cb_data;
      directshow_ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
          idle_finalize_UI_cb;
      directshow_ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
          idle_initialize_UI_cb;
      directshow_ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data.configuration = &mediafoundation_configuration;
      mediafoundation_ui_cb_data.mediaFramework = media_framework_e;
//      ui_cb_data_p = &mediafoundation_ui_cb_data;

      mediafoundation_ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.CBData = &mediafoundation_ui_cb_data;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
          idle_finalize_UI_cb;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
          idle_initialize_UI_cb;
      mediafoundation_ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  media_framework_e));

      Common_Log_Tools::finalizeLogging ();
      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
      return EXIT_FAILURE;
    }
  } // end SWITCH
#else
  struct Stream_CamSave_Configuration configuration;
  struct Stream_CamSave_V4L_UI_CBData v4l_ui_cb_data;
#if defined (LIBCAMERA_SUPPORT)
  struct Stream_CamSave_LibCamera_UI_CBData libcamera_ui_cb_data;
#endif // LIBCAMERA_SUPPORT
  if (use_libcamera)
  {
#if defined (LIBCAMERA_SUPPORT)
    libcamera_ui_cb_data.useLibCamera = true;
    libcamera_ui_cb_data.configuration = &configuration;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("use_libcamera specified, but LIBCAMERA_SUPPORT not set, continuing\n")));
#endif // LIBCAMERA_SUPPORT
  } // end IF
  else
    v4l_ui_cb_data.configuration = &configuration;
//  ui_cb_data_p = &ui_cb_data;

#if defined (GTK_USE)
  if (use_libcamera)
  {
#if defined (LIBCAMERA_SUPPORT)
    libcamera_ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
    libcamera_ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
    libcamera_ui_cb_data.configuration->GTKConfiguration.CBData =
        &libcamera_ui_cb_data;
    libcamera_ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
        idle_finalize_UI_cb;
    libcamera_ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
        idle_initialize_UI_cb;
    libcamera_ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("use_libcamera specified, but LIBCAMERA_SUPPORT not set, continuing\n")));
#endif // LIBCAMERA_SUPPORT
  } // end IF
  else
  {
    v4l_ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
    v4l_ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
    v4l_ui_cb_data.configuration->GTKConfiguration.CBData = &v4l_ui_cb_data;
    v4l_ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
        idle_finalize_UI_cb;
    v4l_ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
        idle_initialize_UI_cb;
    v4l_ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  } // end ELSE
#endif // GTK_USE
#endif // ACE_WIN32 || ACE_WIN64
  //ACE_ASSERT (ui_cb_data_p);
#endif // GUI_SUPPORT

  // step1h: initialize UI framework
#if defined (GUI_SUPPORT)
  Common_UI_Tools::initialize ();
  struct Common_UI_State* ui_state_p = NULL;
#if defined (GTK_USE)
  ui_state_p = &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  Common_UI_wxWidgets_IApplicationBase_t* iapplication_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      ACE_NEW_NORETURN (iapplication_p,
                        Stream_CamSave_DirectShow_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                                          argc_in,
                                                                          Common_UI_WxWidgets_Tools::convertArgV (argc_in, argv_in),
                                                                          COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
      Stream_CamSave_DirectShow_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Stream_CamSave_DirectShow_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
      iinitialize_p->initialize (directshow_ui_cb_data);
      Stream_CamSave_DirectShow_WxWidgetsIApplication_t* iapplication_2 =
        dynamic_cast<Stream_CamSave_DirectShow_WxWidgetsIApplication_t*> (iapplication_p);
      ACE_ASSERT (iapplication_2);
      Stream_CamSave_DirectShow_WxWidgetsApplication_t::STATE_T& state_r =
        const_cast<Stream_CamSave_DirectShow_WxWidgetsApplication_t::STATE_T&> (iapplication_2->getR ());
      Stream_CamSave_DirectShow_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
        const_cast<Stream_CamSave_DirectShow_WxWidgetsApplication_t::CONFIGURATION_T&> (iapplication_2->getR_2 ());
      configuration_r.UIState = &state_r;
      ACE_ASSERT (configuration_r.UIState);
      ui_state_p =
        const_cast<Stream_CamSave_DirectShow_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ACE_NEW_NORETURN (iapplication_p,
                        Stream_CamSave_MediaFoundation_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                                               argc_in,
                                                                               Common_UI_WxWidgets_Tools::convertArgV (argc_in,
                                                                                                                       argv_in),
                                                                               COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
      Stream_CamSave_MediaFoundation_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Stream_CamSave_MediaFoundation_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
      iinitialize_p->initialize (mediafoundation_ui_cb_data);
      Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t* iapplication_2 =
        dynamic_cast<Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t*> (iapplication_p);
      ACE_ASSERT (iapplication_2);
      const Stream_CamSave_MediaFoundation_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
        iapplication_2->getR_2 ();
      ACE_ASSERT (configuration_r.UIState);
      ui_state_p =
        const_cast<Stream_CamSave_MediaFoundation_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  media_framework_e));

      Common_Log_Tools::finalizeLogging ();
      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
      return EXIT_FAILURE;
    }
  } // end SWITCH
#else
//  // *WORKAROUND*: this prevents crashing the wxGTK3 application in Fedora 29
//  GtkCssProvider* css_provider_p = gtk_css_provider_new ();
//  if (!css_provider_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to gtk_css_provider_new(), returning\n")));

//    Common_Log_Tools::finalizeLogging ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // *PORTABILITY*: on Windows, finalize ACE...
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
//    return EXIT_FAILURE;
//  } // end IF
//  GError* error_p = NULL;
  std::string css_profile_path = Common_File_Tools::getWorkingDirectory ();
  css_profile_path += ACE_DIRECTORY_SEPARATOR_STR;
  css_profile_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  css_profile_path += ACE_DIRECTORY_SEPARATOR_STR;
  css_profile_path +=ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_CAMSAVE_UI_CSS_FILE);
//  if (!gtk_css_provider_load_from_path (css_provider_p,
//                                        css_profile_path.c_str (),
//                                        &error_p))
//  { ACE_ASSERT (error_p);
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gtk_css_provider_load_from_path(\"%s\"): \"%s\", returning\n"),
//                ACE_TEXT (css_profile_path.c_str ()),
//                ACE_TEXT (error_p->message)));
//    g_error_free (error_p); error_p = NULL;

//    Common_Log_Tools::finalizeLogging ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // *PORTABILITY*: on Windows, finalize ACE...
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
//    return EXIT_FAILURE;
//  } // end IF
//  GdkDisplay* display_p = gdk_display_open (ACE_TEXT_ALWAYS_CHAR (":0"));
//  ACE_ASSERT (display_p);
//  gdk_display_manager_set_default_display (gdk_display_manager_get (),
//                                           display_p);
//  GdkScreen* screen_p = gdk_screen_get_default ();
//  ACE_ASSERT (screen_p);
//  gtk_style_context_add_provider_for_screen (screen_p,
//                                             GTK_STYLE_PROVIDER (css_provider_p),
//                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  ACE_NEW_NORETURN (iapplication_p,
                    Stream_CamSave_V4L_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                               argc_in,
                                                               Common_UI_WxWidgets_Tools::convertArgV (argc_in,
                                                                                                       argv_in),
                                                               COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
  Stream_CamSave_V4L_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Stream_CamSave_V4L_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
  // *NOTE*: this sets ui_cb_data.UIState
  iinitialize_p->initialize (ui_cb_data);
  Stream_CamSave_V4L_WxWidgetsIApplication_t* iapplication_2 =
    dynamic_cast<Stream_CamSave_V4L_WxWidgetsIApplication_t*> (iapplication_p);
  ACE_ASSERT (iapplication_2);
  const Stream_CamSave_V4L_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
    iapplication_2->getR_2 ();
  ACE_ASSERT (configuration_r.UIState);
  ui_state_p =
    const_cast<Stream_CamSave_V4L_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;
#endif // ACE_WIN32 || ACE_WIN64
  if (!iapplication_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: %m, aborting\n")));

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
#endif // GTK_USE || WXWIDGETS_USE
  ACE_ASSERT (ui_state_p);
#endif // GUI_SUPPORT

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals ((statistic_reporting_interval == 0), // handle SIGUSR1/SIGBREAK
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

    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
#if defined(ACE_WIN32) || defined(ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
#endif // GUI_SUPPORT
  Stream_CamSave_SignalHandler signal_handler;

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false,  // file descriptors
                                        true,   // stack traces
                                        false)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   previous_signal_actions,
                                   previous_signal_mask);
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

#if defined (GUI_SUPPORT)
  if (!UI_definition_filename.empty ())
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (media_framework_e)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
#if defined (GTK_USE)
        result_2 = gtk_manager_p->initialize (directshow_ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
#if defined (GTK_USE)
        result_2 =
          gtk_manager_p->initialize (mediafoundation_ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                    media_framework_e));

        Common_Signal_Tools::finalize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                       previous_signal_actions,
                                       previous_signal_mask);
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
      }
    } // end SWITCH
#else
#if defined (GTK_USE)
  if (use_libcamera)
#if defined (LIBCAMERA_SUPPORT)
    result_2 =
        gtk_manager_p->initialize (libcamera_ui_cb_data.configuration->GTKConfiguration);
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("use_libcamera specified, but LIBCAMERA_SUPPORT not set, continuing\n")));
#endif // LIBCAMERA_SUPPORT
  else
    result_2 =
        gtk_manager_p->initialize (v4l_ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_USE)
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), aborting\n")));

      Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                     previous_signal_actions,
                                     previous_signal_mask);
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
#endif // GTK_USE
  } // end IF
#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (device_identifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#else
           use_libcamera,
#endif // ACE_WIN32 || ACE_WIN64
           target_filename,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           //media_framework_e,
           capturer_e,
#endif // ACE_WIN32 || ACE_WIN64
           display_device_s,
           statistic_reporting_interval,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_configuration,
           mediafoundation_configuration,
#else
           configuration,
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GUI_SUPPORT)
           UI_definition_filename,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_ui_cb_data,
           mediafoundation_ui_cb_data,
#else
           v4l_ui_cb_data,
#if defined (LIBCAMERA_SUPPORT)
           libcamera_ui_cb_data,
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#if defined (WXWIDGETS_USE)
           iapplication_p,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
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

  Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                 previous_signal_actions,
                                 previous_signal_mask);
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
