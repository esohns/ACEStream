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

#ifndef NUMELMS
#if _WIN32_WINNT < 0x0600
#define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#else
#define NUMELMS(aa) ARRAYSIZE(aa)
#endif   
#endif

//#include <streams.h>
#include <strsafe.h>
#include <vfwmsgs.h>
#include <wxdebug.h>

#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_directshow_tools.h"

#include "stream_lib_defines.h"

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
          typename UserDataType>
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::Stream_Dev_Cam_Source_DirectShow_T (ISTREAM_T* stream_in,
                                                                                      bool autoStart_in,
                                                                                      enum Stream_HeadModuleConcurrency concurrency_in)
 : inherited (stream_in,
              autoStart_in,
              concurrency_in,
              true)
 , isFirst_ (true)
 , IAMDroppedFrames_ (NULL)
 , ICaptureGraphBuilder2_ (NULL)
 , IMediaControl_ (NULL)
 , IMediaEventEx_ (NULL)
 , ROTID_ (0)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::Stream_Dev_Cam_Source_DirectShow_T"));

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
          typename UserDataType>
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::~Stream_Dev_Cam_Source_DirectShow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::~Stream_Dev_Cam_Source_DirectShow_T"));

  int result = -1;

  if (ROTID_)
  {
    if (!Stream_Module_Device_DirectShow_Tools::removeFromROT (ROTID_))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                  inherited::mod_->name (),
                  ROTID_));
  } // end IF

//continue_:
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
  if (IAMDroppedFrames_)
    IAMDroppedFrames_->Release ();
  if (ICaptureGraphBuilder2_)
    ICaptureGraphBuilder2_->Release ();
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
          typename UserDataType>
bool
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                              Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::initialize"));

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
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    COM_initialized = true;
  } // end IF

  if (inherited::isInitialized_)
  {
    if (ROTID_)
    {
      if (!Stream_Module_Device_DirectShow_Tools::removeFromROT (ROTID_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                    inherited::mod_->name (),
                    ROTID_));
      ROTID_ = 0;
    } // end IF

//continue_:
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
    if (IAMDroppedFrames_)
    {
      IAMDroppedFrames_->Release ();
      IAMDroppedFrames_ = NULL;
    } // end IF
    if (ICaptureGraphBuilder2_)
    {
      ICaptureGraphBuilder2_->Release ();
      ICaptureGraphBuilder2_ = NULL;
    } // end IF
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(), aborting\n")));

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
          typename UserDataType>
void
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::handleSessionMessage"));

  int result = -1;
  IRunningObjectTable* ROT_p = NULL;

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
      std::string log_file_name;

      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
            itimer_manager_p->schedule_timer (&(inherited::statisticHandler_),                                          // event handler handle
                                              NULL,                                                                     // asynchronous completion token
                                              COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                              inherited::configuration_->statisticCollectionInterval);                  // interval
        if (inherited::timerId_ == -1)
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

      bool COM_initialized = false;
      bool release_session_data_format = false;
      bool is_running = false;
      bool remove_from_ROT = false;

      HRESULT result_2 = CoInitializeEx (NULL,
                                         (COINIT_MULTITHREADED    |
                                          COINIT_DISABLE_OLE1DDE  |
                                          COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      COM_initialized = true;

      IGraphBuilder* builder_p = NULL;
      ISampleGrabber* sample_grabber_p = NULL;
      ULONG reference_count = 0;
      if (inherited::configuration_->builder)
      {
        reference_count = inherited::configuration_->builder->AddRef ();
        builder_p = inherited::configuration_->builder;

        // sanity check(s)
        ACE_ASSERT (!IMediaControl_);
        ACE_ASSERT (!IMediaEventEx_);
        ACE_ASSERT (!IAMDroppedFrames_);

        // retrieve interfaces for media control and the video window
        result_2 =
          inherited::configuration_->builder->QueryInterface (IID_PPV_ARGS (&IMediaControl_));
        if (FAILED (result_2))
          goto error_3;
        result_2 =
          inherited::configuration_->builder->QueryInterface (IID_PPV_ARGS (&IMediaEventEx_));
        if (FAILED (result_2))
          goto error_3;

        IBaseFilter* filter_p = NULL;
        result_2 =
          inherited::configuration_->builder->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                                                &filter_p);
        if (FAILED (result_2))
          goto error_2;
        ACE_ASSERT (filter_p);
        result_2 = filter_p->QueryInterface (IID_PPV_ARGS (&IAMDroppedFrames_));
        if (FAILED (result_2))
          goto error_2;
        ACE_ASSERT (IAMDroppedFrames_);
        filter_p->Release ();
        filter_p = NULL;

        result_2 =
          inherited::configuration_->builder->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB,
                                                                &filter_p);
        if (FAILED (result_2))
          goto error_2;
        ACE_ASSERT (filter_p);
        result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                             (void**)&sample_grabber_p);
        if (FAILED (result_2))
          goto error_2;
        ACE_ASSERT (sample_grabber_p);
        filter_p->Release ();

        goto continue_;
error_3:
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
error_2:
        if (!filter_p)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IBaseFilter::QueryInterface(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (!ICaptureGraphBuilder2_);
      ACE_ASSERT (!IAMDroppedFrames_);

      // *TODO*: remove type inferences
      if (!initialize_DirectShow (inherited::configuration_->device,
                                  inherited::configuration_->window,
                                  ICaptureGraphBuilder2_,
                                  IAMDroppedFrames_,
                                  sample_grabber_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize_DirectShow(), aborting\n")));
        goto error;
      } // end IF
      ACE_ASSERT (ICaptureGraphBuilder2_);
      ACE_ASSERT (IAMDroppedFrames_);
      ACE_ASSERT (sample_grabber_p);

      result_2 = ICaptureGraphBuilder2_->GetFiltergraph (&builder_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF

continue_:
      ACE_ASSERT (builder_p);
      ACE_ASSERT (sample_grabber_p);
      ACE_ASSERT (IMediaControl_);
      ACE_ASSERT (IMediaEventEx_);

      if (!session_data_r.format)
      {
        if (!Stream_Module_Device_DirectShow_Tools::getOutputFormat (builder_p,
                                                                     MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB,
                                                                     session_data_r.format))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::getCaptureFormat(\"%s\"), aborting\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));
          goto error;
        } // end IF
        release_session_data_format = true;
      } // end IF
      ACE_ASSERT (session_data_r.format);

#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("output format: \"%s\"...\n"),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*session_data_r.format, true).c_str ())));

      log_file_name =
        Common_File_Tools::getLogDirectory (std::string (),
                                            0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += MODULE_LIB_DIRECTSHOW_LOGFILE_NAME;
      Stream_Module_Device_DirectShow_Tools::debug (builder_p,
                                                    log_file_name);
#endif

      // setup sample grabber
      result_2 = sample_grabber_p->SetBufferSamples (false);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ISampleGrabber::SetBufferSamples(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      result_2 = sample_grabber_p->SetCallback (this, 0);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      sample_grabber_p->Release ();
      sample_grabber_p = NULL;

      // start displaying video data
      result_2 = IMediaControl_->Run ();
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMediaControl::Run(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      is_running = true;

      // register graph in the ROT (so graphedt.exe can see it)
      ACE_ASSERT (!ROTID_);
      if (!Stream_Module_Device_DirectShow_Tools::addToROT (builder_p,
                                                            ROTID_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::addToROT(), aborting\n")));
        goto error;
      } // end IF
      ACE_ASSERT (ROTID_);
      remove_from_ROT = true;

      builder_p->Release ();

      break;

error:
      if (remove_from_ROT)
      { ACE_ASSERT (ROTID_);
        if (!Stream_Module_Device_DirectShow_Tools::removeFromROT (ROTID_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      ROTID_));
        ROTID_ = 0;
      } // end IF
      if (is_running)
      {
        result_2 = IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      } // end IF

      if (ICaptureGraphBuilder2_)
      {
        if (IAMDroppedFrames_)
        {
          IAMDroppedFrames_->Release ();
          IAMDroppedFrames_ = NULL;
        } // end IF
        ICaptureGraphBuilder2_->Release ();
        ICaptureGraphBuilder2_ = NULL;
      } // end IF

      if (builder_p)
        builder_p->Release ();
      if (sample_grabber_p)
        sample_grabber_p->Release ();

      if (release_session_data_format)
        Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_r.format);

      if (COM_initialized)
        CoUninitialize ();

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      // *TODO*: remove type inference
      //ACE_ASSERT (inherited::configuration_->builder);

      if (inherited::timerId_ != -1)
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      bool COM_initialized = false;
      HRESULT result_2 = CoInitializeEx (NULL,
                                         (COINIT_MULTITHREADED    |
                                          COINIT_DISABLE_OLE1DDE  |
                                          COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      COM_initialized = true;

      // deregister graph from the ROT ?
      if (ROTID_)
      {
        if (!Stream_Module_Device_DirectShow_Tools::removeFromROT (ROTID_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      ROTID_));
        ROTID_ = 0;
      } // end IF

//continue_2:
      if (IMediaEventEx_)
      {
        result_2 = IMediaEventEx_->SetNotifyWindow (NULL,
                                                    0,
                                                    NULL);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        IMediaEventEx_->Release ();
        IMediaEventEx_ = NULL;
      } // end IF

      if (IMediaControl_)
      {
        // stop previewing video data
        // *NOTE*: there may be still be data messages to be delivered at this
        //         point
        // *TODO*: flush the queue ?
        //result_2 = IMediaControl_->StopWhenReady ();
        result_2 = IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        else
        {
          enum _FilterState graph_state;
          result_2 =
            IMediaControl_->GetState (INFINITE,
                                      (OAFilterState*)&graph_state);
          if (FAILED (result_2)) // VFW_S_STATE_INTERMEDIATE: 0x00040237
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IMediaControl::GetState(): \"%s\", continuing\n"),
                        ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
          else
          {
            ACE_ASSERT (graph_state == State_Stopped);
          } // end ELSE
        } // end ELSE

        IMediaControl_->Release ();
        IMediaControl_ = NULL;
      } // end IF

      if (IAMDroppedFrames_)
      {
        IAMDroppedFrames_->Release ();
        IAMDroppedFrames_ = NULL;
      } // end IF
      if (ICaptureGraphBuilder2_)
      {
        ICaptureGraphBuilder2_->Release ();
        ICaptureGraphBuilder2_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      inherited::sessionEndProcessed_ = true;
      if (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
        inherited::TASK_BASE_T::stop (false,  // wait for completion ?
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
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (IAMDroppedFrames_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  // step1: collect data
  HRESULT result = E_FAIL;
  long average_frame_size = 0;

  // *NOTE*: "...Some filters that expose this interface do not implement the
  //         GetDroppedInfo or GetAverageFrameSize method."
  result = IAMDroppedFrames_->GetAverageFrameSize (&average_frame_size);
  if (FAILED (result))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to IAMDroppedFrames::GetAverageFrameSize(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  ACE_UNUSED_ARG (average_frame_size);

  long captured_frames = 0;
  result = IAMDroppedFrames_->GetNumNotDropped (&captured_frames);
  //result = IAMDroppedFrames_->GetNumNotDropped (reinterpret_cast<long*> (&(data_out.capturedFrames)));
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMDroppedFrames::GetNumNotDropped(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  result =
    IAMDroppedFrames_->GetNumDropped (reinterpret_cast<long*> (&(data_out.droppedFrames)));
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMDroppedFrames::GetNumDropped(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

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
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

// ------------------------------------

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
          typename UserDataType>
ULONG
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::Release"));

  // Decrement the object's internal counter.
  ULONG ulRefCount = InterlockedDecrement (&referenceCount_);
  if (0 == referenceCount_)
    delete this;

  return ulRefCount;
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
          typename UserDataType>
HRESULT
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::QueryInterface (REFIID riid_in,
                                                                  void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::QueryInterface"));

  // sanity check(s)
  CheckPointer (interface_out, E_POINTER);

  // Always set out parameter to NULL, validating it first.
  *interface_out = NULL;
  if (riid_in == IID_IUnknown                        ||
      riid_in == IID_IMemAllocatorNotifyCallbackTemp ||
      riid_in == IID_ISampleGrabberCB)
  {
    // Increment the reference count and return the pointer.
    *interface_out = (LPVOID)this;
    AddRef ();

    return NOERROR;
  } // end IF

  return E_NOINTERFACE;
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
          typename UserDataType>
HRESULT
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::NotifyRelease (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::NotifyRelease"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
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
          typename UserDataType>
HRESULT
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::BufferCB (double sampleTime_in,
                                                            BYTE* buffer_in,
                                                            long bufferLen_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::BufferCB"));

  ACE_UNUSED_ARG (sampleTime_in);
  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferLen_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
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
          typename UserDataType>
HRESULT
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::SampleCB (double sampleTime_in,
                                                            IMediaSample* IMediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::SampleCB"));

  ACE_UNUSED_ARG (sampleTime_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  int result = -1;

  ULONG reference_count = IMediaSample_in->AddRef ();

  DataMessageType* message_p = NULL;
  try {
    message_p = dynamic_cast<DataMessageType*> (IMediaSample_in);
  } catch (...) {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to dynamic_cast<DataMessageType*>(0x%@), continuing\n"),
    //            IMediaSample_in));
    message_p = NULL;
  }
  if (!message_p)
  {
    // *TODO*: remove type inference
    message_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);
    typename DataMessageType::DATA_T& data_r =
      const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
    data_r.sample = IMediaSample_in;
    data_r.sampleTime = sampleTime_in;

    BYTE* buffer_p = NULL;
    unsigned int size = static_cast<unsigned int> (IMediaSample_in->GetSize ());
    HRESULT result_2 = IMediaSample_in->GetPointer (&buffer_p);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaSample::GetPointer(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      return result_2;
    } // end IF
    ACE_ASSERT (buffer_p);
    message_p->base (reinterpret_cast<char*> (buffer_p),
                     size,
                     ACE_Message_Block::DONT_DELETE);
    message_p->wr_ptr (size);
  } // end IF

  result = inherited::putq (message_p, NULL);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));

    // clean up
    message_p->release ();

    return E_FAIL;
  } // end IF

  return S_OK;
}

// ------------------------------------

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//int
//Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::svc"));
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
          typename UserDataType>
bool
Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_USE,
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
                                   UserDataType>::initialize_DirectShow (const std::string& deviceName_in,
                                                                         const HWND windowHandle_in,
                                                                         ICaptureGraphBuilder2*& ICaptureGraphBuilder2_out,
                                                                         IAMDroppedFrames*& IAMDroppedFrames_out,
                                                                         ISampleGrabber*& ISampleGrabber_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_DirectShow_T::initialize_DirectShow"));

  // sanity check(s)
  ACE_ASSERT (!ICaptureGraphBuilder2_out);
  ACE_ASSERT (!IAMDroppedFrames_out);
  ACE_ASSERT (!ISampleGrabber_out);

  HRESULT result =
    CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&ICaptureGraphBuilder2_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_CaptureGraphBuilder2): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (ICaptureGraphBuilder2_out);

  Stream_Module_Device_DirectShow_Graph_t graph_layout;
  Stream_Module_Device_DirectShow_GraphConfiguration_t graph_configuration;
  struct Stream_Module_Device_DirectShow_GraphConfigurationEntry graph_entry;
  IGraphBuilder* graph_builder_p = NULL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::loadDeviceGraph (deviceName_in,
                                                               CLSID_VideoInputDeviceCategory,
                                                               graph_builder_p,
                                                               buffer_negotiation_p,
                                                               stream_config_p,
                                                               graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (deviceName_in.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graph_builder_p);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (stream_config_p);

  result = ICaptureGraphBuilder2_out->SetFiltergraph (graph_builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICaptureGraphBuilder2::SetFiltergraph(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  //// *NOTE*: (re-)Connect()ion of the video renderer input pin fails
  ////         consistently, so reuse is not feasible
  ////         --> rebuild the whole graph from scratch each time
  //if (!Stream_Module_Device_Tools::reset (IGraphBuilder_in))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::reset(), aborting\n")));
  //  return false;
  //} // end IF

  // retrieve interfaces for media control and the video window
  result = graph_builder_p->QueryInterface (IID_PPV_ARGS (&IMediaControl_));
  if (FAILED (result))
    goto error_2;
  result = graph_builder_p->QueryInterface (IID_PPV_ARGS (&IMediaEventEx_));
  if (FAILED (result))
    goto error_2;

  goto continue_;

error_2:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  goto error;

continue_:
  ACE_ASSERT (IMediaControl_);
  ACE_ASSERT (IMediaEventEx_);

  // set the window handle used to process graph events
  result =
    IMediaEventEx_->SetNotifyWindow ((OAHWND)windowHandle_in,
                                     MODULE_LIB_DIRECTSHOW_WM_GRAPHNOTIFY_EVENT, 0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  IBaseFilter* filter_p = NULL;
  result =
    graph_builder_p->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                       &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result = filter_p->QueryInterface (IID_PPV_ARGS (&IAMDroppedFrames_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IAMDroppedFrames): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IAMDroppedFrames_out);

  // decompress ?
  struct _AMMediaType* media_type_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::getCaptureFormat (graph_builder_p,
                                                                media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  struct _GUID media_subtype = media_type_p->subtype;
  Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
  struct _GUID decompressor_guid = CLSID_Colour;
  LPCWSTR decompressor_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  bool needs_converter = false;
  if (media_subtype == MEDIASUBTYPE_YUY2)
  {
    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
    decompressor_guid = CLSID_AVIDec;
    decompressor_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
  } // end IF
  else if (media_subtype == MEDIASUBTYPE_MJPG)
  {
    decompressor_guid = CLSID_MjpegDec;
    decompressor_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (media_subtype, false).c_str ())));
    goto error;
  } // end ELSE
  IBaseFilter* filter_2 = NULL;
  if (!windowHandle_in) goto continue_2;
  result =
    graph_builder_p->FindFilterByName (decompressor_name,
                                       &filter_2);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (decompressor_name),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance (decompressor_guid, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoCreateInstance() decompressor: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (media_subtype, false).c_str ()),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_2);
    result =
      graph_builder_p->AddFilter (filter_2,
                                  decompressor_name);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added \"%s\"\n"),
    //            ACE_TEXT_WCHAR_TO_TCHAR (decompressor_name)));
  } // end IF
  ACE_ASSERT (filter_2);

  // grab
  IBaseFilter* filter_3 = NULL;
  result =
    graph_builder_p->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB,
                                       &filter_3);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_3));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_SampleGrabber): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_3);
    result = graph_builder_p->AddFilter (filter_3,
                                         MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added \"%s\"\n"),
    //            ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));
  } // end IF
  ACE_ASSERT (filter_3);
  result = filter_3->QueryInterface (IID_ISampleGrabber,
                                     (void**)&ISampleGrabber_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (ISampleGrabber_out);

  // render to a window (GtkDrawingArea) ?
continue_2:
  IBaseFilter* filter_4 = NULL;
  result =
    graph_builder_p->FindFilterByName ((windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
                                                        : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL),
                                       &filter_4);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
                                                            : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL)),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance ((windowHandle_in ? CLSID_VideoRenderer
                                                : CLSID_NullRenderer), NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_4));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                  (windowHandle_in ? ACE_TEXT ("CLSID_VideoRenderer")
                                   : ACE_TEXT ("CLSID_NullRenderer")),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_4);
    result =
      graph_builder_p->AddFilter (filter_4,
                                  (windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
                                                   : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added \"%s\"\n"),
    //            ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
    //                                                      : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL))));
  } // end IF
  ACE_ASSERT (filter_4);

  struct _AllocatorProperties allocator_properties;
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  allocator_properties.cbAlign = 1;
  allocator_properties.cbBuffer = -1; // <-- use default
  allocator_properties.cbPrefix = -1; // <-- use default
  allocator_properties.cBuffers =
    MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
  result =
    buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF

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
  //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  //  goto error_2;
  //} // end IF
  graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  graph_configuration.push_back (graph_entry);
  graph_entry.filterName = decompressor_name;
  graph_configuration.push_back (graph_entry);
  graph_entry.filterName = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB;
  graph_configuration.push_back (graph_entry);
  graph_entry.filterName =
    (windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
                     : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL);
  graph_configuration.push_back (graph_entry);
  if (!Stream_Module_Device_DirectShow_Tools::connect (graph_builder_p,
                                                       graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::connect(), aborting\n")));
    goto error;
  } // end IF

  // debug info
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result = buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              allocator_properties.cBuffers,
              allocator_properties.cbBuffer,
              allocator_properties.cbAlign,
              allocator_properties.cbPrefix));

  // clean up
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (filter_4)
    filter_4->Release ();

  // *NOTE*: apparently, this is necessary
  //         (see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd373396(v=vs.85).aspx)
  graph_builder_p->Release ();
  graph_builder_p = NULL;
  stream_config_p->Release ();

  return true;

error:
  if (graph_builder_p)
    graph_builder_p->Release ();
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  if (stream_config_p)
    stream_config_p->Release ();

  if (ISampleGrabber_out)
  {
    ISampleGrabber_out->Release ();
    ISampleGrabber_out = NULL;
  } // end IF
  if (IAMDroppedFrames_out)
  {
    IAMDroppedFrames_out->Release ();
    IAMDroppedFrames_out = NULL;
  } // end IF
  if (ICaptureGraphBuilder2_out)
  {
    ICaptureGraphBuilder2_out->Release ();
    ICaptureGraphBuilder2_out = NULL;
  } // end IF

  return false;
}
