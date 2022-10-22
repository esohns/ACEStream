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

#if defined(ACE_WIN32) || defined(ACE_WIN32)
#undef MOUSE_MOVED
#include "curses.h"
#else
#include "ncurses.h"
// *NOTE*: the ncurses "timeout" macros conflict with
//         ACE_Synch_Options::timeout. Since not currently used, it's safe to
//         undefine
#undef timeout
#endif // ACE_WIN32 || ACE_WIN32
#include "panel.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_Curses_Window_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  MediaType>::Stream_Module_Vis_Curses_Window_T (ISTREAM_T* stream_in)
#else
                                  MediaType>::Stream_Module_Vis_Curses_Window_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , closeWindow_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Curses_Window_T::Stream_Module_Vis_Curses_Window_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_Curses_Window_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType,
                                  MediaType>::~Stream_Module_Vis_Curses_Window_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Curses_Window_T::~Stream_Module_Vis_Curses_Window_T"));

  if (closeWindow_)
  {
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Module_Vis_Curses_Window_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType,
                                  MediaType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Curses_Window_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (closeWindow_)
    {
      closeWindow_ = false;
    } // end IF
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_Curses_Window_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType,
                                  MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Curses_Window_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->window_2);

  static const char* char_density =
//    ACE_TEXT_ALWAYS_CHAR ("Ñ@#W$9876543210?!abc;:+=-,._                    ");
    ACE_TEXT_ALWAYS_CHAR ("@#O%+=|i-:.       ");
  static size_t char_density_length = ACE_OS::strlen (char_density);

  int result = ERR;
  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  float r_f, g_f, b_f, ratio_f;
  size_t index_i;

  result = wmove (inherited::configuration_->window_2,
                  0, 0);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));
  for (int y = 0;
       y < getmaxy (inherited::configuration_->window_2);
       ++y)
    for (int x = 0;
         x < getmaxx (inherited::configuration_->window_2);
         ++x)
    {
      r_f = *data_p / 255.0F;
      g_f = *(data_p + 1) / 255.0F;
      b_f = *(data_p + 2) / 255.0F;
      ratio_f = // some sort of 'luminance' between 0.0 and 1.0
        0.2987f * r_f + 0.5870f * g_f + 0.1140f * b_f;
      //ratio_f =
     //   static_cast<float> ((*data_p + *(data_p + 1) + *(data_p + 2))) / (3.0F * 255.0);
      // *NOTE*: this assumes the console is white-on-black, i.e. 'brighter' colors --> lower index
      index_i =
        char_density_length - static_cast<size_t> (static_cast<float> (char_density_length) * ratio_f);
      // clamp off-by-one
      index_i =
        (index_i == char_density_length ? char_density_length - 1 : index_i);

      result = waddch (inherited::configuration_->window_2,
                       char_density[index_i]);
      //if (unlikely (result == ERR))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to waddch(), continuing\n")));

      data_p += 3;
    } // end FOR

  result = wrefresh (inherited::configuration_->window_2);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wrefresh(), continuing\n")));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_Curses_Window_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType,
                                  MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Curses_Window_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
      const MediaType& media_type_r = session_data_r.formats.back ();
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited2::getMediaType (media_type_r,
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_2);

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (closeWindow_)
      {
        closeWindow_ = false;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_Curses_Window_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType,
                                  MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Curses_Window_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
