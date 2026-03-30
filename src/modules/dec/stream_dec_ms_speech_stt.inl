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

#include "common_file_tools.h"
#include "common_os_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_SAPI_STT_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          SessionDataContainerType,
                          MediaType>::Stream_Decoder_SAPI_STT_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inSession_ (false)
 , sessionId_ (0)
 , context_ (NULL)
 , event_ (ACE_INVALID_HANDLE)
 , grammar_ (NULL)
 , recognizer_ (NULL)
 , state_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPI_STT_T::Stream_Decoder_SAPI_STT_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_SAPI_STT_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          SessionDataContainerType,
                          MediaType>::~Stream_Decoder_SAPI_STT_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPI_STT_T::~Stream_Decoder_SAPI_STT_T"));

  if (context_)
    context_->Release ();
  //if (event_ != ACE_INVALID_HANDLE)
  //  CloseHandle (event_);
  if (grammar_)
    grammar_->Release ();
  if (recognizer_)
    recognizer_->Release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Decoder_SAPI_STT_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          SessionDataContainerType,
                          MediaType>::initialize (const ConfigurationType& configuration_in,
                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPI_STT_T::initialize"));

  HRESULT result;

  if (inherited::isInitialized_)
  {
    inSession_ = false;
    sessionId_ = 0;

    if (context_)
    {
      context_->Release (); context_ = NULL;
    } // end IF
    if (event_ != ACE_INVALID_HANDLE)
    {
      //CloseHandle (event_);
      event_ = ACE_INVALID_HANDLE;
    } // end IF
    if (grammar_)
    {
      grammar_->Release (); grammar_ = NULL;
    } // end IF
    if (recognizer_)
    {
      recognizer_->Release (); recognizer_ = NULL;
    } // end IF
    state_ = NULL;
  } // end IF

  result = CoCreateInstance (CLSID_SpSharedRecognizer, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&recognizer_));
  if (FAILED (result) || !recognizer_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_SpSharedRecognizer).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    return false;
  } // end IF

  //*NOTE*: for CLSID_SpInprocRecognizer:
  //ISpObjectToken* token_p = NULL;
  //result = SpGetDefaultTokenFromCategoryId (SPCAT_AUDIOIN, &token_p, FALSE);
  //ACE_ASSERT (SUCCEEDED (result) && token_p);

  //result = recognizer_->SetInput (token_p, TRUE);
  //ACE_ASSERT (SUCCEEDED (result));
  //token_p->Release (); token_p = NULL;

  //ISpAudio* audio_p = NULL;
  //result = SpCreateDefaultObjectFromCategoryId (SPCAT_AUDIOIN, &audio_p, NULL, CLSCTX_ALL);
  //ACE_ASSERT (SUCCEEDED (result) && audio_p);

  //result = recognizer_->SetInput (audio_p, TRUE);
  //ACE_ASSERT (SUCCEEDED (result));
  //audio_p->Release (); audio_p = NULL;

  result = recognizer_->CreateRecoContext (&context_);
  if (FAILED (result) || !context_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISpRecognizer::CreateRecoContext(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    return false;
  } // end IF
  result = context_->Pause (0);
  ACE_ASSERT (SUCCEEDED (result));

  result = context_->CreateGrammar (NULL, &grammar_);
  if (FAILED (result) || !grammar_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISpRecoContext::CreateGrammar(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    return false;
  } // end IF

  result = grammar_->LoadDictation (NULL, SPLO_STATIC);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISpRecoGrammar::LoadDictation(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    return false;
  } // end IF

//  WORD langId = MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US);
//  result = grammar_->ResetGrammar (langId);
//  // TODO: Catch error and use default langId => GetUserDefaultUILanguage()
//  ACE_ASSERT (SUCCEEDED (result));
//
//  // Create rules
//#define RULE_NAME_0 ACE_TEXT_ALWAYS_WCHAR ("Rule_0")
//  result = grammar_->GetRule (RULE_NAME_0,
//                              0,
//                              SPRAF_TopLevel | SPRAF_Active,
//                              true,
//                              &state_);
//  ACE_ASSERT (SUCCEEDED (result) && state_);
//
//  // Add a word
//  std::wstring command_string (ACE_TEXT_ALWAYS_WCHAR ("Hello"));
//  result = grammar_->AddWordTransition (state_,
//                                        NULL,
//                                        command_string.c_str (),
//                                        L" ",
//                                        SPWT_LEXICAL,
//                                        1,
//                                        NULL);
//  ACE_ASSERT (SUCCEEDED (result));
//
//  // Commit changes
//  result = grammar_->Commit (0);
//  ACE_ASSERT (SUCCEEDED (result));

  result = context_->SetNotifyWin32Event ();
  ACE_ASSERT (SUCCEEDED (result));

  event_ = context_->GetNotifyEventHandle ();
  if (unlikely (event_ == INVALID_HANDLE_VALUE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISpRecoContext::GetNotifyEventHandle(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  ULONGLONG interest_i = SPFEI (SPEI_RECOGNITION);
  result = context_->SetInterest (interest_i, interest_i);
  ACE_ASSERT (SUCCEEDED (result));

  // Activate Grammar
  //result = grammar_->SetRuleState (RULE_NAME_0,
  //                                 0,
  //                                 SPRS_ACTIVE);
  //ACE_ASSERT (SUCCEEDED (result));

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_SAPI_STT_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          SessionDataContainerType,
                          MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPI_STT_T::handleDataMessage"));

  passMessageDownstream_out = false;

  message_inout->release (); message_inout = NULL;

//  return;
//
//error:
//  message_inout->release (); message_inout = NULL;
//
//  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_SAPI_STT_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          SessionDataContainerType,
                          MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPI_STT_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      result = grammar_->SetDictationState (SPRS_ACTIVE);
      ACE_ASSERT (SUCCEEDED (result));

      // Enable context
      result = context_->Resume (0);
      ACE_ASSERT (SUCCEEDED (result));

      inSession_ = true;
      sessionId_ = message_inout->sessionId ();

      break;

//error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      inherited::stop (false,  // wait ?
                       false); // high priority ?

      if (grammar_)
      {
        result = grammar_->SetDictationState (SPRS_INACTIVE);
        ACE_ASSERT (SUCCEEDED (result));

        result = grammar_->UnloadDictation ();
        ACE_ASSERT (SUCCEEDED (result));
      } // end IF

      if (context_)
      {
        result = context_->Pause (0);
        ACE_ASSERT (SUCCEEDED (result));

        context_->Release (); context_ = NULL;
      } // end IF
      if (event_ != ACE_INVALID_HANDLE)
      {
        //CloseHandle (event_);
        event_ = ACE_INVALID_HANDLE;
      } // end IF
      if (grammar_)
      {
        grammar_->Release (); grammar_ = NULL;
      } // end IF
      if (recognizer_)
      {
        recognizer_->Release (); recognizer_ = NULL;
      } // end IF
      state_ = NULL;

      inSession_ = false;
      sessionId_ = 0;

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
          typename SessionDataContainerType,
          typename MediaType>
int
Stream_Decoder_SAPI_STT_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          SessionDataContainerType,
                          MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPI_STT_T::svc"));

  int result = -1;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  bool stop_processing = false;
  HANDLE handles_a[1];
  handles_a[0] = event_;
  CSpEvent spEvent;
  HRESULT result_3;
  wchar_t* text_p;
  std::string text_string;
  DataMessageType* message_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  // step1: start processing data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result == 0)
    {
      ACE_ASSERT (message_block_p);
      ACE_Message_Block::ACE_Message_Type message_type =
        message_block_p->msg_type ();
      if (message_type == ACE_Message_Block::MB_STOP)
      {
        // clean up
        message_block_p->release (); message_block_p = NULL;

        result_2 = 0; // OK

        break; // aborted
      } // end IF

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
    } // end IF
    else if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
        break;
      } // end IF
    } // end IF

    if (inSession_)
    {
      //WaitForMultipleObjects (1, handles_a, FALSE, INFINITE);

      result_3 = spEvent.GetFrom (context_);
      if (FAILED (result_3))
        continue;

      switch (spEvent.eEventId)
      {
        case SPEI_RECOGNITION:
        {
          ISpRecoResult* result_p = spEvent.RecoResult ();
          ACE_ASSERT (result_p);

          text_p = NULL;
          result_3 = result_p->GetText (SP_GETWHOLEPHRASE,
                                        SP_GETWHOLEPHRASE,
                                        FALSE,
                                        &text_p,
                                        NULL);
          ACE_ASSERT (SUCCEEDED (result_3) && text_p);
          text_string = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (text_p));
          CoTaskMemFree (text_p);

          message_p =
            inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
          if (unlikely (!message_p))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to allocateMessage(%u): \"%m\", aborting\n"),
                        inherited::mod_->name (),
                        inherited::configuration_->allocatorConfiguration->defaultBufferSize));
            spEvent.Clear ();
            goto error;
          } // end IF

          result = message_p->copy (text_string.c_str (),
                                    text_string.size ());
          if (unlikely (result == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%Q): \"%m\", aborting\n"),
                        inherited::mod_->name (),
                        text_string.size ()));
            message_p->release ();
            spEvent.Clear ();
            goto error;
          } // end IF
          message_p->initialize (sessionId_, NULL);

          result = inherited::put_next (message_p, NULL);
          if (unlikely (result == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_p->release ();
            spEvent.Clear ();
            goto error;
          } // end IF

          break;
        }
        case SPEI_END_SR_STREAM:
          break;
      } // end SWITCH
      spEvent.Clear ();
    } // end IF
  } while (true);

error:
  return result_2;
}
