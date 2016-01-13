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

#include "aviriff.h"
#include "dvdmedia.h"
//#include "Streams.h"

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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::Stream_Dev_Cam_Source_DirectShow_T ()
 : inherited (NULL,  // lock handle
              false, // active ?
              false, // auto-start ?
              false, // run svc() on start() ?
              true)  // generate session messages
 , isInitialized_ (false)
 , ROTID_ (0)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
 , isFirst_ (true)
//, ICaptureGraphBuilder2_ (NULL)
 , IMediaControl_ (NULL)
 , IMediaEventEx_ (NULL)
 , ISampleGrabber_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::Stream_Dev_Cam_Source_DirectShow_T"));

}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::~Stream_Dev_Cam_Source_DirectShow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::~Stream_Dev_Cam_Source_DirectShow_T"));

  int result = -1;

  if (timerID_ != -1)
  {
    const void* act_p = NULL;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                  &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  timerID_));
    else
      ACE_DEBUG ((LM_WARNING, // this should happen in END_SESSION
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  timerID_));
  } // end IF

  if (ROTID_)
  {
    IRunningObjectTable* ROT_p = NULL;
    result = GetRunningObjectTable (0, &ROT_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto continue_;
    } // end IF
    ACE_ASSERT (ROT_p);
    result = ROT_p->Revoke (ROTID_);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d) \"%s\", continuing\n"),
                  ROTID_,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    ROT_p->Release ();
  } // end IF

continue_:
  if (IMediaEventEx_)
  {
    IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
    IMediaEventEx_->Release ();
  } // end IF
  if (IMediaControl_)
  {
    IMediaControl_->Stop ();
    IMediaControl_->Release ();
  } // end IF
  if (ISampleGrabber_)
  {
    ISampleGrabber_->Release ();
    ISampleGrabber_ = NULL;
  } // end IF
//if (ICaptureGraphBuilder2_)
//  ICaptureGraphBuilder2_->Release ();
  //if (inherited::configuration_)
  //  if (inherited::configuration_->builder)
  //  {
  //    inherited::configuration_->builder->Release ();
  //    inherited::configuration_->builder = NULL;
  //  } // end IF
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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::initialize"));

  int result = -1;

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (first_run)
  {
    first_run = false;

    HRESULT result = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    COM_initialized = true;
  } // end IF

  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (timerID_ != -1)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                    &act_p);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    timerID_));
    } // end IF
    timerID_ = -1;

    if (ROTID_)
    {
      IRunningObjectTable* ROT_p = NULL;
      result = GetRunningObjectTable (0, &ROT_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        ROTID_ = 0;

        goto continue_;
      } // end IF
      ACE_ASSERT (ROT_p);
      result = ROT_p->Revoke (ROTID_);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d) \"%s\", continuing\n"),
                    ROTID_,
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      ROT_p->Release ();
      ROTID_ = 0;
    } // end IF

continue_:
    //ACE_ASSERT (inherited::configuration_);

    isFirst_ = true;
    if (IMediaEventEx_)
    {
      IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
      IMediaEventEx_->Release ();
      IMediaEventEx_ = NULL;
    } // end IF
    if (IMediaControl_)
    {
      IMediaControl_->Stop ();
      IMediaControl_->Release ();
      IMediaControl_ = NULL;
    } // end IF
    //if (ICaptureGraphBuilder2_)
    //{
    //  ICaptureGraphBuilder2_->Release ();
    //  ICaptureGraphBuilder2_ = NULL;
    //} // end IF
    //if (inherited::configuration_->builder)
    //{
    //  inherited::configuration_->builder->Release ();
    //  inherited::configuration_->builder = NULL;
    //} // end IF

    isInitialized_ = false;
  } // end IF

  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

//done:
  if (COM_initialized)
    CoUninitialize ();

  return isInitialized_;
}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Cam_Source_DirectShow_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::handleDataMessage"));

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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::handleSessionMessage"));

  int result = -1;
  IRunningObjectTable* ROT_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (isInitialized_);

  //const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
  //    message_inout->get ();
  //SessionDataType& session_data_r =
  //    const_cast<SessionDataType&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      if (inherited::configuration_->streamConfiguration->statisticReportingInterval)
      {
        // schedule regular statistics collection...
        ACE_Time_Value interval (STREAM_STATISTIC_COLLECTION_INTERVAL, 0);
        ACE_ASSERT (timerID_ == -1);
        ACE_Event_Handler* handler_p = &statisticCollectionHandler_;
        timerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                        NULL,                       // argument
                                                                        COMMON_TIME_NOW + interval, // first wakeup time
                                                                        interval);                  // interval
        if (timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", returning\n")));
          return;
        } // end IF
        //        ACE_DEBUG ((LM_DEBUG,
        //                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for interval %#T...\n"),
        //                    timerID_,
        //                    &interval));
      } // end IF

      bool COM_initialized = false;
      bool is_running = false;

      HRESULT result_2 = CoInitializeEx (NULL, COINIT_MULTITHREADED);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      if (!initialize_DirectShow (inherited::configuration_->device,
                                  //ICaptureGraphBuilder2_,
                                  inherited::configuration_->builder,
                                  inherited::configuration_->window))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize_DirectShow(), returning\n")));
        goto error;
      } // end IF
      //ACE_ASSERT (ICaptureGraphBuilder2_);
      ACE_ASSERT (inherited::configuration_->builder);
      ACE_ASSERT (IMediaControl_);
      ACE_ASSERT (IMediaEventEx_);

      // start previewing video data
      result_2 = IMediaControl_->Run ();
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMediaControl::Run(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      is_running = true;

      // register graph in the ROT (GraphEdit.exe)
      IMoniker* moniker_p = NULL;
      WCHAR buffer[BUFSIZ];
      result_2 = GetRunningObjectTable (0, &ROT_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to GetRunningObjectTable(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (ROT_p);
      result_2 =
        ::StringCchPrintfW (buffer, NUMELMS (buffer),
                            ACE_TEXT_ALWAYS_WCHAR ("FilterGraph %08x [PID: %08x]\0"),
                            (DWORD_PTR)inherited::configuration_->builder, ACE_OS::getpid ());
      result_2 = CreateItemMoniker (ACE_TEXT_ALWAYS_WCHAR ("!"), buffer,
                                    &moniker_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CreateItemMoniker(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // clean up
        ROT_p->Release ();

        goto error;
      } // end IF

      // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
      // to the object.  Using this flag will cause the object to remain
      // registered until it is explicitly revoked with the Revoke() method.
      // Not using this flag means that if GraphEdit remotely connects
      // to this graph and then GraphEdit exits, this object registration
      // will be deleted, causing future attempts by GraphEdit to fail until
      // this application is restarted or until the graph is registered again.
      result_2 =
        ROT_p->Register (ROTFLAGS_REGISTRATIONKEEPSALIVE,
                         inherited::configuration_->builder, moniker_p,
                         &ROTID_);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IRunningObjectTable::Register(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // clean up
        ROT_p->Release ();
        moniker_p->Release ();

        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("registered graph in running object table (ID: %d)\n"),
                  ROTID_));

      ROT_p->Release ();
      moniker_p->Release ();

      if (COM_initialized)
        CoUninitialize ();

      break;

error:
      if (is_running)
      {
        result_2 = IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      } // end IF
      if (COM_initialized)
        CoUninitialize ();

      break;
    }
    case STREAM_SESSION_END:
    {
      // sanity check(s)
      // *TODO*: remove type inference
      //ACE_ASSERT (inherited::configuration_->builder);

      if (timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                      &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      timerID_));
        timerID_ = -1;
      } // end IF

      bool COM_initialized = false;
      HRESULT result_2 = CoInitializeEx (NULL, COINIT_MULTITHREADED);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      // deregister graph from the ROT (GraphEdit.exe) ?
      if (ROTID_)
      {
        result_2 = GetRunningObjectTable (0, &ROT_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to GetRunningObjectTable(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

          // clean up
          ROTID_ = 0;

          goto continue_;
        } // end IF
        ACE_ASSERT (ROT_p);
        result_2 = ROT_p->Revoke (ROTID_);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d): \"%s\", continuing\n"),
                      ROTID_,
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("removed graph from running object table (ID was: %d)\n"),
                      ROTID_));

        ROT_p->Release ();
        ROTID_ = 0;
      } // end IF

continue_:
      if (IMediaEventEx_)
      {
        result_2 = IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        IMediaEventEx_->Release ();
        IMediaEventEx_ = NULL;
      } // end IF

      if (IMediaControl_)
      {
        // stop previewing video data
        result_2 = IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        IMediaControl_->Release ();
        IMediaControl_ = NULL;
      } // end IF

      //IGraphBuilder* builder_p = NULL;
      //result_2 =
      //  inherited::configuration_->builder->GetFiltergraph (&builder_p);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      //  goto continue_2;
      //} // end IF
      //ACE_ASSERT (builder_p);

      //if (!Stream_Module_Device_Tools::disconnect (builder_p))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), continuing\n")));

      //builder_p->Release ();

//continue_2:
      if (ISampleGrabber_)
      {
        ISampleGrabber_->Release ();
        ISampleGrabber_ = NULL;
      } // end IF
      //if (ICaptureGraphBuilder2_)
      //{
      //  ICaptureGraphBuilder2_->Release ();
      //  ICaptureGraphBuilder2_ = NULL;
      //} // end IF

      if (COM_initialized)
        CoUninitialize ();

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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::BufferCB (double sampleTime_in,
                                                                      BYTE* buffer_in,
                                                                      long bufferLen_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::BufferCB"));

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
HRESULT
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::SampleCB (double sampleTime_in,
                                                                      IMediaSample* IMediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::SampleCB"));

  ACE_UNUSED_ARG (sampleTime_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  int result = -1;
  ProtocolMessageType* message_p = NULL;
  HRESULT result_2 = E_FAIL;

  if (isFirst_)
  {
    // sanity check(s)
    ACE_ASSERT (ISampleGrabber_);

    struct _AMMediaType media_type;
    ACE_OS::memset (&media_type, 0, sizeof (struct _AMMediaType));
    result_2 = ISampleGrabber_->GetConnectedMediaType (&media_type);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMediaSample::GetMediaType(): \"%m\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return result_2;
    } // end IF
    if ((media_type.formattype != FORMAT_VideoInfo) &&
        (media_type.formattype != FORMAT_VideoInfo2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type, aborting\n")));
      return result_2;
    } // end IF
    struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
    struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
    if (media_type.formattype == FORMAT_VideoInfo)
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type.pbFormat;
    else if (media_type.formattype == FORMAT_VideoInfo2)
      video_info_header2_p = (struct tagVIDEOINFOHEADER2*)media_type.pbFormat;

    // *TODO*: remove type inference
    message_p =
      allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);

    // RIFF header
    struct _rifflist RIFF_list;
    ACE_OS::memset (&RIFF_list, 0, sizeof (struct _rifflist));
    RIFF_list.fcc = FCC ('RIFF');
    // *NOTE*: in a streaming scenario, this would need to be added AFTER the
    //         file has been written (or the disc runs out of space), which is
    //         impossible until/unless this value is preconfigured in some way.
    //         Notice how this oversight confounds the whole standard
    // sizeof (fccListType) [4] + sizeof (data) --> == total (file) size - 8
    RIFF_list.cb = sizeof (FOURCC) +
                   sizeof (struct _rifflist) +           // hdrl
                   sizeof (struct _avimainheader) +
                   // sizeof (LIST strl)
                   sizeof (struct _rifflist) +
                   sizeof (struct _avistreamheader) +    // strh
                   sizeof (struct _riffchunk) +          // strf
                   sizeof (struct tagBITMAPINFOHEADER) + // strf
                   sizeof (struct _riffchunk) +          // JUNK
                   1820 +                                // pad bytes
                   sizeof (struct _rifflist) +           // movi
                   sizeof (struct _riffchunk) +          // 00db
                   IMediaSample_in->GetSize ();          // (part of) frame
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
    RIFF_list.fccListType = FCC ('AVI ');
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list),
                              sizeof (struct _rifflist));

    // hdrl
    RIFF_list.fcc = FCC ('LIST');
    // sizeof (fccListType) [4] + sizeof (LIST data)
    RIFF_list.cb = sizeof (FOURCC)                    +
                   sizeof (struct _avimainheader)     +
                   // sizeof (LIST strl)
                   sizeof (struct _rifflist)          +
                   sizeof (struct _avistreamheader)   + // strh
                   sizeof (struct _riffchunk)         + // strf
                   sizeof (struct tagBITMAPINFOHEADER); // strf
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
    RIFF_list.fccListType = FCC ('hdrl');
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list),
                              sizeof (struct _rifflist));

    // *NOTE*: "...the 'hdrl' list begins with the main AVI header, which is
    //         contained in an 'avih' chunk. ..."
    struct _avimainheader AVI_header_avih;
    ACE_OS::memset (&AVI_header_avih, 0, sizeof (struct _avimainheader));
    AVI_header_avih.fcc = ckidMAINAVIHEADER;
    AVI_header_avih.cb = sizeof (struct _avimainheader) - 8;
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      AVI_header_avih.cb = ACE_SWAP_LONG (AVI_header_avih.cb);
    AVI_header_avih.dwMicroSecPerFrame =
      ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->AvgTimePerFrame
                                                   : video_info_header2_p->AvgTimePerFrame);
    AVI_header_avih.dwMaxBytesPerSec =
      ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->dwBitRate
                                                   : video_info_header2_p->dwBitRate) / 8;
    AVI_header_avih.dwPaddingGranularity = STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN;
    AVI_header_avih.dwFlags = AVIF_WASCAPTUREFILE;
    //AVI_header_avih.dwTotalFrames = 0; // unreliable
    //AVI_header_avih.dwInitialFrames = 0;
    AVI_header_avih.dwStreams = 1;
    //AVI_header_avih.dwSuggestedBufferSize = 0; // unreliable
    AVI_header_avih.dwWidth =
      ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biWidth
                                                   : video_info_header2_p->bmiHeader.biWidth);
    AVI_header_avih.dwHeight =
      ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biHeight
                                                   : video_info_header2_p->bmiHeader.biHeight);
    //AVI_header_avih.dwReserved = {0, 0, 0, 0};
    result = message_p->copy (reinterpret_cast<char*> (&AVI_header_avih),
                              sizeof (struct _avimainheader));

    // *NOTE*: "One or more 'strl' lists follow the main header. A 'strl' list
    //         is required for each data stream. Each 'strl' list contains
    //         information about one stream in the file, and must contain a
    //         stream header chunk ('strh') and a stream format chunk ('strf').
    //         ..."
    // strl
    RIFF_list.fcc = FCC ('LIST');
    // sizeof (fccListType) [4] + sizeof (LIST data)
    RIFF_list.cb = sizeof (FOURCC)                    +
                   sizeof (struct _avistreamheader)   + // strh
                   sizeof (struct _riffchunk)         + // strf
                   sizeof (struct tagBITMAPINFOHEADER); // strf
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
    RIFF_list.fccListType = ckidSTREAMLIST;
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list),
                              sizeof (struct _rifflist));

    // strl --> strh
    struct _avistreamheader AVI_header_strh;
    ACE_OS::memset (&AVI_header_strh, 0, sizeof (struct _avistreamheader));
    AVI_header_strh.fcc = ckidSTREAMHEADER;
    AVI_header_strh.cb = sizeof (struct _avistreamheader) - 8;
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      AVI_header_strh.cb = ACE_SWAP_LONG (AVI_header_strh.cb);
    AVI_header_strh.fccType = streamtypeVIDEO;
    //AVI_header_strh.fccHandler = 0;
    //AVI_header_strh.dwFlags = 0;
    //AVI_header_strh.wPriority = 0;
    //AVI_header_strh.wLanguage = 0;
    //AVI_header_strh.dwInitialFrames = 0;
    // *NOTE*: dwRate / dwScale == fps
    AVI_header_strh.dwScale = 10000; // 100th nanoseconds --> seconds ???
    AVI_header_strh.dwRate =
      ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->AvgTimePerFrame
                                                   : video_info_header2_p->AvgTimePerFrame);
    //AVI_header_strh.dwStart = 0;
    //AVI_header_strh.dwLength = 0;
    //AVI_header_strh.dwSuggestedBufferSize = 0;
    AVI_header_strh.dwQuality = -1; // default
    //AVI_header_strh.dwSampleSize = 0;
    //AVI_header_strh.rcFrame = {0, 0, 0, 0};
    result = message_p->copy (reinterpret_cast<char*> (&AVI_header_strh),
                              sizeof (struct _avistreamheader));

    // strl --> strf
    // *NOTE*: there is no definition for AVI stream format chunks, as their
    //         contents differ, depending on the stream type
    struct _riffchunk RIFF_chunk;
    ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
    RIFF_chunk.fcc = ckidSTREAMFORMAT;
    RIFF_chunk.cb = sizeof (struct tagBITMAPINFOHEADER);
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                              sizeof (struct _riffchunk));
    struct tagBITMAPINFOHEADER AVI_header_strf;
    ACE_OS::memset (&AVI_header_strf, 0, sizeof (struct tagBITMAPINFOHEADER));
    AVI_header_strf =
      ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader
                                                   : video_info_header2_p->bmiHeader);
    result = message_p->copy (reinterpret_cast<char*> (&AVI_header_strf),
                              sizeof (struct tagBITMAPINFOHEADER));

    // strl --> strd
    // strl --> strn

    // --> END strl

    // insert JUNK chunk to align the 'movi' chunk at 2048 bytes
    // --> should speed up CD-ROM access
    unsigned int pad_bytes =
      AVI_header_avih.dwPaddingGranularity - message_p->length () - 8 - 12;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("inserting JUNK chunk (%d pad byte(s))...\n"),
                pad_bytes));
    RIFF_chunk.fcc = FCC ('JUNK');
    RIFF_chunk.cb = pad_bytes;
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                              sizeof (struct _riffchunk));
    ACE_OS::memset (message_p->wr_ptr (), 0, pad_bytes);
    message_p->wr_ptr (RIFF_chunk.cb);

    // movi
    RIFF_list.fcc = FCC ('LIST');
    // *NOTE*: see above
    // sizeof (fccListType) [4] + sizeof (LIST data)
    RIFF_list.cb = sizeof (FOURCC) +
                   sizeof (struct _riffchunk) + // 00db
                   IMediaSample_in->GetSize (); // (part of) frame
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    RIFF_list.fccListType = FCC ('movi');
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list),
                              sizeof (struct _rifflist));

    // db (--> Uncompressed video frame)
    RIFF_chunk.fcc = FCC ('00db');
    RIFF_chunk.cb = IMediaSample_in->GetSize ();
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                              sizeof (struct _riffchunk));

    //struct _avisuperindex AVI_header_indx;
    //ACE_OS::memset (&AVI_header_indx, 0, sizeof (struct _avisuperindex));
    //AVI_header_indx.fcc = 0;
    //AVI_header_indx.cb = 0;
    //AVI_header_indx.wLongsPerEntry = 0;
    //AVI_header_indx.bIndexSubType = 0;
    //AVI_header_indx.bIndexType = 0;
    //AVI_header_indx.nEntriesInUse = 0;
    //AVI_header_indx.dwChunkId = 0;
    //AVI_header_indx.dwReserved = 0;
    ////AVI_header_indx.aIndex = 0;
    //struct _avisuperindex_entry AVI_header_indx_entry_0;
    //ACE_OS::memset (&AVI_header_indx_entry_0, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_1;
    //ACE_OS::memset (&AVI_header_indx_entry_1, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_2;
    //ACE_OS::memset (&AVI_header_indx_entry_2, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_3;
    //ACE_OS::memset (&AVI_header_indx_entry_3, 0, sizeof (struct _avisuperindex_entry));
    //result =
    //  message_p->copy (reinterpret_cast<char*> (&AVI_header_indx),
    //                   sizeof (struct _avisuperindex));

    result = inherited::putq (message_p, NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                    inherited::name ()));

      // clean up
      message_p->release ();

      return E_FAIL;
    } // end IF

    isFirst_ = false;
  } // end IF
  else
  {
    // *TODO*: remove type inference
    message_p =
      allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);

    struct _riffchunk RIFF_chunk;
    ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
    RIFF_chunk.fcc = FCC ('00db');
    RIFF_chunk.cb = IMediaSample_in->GetSize ();
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                              sizeof (struct _riffchunk));

    result = inherited::putq (message_p, NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                    inherited::name ()));

      // clean up
      message_p->release ();

      return E_FAIL;
    } // end IF
  } // end IF

  ULONG reference_count = IMediaSample_in->AddRef ();
  try
  {
    message_p = dynamic_cast<ProtocolMessageType*> (IMediaSample_in);
  }
  catch (...)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("failed to dynamic_cast<ProtocolMessageType*>(0x%@), continuing\n"),
    //            IMediaSample_in));
    message_p = NULL;
  }
  if (!message_p)
  {
    // *TODO*: remove type inference
    message_p =
      allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);
    typename ProtocolMessageType::DATA_T& data_r =
      const_cast<typename ProtocolMessageType::DATA_T&> (message_p->get ());
    data_r.sample = IMediaSample_in;
    data_r.sampleTime = sampleTime_in;

    BYTE* buffer_p = NULL;
    result_2 = IMediaSample_in->GetPointer (&buffer_p);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMediaSample::GetPointer(): \"%m\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return result_2;
    } // end IF
    ACE_ASSERT (buffer_p);
    message_p->base (reinterpret_cast<char*> (buffer_p),
                     IMediaSample_in->GetSize (),
                     ACE_Message_Block::DONT_DELETE);
    message_p->wr_ptr (IMediaSample_in->GetSize ());
  } // end IF

  result = inherited::putq (message_p, NULL);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited::name ()));

    // clean up
    message_p->release ();

    return E_FAIL;
  } // end IF

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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::QueryInterface (const IID&,
                                                                            void **)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::QueryInterface"));

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
ULONG
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::AddRef"));

  return 1;
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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::Release"));

  return 0;
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
//Stream_Dev_Cam_Source_DirectShow_T<LockType,
//                           SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::upStream (Stream_Base_t* streamBase_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::upStream"));

//  ACE_UNUSED_ARG (streamBase_in);

//  ACE_ASSERT (false);
//  ACE_NOTSUP;

//  ACE_NOTREACHED (return;)
//}
//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//Stream_Base_t*
//Stream_Dev_Cam_Source_DirectShow_T<LockType,
//                           SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::upStream () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::upStream"));

//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (NULL);

//  ACE_NOTREACHED (return NULL;)
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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::initialize_DirectShow (const std::string& deviceName_in,
                                                                                   IGraphBuilder* IGraphBuilder_in,
                                                                                   const HWND windowHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::initialize_DirectShow"));

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  // *NOTE*: (re-)Connect()ion of the video renderer input pin fails
  //         consistently, so reuse is not feasible
  //         --> rebuild the whole graph from scratch each time
  if (!Stream_Module_Device_Tools::reset (IGraphBuilder_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::reset(), aborting\n")));
    return false;
  } // end IF

  // retrieve interfaces for media control and the video window
  HRESULT result = IGraphBuilder_in->QueryInterface (IID_IMediaControl,
                                                     (void**)&IMediaControl_);
  if (FAILED (result))
    goto error;
  result = IGraphBuilder_in->QueryInterface (IID_IMediaEventEx,
                                             (void**)&IMediaEventEx_);
  if (FAILED (result))
    goto error;

  goto continue_;

error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  return false;
continue_:
  ACE_ASSERT (IMediaControl_);
  ACE_ASSERT (IMediaEventEx_);
  // set the window handle used to process graph events
  result =
    IMediaEventEx_->SetNotifyWindow ((OAHWND)windowHandle_in,
                                     MODULE_DEV_CAM_UI_WIN32_WM_GRAPHNOTIFY, 0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  IBaseFilter* filter_p = NULL;


  result =
    IGraphBuilder_in->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE,
                                        &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  // grab
  IBaseFilter* filter_2 = NULL;
  result =
    IGraphBuilder_in->FindFilterByName (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRABBER,
                                        &filter_2);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRABBER),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();

      return false;
    } // end IF

    result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                               CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                               (void**)&filter_2);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_SampleGrabber): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (filter_2);
    result =
      IGraphBuilder_in->AddFilter (filter_2,
                                   MODULE_DEV_CAM_WIN32_FILTER_NAME_GRABBER);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      filter_2->Release ();

      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRABBER)));
  } // end IF
  ACE_ASSERT (filter_2);
  ISampleGrabber_ = NULL;
  result = filter_2->QueryInterface (IID_ISampleGrabber,
                                     (void**)&ISampleGrabber_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();

    return false;
  } // end IF
  ACE_ASSERT (ISampleGrabber_);
  ISampleGrabber_->SetBufferSamples (false);
  result = ISampleGrabber_->SetCallback (this, 0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();

    return false;
  } // end IF
  //AM_MEDIA_TYPE media_type;
  //result = ISampleGrabber_->SetMediaType (&media_type);

  // decompress ?
  struct _AMMediaType* media_type_p = NULL;
  if (!Stream_Module_Device_Tools::getFormat (IGraphBuilder_in,
                                              media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getFormat(), aborting\n")));

    // clean up
    filter_p->Release ();
    filter_2->Release ();

    return false;
  } // end IF
  ACE_ASSERT (media_type_p);
  GUID media_subtype = media_type_p->subtype;
  Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  GUID decompressor_guid = CLSID_MjpegDec;
  LPCWSTR decompressor_name = MODULE_DEV_CAM_WIN32_FILTER_NAME_MJPG_DECOMPRESS;
  if (media_subtype == MEDIASUBTYPE_YUY2)
  {
    decompressor_guid = CLSID_AVIDec;
    decompressor_name =
      MODULE_DEV_CAM_WIN32_FILTER_NAME_AVI_DECOMPRESS;
  } // end IF
  else if (media_subtype == MEDIASUBTYPE_MJPG);
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (media_subtype).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();

    return false;
  } // end ELSE
  IBaseFilter* filter_3 = NULL;
  if (!windowHandle_in) goto continue_2;
  result =
    IGraphBuilder_in->FindFilterByName (decompressor_name,
                                        &filter_3);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (decompressor_name),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      filter_2->Release ();

      return false;
    } // end IF

    result = CoCreateInstance (decompressor_guid, NULL,
                               CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                               (void**)&filter_3);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoCreateInstance() decompressor: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (media_subtype).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      filter_2->Release ();

      return false;
    } // end IF
    ACE_ASSERT (filter_3);
    result =
      IGraphBuilder_in->AddFilter (filter_3,
                                   decompressor_name);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      filter_2->Release ();
      filter_3->Release ();

      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (decompressor_name)));
  } // end IF
  ACE_ASSERT (filter_3);

  // render to a window (GtkDrawingArea) ?
continue_2:
  IBaseFilter* filter_4 = NULL;
  result =
    IGraphBuilder_in->FindFilterByName ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_VIDEO_RENDERER
                                                         : MODULE_DEV_CAM_WIN32_FILTER_NAME_NULL_RENDERER),
                                        &filter_4);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_VIDEO_RENDERER
                                                            : MODULE_DEV_CAM_WIN32_FILTER_NAME_NULL_RENDERER)),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      filter_2->Release ();
      if (filter_3)
        filter_3->Release ();

      return false;
    } // end IF

    result = CoCreateInstance ((windowHandle_in ? CLSID_VideoRenderer
                                                : CLSID_NullRenderer), NULL,
                               CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                               (void**)&filter_4);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                  (windowHandle_in ? ACE_TEXT ("CLSID_VideoRenderer")
                                   : ACE_TEXT ("CLSID_NullRenderer")),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      filter_2->Release ();
      if (filter_3)
        filter_3->Release ();

      return false;
    } // end IF
    ACE_ASSERT (filter_4);
    result =
      IGraphBuilder_in->AddFilter (filter_4,
                                   (windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_VIDEO_RENDERER
                                                    : MODULE_DEV_CAM_WIN32_FILTER_NAME_NULL_RENDERER));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      filter_p->Release ();
      filter_2->Release ();
      if (filter_3)
        filter_3->Release ();
      filter_4->Release ();

      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_VIDEO_RENDERER
                                                          : MODULE_DEV_CAM_WIN32_FILTER_NAME_NULL_RENDERER))));
  } // end IF
  ACE_ASSERT (filter_4);

  //result =
  //  ICaptureGraphBuilder2_in->RenderStream (//&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
  //                                          &PIN_CATEGORY_CAPTURE, NULL,
  //                                          filter_p,
  //                                          filter_2,
  //                                          //NULL,
  //                                          filter_4);
  //if (FAILED (result)) // E_INVALIDARG = 0x80070057, 0x80040217 = VFW_E_CANNOT_CONNECT ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ICaptureGraphBuilder::RenderStream(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  filter_p->Release ();
  //  filter_2->Release ();
  //  if (filter_3)
  //    filter_3->Release ();
  //  filter_4->Release ();

  //  return false;
  //} // end IF
  std::list<std::wstring> filter_pipeline;
  filter_pipeline.push_back (MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE);
  filter_pipeline.push_back (MODULE_DEV_CAM_WIN32_FILTER_NAME_GRABBER);
  if (windowHandle_in)
    filter_pipeline.push_back (decompressor_name);
  filter_pipeline.push_back ((windowHandle_in ? MODULE_DEV_CAM_WIN32_FILTER_NAME_VIDEO_RENDERER
                                              : MODULE_DEV_CAM_WIN32_FILTER_NAME_NULL_RENDERER));
  if (!Stream_Module_Device_Tools::connect (IGraphBuilder_in,
                                            filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::connect(), aborting\n")));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    if (filter_3)
      filter_3->Release ();
    filter_4->Release ();

    return false;
  } // end IF
  filter_p->Release ();
  filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  filter_4->Release ();

  return true;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ProtocolMessageType*
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_out = NULL;

  if (inherited::configuration_->streamConfiguration->messageAllocator)
  {
    try
    {
      // *TODO*: remove type inference
      message_out =
          static_cast<ProtocolMessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_out = NULL;
    }
  } // end IF
  else
  {
    ACE_NEW_NORETURN (message_out,
                      ProtocolMessageType (requestedSize_in));
  } // end ELSE
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
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
Stream_Dev_Cam_Source_DirectShow_T<LockType,
                                   SessionMessageType,
                                   ProtocolMessageType,
                                   ConfigurationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // step1: update session state
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

//  // step2: create session data object container
//  SessionDataContainerType* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    SessionDataContainerType (inherited::sessionData_,
//                                              false));
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
//    return false;
//  } // end IF

  // step3: send the statistic data downstream
//  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       *inherited::sessionData_,
                                       inherited::configuration_->streamConfiguration->messageAllocator);
}
