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

#include <mferror.h>
#include <Shlwapi.h>

#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

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
          typename StatisticContainerType>
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::Stream_Dev_Mic_Source_MediaFoundation_T (ISTREAM_T* stream_in,
                                                                                                          bool autoStart_in,
                                                                                                          enum Stream_HeadModuleConcurrency concurrency_in)
 : inherited (stream_in,
              autoStart_in,
              concurrency_in,
              true)
 , baseTimeStamp_ (0)
 , hasFinished_ (false)
 , isFirst_ (true)
 , symbolicLink_ (NULL)
 , symbolicLinkSize_ (0)
 , presentationClock_ (NULL)
 , referenceCount_ (0)
 , sampleGrabberSinkNodeId_ (0)
 , mediaSession_ (NULL)
 , releaseSessionSession_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::Stream_Dev_Mic_Source_MediaFoundation_T"));

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
          typename StatisticContainerType>
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::~Stream_Dev_Mic_Source_MediaFoundation_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::~Stream_Dev_Mic_Source_MediaFoundation_T"));

  HRESULT result = E_FAIL;

  if (symbolicLinkSize_)
    CoTaskMemFree (symbolicLink_);

  if (presentationClock_)
    presentationClock_->Release ();

  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result) &&
        (result != MF_E_SHUTDOWN)) // --> already shut down
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
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
          typename StatisticContainerType>
bool
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::initialize"));

  bool result = false;
  HRESULT result_2 = E_FAIL;

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (first_run)
  {
    first_run = false;

    result_2 = CoInitializeEx (NULL,
                               (COINIT_MULTITHREADED    |
                                COINIT_DISABLE_OLE1DDE  |
                                COINIT_SPEED_OVER_MEMORY));
    if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    COM_initialized = true;
  } // end IF

  if (inherited::isInitialized_)
  {
    baseTimeStamp_ = 0;

    hasFinished_ = false;
    isFirst_ = true;

    if (symbolicLinkSize_)
    {
      CoTaskMemFree (symbolicLink_);
      symbolicLink_ = NULL;
      symbolicLinkSize_ = 0;
    } // end IF

    if (presentationClock_)
    {
      presentationClock_->Release ();
      presentationClock_ = NULL;
    } // end IF
    referenceCount_ = 0;
    sampleGrabberSinkNodeId_ = 0;

    if (mediaSession_)
    {
      result_2 = mediaSession_->Shutdown ();
      if (FAILED (result_2))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      mediaSession_->Release ();
    } // end IF
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  goto continue_;

error:
  if (COM_initialized)
    CoUninitialize ();

continue_:
  return result;
}
//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        SessionMessageType,
//                                        DataMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::start ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::start"));
//
//  inherited::start ();
//
//  //if (mediaSession_)
//  //{
//  //  HRESULT result = mediaSession_->BeginGetEvent (this, NULL);
//  //  if (FAILED (result))
//  //  {
//  //    ACE_DEBUG ((LM_ERROR,
//  //                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
//  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//  //    return;
//  //  } // end IF
//  //} // end IF
//}
//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        SessionMessageType,
//                                        DataMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::stop (bool waitForCompletion_in,
//                                                                       bool lockedAccess_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::stop"));
//
//  // *IMPORTANT NOTE*: the worker thread shuts down when it receives a
//  //                   'MESessionStopped' event (see below)
//  ACE_UNUSED_ARG (waitForCompletion_in);
//  ACE_UNUSED_ARG (lockedAccess_in);
//  //return inherited::stop (waitForCompletion_in,
//  //                        lockedAccess_in);
//}

//template <typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Mic_Source_MediaFoundation_T<SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (DataMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::handleDataMessage"));

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

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
          typename StatisticContainerType>
void
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      bool COM_initialized = false;
      //IMFTopologyNode* source_node_p = NULL;
      //IMFPresentationDescriptor* presentation_descriptor_p = NULL;
      HRESULT result_2 = E_FAIL;
      ULONG reference_count = 0;

      if (inherited::configuration_->statisticCollectionInterval !=
          ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerID_ == -1);
        ACE_Event_Handler* handler_p = &(inherited::statisticCollectionHandler_);
        inherited::timerID_ =
            timer_manager_p->schedule_timer (handler_p,                                                                // event handler
                                             NULL,                                                                     // argument
                                             COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                             inherited::configuration_->statisticCollectionInterval);                  // interval
        if (inherited::timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (ID: %d) for interval %#T...\n"),
//                    inherited::timerID_,
//                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!mediaSession_);

      releaseSessionSession_ = true;
      if (session_data_r.session)
      {
        reference_count = session_data_r.session->AddRef ();
        mediaSession_ = session_data_r.session;
        releaseSessionSession_ = false;
      } // end IF
      else if (inherited::configuration_->session)
      {
        reference_count = inherited::configuration_->session->AddRef ();
        mediaSession_ = inherited::configuration_->session;
        reference_count = mediaSession_->AddRef ();
        session_data_r.session = mediaSession_;
      } // end IF
      else
      {
        // sanity check(s)
        ACE_ASSERT (inherited::configuration_->format);

        IMFTopology* topology_p = NULL;
        if (!Stream_Module_Device_MediaFoundation_Tools::loadAudioRendererTopology (inherited::configuration_->device,
                                                                                    inherited::configuration_->format,
                                                                                    this,
                                                                                    (inherited::configuration_->mute ? -1
                                                                                                                      : inherited::configuration_->audioOutput),
                                                                                    sampleGrabberSinkNodeId_,
                                                                                    session_data_r.rendererNodeId,
                                                                                    topology_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::loadAudioRendererTopology(), aborting\n")));
          goto error;
        } // end IF
        ACE_ASSERT (topology_p);

        if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                           inherited::configuration_->format))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n")));

          // clean up
          topology_p->Release ();

          goto error;
        } // end IF
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("capture format: \"%s\"...\n"),
                    ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (inherited::configuration_->format).c_str ())));
#endif

        IMFAttributes* attributes_p = NULL;
        result_2 = MFCreateAttributes (&attributes_p, 4);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

          // clean up
          topology_p->Release ();

          goto error;
        } // end IF
        result_2 = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
        ACE_ASSERT (SUCCEEDED (result_2));
        result_2 = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
        ACE_ASSERT (SUCCEEDED (result_2));
        //result_2 = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
        //ACE_ASSERT (SUCCEEDED (result_2));
        result_2 = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
        ACE_ASSERT (SUCCEEDED (result_2));
        result_2 = MFCreateMediaSession (attributes_p,
                                         &mediaSession_);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

          // clean up
          attributes_p->Release ();
          topology_p->Release ();

          goto error;
        } // end IF
        attributes_p->Release ();

        DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);// |
                                //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                                //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
        result_2 = mediaSession_->SetTopology (topology_flags,
                                               topology_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

          // clean up
          topology_p->Release ();

          goto error;
        } // end IF
        reference_count = mediaSession_->AddRef ();
        session_data_r.session = mediaSession_;
      } // end ELSE
      ACE_ASSERT (mediaSession_);
      ACE_ASSERT (session_data_r.session);

      break;

error:
      //if (presentation_descriptor_p)
      //  presentation_descriptor_p->Release ();
      //if (source_node_p)
      //  source_node_p->Release ();

      bool shutdown_session = true;
      if (session_data_r.session &&
          releaseSessionSession_)
      {
        result = session_data_r.session->Shutdown ();
        if (FAILED (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        shutdown_session = false;
        session_data_r.session->Release ();
        session_data_r.session = NULL;
      } // end IF
      if (mediaSession_)
      {
        if (shutdown_session)
        {
          result = mediaSession_->Shutdown ();
          if (FAILED (result))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        } // end IF
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF

      if (symbolicLinkSize_)
      {
        CoTaskMemFree (symbolicLink_);
        symbolicLink_ = NULL;
        symbolicLinkSize_ = 0;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::timerID_,
                                                                      &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      inherited::timerID_));
        inherited::timerID_ = -1;
      } // end IF

      bool COM_initialized = false;
      HRESULT result_2 = CoInitializeEx (NULL,
                                         (COINIT_MULTITHREADED    |
                                          COINIT_DISABLE_OLE1DDE  |
                                          COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      COM_initialized = true;

      if (!mediaSession_)
        goto continue_;

      //IMFMediaSource* media_source_p = NULL;
      //if (!Stream_Module_Device_Tools::getMediaSource (mediaSession_,
      //                                                 media_source_p))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), continuing\n")));
      //  goto continue_;
      //} // end IF
      //result_2 = media_source_p->Stop ();
      //if (FAILED (result_2))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSource::Stop(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      //media_source_p->Release ();
continue_:
      bool shutdown_session = true;

      if (presentationClock_)
      {
        presentationClock_->Release ();
        presentationClock_ = NULL;
      } // end IF

      if (session_data_r.session &&
          releaseSessionSession_)
      {
        result_2 = session_data_r.session->Shutdown ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        shutdown_session = false;
        session_data_r.session->Release ();
        session_data_r.session = NULL;
      } // end IF
      if (mediaSession_)
      {
        if (shutdown_session)
        {
          result = mediaSession_->Shutdown ();
          if (FAILED (result))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        } // end IF
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
        this->TASK_BASE_T::stop (false,  // wait for completion ?
                                 false); // N/A

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
          typename StatisticContainerType>
bool
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::collect"));

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

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   DataMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::QueryInterface (const IID& IID_in,
                                                                                 void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (OWN_TYPE_T, IMFSampleGrabberSinkCallback2),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}
//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        SessionMessageType,
//                                        DataMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::OnEvent (DWORD streamIndex_in,
//                                                                          IMFMediaEvent* mediaEvent_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnEvent"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//  ACE_UNUSED_ARG (mediaEvent_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_OK);
//  ACE_NOTREACHED (return S_OK;)
//}
//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        SessionMessageType,
//                                        DataMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::OnFlush (DWORD streamIndex_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnFlush"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_OK);
//  ACE_NOTREACHED (return S_OK;)
//}
//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        SessionMessageType,
//                                        DataMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::OnReadSample (HRESULT result_in,
//                                                                               DWORD streamIndex_in,
//                                                                               DWORD streamFlags_in,
//                                                                               LONGLONG timeStamp_in,
//                                                                               IMFSample* sample_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnReadSample"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//  ACE_UNUSED_ARG (streamFlags_in);
//
//  bool read_next_sample = false;
//  DataMessageType* message_p = NULL;
//  int result = -1;
//  HRESULT result_2 = E_FAIL;
//  IMFMediaBuffer* media_buffer_p = NULL;
//
//  if (isFirst_)
//  {
//    isFirst_ = false;
//    baseTimeStamp_ = timeStamp_in;
//  } // end IF
//  timeStamp_in -= baseTimeStamp_;
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  // *TODO*: remove type inference
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//  if (!sample_in)
//  {
//    // *NOTE*: end of stream ?
//    if (streamFlags_in & MF_SOURCE_READERF_ENDOFSTREAM)
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("reached end of stream, returning\n")));
//      return S_OK;
//    } // end IF
//
//    if (streamFlags_in & MF_SOURCE_READERF_STREAMTICK)
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("gap in the stream at %q, continuing\n"),
//                  timeStamp_in));
//      read_next_sample = true;
//    } // end IF
//
//    goto continue_2;
//  } // end IF
//  if (FAILED (result_in))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::ReadSample(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_in).c_str ())));
//    return result_in;
//  } // end IF
//
//  try
//  {
//    message_p = dynamic_cast<DataMessageType*> (sample_in);
//  }
//  catch (...)
//  {
//    //ACE_DEBUG ((LM_ERROR,
//    //            ACE_TEXT ("failed to dynamic_cast<DataMessageType*>(0x%@), continuing\n"),
//    //            IMediaSample_in));
//    message_p = NULL;
//  }
//  if (message_p) goto continue_;
//
//  bool release_sample = false;
//
//  // *TODO*: remove type inference
//  message_p =
//    inherited::allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage(%d) failed: \"%m\", aborting\n"),
//                inherited::configuration_->streamConfiguration->bufferSize));
//    goto error;
//  } // end IF
//  ACE_ASSERT (message_p);
//
//  typename DataMessageType::DATA_T& data_r =
//    const_cast<typename DataMessageType::DATA_T&> (message_p->get ());
//  ACE_ASSERT (!data_r.sample);
//
//  ULONG reference_count = sample_in->AddRef ();
//  result_2 = sample_in->SetSampleTime (timeStamp_in);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSample::SetSampleTime(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//  data_r.sample = sample_in;
//  data_r.sampleTime = timeStamp_in;
//  release_sample = true;
//
//  //DWORD total_length = 0;
//  //result_2 = sample_in->GetTotalLength (&total_length);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFSample::GetTotalLength(): \"%m\", aborting\n"),
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  DWORD buffer_count = 0;
//  result_2 = sample_in->GetBufferCount (&buffer_count);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSample::GetBufferCount(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (buffer_count == 1);
//  // *TODO*: use IMFSample::ConvertToContiguousBuffer() ?
//  result_2 = sample_in->GetBufferByIndex (0,
//                                          &media_buffer_p);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSample::GetBufferByIndex(0): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (media_buffer_p);
//
//  BYTE* buffer_p = NULL;
//  DWORD maximum_length, current_length;
//  result_2 = media_buffer_p->Lock (&buffer_p,
//                                   &maximum_length,
//                                   &current_length);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaBuffer::Lock(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (buffer_p);
//
//  message_p->base (reinterpret_cast<char*> (buffer_p),
//                    current_length,
//                    ACE_Message_Block::DONT_DELETE);
//  message_p->wr_ptr (current_length);
//
//  media_buffer_p->Unlock ();
//  media_buffer_p->Release ();
//  media_buffer_p = NULL;
//
//continue_:
//  result = inherited::putq (message_p, NULL);
//  if (result == -1)
//  {
//    int error = ACE_OS::last_error ();
//    if (error != ESHUTDOWN)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
//                  inherited::name ()));
//    goto error;
//  } // end IF
//
//  read_next_sample = true;
//
//  goto continue_2;
//
//error:
//  if (message_p)
//    message_p->release ();
//  if (media_buffer_p)
//    media_buffer_p->Release ();
//  if (release_sample)
//    sample_in->Release ();
//
//  return E_FAIL;
//
//continue_2:
//  if (!read_next_sample)
//    goto continue_3;
//
//  // sanity check(s)
//  ACE_ASSERT (sourceReader_);
//
//  result_2 = sourceReader_->ReadSample (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                        0,
//                                        NULL,
//                                        NULL,
//                                        NULL,
//                                        NULL);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::ReadSample(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
//    return result_2;
//  } // end IF
//
//continue_3:
//  return S_OK;
//}
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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockStart (MFTIME systemClockTime_in,
                                                                               LONGLONG clockStartOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnClockStart"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (clockStartOffset_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnClockStop"));

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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockPause (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnClockPause"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockRestart (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnClockRestart"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockSetRate (MFTIME systemClockTime_in,
                                                                                 float playbackRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnClockSetRate"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (playbackRate_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnProcessSample (const struct _GUID& majorMediaType_in,
                                                                                  DWORD flags_in,
                                                                                  LONGLONG timeStamp_in,
                                                                                  LONGLONG duration_in,
                                                                                  const BYTE* buffer_in,
                                                                                  DWORD bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnProcessSample"));

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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnProcessSampleEx (const struct _GUID& majorMediaType_in,
                                                                                    DWORD flags_in,
                                                                                    LONGLONG timeStamp_in,
                                                                                    LONGLONG duration_in,
                                                                                    const BYTE* buffer_in,
                                                                                    DWORD bufferSize_in,
                                                                                    IMFAttributes* attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnProcessSampleEx"));

  ACE_UNUSED_ARG (majorMediaType_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (duration_in);
  ACE_UNUSED_ARG (attributes_in);

  DataMessageType* message_p = NULL;
  int result = -1;
  //HRESULT result_2 = E_FAIL;

  if (isFirst_)
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
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), aborting\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    goto error;
  } // end IF
  ACE_ASSERT (message_p);
  ACE_ASSERT (message_p->capacity () >= bufferSize_in);

  // *TODO*: apparently, there is no way to retrieve the media sample, so a
  //         memcpy is unavoidable...
  result = message_p->copy (reinterpret_cast<const char*> (buffer_in),
                            bufferSize_in);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  result = inherited::putq (message_p, NULL);
  if (result == -1)
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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnSetPresentationClock (IMFPresentationClock* presentationClock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnSetPresentationClock"));

  // sanity check(s)
  if (presentationClock_)
  {
    presentationClock_->Release ();
    presentationClock_ = NULL;
  } // end IF

  if (presentationClock_in)
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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnShutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::OnShutdown"));

  return S_OK;
}
//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        SessionMessageType,
//                                        DataMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::GetParameters (DWORD* flags_out,
//                                                                                DWORD* queue_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::GetParameters"));
//
//  ACE_UNUSED_ARG (flags_out);
//  ACE_UNUSED_ARG (queue_out);
//
//  // *NOTE*: "...If you want default values for both parameters, return
//  //         E_NOTIMPL. ..."
//  return E_NOTIMPL;
//}
//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        SessionMessageType,
//                                        DataMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::Invoke (IMFAsyncResult* result_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::Invoke"));
//
//  HRESULT result = E_FAIL;
//  IMFMediaEvent* media_event_p = NULL;
//  MediaEventType event_type = MEUnknown;
//  HRESULT status = E_FAIL;
//  struct tagPROPVARIANT value;
//  PropVariantInit (&value);
//
//  // sanity check(s)
//  ACE_ASSERT (result_in);
//  ACE_ASSERT (mediaSession_);
//
//  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//  result = media_event_p->GetType (&event_type);
//  ACE_ASSERT (SUCCEEDED (result));
//  result = media_event_p->GetStatus (&status);
//  ACE_ASSERT (SUCCEEDED (result));
//  result = media_event_p->GetValue (&value);
//  ACE_ASSERT (SUCCEEDED (result));
//  switch (event_type)
//  {
//    case MEEndOfPresentation:
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MEEndOfPresentation...\n")));
//      break;
//    }
//    case MEError:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("received MEError: \"%s\"\n"),
//                  ACE_TEXT (Common_Tools::error2String (status).c_str ())));
//      break;
//    }
//    case MESessionClosed:
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionClosed...\n")));
//      break;
//    }
//    case MESessionEnded:
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionEnded...\n")));
//      break;
//    }
//    case MESessionCapabilitiesChanged:
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionCapabilitiesChanged...\n")));
//      break;
//    }
//    case MESessionNotifyPresentationTime:
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionNotifyPresentationTime...\n")));
//      break;
//    }
//    case MESessionStarted:
//    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionStarted...\n")));
//      break;
//    }
//    case MESessionStopped:
//    { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionStopped, shutting down...\n")));
//      inherited::shutdown ();
//      break;
//    }
//    case MESessionTopologySet:
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionTopologySet...\n")));
//      break;
//    }
//    case MESessionTopologyStatus:
//    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MESessionTopologyStatus...\n")));
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("received unknown/invalid media session event (type was: %d), continuing\n"),
//                  event_type));
//      break;
//    }
//  } // end SWITCH
//  PropVariantClear (&value);
//  media_event_p->Release ();
//
//  result = mediaSession_->BeginGetEvent (this, NULL);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF
//
//  return S_OK;
//
//error:
//  if (media_event_p)
//    media_event_p->Release ();
//  PropVariantClear (&value);
//
//  return E_FAIL;
//}

//template <ACE_SYNCH_DECL,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//int
//Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
//                                        ControlMessageType,
//                                        DataMessageType,
//                                        SessionMessageType,
//                                        ConfigurationType,
//                                        StreamControlType,
//                                        StreamNotificationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::svc"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::mod_);
//  ACE_ASSERT (inherited::sessionData_);
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("\"%s\": worker thread (ID: %t) starting...\n"),
//              inherited::mod_->name ()));
//
//  int error = 0;
//  bool has_finished = false;
//  ACE_Message_Block* message_block_p = NULL;
//  ACE_Time_Value no_wait = COMMON_TIME_NOW;
//  bool release_lock = false;
//  int result = -1;
//  int result_2 = -1;
//  const SessionDataType& session_data_r = inherited::sessionData_->get ();
//  //ACE_Time_Value sleep_interval (0,
//  //                               STREAM_DEFAULT_MODULE_SOURCE_EVENT_POLL_INTERVAL * 1000);
//  bool stop_processing = false;
//  //  unsigned int queued, done = 0;
//
//  // step1: start processing data
//  do
//  {
//    message_block_p = NULL;
//    result_2 = inherited::getq (message_block_p,
//                                NULL);
//                                //&no_wait);
//    if (result_2 == -1)
//    {
//      error = ACE_OS::last_error ();
//      if (error != EWOULDBLOCK) // Win32: 10035
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
//
//        if (!has_finished)
//        {
//          has_finished = true;
//          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
//          //         --> continue
//          inherited::finished ();
//        } // end IF
//
//        break;
//      } // end IF
//
//      goto continue_;
//    } // end IF
//    ACE_ASSERT (message_block_p);
//
//    switch (message_block_p->msg_type ())
//    {
//      case ACE_Message_Block::MB_STOP:
//      {
//        // clean up
//        message_block_p->release ();
//        message_block_p = NULL;
//
//        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
//        //         not have been set at this stage
//
//        // signal the controller ?
//        if (!has_finished)
//        {
//          has_finished = true;
//          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
//          //         --> continue
//          inherited::finished ();
//
//          // *NOTE*: (if passive,) STREAM_SESSION_END has been processed
//          //         --> done
//          if (inherited::thr_count_ == 0) goto done; // finished processing
//
//          continue; // process STREAM_SESSION_END
//        } // end IF
//
//done:
//        result = 0;
//
//        goto done_2; // STREAM_SESSION_END has been processed
//      }
//      default:
//      {
//        // sanity check(s)
//        ACE_ASSERT (inherited::configuration_);
//        ACE_ASSERT (inherited::configuration_->ilock);
//
//        // grab lock if processing is 'non-concurrent'
//        if (!inherited::concurrent_)
//          release_lock = inherited::configuration_->ilock->lock (true);
//
//        inherited::handleMessage (message_block_p,
//                                  stop_processing);
//
//        if (release_lock) inherited::configuration_->ilock->unlock (false);
//
//        // finished ?
//        if (stop_processing)
//        {
//          // *IMPORTANT NOTE*: message_block_p has already been released() !
//
//          if (!has_finished)
//          {
//            has_finished = true;
//            // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
//            //         --> continue
//            inherited::finished ();
//          } // end IF
//
//          continue;
//        } // end IF
//
//        break;
//      }
//    } // end SWITCH
//
//continue_:
//    // session aborted ?
//    // sanity check(s)
//    // *TODO*: remove type inferences
//    ACE_ASSERT (session_data_r.lock);
//    {
//      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock, result);
//
//      if (session_data_r.aborted &&
//          !has_finished)
//      {
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("session aborted\n")));
//
//        has_finished = true;
//        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
//        //         --> continue
//        inherited::finished ();
//      } // end IF
//    } // end lock scope
//
//    //result_2 = ACE_OS::sleep (sleep_interval);
//    //if (result_2 == -1)
//    //  ACE_DEBUG ((LM_ERROR,
//    //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
//    //              &sleep_interval));
//  } while (true);
//  result = -1;
//
//done_2:
//  return result;
//}

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
          typename StatisticContainerType>
bool
Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        ConfigurationType,
                                        StreamControlType,
                                        StreamNotificationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::initialize_MediaFoundation (const std::string& deviceName_in,
                                                                                             int audioOutput_in,
                                                                                             const IMFMediaType* IMFMediaType_in,
                                                                                             IMFMediaSource*& IMFMediaSource_inout,
                                                                                             const IMFSampleGrabberSinkCallback* IMFSampleGrabberSinkCallback_in,
                                                                                             IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_MediaFoundation_T::initialize_MediaFoundation"));

  bool release_media_source = false;

  // sanity check(s)
  ACE_ASSERT (!IMFTopology_out);

  if (!IMFMediaSource_inout)
  {
    if (!Stream_Module_Device_Tools::getMediaSource (deviceName_in,
                                                     IMFMediaSource_inout,
                                                     symbolicLink_out,
                                                     symbolicLinkSize_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      return false;
    } // end IF
    release_media_source = true;
  } // end IF
  ACE_ASSERT (IMFMediaSource_inout);

  //if (!Stream_Module_Device_Tools::getSourceReader (IMFMediaSource_inout,
  //                                                  symbolicLink_out,
  //                                                  symbolicLinkSize_out,
  //                                                  IDirect3DDeviceManager_in,
  //                                                  this,
  //                                                  Stream_Module_Device_Tools::isChromaLuminance (IMFMediaType_in),
  //                                                  IMFSourceReaderEx_out))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getSourceReader(), aborting\n")));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (IMFSourceReaderEx_out);
  if (!Stream_Module_Device_Tools::loadRendererTopology (deviceName_in,
                                                         IMFMediaType_in,
                                                         IMFSampleGrabberSinkCallback_in,
                                                         windowHandle_in,
                                                         IMFTopology_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererTopology(), aborting\n")));
    goto error;
  } // end IF

  return true;

error:
  if (release_media_source)
  {
    IMFMediaSource_inout->Release ();
    IMFMediaSource_inout = NULL;
  } // end IF
  if (symbolicLinkSize_out)
  {
    // sanity check(s)
    ACE_ASSERT (symbolicLink_out);

    CoTaskMemFree (symbolicLink_out);
    symbolicLink_out = NULL;
    symbolicLinkSize_out = 0;
  } // end IF
  if (IMFSourceReaderEx_out)
  {
    IMFSourceReaderEx_out->Release ();
    IMFSourceReaderEx_out = NULL;
  } // end IF

  return false;
}
