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

#include "ace/Synch.h"
#include "stream_vis_target_direct3d.h"

#include <dshow.h>
#include <guiddef.h>
#include <mfapi.h>

#include "common_error_tools.h"

#include "stream_vis_defines.h"

// initialize globals
const char libacestream_default_vis_direct3d_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING);

//////////////////////////////////////////

__forceinline BYTE Clip (int clr) { return (BYTE)(clr < 0 ? 0 : (clr > 255 ? 255 : clr)); }
__forceinline RGBQUAD ConvertYCrCbToRGB (int y,
                                         int cr,
                                         int cb)
{
  RGBQUAD rgbq;

  int c = y  - 16;
  int d = cb - 128;
  int e = cr - 128;

  rgbq.rgbRed   = Clip ((298 * c + 409           * e + 128) >> 8);
  rgbq.rgbGreen = Clip ((298 * c - 100 * d - 208 * e + 128) >> 8);
  rgbq.rgbBlue  = Clip ((298 * c + 516 * d + 128)           >> 8);

  return rgbq;
}

//////////////////////////////////////////

//-------------------------------------------------------------------
// TransformImage_RGB24 
//
// RGB-24 to RGB-32
//-------------------------------------------------------------------
__forceinline void libacestream_vis_transform_image_RGB24 (BYTE*       pDest,
                                                           LONG        lDestStride,
                                                           const BYTE* pSrc,
                                                           LONG        lSrcStride,
                                                           DWORD       dwWidthInPixels,
                                                           DWORD       dwHeightInPixels)
{
  DWORD*     pDestPel = NULL;
  RGBTRIPLE* pSrcPel = NULL;
  for (DWORD y = 0; y < dwHeightInPixels; ++y)
  {
    pDestPel = (DWORD*)pDest;
    pSrcPel  = (RGBTRIPLE*)pSrc;

    for (DWORD x = 0; x < dwWidthInPixels; ++x)
      pDestPel[x] = D3DCOLOR_XRGB (pSrcPel[x].rgbtRed,
                                   pSrcPel[x].rgbtGreen,
                                   pSrcPel[x].rgbtBlue);

    pSrc  += lSrcStride;
    pDest += lDestStride;
  } // end FOR
}

//-------------------------------------------------------------------
// TransformImage_RGB32
//
// RGB-32 to RGB-32 
//
// Note: This function is needed to copy the image from system
// memory to the Direct3D surface.
//-------------------------------------------------------------------
__forceinline void libacestream_vis_transform_image_RGB32 (BYTE*       pDest,
                                                           LONG        lDestStride,
                                                           const BYTE* pSrc,
                                                           LONG        lSrcStride,
                                                           DWORD       dwWidthInPixels,
                                                           DWORD       dwHeightInPixels)
{
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  HRESULT result = MFCopyImage (pDest,
                                lDestStride,
                                pSrc,
                                lSrcStride,
                                dwWidthInPixels * 4,
                                dwHeightInPixels);
  if (unlikely (FAILED (result)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCopyImage(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#else
  ACE_ASSERT (::abs (lSrcStride) <= ::abs (lDestStride));
  if (likely (lSrcStride == lDestStride))
    ACE_OS::memcpy (pDest,
                    pSrc,
                    (lSrcStride * dwHeightInPixels));
  else
    for (DWORD y = 0; y < dwHeightInPixels; y++)
      ACE_OS::memcpy (pDest + (y * lDestStride),
                      pSrc + (y * lSrcStride),
                      ((lSrcStride < 0) ? -lSrcStride : lSrcStride));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
}

//-------------------------------------------------------------------
// TransformImage_YUY2 
//
// YUY2 to RGB-32
//-------------------------------------------------------------------
__forceinline void libacestream_vis_transform_image_YUY2 (BYTE*       pDest,
                                                          LONG        lDestStride,
                                                          const BYTE* pSrc,
                                                          LONG        lSrcStride,
                                                          DWORD       dwWidthInPixels,
                                                          DWORD       dwHeightInPixels)
{
  for (DWORD y = 0; y < dwHeightInPixels; ++y)
  {
    RGBQUAD* pDestPel = (RGBQUAD*)pDest;
    WORD*    pSrcPel  = (WORD*)pSrc;

    for (DWORD x = 0; x < dwWidthInPixels; x += 2)
    {
      // Byte order is U0 Y0 V0 Y1
      int y0 = (int)LOBYTE (pSrcPel[x]);
      int u0 = (int)HIBYTE (pSrcPel[x]);
      int y1 = (int)LOBYTE (pSrcPel[x + 1]);
      int v0 = (int)HIBYTE (pSrcPel[x + 1]);

      pDestPel[x] = ConvertYCrCbToRGB (y0, v0, u0);
      pDestPel[x + 1] = ConvertYCrCbToRGB (y1, v0, u0);
    } // end FOR

    pSrc  += lSrcStride;
    pDest += lDestStride;
  } // end FOR
}

//-------------------------------------------------------------------
// TransformImage_NV12
//
// NV12 to RGB-32
//-------------------------------------------------------------------
__forceinline void libacestream_vis_transform_image_NV12 (BYTE* pDst,
                                                          LONG dstStride,
                                                          const BYTE* pSrc,
                                                          LONG srcStride,
                                                          DWORD dwWidthInPixels,
                                                          DWORD dwHeightInPixels)
{
  const BYTE* lpBitsY = pSrc;
  const BYTE* lpBitsCb = lpBitsY + (dwHeightInPixels * srcStride);;
  const BYTE* lpBitsCr = lpBitsCb + 1;

  for (UINT y = 0; y < dwHeightInPixels; y += 2)
  {
    const BYTE* lpLineY1 = lpBitsY;
    const BYTE* lpLineY2 = lpBitsY + srcStride;
    const BYTE* lpLineCr = lpBitsCr;
    const BYTE* lpLineCb = lpBitsCb;

    LPBYTE lpDibLine1 = pDst;
    LPBYTE lpDibLine2 = pDst + dstStride;

    for (UINT x = 0; x < dwWidthInPixels; x += 2)
    {
      int  y0 = (int)lpLineY1[0];
      int  y1 = (int)lpLineY1[1];
      int  y2 = (int)lpLineY2[0];
      int  y3 = (int)lpLineY2[1];
      int  cb = (int)lpLineCb[0];
      int  cr = (int)lpLineCr[0];

      RGBQUAD r = ConvertYCrCbToRGB (y0, cr, cb);
      lpDibLine1[0] = r.rgbBlue;
      lpDibLine1[1] = r.rgbGreen;
      lpDibLine1[2] = r.rgbRed;
      lpDibLine1[3] = 0; // Alpha

      r = ConvertYCrCbToRGB (y1, cr, cb);
      lpDibLine1[4] = r.rgbBlue;
      lpDibLine1[5] = r.rgbGreen;
      lpDibLine1[6] = r.rgbRed;
      lpDibLine1[7] = 0; // Alpha

      r = ConvertYCrCbToRGB (y2, cr, cb);
      lpDibLine2[0] = r.rgbBlue;
      lpDibLine2[1] = r.rgbGreen;
      lpDibLine2[2] = r.rgbRed;
      lpDibLine2[3] = 0; // Alpha

      r = ConvertYCrCbToRGB (y3, cr, cb);
      lpDibLine2[4] = r.rgbBlue;
      lpDibLine2[5] = r.rgbGreen;
      lpDibLine2[6] = r.rgbRed;
      lpDibLine2[7] = 0; // Alpha

      lpLineY1 += 2;
      lpLineY2 += 2;
      lpLineCr += 2;
      lpLineCb += 2;

      lpDibLine1 += 8;
      lpDibLine2 += 8;
    } // end FOR

    pDst     += (2 * dstStride);
    lpBitsY  += (2 * srcStride);
    lpBitsCr += srcStride;
    lpBitsCb += srcStride;
  } // end FOR
}

struct Stream_Vis_Target_Direct3D_Transformation
libacestream_vis_directshow_format_transformations[] =
{
  { MEDIASUBTYPE_ARGB32, libacestream_vis_transform_image_RGB32 },
  { MEDIASUBTYPE_RGB32,  libacestream_vis_transform_image_RGB32 },
  { MEDIASUBTYPE_RGB24,  libacestream_vis_transform_image_RGB24 },
  { MEDIASUBTYPE_YUY2,   libacestream_vis_transform_image_YUY2 },
  { MEDIASUBTYPE_NV12,   libacestream_vis_transform_image_NV12 }
};
struct Stream_Vis_Target_Direct3D_Transformation
libacestream_vis_mediafoundation_format_transformations[] =
{
  { MFVideoFormat_ARGB32, libacestream_vis_transform_image_RGB32 },
  { MFVideoFormat_RGB32,  libacestream_vis_transform_image_RGB32 },
  { MFVideoFormat_RGB24,  libacestream_vis_transform_image_RGB24 },
  { MFVideoFormat_YUY2,   libacestream_vis_transform_image_YUY2 },
  { MFVideoFormat_NV12,   libacestream_vis_transform_image_NV12 }
};

// *TODO*: find a way to set this using the ARRAYSIZE (formatConversions) macro
DWORD libacestream_vis_number_of_format_transformations = 5;
//ARRAYSIZE (Stream_Vis_Target_Direct3D_T<SessionMessageType,\
//                                        MessageType,\
//                                        ConfigurationType,\
//                                        SessionDataType,\
//                                        SessionDataContainerType>::formatTransformations);

//////////////////////////////////////////
