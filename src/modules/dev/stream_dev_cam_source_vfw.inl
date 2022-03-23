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
#include "wxdebug.h"

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
 , CBData_ ()
 , isFirst_ (true)
 , passive_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::Stream_Dev_Cam_Source_VfW_T"));

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
    isFirst_ = true;
    if (likely (!passive_ && window_))
      if (unlikely (DestroyWindow (window_) == 0))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to DestroyWindow(0x%x), continuing\n"),
                    inherited::mod_->name (),
                    window_));
    passive_ = false;
    window_ = NULL;
  } // end IF

  CBData_.allocator = allocator_in;
  CBData_.queue = inherited::msg_queue_;

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
      Common_Image_Resolution_t resolution_s;
      HWND window_h = NULL;
      struct tagBITMAPINFO format_s;
      struct tagCaptureParms capture_parameters_s;
      unsigned int framerate_i;
      char device_name_a[BUFSIZ] = {0}, device_version_a[BUFSIZ] = {0};

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

#if defined (_DEBUG)
      for (UINT i = 0; i < 10; i++)
        if (capGetDriverDescription (i,
                                     device_name_a, sizeof (char[BUFSIZ]),
                                     device_version_a, sizeof (char[BUFSIZ])))
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: found device (index: %u) \"%s\" (version: \"%s\")\n"),
                      inherited::mod_->name (),
                      i,
                      ACE_TEXT (device_name_a),
                      ACE_TEXT (device_version_a)));
#endif // _DEBUG

      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      if (!inherited::configuration_->window)
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
                      ACE_TEXT ("%s: failed to capCreateCaptureWindow(%u,%u), aborting\n"),
                      inherited::mod_->name (),
                      resolution_s.cx, resolution_s.cy));
          goto error;
        } // end IF
      } // end IF
      else
      {
        inherited3::getWindowType (inherited::configuration_->window,
                                   window_);
        ACE_ASSERT (window_);
        passive_ = true;
      } // end ELSE
      window_h = window_;
      ACE_ASSERT (window_h);

      if (unlikely (capDriverConnect (window_h, 0) == 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capDriverConnect(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      ACE_OS::memset (&format_s, 0, sizeof (struct tagBITMAPINFO));
      //Stream_MediaFramework_DirectShow_Tools::toBitmapInfo (media_type_s,
      //                                                      format_s);
      if (unlikely (capGetVideoFormat (window_h,
                                       &format_s,
                                       sizeof (struct tagBITMAPINFO)) == 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capGetVideoFormat(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      format_s.bmiHeader.biWidth = resolution_s.cx;
      format_s.bmiHeader.biHeight = resolution_s.cy;
      if (unlikely (capSetVideoFormat (window_h,
                                       &format_s,
                                       sizeof (struct tagBITMAPINFO)) == 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capSetVideoFormat(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: set capture format\n"),
                  inherited::mod_->name ()));

      framerate_i =
        Stream_MediaFramework_DirectShow_Tools::toFramerate (media_type_s);
      capCaptureGetSetup (window_h,
                          &capture_parameters_s,
                          sizeof (struct tagCaptureParms));
      capture_parameters_s.dwRequestMicroSecPerFrame =
        (DWORD)(1.0e6 / (float)framerate_i);
      capture_parameters_s.fMakeUserHitOKToCapture = FALSE;
      //capture_parameters_s.wPercentDropForError = 10;
      capture_parameters_s.fYield = TRUE;
      //capture_parameters_s.dwIndexSize = 0;
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
      //capture_parameters_s.AVStreamMaster = AVSTREAMMASTER_NONE;
      if (unlikely (capCaptureSetSetup (window_h,
                                        &capture_parameters_s,
                                        sizeof (struct tagCaptureParms)) == 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capCaptureSetSetup(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set framerate to %u/s\n"),
                inherited::mod_->name (),
                framerate_i));

      SetWindowLongPtr (window_h, GWLP_USERDATA, (LONG_PTR)&CBData_);
      //if (unlikely (capSetUserData (window_h, &CBData_) == 0))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to capSetUserData(), aborting\n"),
      //              inherited::mod_->name ()));
      //  goto error;
      //} // end IF

      capSetCallbackOnError (window_h, acestream_vfw_error_cb);
      capSetCallbackOnStatus (window_h, acestream_vfw_status_cb);
      capSetCallbackOnCapControl (window_h, acestream_vfw_control_cb);
      // if (unlikely (capSetCallbackOnFrame (window_h,
      //                                      acestream_vfw_video_cb) == 0))
      if (unlikely (capSetCallbackOnVideoStream (window_h,
                                                 acestream_vfw_video_cb) == 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capSetCallbackOnVideoStream(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      //capPreviewRate (window_h, 33); // ms
      //if (unlikely (capPreview (window_h, TRUE) == 0))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to capPreview(), aborting\n"),
      //              inherited::mod_->name ()));
      //  goto error;
      //} // end IF
      capOverlay (window_h, FALSE);
      capPreview (window_h, FALSE);

      if (unlikely (capCaptureSequenceNoFile (window_h) == 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to capCaptureSequenceNoFile(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started capturing\n"),
                  inherited::mod_->name ()));

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

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

      if (likely (window_))
      {
        // capPreview (window_, FALSE);
        capCaptureStop (window_);

        //if (unlikely (capSetCallbackOnFrame (window_,
        //                                     NULL) == 0))
        if (unlikely (capSetCallbackOnVideoStream (window_,
                                                   NULL) == 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to capSetCallbackOnVideoStream(), continuing\n"),
                      inherited::mod_->name ()));
        capSetCallbackOnCapControl (window_, NULL);
        capSetCallbackOnStatus (window_, NULL);
        capSetCallbackOnError (window_, NULL);

        if (unlikely (capDriverDisconnect (window_) == 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to capDriverDisconnect(), continuing\n"),
                      inherited::mod_->name ()));
      } // end IF

      if (likely (!passive_))
      { ACE_ASSERT (window_);
        DestroyWindow (window_); window_ = NULL;
      } // end IF
      passive_ = false;

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
                            MediaType>::initialize_VfW (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                        HWND windowHandle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_VfW_T::initialize_VfW"));

  return false;
}
