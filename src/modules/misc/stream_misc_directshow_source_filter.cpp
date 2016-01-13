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

#include "stream_misc_directshow_source_filter.h"

#include "ace/Log_Msg.h"

#include "streams.h"
#include "olectl.h"
#include "initguid.h"

#include "reghelper.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

//------------------------------------------------------------------------------
// Define GUIDS used in this sample
//------------------------------------------------------------------------------

// {F9F62434-535B-4934-A695-BE8D10A4C699}
DEFINE_GUID (CLSID_ACEStream_Source_Filter,
0xf9f62434, 0x535b, 0x4934, 0xa6, 0x95, 0xbe, 0x8d, 0x10, 0xa4, 0xc6, 0x99);

// Setup data
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
  &MEDIATYPE_Video,  // Major type
  &MEDIASUBTYPE_NULL // Minor type
};
const REGFILTERPINS2 sudOpPin = 
{
  REG_PINFLAG_B_OUTPUT,  // dwFlags  (also _B_ZERO, _B_MANY, _B_RENDERER)
  1,                     // cInstances
  1,                     // nMediaTypes
  &sudOpPinTypes,        // lpMediaType
  0,                     // nMediums
  NULL,                  // lpMedium
  &PIN_CATEGORY_CAPTURE, // clsPinCategory
};
const AdvFilterInfo sudACEStreamax =
{
  &CLSID_ACEStream_Source_Filter,          // Filter CLSID
  MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE, // String name
  0,                                       // must be 0 for new structure
  1,                                       // Number pins
  &sudOpPin,                               // Pin details
  MERIT_DO_NOT_USE,                        // filter merit
  &CLSID_VideoInputDeviceCategory,
};

// COM global table of objects in this dll
CFactoryTemplate g_Templates[] = {
  { MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE
  , &CLSID_ACEStream_Source_Filter
  , Stream_Misc_DirectShow_Source_Filter::CreateInstance
  , NULL
  , (AMOVIESETUP_FILTER*)&sudACEStreamax }
};
int g_cTemplates = sizeof (g_Templates) / sizeof (g_Templates[0]);

//
// DllRegisterServer
//
// Exported entry points for registration and unregistration
//
STDAPI DllRegisterServer ()
{
  return DShowAdvancedRegister (TRUE);
} // DllRegisterServer

//
// DllUnregisterServer
//
STDAPI DllUnregisterServer ()
{
  return DShowAdvancedRegister (FALSE);
} // DllUnregisterServer

//
// CreateInstance
//
// The only allowed way to create Bouncing balls!
//
CUnknown* WINAPI
Stream_Misc_DirectShow_Source_Filter::CreateInstance (LPUNKNOWN lpunk_in,
                                                      HRESULT* result_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter::CreateInstance"));

  // sanity check(s)
  ACE_ASSERT (result_out);

  CUnknown* unknown_p = NULL;
  ACE_NEW_NORETURN (unknown_p,
                    Stream_Misc_DirectShow_Source_Filter (lpunk_in, result_out));
  if (!unknown_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    *result_out = E_OUTOFMEMORY;
  } // end IF

  return unknown_p;
} // CreateInstance

//
// Constructor
//
Stream_Misc_DirectShow_Source_Filter::Stream_Misc_DirectShow_Source_Filter (LPUNKNOWN lpunk_in,
                                                                            HRESULT* result_out)
 : CSource (NAME ("Bouncing ball"),
            lpunk,
            CLSID_ACEStream_Source_Filter)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter::Stream_Misc_DirectShow_Source_Filter"));

  CAutoLock cAutoLock (&m_cStateLock);

  m_paStreams =
    (CSourceStream**) new Stream_Misc_DirectShow_Source_Filter_OutputPin*[1];
  if (!m_paStreams)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    *result_out = E_OUTOFMEMORY;
    return;
  } // end IF

  m_paStreams[0] =
    new Stream_Misc_DirectShow_Source_Filter_OutputPin (result_out,
                                                        this,
                                                        MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE);
  if (!m_paStreams[0])
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    *result_out = E_OUTOFMEMORY;
    return;
  }
} // (Constructor)

//
// Constructor
//
Stream_Misc_DirectShow_Source_Filter_OutputPin::Stream_Misc_DirectShow_Source_Filter_OutputPin (HRESULT* result_out,
                                                                                                Stream_Misc_DirectShow_Source_Filter* parent_in,
                                                                                                LPCWSTR pinName_in)
 : CSourceStream (NAME (pinName_in),
                  result_out,
                  parent_in,
                  pinName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin::Stream_Misc_DirectShow_Source_Filter_OutputPin"));

} // (Constructor)
//
// Destructor
//
Stream_Misc_DirectShow_Source_Filter_OutputPin::~Stream_Misc_DirectShow_Source_Filter_OutputPin ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin::~Stream_Misc_DirectShow_Source_Filter_OutputPin"));

} // (Destructor)

//
// FillBuffer
//
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin::FillBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin::FillBuffer"));

  BYTE *pData;
  long lDataLen;

  pms->GetPointer (&pData);
  lDataLen = pms->GetSize ();

  // If true then we clear the output buffer and don't attempt to
  // erase a previous drawing of the ball - this will be the case
  // when we start running as the buffer will be full of rubbish
  if (m_bZeroMemory)
  {
    ZeroMemory (pData, lDataLen);
  }

  {
    CAutoLock cAutoLockShared (&m_cSharedState);

    // If we haven't just cleared the buffer delete the old
    // ball and move the ball on

    if (!m_bZeroMemory)
    {
      BYTE aZeroes[4] = {0, 0, 0, 0};
      m_Ball->PlotBall (pData, aZeroes, m_iPixelSize);
      m_Ball->MoveBall (m_rtSampleTime - (LONG)m_iRepeatTime);
    }

    m_Ball->PlotBall (pData, m_BallPixel, m_iPixelSize);

    // The current time is the sample's start
    CRefTime rtStart = m_rtSampleTime;

    // Increment to find the finish time
    m_rtSampleTime += (LONG)m_iRepeatTime;

    pms->SetTime ((REFERENCE_TIME *)&rtStart, (REFERENCE_TIME *)&m_rtSampleTime);
  }

  m_bZeroMemory = FALSE;
  pms->SetSyncPoint (TRUE);
  return NOERROR;
} // FillBuffer

//
// Notify
//
// Alter the repeat rate according to quality management messages sent from
// the downstream filter (often the renderer).  Wind it up or down according
// to the flooding level - also skip forward if we are notified of Late-ness
//
STDMETHODIMP CBallStream::Notify(IBaseFilter * pSender, Quality q)
{
    // Adjust the repeat rate.
    if (q.Proportion<=0) {
        m_iRepeatTime = 1000;        // We don't go slower than 1 per second
    } else {
        m_iRepeatTime = m_iRepeatTime*1000/q.Proportion;
        if (m_iRepeatTime>1000) {
            m_iRepeatTime = 1000;    // We don't go slower than 1 per second
        } else if (m_iRepeatTime<10) {
            m_iRepeatTime = 10;      // We don't go faster than 100/sec
        }
    }

    // skip forwards
    if (q.Late > 0) {
        m_rtSampleTime += q.Late;
    }
    return NOERROR;

} // Notify


//
// GetMediaType
//
// I _prefer_ 5 formats - 8, 16 (*2), 24 or 32 bits per pixel and
// I will suggest these with an image size of 320x240. However
// I can accept any image size which gives me some space to bounce.
//
// A bit of fun:
//      8 bit displays get red balls
//      16 bit displays get blue
//      24 bit see green
//      And 32 bit see yellow
//
// Prefered types should be ordered by quality, zero as highest quality
// Therefore iPosition =
// 0	return a 32bit mediatype
// 1	return a 24bit mediatype
// 2	return 16bit RGB565
// 3	return a 16bit mediatype (rgb555)
// 4	return 8 bit palettised format
// (iPosition > 4 is invalid)
//
HRESULT CBallStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    if (iPosition < 0) {
        return E_INVALIDARG;
    }

    // Have we run off the end of types

    if (iPosition > 4) {
        return VFW_S_NO_MORE_ITEMS;
    }

    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if (NULL == pvi) {
	return(E_OUTOFMEMORY);
    }
    ZeroMemory(pvi, sizeof(VIDEOINFO));

    switch (iPosition) {
	   case 0: {	// Return our highest quality 32bit format

            // since we use RGB888 (the default for 32 bit), there is
            // no reason to use BI_BITFIELDS to specify the RGB
            // masks. Also, not everything supports BI_BITFIELDS

            SetPaletteEntries(Yellow);
    	    pvi->bmiHeader.biCompression = BI_RGB;
    	    pvi->bmiHeader.biBitCount    = 32;
    	}
    	break;

        case 1: {	// Return our 24bit format

            SetPaletteEntries(Green);
    	    pvi->bmiHeader.biCompression = BI_RGB;
    	    pvi->bmiHeader.biBitCount    = 24;
        }
    	break;

        case 2: {       // 16 bit per pixel RGB565

            // Place the RGB masks as the first 3 doublewords in the palette area
            for (int i = 0; i < 3; i++)
                pvi->TrueColorInfo.dwBitMasks[i] = bits565[i];

            SetPaletteEntries(Blue);
            pvi->bmiHeader.biCompression = BI_BITFIELDS;
    	    pvi->bmiHeader.biBitCount    = 16;
    	}
        break;

        case 3: {	// 16 bits per pixel RGB555

            // Place the RGB masks as the first 3 doublewords in the palette area
            for (int i = 0; i < 3; i++)
    	        pvi->TrueColorInfo.dwBitMasks[i] = bits555[i];

            SetPaletteEntries(Blue);
    	    pvi->bmiHeader.biCompression = BI_BITFIELDS;
    	    pvi->bmiHeader.biBitCount    = 16;

        }
    	break;

        case 4: {	// 8 bit palettised

            SetPaletteEntries(Red);
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount    = 8;
			pvi->bmiHeader.biClrUsed		= iPALETTE_COLORS;
    }
    	break;
    }

    // (Adjust the parameters common to all formats...)

    // put the optimal palette in place
    for (int i = 0; i < iPALETTE_COLORS; i++) {
        pvi->TrueColorInfo.bmiColors[i].rgbRed      = m_Palette[i].peRed;
        pvi->TrueColorInfo.bmiColors[i].rgbBlue     = m_Palette[i].peBlue;
        pvi->TrueColorInfo.bmiColors[i].rgbGreen    = m_Palette[i].peGreen;
        pvi->TrueColorInfo.bmiColors[i].rgbReserved = 0;
    }

    pvi->bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth		= m_iImageWidth;
    pvi->bmiHeader.biHeight		= m_iImageHeight;
    pvi->bmiHeader.biPlanes		= 1;
    pvi->bmiHeader.biSizeImage		= GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant	= 0;

    SetRectEmpty(&(pvi->rcSource));	// we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget));	// no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return NOERROR;

} // GetMediaType


//
// CheckMediaType
//
// We will accept 8, 16, 24 or 32 bit video formats, in any
// image size that gives room to bounce.
// Returns E_INVALIDARG if the mediatype is not acceptable
//
HRESULT CBallStream::CheckMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if ((*(pMediaType->Type()) != MEDIATYPE_Video)	// we only output video!
	    || !(pMediaType->IsFixedSize()) ) {		// ...in fixed size samples
                return E_INVALIDARG;
    }

    // Check for the subtypes we support
    const GUID *SubType = pMediaType->Subtype();
    if ((*SubType != MEDIASUBTYPE_RGB8)
            && (*SubType != MEDIASUBTYPE_RGB565)
	    && (*SubType != MEDIASUBTYPE_RGB555)
 	    && (*SubType != MEDIASUBTYPE_RGB24)
	    && (*SubType != MEDIASUBTYPE_RGB32)) {
                return E_INVALIDARG;
    }

    // Get the format area of the media type
    VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();

    if (pvi == NULL)
	return E_INVALIDARG;

    // Check the image size. As my default ball is 10 pixels big
    // look for at least a 20x20 image. This is an arbitary size constraint,
    // but it avoids balls that are bigger than the picture...

    if ((pvi->bmiHeader.biWidth < 20) || (pvi->bmiHeader.biHeight < 20) ) {
	return E_INVALIDARG;
    }

    // Check if the image width & height have changed
    if (pvi->bmiHeader.biWidth != m_Ball->GetImageWidth() || 
        pvi->bmiHeader.biHeight != m_Ball->GetImageHeight())
    {
        // If the image width/height is changed, fail CheckMediaType() to force
        // the renderer to resize the image.
        return E_INVALIDARG;
    }


    return S_OK;  // This format is acceptable.

} // CheckMediaType


//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT CBallStream::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    // Is this allocator unsuitable

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }

    // Make sure that we have only 1 buffer (we erase the ball in the
    // old buffer to save having to zero a 200k+ buffer every time
    // we draw a frame)

    ASSERT( Actual.cBuffers == 1 );
    return NOERROR;

} // DecideBufferSize


//
// SetMediaType
//
// Called when a media type is agreed between filters
//
HRESULT CBallStream::SetMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Pass the call up to my base class

    HRESULT hr = CSourceStream::SetMediaType(pMediaType);
    if (SUCCEEDED(hr)) {

        VIDEOINFO * pvi = (VIDEOINFO *) m_mt.Format();
        switch (pvi->bmiHeader.biBitCount) {
        case 8:		// Make a red pixel

            m_BallPixel[0] = 10;	// 0 is palette index of red
	    m_iPixelSize   = 1;
	    SetPaletteEntries(Red);
	    break;

        case 16:	// Make a blue pixel

            m_BallPixel[0] = 0xf8;	// 00000000 00011111 is blue in rgb555 or rgb565
	    m_BallPixel[1] = 0x0;	// don't forget the byte ordering within the mask word.
	    m_iPixelSize   = 2;
	    SetPaletteEntries(Blue);
	    break;

        case 24:	// Make a green pixel

            m_BallPixel[0] = 0x0;
	    m_BallPixel[1] = 0xff;
	    m_BallPixel[2] = 0x0;
	    m_iPixelSize   = 3;
	    SetPaletteEntries(Green);
	    break;

	case 32:	// Make a yellow pixel

            m_BallPixel[0] = 0x0;
	    m_BallPixel[1] = 0xff;
	    m_BallPixel[2] = 0xff;
	    m_BallPixel[3] = 0x00;
	    m_iPixelSize   = 4;
            SetPaletteEntries(Yellow);
	    break;

        default:
            // We should never agree any other pixel sizes
	    ASSERT("Tried to agree inappropriate format");
        }

         CBall *pNewBall = new CBall(pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight);

         if( pNewBall ){
             delete m_Ball;
             m_Ball = pNewBall;
         } else
             hr = E_OUTOFMEMORY;

         return NOERROR;
    } else {
        return hr;
    }

} // SetMediaType


//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
HRESULT CBallStream::OnThreadCreate()
{
    CAutoLock cAutoLockShared(&m_cSharedState);
    m_rtSampleTime = 0;

    // we need to also reset the repeat time in case the system
    // clock is turned off after m_iRepeatTime gets very big
    m_iRepeatTime = m_iDefaultRepeatTime;

    // Zero the output buffer on the first frame.
    m_bZeroMemory = TRUE;

    return NOERROR;

} // OnThreadCreate


//
// SetPaletteEntries
//
// If we set our palette to the current system palette + the colours we want
// the system has the least amount of work to do whilst plotting our images,
// if this stream is rendered to the current display. The first non reserved
// palette slot is at m_Palette[10], so put our first colour there. Also
// guarantees that black is always represented by zero in the frame buffer
//
HRESULT CBallStream::SetPaletteEntries(Colour colour)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    HDC hdc = GetDC(NULL);	// hdc for the current display.
    UINT res = GetSystemPaletteEntries(hdc, 0, iPALETTE_COLORS, (LPPALETTEENTRY) &m_Palette);
    ReleaseDC(NULL, hdc);

    if (res == 0) {
        return E_FAIL;
    }

    switch (colour) {
        case Red:
            m_Palette[10].peBlue  = 0;
            m_Palette[10].peGreen = 0;
            m_Palette[10].peRed   = 0xff;
            break;
        case Yellow:
            m_Palette[10].peBlue  = 0;
            m_Palette[10].peGreen = 0xff;
            m_Palette[10].peRed   = 0xff;
            break;
        case Blue:
            m_Palette[10].peBlue  = 0xff;
            m_Palette[10].peGreen = 0;
            m_Palette[10].peRed   = 0;
            break;
        case Green:
            m_Palette[10].peBlue  = 0;
            m_Palette[10].peGreen = 0xff;
            m_Palette[10].peRed   = 0;
            break;
    }

    m_Palette[10].peFlags = 0;
    return NOERROR;

} // SetPaletteEntries

