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

#include "shlwapi.h"

#include "d3d9.h"
#include "d3d9types.h"
#include "dxva2api.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_defines.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::Stream_Dev_Cam_Source_MediaFoundation_T ()
 : inherited (NULL,  // lock handle
              true,  // active ?
              false, // auto-start ?
              false, // run svc() on start() ?
              true)  // generate session messages
 , baseTimeStamp_ (0)
 , isFirst_ (true)
 , mediaSource_ (NULL)
 , presentationClock_ (NULL)
 , referenceCount_ (0)
 , symbolicLink_ (NULL)
 , symbolicLinkSize_ (0)
 , topology_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::Stream_Dev_Cam_Source_MediaFoundation_T"));

}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                         SessionMessageType,
                                         ProtocolMessageType,
                                         ConfigurationType,
                                         StreamStateType,
                                         SessionDataType,
                                         SessionDataContainerType,
                                         StatisticContainerType>::~Stream_Dev_Cam_Source_MediaFoundation_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::~Stream_Dev_Cam_Source_MediaFoundation_T"));

  if (presentationClock_)
    presentationClock_->Release ();
  if (topology_)
    topology_->Release ();
  if (mediaSource_)
    mediaSource_->Release ();
  if (symbolicLinkSize_)
    CoTaskMemFree (symbolicLink_);
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::initialize"));

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
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      return false;
    } // end IF
    COM_initialized = true;
  } // end IF

  if (inherited::initialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    isFirst_ = true;

    referenceCount_ = 0;

    if (presentationClock_)
    {
      presentationClock_->Release ();
      presentationClock_ = NULL;
    } // end IF
    if (topology_)
    {
      topology_->Release ();
      topology_ = NULL;
    } // end IF
    if (mediaSource_)
    {
      mediaSource_->Release ();
      mediaSource_ = NULL;
    } // end IF
    if (symbolicLinkSize_)
    {
      CoTaskMemFree (symbolicLink_);
      symbolicLink_ = NULL;
      symbolicLinkSize_ = 0;
    } // end IF

    baseTimeStamp_ = 0;

    if (inherited::sessionData_)
    {
      inherited::sessionData_->decrease ();
      inherited::sessionData_ = NULL;
    } // end IF
  } // end IF

  result = inherited::initialize (configuration_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

//done:
  if (COM_initialized)
    CoUninitialize ();

  return result;
}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Cam_Source_MediaFoundation_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::handleDataMessage"));

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::initialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->streamConfiguration);
      ACE_ASSERT (session_data_r.format);

      bool COM_initialized = false;
      bool is_running = false;
      bool is_reading = false;
      bool release_device = false;
      IDirect3DDeviceManager9* direct3D_manager_p = NULL;
      //IMFTopologyNode* source_node_p = NULL;
      bool release_media_source = false;
      bool release_topology = false;
      //IMFPresentationDescriptor* presentation_descriptor_p = NULL;
      HRESULT result_2 = E_FAIL;

      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerID_ == -1);
        ACE_Event_Handler* handler_p = &(inherited::statisticCollectionHandler_);
        inherited::timerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                                                                // event handler
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

      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!mediaSource_);
      ACE_ASSERT (!topology_);
      ACE_ASSERT (!session_data_r.topology);

      if (!session_data_r.direct3DDevice)
      {
        // *TODO*: remove type inferences
        struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters;
        if (!Stream_Module_Device_Tools::getDirect3DDevice (inherited::configuration_->window,
                                                            session_data_r.format,
                                                            session_data_r.direct3DDevice,
                                                            d3d_presentation_parameters,
                                                            direct3D_manager_p,
                                                            session_data_r.resetToken))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_Tools::getDirect3DDevice(), aborting\n")));
          goto error;
        } // end IF
        release_device = true;
      } // end IF

      //if (!inherited::configuration_->sourceReader)
      //  if (!Stream_Module_Device_Tools::initializeDirect3DManager (session_data_r.direct3DDevice,
      //                                                              direct3D_manager_p,
      //                                                              session_data_r.resetToken))
      //  {
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("failed to Stream_Module_Device_Tools::initializeDirect3DManager(), aborting\n")));
      //    goto error;
      //  } // end IF

      ULONG reference_count = 0;
      if (inherited::configuration_->topology)
      {
        reference_count = inherited::configuration_->topology->AddRef ();
        topology_ = inherited::configuration_->topology;
        reference_count = topology_->AddRef ();
        session_data_r.topology = topology_;

        // sanity check(s)
        ACE_ASSERT (inherited::configuration_->mediaSource);

        reference_count = inherited::configuration_->mediaSource->AddRef ();
        mediaSource_ = inherited::configuration_->mediaSource;
      } // end IF
      else
      {
        if (inherited::configuration_->mediaSource)
        {
          reference_count = inherited::configuration_->mediaSource->AddRef ();
          mediaSource_ = inherited::configuration_->mediaSource;
        } // end IF
        else
        {
          //// sanity check(s)
          //ACE_ASSERT (direct3D_manager_p);

          //// *TODO*: remove type inferences
          //if (!initialize_MediaFoundation (inherited::configuration_->device,
          //                                 inherited::configuration_->window,
          //                                 direct3D_manager_p,
          //                                 session_data_r.format,
          //                                 mediaSource_,
          //                                 symbolicLink_,
          //                                 symbolicLinkSize_,
          //                                 this,
          //                                 sourceReader_))
          //{
          //  ACE_DEBUG ((LM_ERROR,
          //              ACE_TEXT ("failed to initialize_MediaFoundation(), aborting\n")));
          //  goto error;
          //} // end IF
          if (!Stream_Module_Device_Tools::getMediaSource (inherited::configuration_->device,
                                                           mediaSource_,
                                                           symbolicLink_,
                                                           symbolicLinkSize_))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), aborting\n")));
            goto error;
          } // end IF
          release_media_source = true;
        } // end ELSE

        if (!Stream_Module_Device_Tools::loadDeviceTopology (inherited::configuration_->device,
                                                             mediaSource_,
                                                             topology_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceTopology(), aborting\n")));
          goto error;
        } // end IF
        release_topology = true;

        reference_count = topology_->AddRef ();
        session_data_r.topology = topology_;
      } // end ELSE
      if (direct3D_manager_p)
      {
        direct3D_manager_p->Release ();
        direct3D_manager_p = NULL;
      } // end IF
      ACE_ASSERT (mediaSource_);
      //ACE_ASSERT (sourceReader_);
      ACE_ASSERT (topology_);
      ACE_ASSERT (session_data_r.topology);

      if (!Stream_Module_Device_Tools::setCaptureFormat (topology_,
                                                         session_data_r.format))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
        goto error;
      } // end IF
      if (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("capture format: \"%s\"...\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (session_data_r.format).c_str ())));

      break;

error:
      if (direct3D_manager_p)
        direct3D_manager_p->Release ();
      //if (presentation_descriptor_p)
      //  presentation_descriptor_p->Release ();
      //if (source_node_p)
      //  source_node_p->Release ();

      if (is_running)
      {
        // sanity check(s)
        ACE_ASSERT (mediaSource_);

        result_2 = mediaSource_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSource::Stop(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      } // end IF

      if (release_device)
      {
        session_data_r.direct3DDevice->Release ();
        session_data_r.direct3DDevice = NULL;
        session_data_r.resetToken = 0;
      } // end IF
      if (session_data_r.format)
      {
        session_data_r.format->Release ();
        session_data_r.format = NULL;
      } // end IF
      if (session_data_r.topology)
      {
        session_data_r.topology->Release ();
        session_data_r.topology = NULL;
      } // end IF

      if (release_topology)
      {
        topology_->Release ();
        topology_ = NULL;
      } // end IF

      if (release_media_source)
      {
        mediaSource_->Release ();
        mediaSource_ = NULL;
      } // end IF
      if (symbolicLinkSize_)
      {
        CoTaskMemFree (symbolicLink_);
        symbolicLink_ = NULL;
        symbolicLinkSize_ = 0;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      session_data_r.aborted = true;

      break;
    }
    case STREAM_SESSION_END:
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

      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      result_2 = mediaSource_->Stop ();
      if (FAILED (result_2))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSource::Stop(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

      if (presentationClock_)
      {
        presentationClock_->Release ();
        presentationClock_ = NULL;
      } // end IF
      if (topology_)
      {
        topology_->Release ();
        topology_ = NULL;
      } // end IF

      if (mediaSource_)
      {
        mediaSource_->Release ();
        mediaSource_ = NULL;
      } // end IF

      if (session_data_r.format)
      {
        session_data_r.format->Release ();
        session_data_r.format = NULL;
      } // end IF
      if (session_data_r.topology)
      {
        session_data_r.topology->Release ();
        session_data_r.topology = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (inherited::sessionData_)
      {
        inherited::sessionData_->decrease ();
        inherited::sessionData_ = NULL;
      } // end IF

      inherited::shutdown ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

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

//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::QueryInterface (const IID& IID_in,
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
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ULONG
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ULONG
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  if (count == 0);
    //delete this;

  return count;
}
//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
//                                        SessionMessageType,
//                                        ProtocolMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::OnEvent (DWORD streamIndex_in,
//                                                                          IMFMediaEvent* mediaEvent_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnEvent"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//  ACE_UNUSED_ARG (mediaEvent_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_OK);
//  ACE_NOTREACHED (return S_OK;)
//}
//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
//                                        SessionMessageType,
//                                        ProtocolMessageType,
//                                        ConfigurationType,
//                                        StreamStateType,
//                                        SessionDataType,
//                                        SessionDataContainerType,
//                                        StatisticContainerType>::OnFlush (DWORD streamIndex_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnFlush"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_OK);
//  ACE_NOTREACHED (return S_OK;)
//}
//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//HRESULT
//Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
//                                        SessionMessageType,
//                                        ProtocolMessageType,
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
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnReadSample"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//  ACE_UNUSED_ARG (streamFlags_in);
//
//  bool read_next_sample = false;
//  ProtocolMessageType* message_p = NULL;
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
//    message_p = dynamic_cast<ProtocolMessageType*> (sample_in);
//  }
//  catch (...)
//  {
//    //ACE_DEBUG ((LM_ERROR,
//    //            ACE_TEXT ("failed to dynamic_cast<ProtocolMessageType*>(0x%@), continuing\n"),
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
//  typename ProtocolMessageType::DATA_T& data_r =
//    const_cast<typename ProtocolMessageType::DATA_T&> (message_p->get ());
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
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockStart (MFTIME systemClockTime_in,
                                                                               LONGLONG clockStartOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockStart"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (clockStartOffset_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockStop"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockPause (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockPause"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockRestart (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockRestart"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnClockSetRate (MFTIME systemClockTime_in,
                                                                                 float playbackRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnClockSetRate"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (playbackRate_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnProcessSample"));

  ACE_UNUSED_ARG (majorMediaType_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (duration_in);

  ProtocolMessageType* message_p = NULL;
  int result = -1;
  HRESULT result_2 = E_FAIL;

  if (isFirst_)
  {
    isFirst_ = false;
    baseTimeStamp_ = timeStamp_in;
  } // end IF
  timeStamp_in -= baseTimeStamp_;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // *TODO*: remove type inference
  message_p =
    inherited::allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage(%d) failed: \"%m\", aborting\n"),
                inherited::configuration_->streamConfiguration->bufferSize));
    goto error;
  } // end IF
  ACE_ASSERT (message_p);
  ACE_ASSERT (message_p->capacity () >= bufferSize_in);

  // *TODO*: copy this data into the message buffer ?
  message_p->base (reinterpret_cast<char*> (const_cast<BYTE*> (buffer_in)),
                   bufferSize_in,
                   ACE_Message_Block::DONT_DELETE);
  message_p->wr_ptr (bufferSize_in);

  result = inherited::putq (message_p, NULL);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited::name ()));
    goto error;
  } // end IF

  return S_OK;

error:
  if (message_p)
    message_p->release ();

  return E_FAIL;
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnSetPresentationClock (IMFPresentationClock* presentationClock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnSetPresentationClock"));

  // sanity check(s)
  if (presentationClock_)
  {
    presentationClock_->Release ();
    presentationClock_ = NULL;
  } // end IF

  ULONG reference_count = presentationClock_in->AddRef ();
  presentationClock_ = presentationClock_in;

  return S_OK;
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::OnShutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnShutdown"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
                                        SessionMessageType,
                                        ProtocolMessageType,
                                        ConfigurationType,
                                        StreamStateType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        StatisticContainerType>::initialize_MediaFoundation (const std::string& deviceName_in,
                                                                                             const HWND windowHandle_in,
                                                                                             const IDirect3DDeviceManager9* IDirect3DDeviceManager_in,
                                                                                             const IMFMediaType* IMFMediaType_in,
                                                                                             IMFMediaSource*& IMFMediaSource_inout,
                                                                                             WCHAR*& symbolicLink_out,
                                                                                             UINT32& symbolicLinkSize_out,
                                                                                             const IMFSampleGrabberSinkCallback* IMFSampleGrabberSinkCallback_in,
                                                                                             IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::initialize_MediaFoundation"));

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
