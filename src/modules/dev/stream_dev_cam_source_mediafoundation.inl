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

#include "d3d9.h"
#include "d3d9types.h"
#include "dxva2api.h"
#include "Mferror.h"
#include "Shlwapi.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"

#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::Stream_Dev_Cam_Source_MediaFoundation_T (ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , baseTimeStamp_ (0)
 , hasFinished_ (false)
 , isFirst_ (true)
 , symbolicLink_ (NULL)
 , symbolicLinkSize_ (0)
 , presentationClock_ (NULL)
 , referenceCount_ (0)
 , sampleGrabberSinkNodeId_ (0)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
 , mediaSession_ (NULL)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::Stream_Dev_Cam_Source_MediaFoundation_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::~Stream_Dev_Cam_Source_MediaFoundation_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::~Stream_Dev_Cam_Source_MediaFoundation_T"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  HRESULT result = E_FAIL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  if (unlikely (symbolicLinkSize_))
    CoTaskMemFree (symbolicLink_);

  if (unlikely (presentationClock_))
    presentationClock_->Release ();

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (unlikely (mediaSession_))
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result) &&
        (result != MF_E_SHUTDOWN)) // already shut down...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::initialize (const ConfigurationType& configuration_in,
                                                                Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::initialize"));

  bool result = false;
  HRESULT result_2 = E_FAIL;

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (likely (first_run))
  {
    first_run = false;
    COM_initialized = Common_Tools::initializeCOM ();
  } // end IF

  if (unlikely (inherited::isInitialized_))
  {
    baseTimeStamp_ = 0;

    hasFinished_ = false;
    isFirst_ = true;

    if (symbolicLinkSize_)
    {
      CoTaskMemFree (symbolicLink_); symbolicLink_ = NULL;
      symbolicLinkSize_ = 0;
    } // end IF

    if (presentationClock_)
    {
      presentationClock_->Release (); presentationClock_ = NULL;
    } // end IF
    referenceCount_ = 0;
    sampleGrabberSinkNodeId_ = 0;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (mediaSession_)
    {
      result_2 = mediaSession_->Shutdown ();
      if (FAILED (result_2))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      mediaSession_->Release (); mediaSession_ = NULL;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (unlikely (!result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

error:
  if (COM_initialized) Common_Tools::finalizeCOM ();

  return result;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
void
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::handleSessionMessage"));

  int result = -1;
  HRESULT result_2 = E_FAIL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      bool COM_initialized = Common_Tools::initializeCOM ();
      bool release_device = false;
      ULONG reference_count = 0;

      if (inherited::configuration_->statisticCollectionInterval !=
          ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_),                                          // event handler handle
                                            NULL,                                                                     // asynchronous completion token
                                            COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                            inherited::configuration_->statisticCollectionInterval);                  // interval
        if (unlikely (inherited::timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (id: %d) for interval %#T\n"),
//                    inherited::timerId_,
//                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
      // sanity check(s)
      ACE_ASSERT (!mediaSession_);
      ACE_ASSERT (!session_data_r.session);

      if (unlikely (inherited::configuration_->session))
      {
        reference_count = inherited::configuration_->session->AddRef ();
        mediaSession_ = inherited::configuration_->session;
        reference_count = mediaSession_->AddRef ();
        session_data_r.session = mediaSession_;
      } // end IF
      else
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
      {
        // sanity check(s)
        ACE_ASSERT (!session_data_r.formats.empty ());
        ACE_ASSERT (!session_data_r.rendererNodeId);

        IMFTopology* topology_p = NULL;
        IMFMediaType* media_type_p = NULL;
        MediaType media_type_2;

        inherited2::getMediaType (session_data_r.formats.back (),
                                  STREAM_MEDIATYPE_VIDEO,
                                  media_type_p);
        ACE_ASSERT (media_type_p);

        if (unlikely (!Stream_Module_Decoder_Tools::loadVideoRendererTopology (inherited::configuration_->deviceIdentifier,
                                                                               media_type_p,
                                                                               this,
                                                                               //inherited::configuration_->window,
                                                                               NULL,
                                                                               sampleGrabberSinkNodeId_,
                                                                               session_data_r.rendererNodeId,
                                                                               topology_p)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Decoder_Tools::loadVideoRendererTopology(), aborting\n")));
          media_type_p->Release (); media_type_p = NULL;
          goto error;
        } // end IF
        ACE_ASSERT (topology_p);

        if (unlikely (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                              media_type_p)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n")));
          media_type_p->Release (); media_type_p = NULL;
          topology_p->Release (); topology_p = NULL;
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("capture format: \"%s\"\n"),
                    ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_p).c_str ())));
        media_type_p->Release (); media_type_p = NULL;

        if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                           sampleGrabberSinkNodeId_,
                                                                           media_type_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(), aborting\n"),
                      ACE_TEXT (stream_name_string_)));
          topology_p->Release (); topology_p = NULL;
          goto error;
        } // end IF
        ACE_ASSERT (media_type_p);

        ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));
        inherited2::set (media_type_p,
                         STREAM_MEDIATYPE_VIDEO,
                         media_type_2);
        session_data_r.formats.push_back (media_type_2);
        media_type_p = NULL;

        IMFAttributes* attributes_p = NULL;
        result_2 = MFCreateAttributes (&attributes_p, 4);
        if (unlikely (FAILED (result_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          topology_p->Release (); topology_p = NULL;
          goto error;
        } // end IF
        result_2 = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
        ACE_ASSERT (SUCCEEDED (result_2));
        result_2 = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
        ACE_ASSERT (SUCCEEDED (result_2));
        //result_2 = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
        //ACE_ASSERT (SUCCEEDED (result_2));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
        result_2 = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
        ACE_ASSERT (SUCCEEDED (result_2));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
        result_2 = MFCreateMediaSession (attributes_p,
                                         &mediaSession_);
        if (unlikely (FAILED (result_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          attributes_p->Release (); attributes_p = NULL;
          topology_p->Release (); topology_p = NULL;
          goto error;
        } // end IF
        ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
        attributes_p->Release (); attributes_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
        DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);// |
                                //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                                //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
        result_2 = mediaSession_->SetTopology (topology_flags,
                                               topology_p);
        if (unlikely (FAILED (result_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          topology_p->Release (); topology_p = NULL;
          goto error;
        } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
        topology_p->Release (); topology_p = NULL;
      } // end ELSE
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT (mediaSession_);
      reference_count = mediaSession_->AddRef ();
      session_data_r.session = mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      if (COM_initialized) Common_Tools::finalizeCOM ();

      break;

error:
      //if (direct3D_manager_p)
      //  direct3D_manager_p->Release ();
      //if (presentation_descriptor_p)
      //  presentation_descriptor_p->Release ();
      //if (source_node_p)
      //  source_node_p->Release ();

      //if (release_device)
      //{
      //  session_data_r.direct3DDevice->Release (); session_data_r.direct3DDevice = NULL;
      //  session_data_r.direct3DManagerResetToken = 0;
      //} // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (session_data_r.session)
      {
        result = session_data_r.session->Shutdown ();
        if (FAILED (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        session_data_r.session->Release (); session_data_r.session = NULL;
      } // end IF
      if (mediaSession_)
      {
        result = mediaSession_->Shutdown ();
        if (FAILED (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        mediaSession_->Release (); mediaSession_ = NULL;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      if (symbolicLinkSize_)
      {
        CoTaskMemFree (symbolicLink_); symbolicLink_ = NULL;
        symbolicLinkSize_ = 0;
      } // end IF

      if (COM_initialized) Common_Tools::finalizeCOM ();

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (unlikely (inherited::sessionEndProcessed_))
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (inherited::timerId_ != -1)
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      bool COM_initialized = Common_Tools::initializeCOM ();
      bool close_session = true;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (unlikely (!mediaSession_))
        goto continue_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      //IMFMediaSource* media_source_p = NULL;
      //if (!Stream_Device_Tools::getMediaSource (mediaSession_,
      //                                                 media_source_p))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Device_Tools::getMediaSource(), continuing\n")));
      //  goto continue_;
      //} // end IF
      //result_2 = media_source_p->Stop ();
      //if (FAILED (result_2))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSource::Stop(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      //media_source_p->Release ();
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
continue_:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      if (likely (presentationClock_))
      {
        presentationClock_->Release (); presentationClock_ = NULL;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (likely (session_data_r.session))
      {
        result_2 = session_data_r.session->Close ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        // *NOTE*: IMFMediaSession::Close() is asynchronous
        //         --> (try to) wait for the next MESessionClosed event
        // *TODO*: this will not work, as the stream is already dispatching
        //         session events (MF_E_MULTIPLE_SUBSCRIBERS)
        //IMFMediaEvent* media_event_p = NULL;
        //bool received_topology_set_event = false;
        //MediaEventType event_type = MEUnknown;
        //do
        //{
        //  media_event_p = NULL;
        //  result_2 = session_data_r.session->GetEvent (0,
        //                                               &media_event_p);
        //  if (FAILED (result_2)) // MF_E_MULTIPLE_SUBSCRIBERS: 0xC00D36DAL
        //  {
        //    ACE_DEBUG ((LM_ERROR,
        //                ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
        //                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        //    break;
        //  } // end IF
        //  ACE_ASSERT (media_event_p);
        //  result_2 = media_event_p->GetType (&event_type);
        //  ACE_ASSERT (SUCCEEDED (result_2));
        //  if (event_type == MESessionClosed)
        //    received_topology_set_event = true;
        //  media_event_p->Release ();
        //} while (!received_topology_set_event);
        close_session = false;
        session_data_r.session->Release (); session_data_r.session = NULL;
      } // end IF
      if (likely (mediaSession_))
      {
        if (close_session)
        {
          result_2 = mediaSession_->Close ();
          if (FAILED (result_2))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                        ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        } // end IF
        mediaSession_->Release (); mediaSession_ = NULL;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      if (COM_initialized) Common_Tools::finalizeCOM ();

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      { Common_ITask* itask_p = this; // *TODO*: is the no other way ?
        itask_p->stop (false,  // wait for completion ?
                       false); // high priority ?
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  // step1: collect data

  //// step2: send the information downstream
  //if (!inherited::putStatisticMessage (data_out)) // data container
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n")));
  //  return false;
  //} // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::QueryInterface (const IID& IID_in,
                                                                    void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    //QITABENT (OWN_TYPE_T, IMFSourceReaderCallback),
    QITABENT (OWN_TYPE_T, IMFSampleGrabberSinkCallback),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnClockStart (MFTIME systemClockTime_in,
                                                                  LONGLONG clockStartOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockStart"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (clockStartOffset_in);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockStop"));

  ACE_UNUSED_ARG (systemClockTime_in);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnClockPause (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockPause"));

  ACE_UNUSED_ARG (systemClockTime_in);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnClockRestart (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockRestart"));

  ACE_UNUSED_ARG (systemClockTime_in);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnClockSetRate (MFTIME systemClockTime_in,
                                                                    float playbackRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockSetRate"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (playbackRate_in);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnProcessSample (REFGUID majorMediaType_in,
                                                                     DWORD flags_in,
                                                                     LONGLONG timeStamp_in,
                                                                     LONGLONG duration_in,
                                                                     const BYTE* buffer_in,
                                                                     DWORD bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnProcessSample"));

  IMFAttributes* attributes_p = NULL;

  return OnProcessSampleEx (majorMediaType_in,
                            flags_in,
                            timeStamp_in,
                            duration_in,
                            buffer_in,
                            bufferSize_in,
                            attributes_p);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnProcessSampleEx (REFGUID majorMediaType_in,
                                                                       DWORD flags_in,
                                                                       LONGLONG timeStamp_in,
                                                                       LONGLONG duration_in,
                                                                       const BYTE* buffer_in,
                                                                       DWORD bufferSize_in,
                                                                       IMFAttributes* attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnProcessSampleEx"));

  ACE_UNUSED_ARG (majorMediaType_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (duration_in);
  ACE_UNUSED_ARG (attributes_in);

  DataMessageType* message_p = NULL;
  int result = -1;
  //HRESULT result_2 = E_FAIL;

  if (unlikely (isFirst_))
  {
    isFirst_ = false;
    baseTimeStamp_ = timeStamp_in;
  } // end IF
  timeStamp_in -= baseTimeStamp_;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inferences
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  // *TODO*: remove type inference
  message_p =
    inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::allocateMessage(%d): \"%m\", aborting\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    goto error;
  } // end IF
  ACE_ASSERT (message_p->capacity () >= bufferSize_in);

  // *TODO*: forward the buffer itself to avoid the copy
  result = message_p->copy (reinterpret_cast<const char*> (buffer_in),
                            bufferSize_in);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  result = inherited::putq (message_p, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    goto error;
  } // end IF

  return S_OK;

error:
  if (message_p)
    message_p->release ();

  return E_FAIL;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnSetPresentationClock (IMFPresentationClock* presentationClock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnSetPresentationClock"));

  // sanity check(s)
  if (likely (presentationClock_))
  {
    presentationClock_->Release (); presentationClock_ = NULL;
  } // end IF

  if (likely (presentationClock_in))
  {
    ULONG reference_count = presentationClock_in->AddRef ();
    ACE_UNUSED_ARG (reference_count);
    presentationClock_ = presentationClock_in;
  } // end IF

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::OnShutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnShutdown"));

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        UserDataType,
                                        MediaType>::initialize_MediaFoundation (const std::string& deviceName_in,
                                                                                HWND windowHandle_in,
                                                                                IDirect3DDeviceManager9* IDirect3DDeviceManager_in,
                                                                                IMFMediaType* IMFMediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                                                IMFMediaSourceEx*& IMFMediaSource_inout,
#else
                                                                                IMFMediaSource*& IMFMediaSource_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                                                                WCHAR*& symbolicLink_out,
                                                                                UINT32& symbolicLinkSize_out,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                                                IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback_in,
#else
                                                                                IMFSampleGrabberSinkCallback* IMFSampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                                                IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::initialize_MediaFoundation"));

  bool release_media_source = false;

  // sanity check(s)
  ACE_ASSERT (!IMFTopology_out);

  if (likely (!IMFMediaSource_inout))
  {
    if (unlikely (!Stream_Device_Tools::getMediaSource (deviceName_in,
                                                        IMFMediaSource_inout,
                                                        symbolicLink_out,
                                                        symbolicLinkSize_out)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      return false;
    } // end IF
    release_media_source = true;
  } // end IF
  ACE_ASSERT (IMFMediaSource_inout);

  //if (!Stream_Device_Tools::getSourceReader (IMFMediaSource_inout,
  //                                                  symbolicLink_out,
  //                                                  symbolicLinkSize_out,
  //                                                  IDirect3DDeviceManager_in,
  //                                                  this,
  //                                                  Stream_Device_Tools::isChromaLuminance (IMFMediaType_in),
  //                                                  IMFSourceReaderEx_out))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Device_Tools::getSourceReader(), aborting\n")));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (IMFSourceReaderEx_out);
  if (unlikely (!Stream_Device_Tools::loadRendererTopology (deviceName_in,
                                                            IMFMediaType_in,
                                                            IMFSampleGrabberSinkCallback_in,
                                                            windowHandle_in,
                                                            IMFTopology_out)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::loadRendererTopology(), aborting\n")));
    goto error;
  } // end IF

  return true;

error:
  if (release_media_source)
  {
    IMFMediaSource_inout->Release (); IMFMediaSource_inout = NULL;
  } // end IF
  if (symbolicLinkSize_out)
  {
    // sanity check(s)
    ACE_ASSERT (symbolicLink_out);

    CoTaskMemFree (symbolicLink_out); symbolicLink_out = NULL;
    symbolicLinkSize_out = 0;
  } // end IF
  //if (IMFSourceReaderEx_out)
  //{
  //  IMFSourceReaderEx_out->Release (); IMFSourceReaderEx_out = NULL;
  //} // end IF

  return false;
}
