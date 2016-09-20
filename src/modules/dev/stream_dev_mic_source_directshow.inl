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

#include "strsafe.h"

#include "ace/Log_Msg.h"

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
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::Stream_Dev_Mic_Source_DirectShow_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                                                bool autoStart_in)
 : inherited (lock_in,      // lock handle
              autoStart_in, // auto-start ?
              true)         // generate session messages ?
 //, isFirst_ (true)
 , IAMDroppedFrames_ (NULL)
 , ICaptureGraphBuilder2_ (NULL)
 , IMediaControl_ (NULL)
 , IMediaEventEx_ (NULL)
 //, manageCOM_ (false)
 , ROTID_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::Stream_Dev_Mic_Source_DirectShow_T"));

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
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::~Stream_Dev_Mic_Source_DirectShow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::~Stream_Dev_Mic_Source_DirectShow_T"));

  int result = -1;

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
  if (IAMDroppedFrames_)
    IAMDroppedFrames_->Release ();
  if (ICaptureGraphBuilder2_)
    ICaptureGraphBuilder2_->Release ();

  //if (manageCOM_)
  //  CoUninitialize ();
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
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::initialize"));

  bool result = false;
  HRESULT result_2 = E_FAIL;

  //// initialize COM ?
  //if (isFirst_)
  //{
  //  isFirst_ = false;

  //  if (configuration_in.manageCOM)
  //  {
  //    result_2 = CoInitializeEx (NULL,
  //                               (COINIT_MULTITHREADED    |
  //                                COINIT_DISABLE_OLE1DDE  |
  //                                COINIT_SPEED_OVER_MEMORY));
  //    if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
  //                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  //    manageCOM_ = true;
  //  } // end IF
  //} // end IF

  if (inherited::isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    if (ROTID_)
    {
      IRunningObjectTable* ROT_p = NULL;
      result_2 = GetRunningObjectTable (0, &ROT_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // clean up
        ROTID_ = 0;

        goto continue_;
      } // end IF
      ACE_ASSERT (ROT_p);
      result_2 = ROT_p->Revoke (ROTID_);
      if (FAILED (result_2))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d) \"%s\", continuing\n"),
                    ROTID_,
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      ROT_p->Release ();
      ROTID_ = 0;
    } // end IF

continue_:
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

    manageCOM_ = false;
  } // end IF
  manageCOM_ = configuration_in.manageCOM;

  result = inherited::initialize (configuration_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

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
          typename StatisticContainerType>
void
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::handleSessionMessage"));

  int result = -1;
  IRunningObjectTable* ROT_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

#if defined (_DEBUG)
      std::string media_type_string, log_file_name;
#endif

      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerID_ == -1);
        ACE_Event_Handler* handler_p =
          &(inherited::statisticCollectionHandler_);
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

      bool COM_initialized = false;
      bool is_running = false;
      HRESULT result_2 = E_FAIL;

      if (manageCOM_)
      {
        result_2 = CoInitializeEx (NULL,
                                  (COINIT_MULTITHREADED    |
                                   COINIT_DISABLE_OLE1DDE  |
                                   COINIT_SPEED_OVER_MEMORY));
        if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        COM_initialized = true;
      } // end IF

      IGraphBuilder* builder_p = NULL;
      ISampleGrabber* sample_grabber_p = NULL;
      ULONG reference_count = 0;

      if (inherited::configuration_->builder)
      {
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
          inherited::configuration_->builder->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO,
                                                                &filter_p);
        if (FAILED (result_2))
          goto error_2;
        ACE_ASSERT (filter_p);
        result_2 = filter_p->QueryInterface (IID_PPV_ARGS (&IAMDroppedFrames_));
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IAMDroppedFrames): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        filter_p->Release ();

        reference_count = inherited::configuration_->builder->AddRef ();
        builder_p = inherited::configuration_->builder;

        goto continue_;
error_3:
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
error_2:
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (!ICaptureGraphBuilder2_);
      ACE_ASSERT (!IAMDroppedFrames_);

      // *TODO*: remove type inferences
      if (!initialize_DirectShow (inherited::configuration_->device,
                                  inherited::configuration_->audioOutput,
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

      result_2 = sample_grabber_p->SetBufferSamples (false);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ISampleGrabber::SetBufferSamples(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      result_2 = sample_grabber_p->SetCallback (this, 0);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      sample_grabber_p->Release ();

      result_2 = ICaptureGraphBuilder2_->GetFiltergraph (&builder_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      reference_count = builder_p->AddRef ();
      inherited::configuration_->builder = builder_p;

continue_:
      ACE_ASSERT (builder_p);
      ACE_ASSERT (IMediaControl_);
      ACE_ASSERT (IMediaEventEx_);

      //ACE_ASSERT (!session_data_r.format);
      //if (!Stream_Module_Device_Tools::getOutputFormat (builder_p,
      //                                                  session_data_r.format))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
      //  goto error;
      //} // end IF
      ACE_ASSERT (session_data_r.format);

#if defined (_DEBUG)
      media_type_string =
        Stream_Module_Device_Tools::mediaTypeToString (*session_data_r.format);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("output format: \"%s\"...\n"),
                  ACE_TEXT (media_type_string.c_str ())));

      log_file_name =
        Common_File_Tools::getLogDirectory (ACE_TEXT_ALWAYS_CHAR (""),
                                            0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
      Stream_Module_Device_Tools::debug (builder_p,
                                         log_file_name);
#endif

      // start capturing audio data
      result_2 = IMediaControl_->Run ();
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMediaControl::Run(): \"%s\", aborting\n"),
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
                    ACE_TEXT ("failed to GetRunningObjectTable(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (ROT_p);
      result_2 =
        ::StringCchPrintfW (buffer, NUMELMS (buffer),
                            ACE_TEXT_ALWAYS_WCHAR ("FilterGraph %08x [PID: %08x]\0"),
                            (DWORD_PTR)builder_p, ACE_OS::getpid ());
      result_2 = CreateItemMoniker (ACE_TEXT_ALWAYS_WCHAR ("!"), buffer,
                                    &moniker_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CreateItemMoniker(): \"%s\", aborting\n"),
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
                         builder_p, moniker_p,
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
      builder_p->Release ();
      builder_p = NULL;

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

      if (IAMDroppedFrames_)
      {
        IAMDroppedFrames_->Release ();
        IAMDroppedFrames_ = NULL;
      } // end IF

      if (builder_p)
        builder_p->Release ();

      if (ICaptureGraphBuilder2_)
      {
        ICaptureGraphBuilder2_->Release ();
        ICaptureGraphBuilder2_ = NULL;
      } // end IF

      //if (session_data_r.format)
      //  Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);

      if (manageCOM_ && COM_initialized)
        CoUninitialize ();

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

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

      // *TODO*: (without more overhead,) this can only assme that the calling
      //         thread is the same as the one that dispatched the session begin
      //         message, which is not a safe assumption
      bool COM_initialized = false;
      HRESULT result_2 = E_FAIL;
      
      if (manageCOM_)
      {
        result_2 = CoInitializeEx (NULL,
                                   (COINIT_MULTITHREADED    |
                                    COINIT_DISABLE_OLE1DDE  |
                                    COINIT_SPEED_OVER_MEMORY));
        if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        COM_initialized = true;
      } // end IF

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

          goto continue_2;
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

continue_2:
      if (IMediaEventEx_)
      {
        result_2 = IMediaEventEx_->SetNotifyWindow (NULL,
                                                    0,
                                                    NULL);
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
        // *NOTE*: there may be still be data messages to be delivered at this
        //         point
        // *TODO*: flush the queue ?
        //result_2 = IMediaControl_->StopWhenReady ();
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

      //if (session_data_r.format)
      //  Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);

      if (manageCOM_ && COM_initialized)
        CoUninitialize ();

      inherited::stop (false,
                       true);

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
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::collect"));

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  ACE_UNUSED_ARG (average_frame_size);

  long captured_frames = 0;
  result = IAMDroppedFrames_->GetNumNotDropped (&captured_frames);
  //result = IAMDroppedFrames_->GetNumNotDropped (reinterpret_cast<long*> (&(data_out.capturedFrames)));
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMDroppedFrames::GetNumNotDropped(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  result = IAMDroppedFrames_->GetNumDropped (reinterpret_cast<long*> (&(data_out.droppedMessages)));
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMDroppedFrames::GetNumDropped(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

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
//Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::report"));
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
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::NotifyRelease (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::NotifyRelease"));

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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::BufferCB (double sampleTime_in,
                                                                      BYTE* buffer_in,
                                                                      long bufferLen_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::BufferCB"));

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
          typename StatisticContainerType>
HRESULT
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::SampleCB (double sampleTime_in,
                                                                      IMediaSample* IMediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::SampleCB"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

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
      inherited::allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);
    typename DataMessageType::DATA_T& data_r =
      const_cast<typename DataMessageType::DATA_T&> (message_p->get ());
    data_r.sample = IMediaSample_in;
    data_r.sampleTime = sampleTime_in;

    long size = IMediaSample_in->GetSize ();
    BYTE* buffer_p = NULL;
    HRESULT result_2 = IMediaSample_in->GetPointer (&buffer_p);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMediaSample::GetPointer(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      return result_2;
    } // end IF
    ACE_ASSERT (buffer_p);
    message_p->base (reinterpret_cast<char*> (buffer_p),
                     size,
                     ACE_Message_Block::DONT_DELETE);
    message_p->wr_ptr (size);
  } // end IF

  if (inherited::configuration_->sinus)
  {
    static double sinus_phase = 0.0;
    ACE_ASSERT (inherited::configuration_->format->formattype == FORMAT_WaveFormatEx);
    struct tWAVEFORMATEX* waveformatex_p =
      (struct tWAVEFORMATEX*)inherited::configuration_->format->pbFormat;
    Stream_Module_Decoder_Tools::sinus (inherited::configuration_->sinusFrequency,
                                        waveformatex_p->nSamplesPerSec,
                                        waveformatex_p->nBlockAlign,
                                        waveformatex_p->nChannels,
                                        message_p->rd_ptr (),
                                        (message_p->length () / waveformatex_p->nBlockAlign),
                                        sinus_phase);
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
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::QueryInterface (const IID&,
                                                                            void**)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::QueryInterface"));

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
          typename StatisticContainerType>
ULONG
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::AddRef"));

  return 1;
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
ULONG
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::Release"));

  return 0;
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//int
//Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::svc"));
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
          typename StatisticContainerType>
bool
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType>::initialize_DirectShow (const std::string& deviceName_in,
                                                                                   int audioOutput_in,
                                                                                   ICaptureGraphBuilder2*& ICaptureGraphBuilder2_out,
                                                                                   IAMDroppedFrames*& IAMDroppedFrames_out,
                                                                                   ISampleGrabber*& ISampleGrabber_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::initialize_DirectShow"));

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (ICaptureGraphBuilder2_out);

  std::list<std::wstring> filter_pipeline;

  IGraphBuilder* graph_builder_p = NULL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;
  if (!Stream_Module_Device_Tools::loadDeviceGraph (deviceName_in,
                                                    CLSID_AudioInputDeviceCategory,
                                                    graph_builder_p,
                                                    buffer_negotiation_p,
                                                    stream_config_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  goto error;

continue_:
  ACE_ASSERT (IMediaControl_);
  ACE_ASSERT (IMediaEventEx_);

  // set the window handle used to process graph events
  //result =
  //  IMediaEventEx_->SetNotifyWindow ((OAHWND)windowHandle_in,
  //                                   MODULE_DEV_CAM_UI_WIN32_WM_GRAPHNOTIFY, 0);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  IBaseFilter* filter_p = NULL;
  result =
    graph_builder_p->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO,
                                       &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result = filter_p->QueryInterface (IID_PPV_ARGS (&IAMDroppedFrames_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IAMDroppedFrames): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IAMDroppedFrames_out);

  //// convert PCM to WAV ?
  //struct _AMMediaType* media_type_p = NULL;
  //if (!Stream_Module_Device_Tools::getCaptureFormat (graph_builder_p,
  //                                                   media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (media_type_p);
  //struct _GUID media_subtype = media_type_p->subtype;
  //Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  //struct _GUID converter_guid = CLSID_Colour;
  //LPCWSTR converter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_PCM;
  //bool needs_converter = false;
  //if (media_subtype == MEDIASUBTYPE_PCM)
  //{} // end IF
  //else if (media_subtype == MEDIASUBTYPE_WAV)
  //{} // end IF
  //else
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
  //              ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (media_subtype).c_str ())));
  //  goto error;
  //} // end ELSE
  IBaseFilter* filter_2 = NULL;
  //if (!windowHandle_in) goto continue_2;
  //result =
  //  graph_builder_p->FindFilterByName (converter_name,
  //                                     &filter_2);
  //if (FAILED (result))
  //{
  //  if (result != VFW_E_NOT_FOUND)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
  //                ACE_TEXT_WCHAR_TO_TCHAR (converter_name),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    goto error;
  //  } // end IF

  //  result = CoCreateInstance (converter_guid, NULL,
  //                             CLSCTX_INPROC_SERVER,
  //                             IID_PPV_ARGS (&filter_2));
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to CoCreateInstance() converter: \"%s\", aborting\n"),
  //                ACE_TEXT (Stream_Module_Device_Tools::mediaSubTypeToString (media_subtype).c_str ()),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    goto error;
  //  } // end IF
  //  ACE_ASSERT (filter_2);
  //  result =
  //    graph_builder_p->AddFilter (filter_2,
  //                                converter_name);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    goto error;
  //  } // end IF
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("added \"%s\"...\n"),
  //              ACE_TEXT_WCHAR_TO_TCHAR (converter_name)));
  //} // end IF
  //ACE_ASSERT (filter_2);

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
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_3));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_SampleGrabber): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_3);
    result = graph_builder_p->AddFilter (filter_3,
                                         MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));
  } // end IF
  ACE_ASSERT (filter_3);
  result = filter_3->QueryInterface (IID_ISampleGrabber,
                                     (void**)&ISampleGrabber_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (ISampleGrabber_out);

  // render to a sound card ?
//continue_2:
  IBaseFilter* filter_4 = NULL;
  result =
    graph_builder_p->FindFilterByName ((audioOutput_in ? MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                                       : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL),
                                       &filter_4);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR ((audioOutput_in ? MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                                           : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL)),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance ((audioOutput_in ? CLSID_AudioRender
                                               : CLSID_NullRenderer), NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_4));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                  (audioOutput_in ? ACE_TEXT ("CLSID_AudioRender")
                                  : ACE_TEXT ("CLSID_NullRenderer")),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_4);
    result =
      graph_builder_p->AddFilter (filter_4,
                                  (audioOutput_in ? MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                                  : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added \"%s\"...\n"),
                ACE_TEXT_WCHAR_TO_TCHAR ((audioOutput_in ? MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                                         : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL))));
  } // end IF
  ACE_ASSERT (filter_4);

//  // render to a window (GtkDrawingArea) ?
//continue_2:
//  IBaseFilter* filter_4 = NULL;
//  result =
//    graph_builder_p->FindFilterByName ((windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                        : MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_NULL),
//                                       &filter_4);
//  if (FAILED (result))
//  {
//    if (result != VFW_E_NOT_FOUND)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                            : MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_NULL)),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//      goto error;
//    } // end IF
//
//    result = CoCreateInstance ((windowHandle_in ? CLSID_VideoRenderer
//                                                : CLSID_NullRenderer), NULL,
//                               CLSCTX_INPROC_SERVER,
//                               IID_PPV_ARGS (&filter_4));
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
//                  (windowHandle_in ? ACE_TEXT ("CLSID_VideoRenderer")
//                                   : ACE_TEXT ("CLSID_NullRenderer")),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//      goto error;
//    } // end IF
//    ACE_ASSERT (filter_4);
//    result =
//      graph_builder_p->AddFilter (filter_4,
//                                  (windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                   : MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_NULL));
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//      goto error;
//    } // end IF
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("added \"%s\"...\n"),
//                ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                          : MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_NULL))));
//  } // end IF
//  ACE_ASSERT (filter_4);
//
  struct _AllocatorProperties allocator_properties;
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  allocator_properties.cbAlign = 1;
  allocator_properties.cbBuffer = 0;
  allocator_properties.cbPrefix = 0;
  allocator_properties.cBuffers =
    MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
//  result = buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//    goto error;
//  } // end IF

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
  //  goto error_2;
  //} // end IF
  filter_pipeline.push_back (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO);
  //filter_pipeline.push_back (converter_name);
  filter_pipeline.push_back (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB);
  filter_pipeline.push_back ((audioOutput_in ? MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                             : MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL));
  if (!Stream_Module_Device_Tools::connect (graph_builder_p,
                                            filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::connect(), aborting\n")));
    goto error;
  } // end IF

  // debug info
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result = buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
