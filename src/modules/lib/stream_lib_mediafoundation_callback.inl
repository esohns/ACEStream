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

#include <mfidl.h>
#include <shlwapi.h>

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_common.h"
#include "stream_macros.h"

//#include "stream_dev_mediafoundation_tools.h"

template <typename ConfigurationType>
Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType>::Stream_MediaFramework_MediaFoundation_Callback_T ()
 : configuration_ (NULL)
 , controller_ (NULL)
 , mediaSession_ (NULL)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Callback_T::Stream_MediaFramework_MediaFoundation_Callback_T"));

}

template <typename ConfigurationType>
Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType>::~Stream_MediaFramework_MediaFoundation_Callback_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Callback_T::~Stream_MediaFramework_MediaFoundation_Callback_T"));

  if (mediaSession_)
    mediaSession_->Release ();
}

template <typename ConfigurationType>
bool
Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Callback_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  if (controller_)
    controller_ = NULL;

  if (mediaSession_)
  {
    mediaSession_->Release ();
    mediaSession_ = NULL;
  } // end IF

  controller_ = configuration_->controller;

  // *TODO*: remove type inferences
  // sanity check(s)
  ACE_ASSERT (configuration_->mediaSession);
  mediaSession_ = configuration_->mediaSession;
  mediaSession_->AddRef ();

  return true;
}

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType>::QueryInterface (REFIID IID_in,
                                                                                     void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Callback_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (OWN_TYPE_T, IMFAsyncCallback),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}

template <typename ConfigurationType>
ULONG
Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Callback_T::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0)
  //delete this;

  return count;
}

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType>::GetParameters (DWORD* flags_out,
                                                                                    DWORD* queue_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Callback_T::GetParameters"));

  // sanity check(s)
  ACE_ASSERT (flags_out);
  ACE_ASSERT (queue_out);

  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  return E_NOTIMPL;
}

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType>::Invoke (IMFAsyncResult* asyncResult_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Callback_T::Invoke"));

  // sanity check(s)
  ACE_ASSERT (asyncResult_in);

  HRESULT result = E_FAIL;
  IMFMediaEvent* media_event_p = NULL;
  MediaEventType event_type = MEUnknown;
  HRESULT status = E_FAIL;
  struct tagPROPVARIANT value;
  PropVariantInit (&value);

  // sanity check(s)
  ACE_ASSERT (mediaSession_);

  result = mediaSession_->EndGetEvent (asyncResult_in,
                                       &media_event_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = media_event_p->GetType (&event_type);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetStatus (&status);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetValue (&value);
  ACE_ASSERT (SUCCEEDED (result));
  switch (event_type)
  {
    case MEEndOfPresentation:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MEEndOfPresentation...\n")));
      break;
    }
    case MEError:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received MEError: \"%s\"\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionClosed:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionClosed, shutting down...\n")));
      //IMFMediaSource* media_source_p = NULL;
      //if (!Stream_Module_Device_Tools::getMediaSource (mediaSession_,
      //                                                 media_source_p))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), continuing\n")));
      //  goto continue_;
      //} // end IF
      //ACE_ASSERT (media_source_p);
      //result = media_source_p->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSource::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      //media_source_p->Release ();
  //continue_:
      // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
      //result = mediaSession_->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case MESessionEnded:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionEnded, closing sesion...\n")));
      result = mediaSession_->Close ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case MESessionCapabilitiesChanged:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionCapabilitiesChanged...\n")));
      break;
    }
    case MESessionNotifyPresentationTime:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionNotifyPresentationTime...\n")));
      break;
    }
    case MESessionStarted:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionStarted...\n")));
      break;
    }
    case MESessionStopped:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionStopped, stopping...\n")));

      // *TODO*: remove type inferences
      // sanity check(s)
      ACE_ASSERT (controller_);

      if (controller_->isRunning ())
        controller_->stop (false, // wait ?
                           false, // recurse upstream ?
                           true); // locked access ?
      break;
    }
    case MESessionTopologySet:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionTopologySet (status was: \"%s\")...\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionTopologyStatus:
    {
      UINT32 attribute_value = 0;
      result = media_event_p->GetUINT32 (MF_EVENT_TOPOLOGY_STATUS,
                                         &attribute_value);
      ACE_ASSERT (SUCCEEDED (result));
      MF_TOPOSTATUS topology_status =
        static_cast<MF_TOPOSTATUS> (attribute_value);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received MESessionTopologyStatus: \"%s\"...\n"),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (topology_status).c_str ())));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received unknown/invalid media session event (type was: %d), continuing\n"),
                  event_type));
      break;
    }
  } // end SWITCH
  PropVariantClear (&value);
  media_event_p->Release ();

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  return S_OK;

error:
  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}
