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
 , isFirst_ (true)
 , mediaSource_ (NULL)
 , referenceCount_ (0)
 , sourceReader_ (NULL)
 , baseTimeStamp_ (0)
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

  if (sourceReader_)
    sourceReader_->Release ();
  if (mediaSource_)
    mediaSource_->Release ();
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

    if (sourceReader_)
    {
      sourceReader_->Release ();
      sourceReader_ = NULL;
    } // end IF
    if (mediaSource_)
    {
      mediaSource_->Release ();
      mediaSource_ = NULL;
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
      IMFPresentationDescriptor* presentation_descriptor_p = NULL;

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

      HRESULT result_2 = CoInitializeEx (NULL,
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
      ACE_ASSERT (!sourceReader_);

      bool release_device = false;
      IDirect3DDeviceManager9* direct3D_manager_p = NULL;
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
      else
      {
        if (!inherited::configuration_->sourceReader)
          if (!Stream_Module_Device_Tools::initializeDirect3DManager (session_data_r.direct3DDevice,
                                                                      direct3D_manager_p,
                                                                      session_data_r.resetToken))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_Module_Device_Tools::initializeDirect3DManager(), aborting\n")));
            goto error;
          } // end IF
      } // end ELSE
      //if (inherited::configuration_->direct3DDevice)
      //{
      //  inherited::configuration_->direct3DDevice->AddRef ();
      //  Direct3DDevice_ = inherited::configuration_->direct3DDevice;

      //  // sanity check(s)
      //  ACE_ASSERT (session_data_r.resetToken);
      //} // end IF

      if (inherited::configuration_->sourceReader)
      {
        inherited::configuration_->sourceReader->AddRef ();
        sourceReader_ = inherited::configuration_->sourceReader;

        // sanity check(s)
        ACE_ASSERT (inherited::configuration_->mediaSource);

        inherited::configuration_->mediaSource->AddRef ();
        mediaSource_ = inherited::configuration_->mediaSource;
      } // end IF
      else
      {
        if (inherited::configuration_->mediaSource)
        {
          inherited::configuration_->mediaSource->AddRef ();
          mediaSource_ = inherited::configuration_->mediaSource;
        } // end IF
        else
        {
          // sanity check(s)
          ACE_ASSERT (direct3D_manager_p);

          // *TODO*: remove type inferences
          if (!initialize_MediaFoundation (inherited::configuration_->device,
                                           inherited::configuration_->window,
                                           direct3D_manager_p,
                                           mediaSource_,
                                           this,
                                           sourceReader_))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to initialize_MediaFoundation(), aborting\n")));
            goto error;
          } // end IF
        } // end ELSE
      } // end ELSE
      if (direct3D_manager_p)
      {
        direct3D_manager_p->Release ();
        direct3D_manager_p = NULL;
      } // end IF
      ACE_ASSERT (sourceReader_);

      if (!mediaSource_)
      {
        ACE_ASSERT (false);

        //result_2 = MFGetService (sourceReader_,
        //                         MF_MEDIASOURCE_SERVICE,
        //                         IID_IMFMediaSource,
        //                         (void**)&media_source_p);
        //result_2 =
        //  sourceReader_->GetServiceForStream (MF_SOURCE_READER_MEDIASOURCE,
        //                                      MF_MEDIASOURCE_SERVICE,
        //                                      IID_IMFMediaSource,
        //                                      (void**)&media_source_p);
        //result_2 =
        //  sourceReader_->QueryInterface (IID_PPV_ARGS (&media_source_p));
        //if (FAILED (result_2))
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              //ACE_TEXT ("failed to MFGetService(MF_MEDIASOURCE_SERVICE): \"%s\", aborting\n"),
        //              //ACE_TEXT ("failed to IMFSourceReader::GetServiceForStream(MF_MEDIASOURCE_SERVICE): \"%s\", aborting\n"),
        //              ACE_TEXT ("failed to IMFSourceReader::QueryInterface(IID_IMFMediaSource): \"%s\", aborting\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        //  goto error;
        //} // end IF
      } // end IF
      ACE_ASSERT (mediaSource_);

      if (!Stream_Module_Device_Tools::setCaptureFormat (sourceReader_,
                                                         session_data_r.format))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
        goto error;
      } // end IF
      if (!Stream_Module_Device_Tools::getCaptureFormat (sourceReader_,
                                                         session_data_r.format))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
        goto error;
      } // end IF

      if (_DEBUG)
      {
        std::string media_type_string =
          Stream_Module_Device_Tools::mediaTypeToString (session_data_r.format);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("capture format: \"%s\"...\n"),
                    ACE_TEXT (media_type_string.c_str ())));
      } // end IF

      result_2 =
        mediaSource_->CreatePresentationDescriptor (&presentation_descriptor_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (presentation_descriptor_p);
      //DWORD stream_descriptor_count = 0;
      //result_2 =
      //  presentation_descriptor_p->GetStreamDescriptorCount (&stream_descriptor_count);
      //if (FAILED (result_2))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptorCount(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      //  goto error;
      //} // end IF
      //ACE_ASSERT (stream_descriptor_count > 0);
      struct tagPROPVARIANT start_position;
      PropVariantInit (&start_position);
      start_position.vt = VT_EMPTY;
      result_2 = mediaSource_->Start (presentation_descriptor_p, // presentation descriptor handle
                                      &GUID_NULL,                // time format
                                      &start_position);          // start position
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSource::Start(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      presentation_descriptor_p->Release ();
      presentation_descriptor_p = NULL;
      is_running = true;

      // start displaying video data (asynchronous mode)
      result_2 = sourceReader_->ReadSample (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                            0,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFSourceReader::ReadSample(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      is_reading = true;

      break;

error:
      if (direct3D_manager_p)
        direct3D_manager_p->Release ();
      if (presentation_descriptor_p)
        presentation_descriptor_p->Release ();

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

      if (sourceReader_)
      {
        sourceReader_->Release ();
        sourceReader_ = NULL;
      } // end IF

      if (mediaSource_)
      {
        mediaSource_->Release ();
        mediaSource_ = NULL;
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

      if (sourceReader_)
      {
        sourceReader_->Release ();
        sourceReader_ = NULL;
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

  static const QITAB qit[] =
  {
    QITABENT (OWN_TYPE_T, IMFSourceReaderCallback),
    { 0 },
  };
  return QISearch (this, qit, IID_in, interface_out);
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

  ULONG uCount = InterlockedDecrement (&referenceCount_);
  if (uCount == 0);
    //delete this;

  return uCount;
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
                                        StatisticContainerType>::OnEvent (DWORD streamIndex_in,
                                                                          IMFMediaEvent* mediaEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnEvent"));

  ACE_UNUSED_ARG (streamIndex_in);
  ACE_UNUSED_ARG (mediaEvent_in);

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
                                        StatisticContainerType>::OnFlush (DWORD streamIndex_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnFlush"));

  ACE_UNUSED_ARG (streamIndex_in);

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
                                        StatisticContainerType>::OnReadSample (HRESULT result_in,
                                                                               DWORD streamIndex_in,
                                                                               DWORD streamFlags_in,
                                                                               LONGLONG timeStamp_in,
                                                                               IMFSample* sample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::OnReadSample"));

  ACE_UNUSED_ARG (streamIndex_in);
  ACE_UNUSED_ARG (streamFlags_in);

  bool read_next_sample = false;
  ProtocolMessageType* message_p = NULL;
  int result = -1;
  HRESULT result_2 = E_FAIL;
  IMFMediaBuffer* media_buffer_p = NULL;

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
  if (!sample_in)
  {
    // *NOTE*: end of stream ?
    if (streamFlags_in & MF_SOURCE_READERF_ENDOFSTREAM)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("reached end of stream, returning\n")));
      return S_OK;
    } // end IF

    if (streamFlags_in & MF_SOURCE_READERF_STREAMTICK)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("gap in the stream at %q, continuing\n"),
                  timeStamp_in));
      read_next_sample = true;
    } // end IF

    goto continue_2;
  } // end IF
  if (FAILED (result_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFSourceReader::ReadSample(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_in).c_str ())));
    return result_in;
  } // end IF

  try
  {
    message_p = dynamic_cast<ProtocolMessageType*> (sample_in);
  }
  catch (...)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to dynamic_cast<ProtocolMessageType*>(0x%@), continuing\n"),
    //            IMediaSample_in));
    message_p = NULL;
  }
  if (message_p) goto continue_;

  bool release_sample = false;

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

  typename ProtocolMessageType::DATA_T& data_r =
    const_cast<typename ProtocolMessageType::DATA_T&> (message_p->get ());
  ACE_ASSERT (!data_r.sample);

  ULONG reference_count = sample_in->AddRef ();
  result_2 = sample_in->SetSampleTime (timeStamp_in);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFSample::SetSampleTime(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  data_r.sample = sample_in;
  data_r.sampleTime = timeStamp_in;
  release_sample = true;

  //DWORD total_length = 0;
  //result_2 = sample_in->GetTotalLength (&total_length);
  //if (FAILED (result_2))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFSample::GetTotalLength(): \"%m\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  //  goto error;
  //} // end IF
  DWORD buffer_count = 0;
  result_2 = sample_in->GetBufferCount (&buffer_count);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFSample::GetBufferCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (buffer_count == 1);
  // *TODO*: use IMFSample::ConvertToContiguousBuffer() ?
  result_2 = sample_in->GetBufferByIndex (0,
                                          &media_buffer_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFSample::GetBufferByIndex(0): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_buffer_p);

  BYTE* buffer_p = NULL;
  DWORD maximum_length, current_length;
  result_2 = media_buffer_p->Lock (&buffer_p,
                                   &maximum_length,
                                   &current_length);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaBuffer::Lock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (buffer_p);

  message_p->base (reinterpret_cast<char*> (buffer_p),
                    current_length,
                    ACE_Message_Block::DONT_DELETE);
  message_p->wr_ptr (current_length);

  media_buffer_p->Unlock ();
  media_buffer_p->Release ();
  media_buffer_p = NULL;

continue_:
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

  read_next_sample = true;

  goto continue_2;

error:
  if (message_p)
    message_p->release ();
  if (media_buffer_p)
    media_buffer_p->Release ();
  if (release_sample)
    sample_in->Release ();

  return E_FAIL;

continue_2:
  if (!read_next_sample)
    goto continue_3;

  // sanity check(s)
  ACE_ASSERT (sourceReader_);

  result_2 = sourceReader_->ReadSample (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                        0,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFSourceReader::ReadSample(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return result_2;
  } // end IF

continue_3:
  return S_OK;
}

//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//int
//Stream_Dev_Cam_Source_MediaFoundation_T<LockType,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::svc"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::initialized_);
//  ACE_ASSERT (inherited::sessionData_);
//
//  int result = -1;
//  int result_2 = -1;
//  ACE_Message_Block* message_block_p = NULL;
//  ACE_Time_Value no_wait = COMMON_TIME_NOW;
//  ACE_Time_Value sleep_interval (0,
//                                 STREAM_DEFAULT_MODULE_SOURCE_EVENT_POLL_INTERVAL * 1000);
//  const SessionDataType& session_data_r = inherited::sessionData_->get ();
//  int message_type = -1;
//  bool finished = false;
//  bool stop_processing = false;
//
//  // step1: start processing data...
//  //   ACE_DEBUG ((LM_DEBUG,
//  //               ACE_TEXT ("entering processing loop...\n")));
//  do
//  {
//    message_block_p = NULL;
//    result = inherited::getq (message_block_p,
//                              &no_wait);
//    if (result == 0)
//    {
//      ACE_ASSERT (message_block_p);
//      message_type = message_block_p->msg_type ();
//      switch (message_type)
//      {
//        case ACE_Message_Block::MB_STOP:
//        {
//          // clean up
//          message_block_p->release ();
//          message_block_p = NULL;
//
//          // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
//          //         not have been set at this stage
//
//          // signal the controller ?
//          if (!finished)
//          {
//            finished = true;
//            // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
//            //         --> continue
//            inherited::finished ();
//            // *NOTE*: (if passive,) STREAM_SESSION_END has been processed
//            //         --> done
//            if (inherited::thr_count_ == 0)
//            {
//              result_2 = 0; // success
//              goto done; // finished processing
//            } // end IF
//
//          } // end IF
//          continue;
//        }
//        default:
//          break;
//      } // end SWITCH
//
//      // process
//      // *NOTE*: fire-and-forget message_block_p here
//      inherited::handleMessage (message_block_p,
//                                stop_processing);
//      if (stop_processing)
//      {
//        //        SessionMessageType* session_message_p = NULL;
//        //        // downcast message
//        //        session_message_p = dynamic_cast<SessionMessageType*> (message_block_p);
//        //        if (!session_message_p)
//        //        {
//        //          if (inherited::module ())
//        //            ACE_DEBUG ((LM_ERROR,
//        //                        ACE_TEXT ("%s: dynamic_cast<SessionMessageType*>(0x%@) failed (type was: %d), aborting\n"),
//        //                        inherited::name (),
//        //                        message_block_p,
//        //                        message_type));
//        //          else
//        //            ACE_DEBUG ((LM_ERROR,
//        //                        ACE_TEXT ("dynamic_cast<SessionMessageType*>(0x%@) failed (type was: %d), aborting\n"),
//        //                        message_block_p,
//        //                        message_type));
//        //          break;
//        //        } // end IF
//        //        if (session_message_p->type () == STREAM_SESSION_END)
//        result_2 = 0; // success
//        goto done; // finished processing
//      } // end IF
//    } // end IF
//    else if (result == -1)
//    {
//      int error = ACE_OS::last_error ();
//      if (error != EWOULDBLOCK) // Win32: 10035
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
//
//        // signal the controller ?
//        if (!finished)
//        {
//          finished = true;
//          inherited::finished ();
//        } // end IF
//
//        break;
//      } // end IF
//
//      // session aborted ? (i.e. connection failed)
//      if (session_data_r.aborted)
//      {
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("session aborted...\n")));
//
//        inherited::shutdown ();
//
//        continue;
//      } // end IF
//      else
//      {
//        result = ACE_OS::sleep (sleep_interval);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_OS::sleep(@#T): \"%m\", continuing\n"),
//                      &sleep_interval));
//      } // end ELSE
//    } // end IF
//  } while (true);
//
//done:
//  return result_2;
//}

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
                                                                                             IMFMediaSource*& IMFMediaSource_inout,
                                                                                             IMFSourceReaderCallback* IMFSourceReaderCallback_in,
                                                                                             IMFSourceReader*& IMFSourceReader_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_MediaFoundation_T::initialize_MediaFoundation"));

  bool release_media_source = false;

  if (!IMFMediaSource_inout)
    if (!Stream_Module_Device_Tools::getMediaSource (deviceName_in,
                                                     IMFMediaSource_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceName_in.c_str ())));
      return false;
    } // end IF
    else
      release_media_source = true;
  ACE_ASSERT (IMFMediaSource_inout);

  // *NOTE*: the source reader assumes responsibility for the media source
  //         handle
  if (!Stream_Module_Device_Tools::getSourceReader (IMFMediaSource_inout,
                                                    IDirect3DDeviceManager_in,
                                                    this,
                                                    sourceReader_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getSourceReader(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (sourceReader_);

  return true;

error:
  if (release_media_source)
  {
    IMFMediaSource_inout->Release ();
    IMFMediaSource_inout = NULL;
  } // end IF
  if (sourceReader_)
  {
    sourceReader_->Release ();
    sourceReader_ = NULL;
  } // end IF

  return false;
}
