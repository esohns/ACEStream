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

#include <limits>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif /* __cplusplus */
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
#include "wand/wand_api.h"
#else
#if defined (ACE_LINUX)
#if defined (IS_UBUNTU_LINUX) // *NOTE*: github "*-latest" runners lag behind:
#include "wand/MagickWand.h" //          - Ubuntu 'noble' still is on ImageMagick-6
#else
#include "MagickWand/MagickWand.h"
#endif // IS_UBUNTU_LINUX
#else
#include "MagickWand/MagickWand.h"
#endif // ACE_LINUX
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK

#include "ace/Log_Msg.h"

#if defined (_DEBUG)
#include "common_file_tools.h"
#endif // _DEBUG

#include "common_image_tools.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Test_U_ImageMagick_Target_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::Test_U_ImageMagick_Target_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , context_ (NULL)
 , counter_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_ImageMagick_Target_T::Test_U_ImageMagick_Target_T"));

//  InitializeMagick (NULL);
  //MagickWandGenesis ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Test_U_ImageMagick_Target_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::~Test_U_ImageMagick_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_ImageMagick_Target_T::~Test_U_ImageMagick_Target_T"));

  if (context_)
    DestroyMagickWand (context_);

  //MagickWandTerminus ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Test_U_ImageMagick_Target_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::initialize (const ConfigurationType& configuration_in,
                                                    Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_ImageMagick_Target_T::initialize"));

//  int result = -1;

  if (inherited::isInitialized_)
  {
    if (context_)
    {
      DestroyMagickWand (context_); context_ = NULL;
    } // end IF
    counter_ = 0;
  } // end IF

  context_ = NewMagickWand ();
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

//  MagickSetImageType (context_, TrueColorType);
//  MagickSetImageColorspace (context_, sRGBColorspace);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_U_ImageMagick_Target_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_ImageMagick_Target_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

//  DataMessageType* message_p = NULL;
//  ACE_Message_Block* message_block_p = NULL;
  const typename DataMessageType::DATA_T& message_data_r =
      message_inout->getR ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  inherited2::getMediaType (message_data_r.format,
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_s);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
  ACE_ASSERT (media_type_s.codecId == AV_CODEC_ID_NONE);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
  unsigned int result = 0;
#else
  MagickBooleanType result = MagickFalse;
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
  result = MagickSetFormat (context_, ACE_TEXT_ALWAYS_CHAR ("RGBA"));
  ACE_ASSERT (result == MagickTrue);

  result = MagickSetSize (context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                          resolution_s.cx, resolution_s.cy);
#else
                          media_type_s.resolution.width, media_type_s.resolution.height);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result == MagickTrue);
  result = MagickSetDepth (context_, 8);
  ACE_ASSERT (result == MagickTrue);

  result =
    MagickReadImageBlob (context_,
                         reinterpret_cast<unsigned char*> (message_inout->rd_ptr ()),
                         message_inout->length ());
  if (unlikely (result != MagickTrue))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MagickReadImageBlob(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (context_).c_str ())));
    goto error;
  } // end IF
  message_inout->release (); message_inout = NULL;

  result = MagickSetImageDelay (context_, 0); // *NOTE*: "ticks-per-second units"
  ACE_ASSERT (result == MagickTrue);

  ++counter_;

  return;

error:
  message_inout->release (); message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Test_U_ImageMagick_Target_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_ImageMagick_Target_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
      break;
    case STREAM_SESSION_MESSAGE_END:
    {
      if (unlikely (counter_ <= 0))
        break;

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      //MagickBooleanType result = MagickSetImageDepth (context_, 8);
      //ACE_ASSERT (result == MagickTrue);

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
      unsigned int result = 0;
#else
      MagickBooleanType result = MagickFalse;
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
      result = MagickSetImageFormat (context_, ACE_TEXT_ALWAYS_CHAR ("GIF"));
      ACE_ASSERT (result == MagickTrue);
      result = MagickSetImageIterations (context_, 0); // loop forever
      ACE_ASSERT (result == MagickTrue);

      result =
        MagickWriteImages (context_,
                           inherited::configuration_->fileIdentifier.identifier.c_str (),
                           MagickTrue);
      ACE_ASSERT (result == MagickTrue);

      break;
    }
    default:
      break;
  } // end SWITCH
}
