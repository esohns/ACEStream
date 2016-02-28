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

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "stream_misc_defines.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::Stream_Misc_DirectShow_Source_T ()
 : inherited ()
 , inherited2 ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , isInitialized_ (false)
 , IMemAllocator_ (NULL)
 , IMemInputPin_ (NULL)
 //, IGraphBuilder_ (NULL)
 , IMediaControl_ (NULL)
 , IMediaEventEx_ (NULL)
 , ROTID_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::Stream_Misc_DirectShow_Source_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::~Stream_Misc_DirectShow_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::~Stream_Misc_DirectShow_Source_T"));

  if (IMemAllocator_)
    IMemAllocator_->Release ();
  if (IMemInputPin_)
    IMemInputPin_->Release ();

  if (ROTID_)
  {
    IRunningObjectTable* ROT_p = NULL;
    HRESULT result = GetRunningObjectTable (0, &ROT_p);
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
  //if (IGraphBuilder_)
  //  IGraphBuilder_->Release ();
  if (configuration_->builder)
    configuration_->builder->Release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
bool
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::initialize"));

  // initialize COM ?
  HRESULT result = E_FAIL;
  static bool first_run = true;
  bool COM_initialized = false;
  if (first_run)
  {
    first_run = false;

    result = CoInitializeEx (NULL, COINIT_MULTITHREADED);
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

    // clean up ?
    if (IMemAllocator_)
    {
      IMemAllocator_->Release ();
      IMemAllocator_ = NULL;
    } // end IF
    if (IMemInputPin_)
    {
      IMemInputPin_->Release ();
      IMemInputPin_ = NULL;
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
    //if (IGraphBuilder_)
    //{
    //  IGraphBuilder_->Release ();
    //  IGraphBuilder_ = NULL;
    //} // end IF
    if (configuration_->builder)
    {
      configuration_->builder->Release ();
      configuration_->builder = NULL;
    } // end IF

    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  IBaseFilter* filter_p = NULL;
  result =
    configuration_in.builder->FindFilterByName (configuration_in.sourceFilter,
                                                &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IEnumPins* enumerator_p = NULL;
  result = filter_p->EnumPins (&enumerator_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);
  filter_p->Release ();
  IPin* pin_p = NULL;
  PIN_DIRECTION pin_direction;
  while (enumerator_p->Next (1, &pin_p, NULL) == S_OK)
  {
    ACE_ASSERT (pin_p);

    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      pin_p->Release ();
      enumerator_p->Release ();

      return false;
    } // end IF
    if (pin_direction != PINDIR_INPUT)
    {
      pin_p->Release ();
      pin_p = NULL;

      continue;
    } // end IF

    break;
  } // end WHILE
  enumerator_p->Release ();
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: has no input pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (configuration_in.sourceFilter)));
    return false;
  } // end IF

  result = pin_p->QueryInterface (IID_IMemInputPin,
                                  (void**)&IMemInputPin_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IMemInputPin): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    pin_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (IMemInputPin_);
  pin_p->Release ();

  result = IMemInputPin_->GetAllocator (&IMemAllocator_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemInputPin::GetAllocator(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    IMemInputPin_->Release ();
    IMemInputPin_ = NULL;

    return false;
  } // end IF
  ACE_ASSERT (IMemAllocator_);

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
const ConfigurationType&
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
void
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  //ACE_ASSERT (IMemAllocator_);
  ACE_ASSERT (IMemInputPin_);

  IMediaSample* media_sample_p = NULL;
  media_sample_p = message_inout;
  media_sample_p->AddRef ();
  HRESULT result = E_FAIL;
  //BYTE* buffer_p = NULL;
  //long remaining = message_inout->total_length ();
  //long to_copy = -1;
  //do
  //{
  //  result = IMemAllocator_->GetBuffer (&media_sample_p,
  //                                      NULL,
  //                                      NULL,
  //                                      0);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMemAllocator::GetBuffer(): \"%s\", returning\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //    return;
  //  } // end IF
  //  ACE_ASSERT (media_sample_p);

  //  result = media_sample_p->GetPointer (&buffer_p);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMediaSample::GetPointer(): \"%s\", returning\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //    // clean up
  //    media_sample_p->Release ();

  //    return;
  //  } // end IF
  //  ACE_ASSERT (buffer_p);

  //  to_copy =
  //    ((media_sample_p->GetSize () >= remaining) ? remaining
  //                                               : media_sample_p->GetSize ());
  //  ACE_OS::memcpy (buffer_p, message_inout->rd_ptr (), to_copy);
  //  result = media_sample_p->SetActualDataLength (to_copy);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMediaSample::SetActualDataLength(%d): \"%s\", returning\n"),
  //                to_copy,
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //    // clean up
  //    media_sample_p->Release ();

  //    return;
  //  } // end IF
  //  message_inout->rd_ptr (to_copy);

    //// *TODO*: use ReceiveMultiple ()
    result = IMemInputPin_->Receive (media_sample_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMemInputPin::Receive(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_sample_p->Release ();

      return;
    } // end IF
    media_sample_p->Release ();

  //  remaining -= to_copy;
  //} while (remaining);
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
void
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::handleSessionMessage"));

  int result = -1;
  IRunningObjectTable* ROT_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (sessionData_);
      // *TODO*: remove type inference
      ACE_ASSERT (configuration_->streamConfiguration);

      //inherited::sessionData_ =
      //  &const_cast<SessionDataContainerType&> (message_inout->get ());
      //inherited::sessionData_->increase ();
      typename SessionDataType::DATA_T& session_data_r =
        const_cast<typename SessionDataType::DATA_T&> (sessionData_->get ());

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

      if (!initialize_DirectShow (configuration_->window,
                                  configuration_->builder))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize_DirectShow(), returning\n")));
        goto error;
      } // end IF
      //ACE_ASSERT (IGraphBuilder_);
      ACE_ASSERT (configuration_->builder);
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
                            (DWORD_PTR)configuration_->builder, ACE_OS::getpid ());
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
                         configuration_->builder, moniker_p,
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

      if (!Stream_Module_Device_Tools::disconnect (configuration_->builder))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::disconnect(), continuing\n")));
      //if (ICaptureGraphBuilder2_)
      //{
      //  ICaptureGraphBuilder2_->Release ();
      //  ICaptureGraphBuilder2_ = NULL;
      //} // end IF
      if (configuration_->builder)
      {
        configuration_->builder->Release ();
        configuration_->builder = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
bool
Stream_Misc_DirectShow_Source_T<SessionMessageType,
                                MessageType,
                                ConfigurationType,
                                SessionDataType>::initialize_DirectShow (const HWND windowHandle_in,
                                                                         IGraphBuilder*& IGraphBuilder_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_T::initialize_DirectShow"));

  // sanity check(s)
  if (IGraphBuilder_out)
    IGraphBuilder_out->Release ();

  // initialize return value(s)
  IGraphBuilder_out = NULL;

  std::list<std::wstring> filter_pipeline;
  if (!Stream_Module_Device_Tools::load (windowHandle_in,
                                         IGraphBuilder_out,
                                         filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::load(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);

  //// *NOTE*: (re-)connect()ion of the video renderer input pin fails
  ////         consistently, so, apparently, reuse is not foreseen
  ////         --> rebuild the whole graph each session
  //if (!Stream_Module_Device_Tools::reset (IGraphBuilder_out))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::reset(), aborting\n")));
  //  return false;
  //} // end IF

  HRESULT result =
    IGraphBuilder_out->AddFilter (this,
                                  MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    IGraphBuilder_out->Release ();
    IGraphBuilder_out = NULL;

    return false;
  } // end IF
  filter_pipeline.push_front (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"...\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE_L)));

  if (!Stream_Module_Device_Tools::connect (IGraphBuilder_out,
                                            filter_pipeline))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::connect(), aborting\n")));

    // clean up
    IGraphBuilder_out->Release ();
    IGraphBuilder_out = NULL;

    return false;
  } // end IF

  // retrieve interfaces for media control and the video window
  result =
    IGraphBuilder_out->QueryInterface (IID_IMediaControl,
                                       (void**)&IMediaControl_);
  if (FAILED (result))
    goto error;
  result = IGraphBuilder_out->QueryInterface (IID_IMediaEventEx,
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

  return true;
}
