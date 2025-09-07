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

#include "test_u_imagescreen_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

#include "stream_file_defines.h"

#include "stream_stat_defines.h"

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

Stream_ImageScreen_Stream::Stream_ImageScreen_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING))
#if defined (FFMPEG_SUPPORT)
 , ffmpeg_decode_ (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING))
 , ffmpeg_resize_ (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING))
 , ffmpeg_convert_ (this,
                    ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING))
#endif // FFMPEG_SUPPORT
#if defined (IMAGEMAGICK_SUPPORT)
  , imagemagick_source_ (this,
                         ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_IMAGEMAGICK_SOURCE_DEFAULT_NAME_STRING))
  , imagemagick_resize_ (this,
                         ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_IMAGEMAGICK_RESIZE_DEFAULT_NAME_STRING))
// , imagemagick_decode_ (this,
//                        ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_IMAGEMAGICK_DECODER_DEFAULT_NAME_STRING))
#endif // IMAGEMAGICK_SUPPORT
 , delay_ (this,
           ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DELAY_DEFAULT_NAME_STRING))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , display2D_ (this,
               ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT2D_DEFAULT_NAME_STRING))
 , display3D_ (this,
               ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING))
#else
 , displayX11_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING))
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_SUPPORT)
 , displayGTK_ (this,
                ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING))
#endif // GTK_SUPPORT
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Stream::Stream_ImageScreen_Stream"));

}

Stream_ImageScreen_Stream::~Stream_ImageScreen_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Stream::~Stream_ImageScreen_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Stream_ImageScreen_Stream::load (Stream_ILayout* layout_in,
                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Stream::load"));

  // initialize return value(s)
  delete_out = false;

#if defined (IMAGEMAGICK_SUPPORT)
  layout_in->append (&imagemagick_source_, NULL, 0);
  layout_in->append (&imagemagick_resize_, NULL, 0); // output is window size/fullscreen
#else
  layout_in->append (&source_, NULL, 0);
#if defined (FFMPEG_SUPPORT)
  layout_in->append (&ffmpeg_decode_, NULL, 0); // output is uncompressed RGBA
  layout_in->append (&ffmpeg_resize_, NULL, 0); // output is window size/fullscreen
  layout_in->append (&ffmpeg_convert_, NULL, 0); // output is uncompressed BGRA
#endif // FFMPEG_SUPPORT
#endif // IMAGEMAGICK_SUPPORT
  layout_in->append (&delay_, NULL, 0);
#if defined (GTK_SUPPORT)
  layout_in->append (&displayGTK_, NULL, 0);
#elif defined (ACE_WIN32) || defined (ACE_WIN64)
  layout_in->append (&display2D_, NULL, 0);
  //layout_in->append (&display3D_, NULL, 0);
#else
  layout_in->append (&displayX11_, NULL, 0);
#endif // GTK_SUPPORT || ACE_WIN32 || ACE_WIN64

  return true;
}

bool
Stream_ImageScreen_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Stream_ImageScreen_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  media_type_s.majortype = MEDIATYPE_Video;
  media_type_s.subtype = MEDIASUBTYPE_RGB32;
  media_type_s.bFixedSizeSamples = TRUE;
  media_type_s.bTemporalCompression = FALSE;
  media_type_s.formattype = FORMAT_VideoInfo;
  media_type_s.cbFormat = sizeof (struct tagVIDEOINFOHEADER);
  media_type_s.pbFormat =
    reinterpret_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tagVIDEOINFOHEADER)));
  if (unlikely (!media_type_s.pbFormat))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_OS::memset (media_type_s.pbFormat, 0, sizeof (struct tagVIDEOINFOHEADER));
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_s.pbFormat);
  // *NOTE*: empty --> use entire video
  BOOL result_2 = SetRectEmpty (&video_info_header_p->rcSource);
  ACE_ASSERT (result_2);
  result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
  // *NOTE*: empty --> fill entire buffer
  ACE_ASSERT (result_2);
  //video_info_header_p->dwBitRate = ;
  video_info_header_p->dwBitErrorRate = 0;
  video_info_header_p->AvgTimePerFrame = 1;
  video_info_header_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_header_p->bmiHeader.biWidth = 640;
  video_info_header_p->bmiHeader.biHeight = 480;
  //if (video_info_header_p->bmiHeader.biHeight > 0)
  //  video_info_header_p->bmiHeader.biHeight =
  //    -video_info_header_p->bmiHeader.biHeight;
  //ACE_ASSERT (video_info_header_p->bmiHeader.biHeight < 0);
  video_info_header_p->bmiHeader.biPlanes = 1;
  video_info_header_p->bmiHeader.biBitCount =
    Stream_MediaFramework_Tools::toBitCount (media_type_s.subtype);
  //ACE_ASSERT (video_info_header_p->bmiHeader.biBitCount);
  video_info_header_p->bmiHeader.biCompression = BI_RGB;
  video_info_header_p->bmiHeader.biSizeImage =
    DIBSIZE (video_info_header_p->bmiHeader);
  ////video_info_header_p->bmiHeader.biXPelsPerMeter;
  ////video_info_header_p->bmiHeader.biYPelsPerMeter;
  ////video_info_header_p->bmiHeader.biClrUsed;
  ////video_info_header_p->bmiHeader.biClrImportant;
  media_type_s.lSampleSize =
    video_info_header_p->bmiHeader.biSizeImage;
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  Test_U_SessionManager_t* session_manager_p =
    Test_U_SessionManager_t::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  session_data_p =
    &const_cast<Stream_ImageScreen_SessionData&> (session_manager_p->getR ());
  ACE_ASSERT (session_data_p->formats.empty ());
  // *TODO*: remove type inferences
//  session_data_p->formats.push_back (configuration_in.configuration->format);

  // sanity check(s)
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_DirectShow_Tools::setFormat (MEDIASUBTYPE_RGB32,
                                                     media_type_s);
  Common_Image_Resolution_t resolution_s;
  resolution_s.cx = 640;
  resolution_s.cy = 480;
  Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_s,
                                                         media_type_s);
#else
  media_type_s.format = AV_PIX_FMT_RGB32;
  media_type_s.resolution.width  = 640;
  media_type_s.resolution.height = 480;
#endif // ACE_WIN32 || ACE_WIN64
  session_data_p->formats.push_back (media_type_s);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
