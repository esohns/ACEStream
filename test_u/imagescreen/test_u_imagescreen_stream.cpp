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

//#include "ace/Synch.h"
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
 , ffmpeg_source_ (this,
                   ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SOURCE_DEFAULT_NAME_STRING))
#if defined (FFMPEG_SUPPORT)
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
#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
// , GTKCairoDisplay_ (this,
//                     ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING))
 , display_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECT3D_DEFAULT_NAME_STRING))
#else
 , display_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING))
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT
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
  layout_in->append (&ffmpeg_source_, NULL, 0);
  //layout_in->append (&ffmpeg_decode_, NULL, 0); // output is uncompressed RGBA
  layout_in->append (&ffmpeg_resize_, NULL, 0); // output is window size/fullscreen
  layout_in->append (&ffmpeg_convert_, NULL, 0); // output is uncompressed BGRA
#endif // IMAGEMAGICK_SUPPORT
  layout_in->append (&delay_, NULL, 0);
  layout_in->append (&display_, NULL, 0);

  return true;
}

bool
Stream_ImageScreen_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ImageScreen_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration->setupPipeline;
  bool reset_setup_pipeline = false;
  Stream_ImageScreen_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
  struct Stream_ImageScreen_ModuleHandlerConfiguration* configuration_p = NULL;
#if defined (IMAGEMAGICK_SUPPORT)
  Stream_ImageScreen_ImageMagick_Source* source_impl_p = NULL;
#else
  Stream_ImageScreen_FFMPEG_Source* source_impl_p = NULL;
#endif // IMAGEMAGICK_SUPPORT
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<Stream_ImageScreen_SessionData&> (inherited::sessionData_->getR ());
  ACE_ASSERT (session_data_p->formats.empty ());
  // *TODO*: remove type inferences
//  session_data_p->formats.push_back (configuration_in.configuration->format);

  // sanity check(s)
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  configuration_p =
      dynamic_cast<struct Stream_ImageScreen_ModuleHandlerConfiguration*> (&(*iterator).second.second);
  ACE_ASSERT (configuration_p);

//  if (!Stream_Device_Tools::getFormat (configuration_in.moduleHandlerConfiguration->fileDescriptor,
//                                       session_data_r.v4l2Format))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Device_Tools::getFormat(%d), aborting\n"),
//                configuration_in.moduleHandlerConfiguration->fileDescriptor));
//    return false;
//  } // end IF
//  if (!Stream_Device_Tools::getFrameRate (configuration_in.moduleHandlerConfiguration->fileDescriptor,
//                                          session_data_r.v4l2FrameRate))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Device_Tools::getFrameRate(%d), aborting\n"),
//                configuration_in.moduleHandlerConfiguration->fileDescriptor));
//    return false;
//  } // end IF
  media_type_s.format = AV_PIX_FMT_RGB32;
  session_data_p->formats.push_front (media_type_s);
  //  session_data_p->targetFileName = configuration_p->fileIdentifier.identifier;

  // ---------------------------------------------------------------------------

  // ******************* Camera Source ************************
  source_impl_p =
#if defined (IMAGEMAGICK_SUPPORT)
    dynamic_cast<Stream_ImageScreen_ImageMagick_Source*> (imagemagick_source_.writer ());
#else
    dynamic_cast<Stream_ImageScreen_FFMPEG_Source*> (ffmpeg_source_.writer ());
#endif // IMAGEMAGICK_SUPPORT
  ACE_ASSERT (source_impl_p);
  source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
#if defined (IMAGEMAGICK_SUPPORT)
  imagemagick_source_.arg (inherited::sessionData_);
#else
  ffmpeg_source_.arg (inherited::sessionData_);
#endif // IMAGEMAGICK_SUPPORT

  if (configuration_in.configuration->setupPipeline)
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
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration->setupPipeline =
      setup_pipeline;

  return false;
}
