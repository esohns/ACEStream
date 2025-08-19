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
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "test_u_common_modules.h"

Test_U_Stream::Test_U_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::Test_U_Stream"));

}

bool
Test_U_Stream::load (Stream_ILayout* layout_inout,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::load"));

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (*inherited::configuration_).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_U_DirectShow_Source_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_U_V4L_Source_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING)),
                  false);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //ACE_NEW_RETURN (module_p,
  //                Test_U_DirectShow_LibAVConverter_Module (this,
  //                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_inout->append (module_p, NULL, 0);

  ACE_NEW_RETURN (module_p,
                  Test_U_DirectShow_RGB24Flip_Module (this,
                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_RGB24_HFLIP_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
#else
#if defined (FFMPEG_SUPPORT)
  if ((*iterator).second.second->codecConfiguration->codecId != AV_CODEC_ID_NONE)
    ACE_NEW_RETURN (module_p,
                    Test_U_LibAVDecoder_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING)),
                    false);
  else
    ACE_NEW_RETURN (module_p,
                    Test_U_LibAVConverter_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)),
                    false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  module_p = NULL;
#if defined (OPENCV_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_U_DirectShow_QRDecoder_Module (this,
                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_OPENCV_QR_DECODER_DEFAULT_NAME_STRING)),
                  false);

#else
  ACE_NEW_RETURN (module_p,
                  Test_U_QRDecoder_Module (this,
                                           ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_OPENCV_QR_DECODER_DEFAULT_NAME_STRING)),
                  false);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
#endif // OPENCV_SUPPORT

  delete_out = true;

  return true;
}

bool
Test_U_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::initialize"));

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());

  // ---------------------------------------------------------------------------
  // step1: allocate a new session state, reset stream
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
//  bool reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
//  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  // ACE_ASSERT ((*iterator).second.second->direct3DConfiguration);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  QRDecode_DirectShow_SessionData* session_data_p =
    &const_cast<QRDecode_DirectShow_SessionData&> (inherited::sessionData_->getR ());
#else
  QRDecode_SessionData* session_data_p =
    &const_cast<QRDecode_SessionData&> (inherited::sessionData_->getR ());
#endif // ACE_WIN32 || ACE_WIN64

  // ---------------------------------------------------------------------------
  // step5: update session data
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  Stream_MediaFramework_DirectShow_Tools::copy (configuration_in.configuration_->format, media_type_s);
#else
  struct Stream_MediaFramework_V4L_MediaType media_type_s =
    configuration_in.configuration_->format;
#endif // ACE_WIN32 || ACE_WIN64
  session_data_p->formats.push_back (media_type_s);

  // step7: assemble stream
  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      return false;
    } // end IF

  return true;
}
