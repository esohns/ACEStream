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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename TaskType,
          typename MediaType>
Test_I_CameraAR_Module_PGE_T<TaskType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             MediaType>::Test_I_CameraAR_Module_PGE_T (typename TaskType::ISTREAM_T* stream_in)
#else
                             MediaType>::Test_I_CameraAR_Module_PGE_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::Test_I_CameraAR_Module_PGE_T"));

}

template <typename TaskType,
          typename MediaType>
void
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::handleDataMessage (typename inherited::DATA_MESSAGE_T*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::handleDataMessage"));
}

template <typename TaskType,
          typename MediaType>
void
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::handleSessionMessage (typename inherited::SESSION_MESSAGE_T*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  const typename inherited::SESSION_MESSAGE_T::DATA_T& session_data_container_r =
    message_inout->getR ();
  typename inherited::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
    const_cast<typename inherited::SESSION_MESSAGE_T::DATA_T::DATA_T&> (session_data_container_r.getR ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      Common_Image_Resolution_t resolution_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
#else
      resolution_s = media_type_s.resolution;
#endif // ACE_WIN32 || ACE_WIN64

      olc::rcode result =
        inherited3::Construct (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               resolution_s.cx, resolution_s.cy,
#else
                               resolution_s.width, resolution_s.height,
#endif // ACE_WIN32 || ACE_WIN64
                               1, 1,
                               false,  // fullscreen ?
                               false,  // vsync ?
                               false); // cohesion ?
      if (unlikely (result == olc::FAIL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to olc::PixelGameEngine::Construct(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result = inherited3::Start ();

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::OnUserCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::OnUserCreate"));

  return true;
}

template <typename TaskType,
          typename MediaType>
bool
Test_I_CameraAR_Module_PGE_T<TaskType,
                             MediaType>::OnUserUpdate (float fElapsedTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_CameraAR_Module_PGE_T::OnUserUpdate"));

  return true;
}
