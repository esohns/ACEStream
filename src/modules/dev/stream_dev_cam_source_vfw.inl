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

#include "strmif.h"
#include "vfwmsgs.h"
//// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
//#include "wxdebug.h"

#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_defines.h"
#include "stream_lib_tools.h"

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
Stream_Dev_Cam_Source_VfW_T<ACE_SYNCH_USE,
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
                            MediaType>::Stream_Dev_Cam_Source_VfW_T (ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , inherited3 ()
 , capabilities_ ()
 , CBData_ ()
 , passive_ (false)
 , preview_ (STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_PREVIEW_MODE)
 , window_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::Stream_Dev_Cam_Source_VfW_T"));

  ACE_OS::memset (&capabilities_, 0, sizeof (struct tagCapDriverCaps));
  ACE_OS::memset (&CBData_, 0, sizeof (struct acestream_vfw_cbdata));
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
Stream_Dev_Cam_Source_VfW_T<ACE_SYNCH_USE,
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
                                   MediaType>::~Stream_Dev_Cam_Source_VfW_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::~Stream_Dev_Cam_Source_VfW_T"));

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
Stream_Dev_Cam_Source_VfW_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (likely (!passive_ && window_))
      if (unlikely (DestroyWindow (window_) == 0))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to DestroyWindow(0x%x), continuing\n"),
                    inherited::mod_->name (),
                    window_));
    passive_ = false;
    preview_ = STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_PREVIEW_MODE;
    window_ = NULL;
  } // end IF

  CBData_.allocator = allocator_in;
  CBData_.queue = inherited::msg_queue_;

  preview_ = configuration_in.preview;

  return inherited::initialize (configuration_in,
                                allocator_in);
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
Stream_Dev_Cam_Source_VfW_T<ACE_SYNCH_USE,
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
                            MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  message_inout->initialize (session_data_r.sessionId,
                             NULL); // keep data block
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
Stream_Dev_Cam_Source_VfW_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      HWND window_h = NULL;
      struct tagBITMAPINFO format_s;
      struct tagCaptureParms capture_parameters_s;
      unsigned int framerate_i;
      BOOL result_2 = FALSE;

      CBData_.sessionId = session_data_r.sessionId;

      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);

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
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    inherited::timerId_,
                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      if (inherited::configuration_->window.type == Common_UI_Window::TYPE_INVALID)
      {
        window_ =
          capCreateCaptureWindow (NULL,         // window name if pop-up
                                  0,            // window style
                                  0, 0, 0, 0,   // window position and dimensions
                                  HWND_MESSAGE, // parent window
                                  0);           // child ID
        if (unlikely (!window_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to capCreateCaptureWindow(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end IF
      else
      {
        window_ = inherited3::convert (inherited::configuration_->window);
        ACE_ASSERT (window_);
        passive_ = true;
      } // end ELSE
      window_h = window_;
      ACE_ASSERT (window_h);

      ACE_ASSERT (inherited::configuration_->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      if (unlikely (capDriverConnect (window_h,
                                      inherited::configuration_->deviceIdentifier.identifier._id) == FALSE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capDriverConnect(%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->deviceIdentifier.identifier._id));
        goto error;
      } // end IF

      if (unlikely (capDriverGetCaps (window_h,
                                      &capabilities_,
                                      sizeof (struct tagCapDriverCaps)) == FALSE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capDriverGetCaps(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: read driver capabilities\n"),
      //            inherited::mod_->name ()));

      ACE_OS::memset (&format_s, 0, sizeof (struct tagBITMAPINFO));
      Stream_MediaFramework_DirectShow_Tools::toBitmapInfo (media_type_s,
                                                            format_s);
      if (unlikely (capSetVideoFormat (window_h,
                                       &format_s,
                                       sizeof (struct tagBITMAPINFO)) == FALSE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capSetVideoFormat(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: set capture format\n"),
      //            inherited::mod_->name ()));

      framerate_i =
        Stream_MediaFramework_DirectShow_Tools::toFramerate (media_type_s);
      result_2 = capCaptureGetSetup (window_h,
                                     &capture_parameters_s,
                                     sizeof (struct tagCaptureParms));
      ACE_ASSERT (result_2 == TRUE);
      capture_parameters_s.dwRequestMicroSecPerFrame =
        (DWORD)(1.0e6 / (float)framerate_i);
      capture_parameters_s.fMakeUserHitOKToCapture = FALSE;
      //capture_parameters_s.wPercentDropForError = 10;
      capture_parameters_s.fYield = TRUE;
      // capture_parameters_s.dwIndexSize = 0;
      //capture_parameters_s.wChunkGranularity = 0;
      //capture_parameters_s.fUsingDOSMemory = FALSE;
      capture_parameters_s.wNumVideoRequested =
        STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_DEVICE_BUFFERS;
      capture_parameters_s.fCaptureAudio = FALSE;
      //capture_parameters_s.wNumAudioRequested = 0;
      //capture_parameters_s.vKeyAbort = VK_ESCAPE;
      capture_parameters_s.fAbortLeftMouse = FALSE;
      capture_parameters_s.fAbortRightMouse = FALSE;
      //capture_parameters_s.fLimitEnabled = FALSE;
      //capture_parameters_s.wTimeLimit = 0;
      //capture_parameters_s.fMCIControl = FALSE;
      //capture_parameters_s.fStepMCIDevice = FALSE;
      //capture_parameters_s.dwMCIStartTime = 0;
      //capture_parameters_s.dwMCIStopTime = 0;
      //capture_parameters_s.fStepCaptureAt2x = TRUE;
      //capture_parameters_s.wStepCaptureAverageFrames = 5;
      //capture_parameters_s.dwAudioBufferSize = 0;
      //capture_parameters_s.fDisableWriteCache = FALSE;
      capture_parameters_s.AVStreamMaster = AVSTREAMMASTER_NONE;
      if (unlikely (capCaptureSetSetup (window_h,
                                        &capture_parameters_s,
                                        sizeof (struct tagCaptureParms)) == FALSE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capCaptureSetSetup(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //          ACE_TEXT ("%s: set framerate to %u/s\n"),
      //          inherited::mod_->name (),
      //          framerate_i));

      SetWindowLongPtr (window_h, GWLP_USERDATA, (LONG_PTR)&CBData_);
      //if (unlikely (capSetUserData (window_h, &CBData_) == 0))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to capSetUserData(), aborting\n"),
      //              inherited::mod_->name ()));
      //  goto error;
      //} // end IF

      result_2 = capSetCallbackOnError (window_h, acestream_vfw_error_cb);
      ACE_ASSERT (result_2 == TRUE);
      result_2 = capSetCallbackOnStatus (window_h, acestream_vfw_status_cb);
      ACE_ASSERT (result_2 == TRUE);
      result_2 = capSetCallbackOnCapControl (window_h, acestream_vfw_control_cb);
      ACE_ASSERT (result_2 == TRUE);
      result_2 =
        (preview_ ? capSetCallbackOnFrame (window_h, acestream_vfw_video_cb)
                  : capSetCallbackOnVideoStream (window_h, acestream_vfw_video_cb));
      ACE_ASSERT (result_2 == TRUE);

      // *NOTE*: "...Enabling overlay mode automatically disables preview mode. ..."
      if (!preview_ &&
          capabilities_.fHasOverlay)
        result_2 = capOverlay (window_h, TRUE);
      else
        result_2 = capOverlay (window_h, FALSE);
      //ACE_ASSERT (result_2 == TRUE);
      
      if (preview_)
      {
        result_2 =
          capPreviewRate (window_h,
                          capture_parameters_s.dwRequestMicroSecPerFrame / 1000); // ms
        ACE_ASSERT (result_2 == TRUE);
        result_2 = capPreview (window_h, TRUE);
      } // end IF
      else
        result_2 = capCaptureSequenceNoFile (window_h);
      if (unlikely (result_2 == FALSE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to %s(), aborting\n"),
                    inherited::mod_->name (),
                    (preview_ ? ACE_TEXT ("capPreview") : ACE_TEXT ("capCaptureSequenceNoFile"))));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started capturing\n"),
                  inherited::mod_->name ()));

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      //if (capture_parameters_s.fYield == FALSE)
      //{
      //  inherited::threadCount_ = 1;
      //  inherited::TASK_BASE_T::start (NULL);
      //  inherited::threadCount_ = 0;
      //} // end IF

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
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

      BOOL result_2 = FALSE;
      if (likely (window_))
      {
        result_2 =
          (preview_ ? capPreview (window_, FALSE)
                    : capCaptureStop (window_));
        ACE_ASSERT (result_2 == TRUE);

        result_2 =
          (preview_ ? capSetCallbackOnFrame (window_, NULL)
                    : capSetCallbackOnVideoStream (window_, NULL));
        //ACE_ASSERT (result_2 == TRUE);
        result_2 = capSetCallbackOnCapControl (window_, NULL);
        //ACE_ASSERT (result_2 == TRUE);
        result_2 = capSetCallbackOnStatus (window_, NULL);
        ACE_ASSERT (result_2 == TRUE);
        result_2 = capSetCallbackOnError (window_, NULL);
        ACE_ASSERT (result_2 == TRUE);

        if (unlikely (capDriverDisconnect (window_) == FALSE))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to capDriverDisconnect(), continuing\n"),
                      inherited::mod_->name ()));
      } // end IF

      if (likely (!passive_))
      { ACE_ASSERT (window_);
        DestroyWindow (window_); window_ = NULL;
      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      {
        Common_ITask* itask_p = this; // *TODO*: is the no other way ?
        itask_p->stop (false,         // wait for completion ?
                       false);        // high priority ?
      }                               // end IF

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
Stream_Dev_Cam_Source_VfW_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::collect"));

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
int
Stream_Dev_Cam_Source_VfW_T<ACE_SYNCH_USE,
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
                            MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::svc"));

  // sanity check(s)
  if (!window_)
    return inherited::svc ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window message loop starting (id: %t)\n"),
              inherited::mod_->name ()));

  int result = 0;
  struct tagMSG message_s;
  BOOL result_2 = FALSE;

  while ((result_2 = GetMessage (&message_s, window_, 0, 0)) != FALSE)
  { 
    if (unlikely (result_2 == -1))
    {
      // handle the error and possibly exit
      result = -1;
      break;
    }

    TranslateMessage (&message_s);
    DispatchMessage (&message_s);
  } // end WHILE

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window message loop ending (id: %t)\n"),
              inherited::mod_->name ()));

  return result;
}
