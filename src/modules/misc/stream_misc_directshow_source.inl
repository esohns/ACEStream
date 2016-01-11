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

//#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::Stream_Misc_DirectShow_Source_T ()
 : inherited ()
 , IMemAllocator_ (NULL)
 , IMemInputPin_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::Stream_Misc_DirectShow_Source_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::~Stream_Misc_DirectShow_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::~Stream_Misc_DirectShow_Source_T"));

  if (IMemAllocator_)
    IMemAllocator_->Release ();
  if (IMemInputPin_)
    IMemInputPin_->Release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
bool
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::initialize"));

  // clean up ?
  if (IMemAllocator_)
  {
    IMemAllocator_->Release ();
    IMemAllocator_ = NULL;
  } // end IF
  if (IMemInputPin_)
  {
    IMemInputPin_->Release ();
    IMemInputPin_ = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.builder);

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    configuration_in.builder->FindFilterByName (configuration_in.sourceFilter,
                                                &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IEnumPins* enumerator_p = NULL;
  result = filter_p->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  while (enumerator_p->Next (1, &pin_p, NULL) == S_OK)
  {
    ACE_ASSERT (pin_p);

    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF
    if (pin_direction != PINDIR_INPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF

    break;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no input pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter)));
    return false;
  } // end IF

  result = pin_p->QueryInterface (IID_IMemInputPin,
                                  (void**)&IMemInputPin_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IMemInputPin): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (IMemInputPin_);
  pin_p->Release ();

  result = IMemInputPin_->GetAllocator (&IMemAllocator_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemInputPin::GetAllocator(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    IMemInputPin_->Release ();
    IMemInputPin_ = NULL;

    return false;
  } // end IF
  ACE_ASSERT (IMemAllocator_);

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
const ConfigurationType&
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
void
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  //ACE_ASSERT (IMemAllocator_);
  ACE_ASSERT (IMemInputPin_);

  IMediaSample* media_sample_p = NULL;
  media_sample_p = message_inout;
  media_sample_p->AddRef ();
  HRESULT result = E_FAIL;
  //BYTE* buffer_p = NULL;
  //long remaining = message_inout->total_length ();
  //long to_copy = -1;
  //do
  //{
  //  result = IMemAllocator_->GetBuffer (&media_sample_p,
  //                                      NULL,
  //                                      NULL,
  //                                      0);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMemAllocator::GetBuffer(): \"%s\", returning\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    return;
  //  } // end IF
  //  ACE_ASSERT (media_sample_p);

  //  result = media_sample_p->GetPointer (&buffer_p);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMediaSample::GetPointer(): \"%s\", returning\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //    // clean up
  //    media_sample_p->Release ();

  //    return;
  //  } // end IF
  //  ACE_ASSERT (buffer_p);

  //  to_copy =
  //    ((media_sample_p->GetSize () >= remaining) ? remaining
  //                                               : media_sample_p->GetSize ());
  //  ACE_OS::memcpy (buffer_p, message_inout->rd_ptr (), to_copy);
  //  result = media_sample_p->SetActualDataLength (to_copy);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMediaSample::SetActualDataLength(%d): \"%s\", returning\n"),
  //                to_copy,
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //    // clean up
  //    media_sample_p->Release ();

  //    return;
  //  } // end IF
  //  message_inout->rd_ptr (to_copy);

    //// *TODO*: use ReceiveMultiple ()
    result = IMemInputPin_->Receive (media_sample_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMemInputPin::Receive(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_sample_p->Release ();

      return;
    } // end IF
    media_sample_p->Release ();

  //  remaining -= to_copy;
  //} while (remaining);
}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType>
//void
//Stream_Misc_DirectShow_Source_T<SessionMessageType,
//                                MessageType,
//                                ConfigurationType,
//                                SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
//                                                                        bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::handleSessionMessage"));
//
//  int result = -1;
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//
//  switch (message_inout->type ())
//  {
//    case STREAM_SESSION_BEGIN:
//    {
//      break;
//    }
//    case STREAM_SESSION_END:
//    {
//      break;
//    }
//    default:
//      break;
//  } // end SWITCH
//}
