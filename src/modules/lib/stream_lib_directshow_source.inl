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

#include "common_log_tools.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_lib_defines.h"
#include "stream_lib_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
Stream_MediaFramework_DirectShow_Source_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          MediaType>::Stream_MediaFramework_DirectShow_Source_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , IGraphBuilder_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_T::Stream_MediaFramework_DirectShow_Source_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
Stream_MediaFramework_DirectShow_Source_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          MediaType>::~Stream_MediaFramework_DirectShow_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_T::~Stream_MediaFramework_DirectShow_Source_T"));

  if (IGraphBuilder_)
    IGraphBuilder_->Release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
bool
Stream_MediaFramework_DirectShow_Source_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          MediaType>::initialize (const ConfigurationType& configuration_in,
                                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_T::initialize"));

  bool result = false;
  HRESULT result_2 = E_FAIL;

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (likely (first_run))
  {
    first_run = false;
    COM_initialized = Common_Tools::initializeCOM ();
  } // end IF

  if (inherited::isInitialized_)
  {
    if (IGraphBuilder_)
    {
      IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
    } // end IF
  } // end IF

  // sanity check(s)
  if (!configuration_in.builder)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no graph builder, aborting\n"),
                inherited::mod_->name ()));
    if (COM_initialized) Common_Tools::finalizeCOM ();
    return false;
  } // end IF
  IGraphBuilder_ = configuration_in.builder;
  IGraphBuilder_->AddRef ();

  // retrieve sample grabber filter
  IBaseFilter* filter_p = NULL;
  result_2 = IGraphBuilder_->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                               &filter_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IFilterGraph::FindFilterByName(%s): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
    IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
    if (COM_initialized) Common_Tools::finalizeCOM ();
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  ISampleGrabber* sample_grabber_p = NULL;
  result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                       (void**)&sample_grabber_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IUnknown::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
    filter_p->Release (); filter_p = NULL;
    IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
    if (COM_initialized) Common_Tools::finalizeCOM ();
    return false;
  } // end IF
  ACE_ASSERT (sample_grabber_p);
  filter_p->Release (); filter_p = NULL;

  // set up sample grabber
  result_2 = sample_grabber_p->SetBufferSamples (false);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
    sample_grabber_p->Release (); sample_grabber_p = NULL;
    IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
    if (COM_initialized) Common_Tools::finalizeCOM ();
    return false;
  } // end IF
  result_2 = sample_grabber_p->SetCallback (this, 0);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
    sample_grabber_p->Release (); sample_grabber_p = NULL;
    IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
    if (COM_initialized) Common_Tools::finalizeCOM ();
    return false;
  } // end IF
  sample_grabber_p->Release (); sample_grabber_p = NULL;

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(), aborting\n")));

  if (COM_initialized) Common_Tools::finalizeCOM ();

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
void
Stream_MediaFramework_DirectShow_Source_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_T::handleDataMessage"));

  passMessageDownstream_out = false;
  message_inout->release (); message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
void
Stream_MediaFramework_DirectShow_Source_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result_2 = E_FAIL;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (IGraphBuilder_);

      bool COM_initialized = Common_Tools::initializeCOM ();
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      struct _AMMediaType media_type_s;
      MediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));

#if defined (_DEBUG)
      std::string log_file_name =
        Common_Log_Tools::getLogDirectory (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                            0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += STREAM_LIB_DIRECTSHOW_LOGFILE_NAME;
      Stream_MediaFramework_DirectShow_Tools::debug (IGraphBuilder_,
                                                     log_file_name);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("set DirectShow logfile: \"%s\"\n"),
                  ACE_TEXT (log_file_name.c_str ())));
#endif // _DEBUG

      // retrieve sample grabber filter
      IBaseFilter* filter_p = NULL;
      ISampleGrabber* sample_grabber_p = NULL;
      result_2 =
        IGraphBuilder_->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                          &filter_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IFilterGraph::FindFilterByName(%s): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (filter_p);
      result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                           (void**)&sample_grabber_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IUnknown::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        filter_p->Release (); filter_p = NULL;
        goto error;
      } // end IF
      ACE_ASSERT (sample_grabber_p);
      filter_p->Release (); filter_p = NULL;
      result_2 = sample_grabber_p->GetConnectedMediaType (&media_type_s);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ISampleGrabber::GetConnectedMediaType(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        sample_grabber_p->Release (); sample_grabber_p = NULL;
        goto error;
      } // end IF
      sample_grabber_p->Release (); sample_grabber_p = NULL;
      inherited2::getMediaType (media_type_s,
                                STREAM_MEDIATYPE_INVALID, // N/A
                                media_type_2);
      session_data_r.formats.push_back (media_type_2);

      if (COM_initialized)
        Common_Tools::finalizeCOM ();

      break;

error:
      if (COM_initialized)
        Common_Tools::finalizeCOM ();

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      bool COM_initialized = Common_Tools::initializeCOM ();

      if (IGraphBuilder_)
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          MediaType>::SampleCB (double sampleTime_in,
                                                                IMediaSample* sample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_T::SampleCB"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;
  DataMessageType* message_p = NULL;
  if (inherited::configuration_->sampleIsDataMessage)
  {
    try {
      message_p = dynamic_cast<DataMessageType*> (sample_in);
    } catch (...) {
      message_p = NULL;
    }
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<DataMessageType*>(0x%@), aborting\n"),
                  inherited::mod_->name (),
                  sample_in));
      return E_FAIL;
    } // end IF
  } // end IF
  else
  {
    // sanity check(s)
    // *TODO*: remove type inference
    ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
    ACE_ASSERT (inherited::configuration_->allocatorConfiguration->defaultBufferSize);

    message_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!message_p))
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
    // *TODO*: remove type inference
    data_r.sample = sample_in;
    data_r.sampleTime = sampleTime_in;

    BYTE* buffer_p = NULL;
    HRESULT result_2 = sample_in->GetPointer (&buffer_p);
    if (unlikely (FAILED (result_2) || !buffer_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaSample::GetPointer(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
      message_p->release (); message_p = NULL;
      return result_2;
    } // end IF
    unsigned int size = static_cast<unsigned int> (sample_in->GetSize ());
    message_p->base (reinterpret_cast<char*> (buffer_p),
                     size,
                     ACE_Message_Block::DONT_DELETE);
    message_p->wr_ptr (size);
  } // end ELSE
  ACE_ASSERT (message_p);

  result = inherited::put_next (message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    return E_FAIL;
  } // end IF

  return S_OK;
}
