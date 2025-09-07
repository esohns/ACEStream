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

#include "test_u_stream.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "amvideo.h"
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_file_defines.h"
#include "stream_vis_defines.h"

#include "test_u_animated_gif_defines.h"

Test_U_Stream::Test_U_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::Test_U_Stream"));

}

Test_U_Stream::~Test_U_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::~Test_U_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_U_Stream::load (Stream_ILayout* layout_inout,
                              bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  Stream_Module_t* module_p = NULL;
#if defined (IMAGEMAGICK_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_U_ImageMagick_Source_Module (this,
                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_IMAGEMAGICK_SOURCE_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_U_FileReader_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING)),
                  false);
#endif // IMAGEMAGICK_SUPPORT
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_StatisticReport_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
#if defined (IMAGEMAGICK_SUPPORT)
  ACE_NEW_RETURN (module_p,
                  Test_U_ImageMagick_Resize_Module (this,
                                                    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_IMAGEMAGICK_RESIZE_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Test_U_ImageMagick_Target_Module (this,
                                                    ACE_TEXT_ALWAYS_CHAR (TEST_U_ANIMATED_GIF_DEFAULT_WRITER_MODULE_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
#endif // IMAGEMAGICK_SUPPORT

  delete_out = true;

  return true;
}

bool
Test_U_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Test_U_AnimatedGIF_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator;
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
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  session_data_p =
    &const_cast<Test_U_AnimatedGIF_SessionData&> (session_manager_p->getR ());
  ACE_ASSERT (session_data_p->formats.empty ());
  iterator = inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  // *TODO*: remove type inferences
  session_data_p->fileIdentifier = (*iterator).second.second->fileIdentifier;
  session_data_p->formats.push_back (media_type_s);

  // ---------------------------------------------------------------------------

  if (inherited::configuration_->configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
