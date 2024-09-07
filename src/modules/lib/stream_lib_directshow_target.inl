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

 #include <string>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_os_tools.h"

#include "common_log_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"

template <typename TaskType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType,
          typename FilterType>
Stream_MediaFramework_DirectShow_Target_T<TaskType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          MediaType,
                                          FilterType>::Stream_MediaFramework_DirectShow_Target_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 //, inherited4 ()
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
 , IGraphBuilder_ (NULL)
//, IMemAllocator_ (NULL)
//, IMemInputPin_ (NULL)
 , IMediaControl_ (NULL)
 , IMediaEventEx_ (NULL)
 , ROTID_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Target_T::Stream_MediaFramework_DirectShow_Target_T"));

}

template <typename TaskType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType,
          typename FilterType>
Stream_MediaFramework_DirectShow_Target_T<TaskType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          MediaType,
                                          FilterType>::~Stream_MediaFramework_DirectShow_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Target_T::~Stream_MediaFramework_DirectShow_Target_T"));

  int result = -1;

  //if (IMemAllocator_)
  //  IMemAllocator_->Release ();
  //if (IMemInputPin_)
  //  IMemInputPin_->Release ();
  //if (!push_)
  //{
    //result = inherited::queue_.flush ();
    //if (result == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("%s: failed to ACE_Message_Queue::flush() \"%m\", continuing\n"),
    //              inherited::mod_->name ()));
  inherited::idle ();
  //} // end IF

  if (ROTID_)
  {
    if (!Stream_MediaFramework_DirectShow_Tools::removeFromROT (ROTID_))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::removeFromROT(%d), continuing\n"),
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

  if (IGraphBuilder_)
    IGraphBuilder_->Release ();
}

template <typename TaskType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType,
          typename FilterType>
bool
Stream_MediaFramework_DirectShow_Target_T<TaskType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          MediaType,
                                          FilterType>::initialize (const typename TaskType::CONFIGURATION_T& configuration_in,
                                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Target_T::initialize"));

  HRESULT result = E_FAIL;

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (likely (first_run))
  {
    first_run = false;
    COM_initialized = Common_Tools::initializeCOM ();
  } // end IF

  int result_2 = -1;
  if (inherited::isInitialized_)
  {
    queue_.flush ();

    if (ROTID_)
    {
      if (!Stream_MediaFramework_DirectShow_Tools::removeFromROT (ROTID_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                    inherited::mod_->name (),
                    ROTID_));
      ROTID_ = 0;
    } // end IF

    if (IMediaEventEx_)
    {
      IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
      IMediaEventEx_->Release (); IMediaEventEx_ = NULL;
    } // end IF
    if (IMediaControl_)
    {
      IMediaControl_->Stop ();
      IMediaControl_->Release (); IMediaControl_ = NULL;
    } // end IF

    if (IGraphBuilder_)
    {
      IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
    } // end IF
  } // end IF

  result_2 = queue_.activate ();
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate() \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // *TODO*: remove type inference
  if (configuration_in.builder)
  {
    ULONG reference_count = configuration_in.builder->AddRef ();
    IGraphBuilder_ = configuration_in.builder;
  } // end IF

  // *TODO*: remove type inference
  if (configuration_in.filterConfiguration)
  { //ACE_ASSERT (!configuration_in.filterConfiguration->module);
    configuration_in.filterConfiguration->module = inherited::mod_;
    ACE_ASSERT (configuration_in.filterConfiguration->pinConfiguration);
    //ACE_ASSERT (!configuration_in.filterConfiguration->pinConfiguration->queue);
    configuration_in.filterConfiguration->pinConfiguration->queue = &queue_;
  } // end IF

  if (COM_initialized)
    Common_Tools::finalizeCOM ();

  return inherited::initialize (configuration_in,
                                allocator_in);

error:
  result_2 = queue_.deactivate ();
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate() \"%m\", continuing\n"),
                inherited::mod_->name ()));
  if (IGraphBuilder_)
  {
    IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
  } // end IF
  if (COM_initialized)
    Common_Tools::finalizeCOM ();

  return false;
}

template <typename TaskType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType,
          typename FilterType>
void
Stream_MediaFramework_DirectShow_Target_T<TaskType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          MediaType,
                                          FilterType>::handleDataMessage (typename TaskType::DATA_MESSAGE_T*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Target_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  
  message_block_p = message_inout->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = queue_.enqueue_tail (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release ();
    return;
  } // end IF
}

template <typename TaskType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType,
          typename FilterType>
void
Stream_MediaFramework_DirectShow_Target_T<TaskType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          MediaType,
                                          FilterType>::handleSessionMessage (typename TaskType::SESSION_MESSAGE_T*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Target_T::handleSessionMessage"));

  int result = -1;
  HRESULT result_2 = E_FAIL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
        const_cast<typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());

      bool COM_initialized = Common_Tools::initializeCOM ();
      bool is_running = false;
      bool remove_from_ROT = false;
#if defined (_DEBUG)
      std::string log_file_name;
#endif // _DEBUG
      IAMBufferNegotiation* buffer_negotiation_p = NULL;
      IVideoWindow* video_window_p = NULL;
      HWND window_h = NULL;
      ULONG reference_count = 0;
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));

      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_INVALID, // N/A
                                media_type_s);

      if (!IGraphBuilder_)
      {
        //if (session_data_r.builder)
        //{
        //  reference_count = session_data_r.builder->AddRef ();
        //  IGraphBuilder_ = session_data_r.builder;
        //  goto continue_;
        //} // end IF

        // sanity check(s)
        if (InlineIsEqualGUID (media_type_s.majortype, MEDIATYPE_Video))
        {
          ACE_ASSERT (inherited::configuration_->window);
          inherited3::getWindowType (inherited::configuration_->window,
                                     window_h);
          ACE_ASSERT (window_h);
        } // end IF
        // *TODO*: remove type inferences
        ACE_ASSERT (inherited::configuration_->filterConfiguration);
        if (!loadGraph (inherited::configuration_->filterCLSID,
                        *inherited::configuration_->filterConfiguration,
                        media_type_s,
                        window_h,
                        IGraphBuilder_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Target_T::loadGraph(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end IF
//continue_:
      ACE_ASSERT (IGraphBuilder_);

#if defined (_DEBUG)
      log_file_name =
        Common_Log_Tools::getLogDirectory (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME), 0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += STREAM_LIB_DIRECTSHOW_LOGFILE_NAME;
      Stream_MediaFramework_DirectShow_Tools::debug (IGraphBuilder_,
                                                     log_file_name);
#endif // _DEBUG

      // retrieve interfaces for media control (and the video window)
      // sanity check(s)
      ACE_ASSERT (!IMediaControl_);
      ACE_ASSERT (!IMediaEventEx_);

      result_2 =
        IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&IMediaControl_));
      if (unlikely (FAILED (result_2)))
        goto error_2;
      result_2 =
        IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&IMediaEventEx_));
      if (unlikely (FAILED (result_2)))
        goto error_2;
      ACE_ASSERT (IMediaControl_);
      ACE_ASSERT (IMediaEventEx_);

      if (InlineIsEqualGUID (media_type_s.majortype, MEDIATYPE_Video))
      {
        // set the window handle used to process graph events
        ACE_ASSERT (inherited::configuration_->window);
        window_h = NULL;
        inherited3::getWindowType (inherited::configuration_->window,
                                   window_h);
        result_2 =
          IMediaEventEx_->SetNotifyWindow (reinterpret_cast<OAHWND> (window_h),
                                           STREAM_LIB_DIRECTSHOW_WM_GRAPHNOTIFY_EVENT, 0);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
          goto error;
        } // end IF
      } // end IF

      goto do_run;
error_2:
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
      goto error;

do_run:
      ACE_ASSERT (IMediaControl_);
      ACE_ASSERT (IMediaEventEx_);

      // start forwarding data
      // *NOTE*: this may query the source filter for IMediaSeeking, IMediaPosition
      result_2 = IMediaControl_->Run ();
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaControl::Run(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        goto error;
      } // end IF
      is_running = true;

      // register graph in the ROT (so graphedt.exe can see it)
      ACE_ASSERT (!ROTID_);
      if (unlikely (!Stream_MediaFramework_DirectShow_Tools::addToROT (IGraphBuilder_,
                                                                       ROTID_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::addToROT(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (ROTID_);
      remove_from_ROT = true;

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (COM_initialized)
        Common_Tools::finalizeCOM ();

      break;

error:
      if (remove_from_ROT)
      { ACE_ASSERT (ROTID_);
        if (!Stream_MediaFramework_DirectShow_Tools::removeFromROT (ROTID_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      inherited::mod_->name (),
                      ROTID_));
        ROTID_ = 0;
      } // end IF

      if (is_running)
      { ACE_ASSERT (IMediaControl_);
        result_2 = IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
      } // end IF

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (COM_initialized)
        Common_Tools::finalizeCOM ();

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      bool COM_initialized = Common_Tools::initializeCOM ();

      // deregister graph from the ROT ?
      if (likely (ROTID_))
      {
        if (unlikely (!Stream_MediaFramework_DirectShow_Tools::removeFromROT (ROTID_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      inherited::mod_->name (),
                      ROTID_));
        ROTID_ = 0;
      } // end IF

      if (likely (IMediaEventEx_))
      {
        result_2 = IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        IMediaEventEx_->Release (); IMediaEventEx_ = NULL;
      } // end IF

      stop (false,  // wait for completion ?
            false); // high priority ?

      if (likely (IMediaControl_))
      {
        // stop previewing video data (blocks)
        result_2 = IMediaControl_->Stop ();
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        IMediaControl_->Release (); IMediaControl_ = NULL;
      } // end IF

      result = queue_.flush ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Mesage_Queue::flush(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));

      if (likely (IGraphBuilder_))
      {
#if defined (_DEBUG)
        Stream_MediaFramework_DirectShow_Tools::debug (IGraphBuilder_,
                                                       ACE_TEXT_ALWAYS_CHAR (""));
#endif // _DEBUG
        IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
      } // end IF

      if (COM_initialized)
        Common_Tools::finalizeCOM ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TaskType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType,
          typename FilterType>
bool
Stream_MediaFramework_DirectShow_Target_T<TaskType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          MediaType,
                                          FilterType>::loadGraph (REFGUID filterIdentifier_in,
                                                                  const FilterConfigurationType& filterConfiguration_in,
                                                                  const struct _AMMediaType& mediaType_in,
                                                                  HWND windowHandle_in,
                                                                  IGraphBuilder*& IGraphBuilder_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Target_T::loadGraph"));

  // sanity check(s)
  if (IGraphBuilder_out)
    IGraphBuilder_out->Release ();

  // initialize return value(s)
  IGraphBuilder_out = NULL;

  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;
  std::wstring render_filter_name =
    (windowHandle_in ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO_L
                     : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL_L);
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  bool release_configuration = false;

  if (!InlineIsEqualGUID (filterIdentifier_in, GUID_NULL))
  {
    result =
      CoCreateInstance (filterIdentifier_in, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&filter_p));
    if (FAILED (result)) // REGDB_E_CLASSNOTREG: 0x80040154
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (filterIdentifier_in).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      return false;
    } // end IF
  } // end IF
  else
  {
    CUnknown* unknown_p = FilterType::CreateInstance (NULL, &result);
    if (!unknown_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
    result = unknown_p->NonDelegatingQueryInterface (IID_PPV_ARGS (&filter_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CUnknown::NonDelegatingQueryInterface(IID_IBaseFilter): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      unknown_p->NonDelegatingRelease ();
      return false;
    } // end IF
    FilterType* filter_2 = static_cast<FilterType*> (unknown_p);
    //ACE_ASSERT (filter_2);
    if (!filter_2->initialize (*inherited::configuration_->filterConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to FilterType::initialize(), aborting\n"),
                  inherited::mod_->name ()));
      unknown_p->NonDelegatingRelease ();
      return false;
    } // end IF
    unknown_p->NonDelegatingRelease ();
  } // end ELSE
  ACE_ASSERT (filter_p);
  IINITIALIZE_FILTER_T* iinitialize_p =
    dynamic_cast<IINITIALIZE_FILTER_T*> (filter_p);
  if (!iinitialize_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Common_IInitialize_T*>(%@), aborting\n"),
                inherited::mod_->name (),
                filter_p));
    goto error;
  } // end IF
  // *TODO*: remove type inference
  if (!iinitialize_p->initialize (filterConfiguration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IInitialize_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  if (!Stream_Module_Decoder_Tools::loadTargetRendererGraph (filter_p,
                                                             (inherited::configuration_->push ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L
                                                                                              : STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L),
                                                             mediaType_in,
                                                             windowHandle_in,
                                                             IGraphBuilder_out,
                                                             buffer_negotiation_p,
                                                             graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadTargetRendererGraph(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  release_configuration = true;
  filter_p->Release (); filter_p = NULL;
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (buffer_negotiation_p);

  // *TODO*: remove type inference
  ACE_ASSERT (filterConfiguration_in.allocatorProperties);
  result =
      buffer_negotiation_p->SuggestAllocatorProperties (filterConfiguration_in.allocatorProperties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    
    // clean up
    buffer_negotiation_p->Release ();
    
    goto error;
  } // end IF
  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  if (!Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder_out,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  filter_p = NULL;
  result =
    IGraphBuilder_out->FindFilterByName (render_filter_name.c_str (),
                                         &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT_WCHAR_TO_TCHAR (render_filter_name.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  IPin* pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                             PINDIR_INPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: no input pin, aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT_WCHAR_TO_TCHAR (render_filter_name.c_str ())));
    goto error;
  } // end IF
  filter_p->Release (); filter_p = NULL;

  //result = pin_p->QueryInterface (IID_PPV_ARGS (&IMemInputPin_));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IPin::QueryInterface(IID_IMemInputPin): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  //  // clean up
  //  pin_p->Release ();

  //  return false;
  //} // end IF
  //ACE_ASSERT (IMemInputPin_);
  pin_p->Release (); pin_p = NULL;

  //result = IMemInputPin_->GetAllocator (&IMemAllocator_);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMemInputPin::GetAllocator(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  //  // clean up
  //  IMemInputPin_->Release (); IMemInputPin_ = NULL;

  //  return false;
  //} // end IF
  //ACE_ASSERT (IMemAllocator_);

  // clean up
  for (Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t iterator = graph_configuration.begin ();
       iterator != graph_configuration.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_MediaFramework_DirectShow_Tools::delete_ ((*iterator).mediaType, false);

  return true;

error:
  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
  } // end IF
  if (filter_p)
    filter_p->Release ();
  if (release_configuration)
    for (Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t iterator = graph_configuration.begin ();
       iterator != graph_configuration.end ();
       ++iterator)
      if ((*iterator).mediaType)
        Stream_MediaFramework_DirectShow_Tools::delete_ ((*iterator).mediaType, false);

  return false;
}

template <typename TaskType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType,
          typename FilterType>
void
Stream_MediaFramework_DirectShow_Target_T<TaskType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          MediaType,
                                          FilterType>::stop (bool waitForCompletion_in,
                                                             bool highPriority_in)
{
  COMMON_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Target_T::stop"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // enqueue a ACE_Message_Block::MB_STOP message
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                  inherited::mod_->name ()));
    return;
  } // end IF

  result = (highPriority_in ? queue_.enqueue_head (message_block_p, NULL)
                            : queue_.enqueue_tail (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
}
