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

#include "stream_dev_vfw_tools.h"

#include "amvideo.h"
#include "Vfw.h"

#include "stream_lib_directshow_tools.h"

struct Stream_Device_Identifier
Stream_Device_VideoForWindows_Tools::getDefaultCaptureDevice ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_VideoForWindows_Tools::getDefaultCaptureDevice"));

  // initialize return value(s)
  struct Stream_Device_Identifier result;

  Stream_Device_List_t devices_a =
    Stream_Device_VideoForWindows_Tools::getCaptureDevices ();
  if (likely (!devices_a.empty ()))
    return devices_a.front ();

  return result;
}

Stream_Device_List_t
Stream_Device_VideoForWindows_Tools::getCaptureDevices ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_VideoForWindows_Tools::getCaptureDevices"));

  // initialize return value(s)
  Stream_Device_List_t result;

  struct Stream_Device_Identifier device_s;
  device_s.identifierDiscriminator = Stream_Device_Identifier::ID;
  char device_name_a[BUFSIZ] = {0}, device_version_a[BUFSIZ] = {0};

  for (UINT i = 0; i < 10; i++) // *NOTE*: yes, it's 10
    if (capGetDriverDescription (i,
                                 device_name_a, sizeof (char[BUFSIZ]),
                                 device_version_a, sizeof (char[BUFSIZ])) == TRUE)
    {
      device_s.identifier._id = i;
      device_s.description = device_name_a;
      result.push_back (device_s);

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%u: found device: \"%s\" (version: \"%s\")\n"),
                  i,
                  ACE_TEXT (device_name_a),
                  ACE_TEXT (device_version_a)));
    } // end IF

  return result;
}

bool
Stream_Device_VideoForWindows_Tools::getCaptureFormat (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                       struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_VideoForWindows_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (deviceIdentifier_in.identifierDiscriminator == Stream_Device_Identifier::ID);

  // initialize return value(s)
  ACE_OS::memset (&mediaType_out, 0, sizeof (struct _AMMediaType));

  HWND window_h =
    capCreateCaptureWindow (NULL,         // window name if pop-up
                            0,            // window style
                            0, 0, 0, 0,   // window position and dimensions
                            HWND_MESSAGE, // parent window
                            0);           // child ID
  if (unlikely (!window_h))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to capCreateCaptureWindow(), aborting\n")));
    return false;
  } // end IF

  if (unlikely (capDriverConnect (window_h,
                                  deviceIdentifier_in.identifier._id) == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to capDriverConnect(%u), aborting\n"),
                deviceIdentifier_in.identifier._id));
    DestroyWindow (window_h);
    return false;
  } // end IF

  struct tagBITMAPINFO format_s;
  ACE_OS::memset (&format_s, 0, sizeof (struct tagBITMAPINFO));
  if (unlikely (capGetVideoFormat (window_h,
                                   &format_s,
                                   sizeof (struct tagBITMAPINFO)) == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to capGetVideoFormat(), aborting\n")));
    capDriverDisconnect (window_h);
    DestroyWindow (window_h);
    return false;
  } // end IF

  if (unlikely (capDriverDisconnect (window_h) == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to capDriverDisconnect(), aborting\n")));
    DestroyWindow (window_h);
    return false;
  } // end IF

  DestroyWindow (window_h); window_h = NULL;

  mediaType_out.bFixedSizeSamples = TRUE;
  mediaType_out.bTemporalCompression = FALSE;
  mediaType_out.cbFormat = sizeof (struct tagVIDEOINFOHEADER);
  mediaType_out.formattype = FORMAT_VideoInfo;
  mediaType_out.lSampleSize = DIBSIZE (format_s.bmiHeader);
  mediaType_out.majortype = MEDIATYPE_Video;
  if (format_s.bmiHeader.biCompression == BI_RGB)
    switch (format_s.bmiHeader.biBitCount)
    {
      case 24:
        mediaType_out.subtype = MEDIASUBTYPE_RGB24;
        break;
      case 32:
        mediaType_out.subtype = MEDIASUBTYPE_RGB32;
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown bit count (was: %u), aborting\n"),
                    format_s.bmiHeader.biBitCount));
        return false;
      }
    } // end SWITCH
  else
    mediaType_out.subtype =
      Stream_MediaFramework_DirectShow_Tools::compressionToSubType (format_s.bmiHeader.biCompression);
  mediaType_out.pbFormat = (BYTE*)CoTaskMemAlloc (mediaType_out.cbFormat);
  if (unlikely (!mediaType_out.pbFormat))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (%u byte(s)), aborting\n"),
                mediaType_out.cbFormat));
    return false;
  } // end IF
  ACE_OS::memset (mediaType_out.pbFormat, 0, mediaType_out.cbFormat);
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_out.pbFormat);
  video_info_header_p->bmiHeader = format_s.bmiHeader;

  return true;
}
