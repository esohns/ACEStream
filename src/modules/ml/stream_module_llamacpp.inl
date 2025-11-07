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

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_module_ml_defines.h"

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_LlamaCpp_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::Stream_Module_LlamaCpp_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , context_ (NULL)
 , model_ (NULL)
 , sampler_ (NULL)
 , template_ (NULL)
 , vocab_ (NULL)
 , formatted_ ()
 , messages_ ()
 , previousLength_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LlamaCpp_T::Stream_Module_LlamaCpp_T"));

  // only print errors ?
  llama_log_set (acestream_ggml_log_callback, NULL);

  // load dynamic backends
  ggml_backend_load_all ();
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_LlamaCpp_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::~Stream_Module_LlamaCpp_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LlamaCpp_T::~Stream_Module_LlamaCpp_T"));

  llama_sampler_free (sampler_);
  llama_free (context_);
  llama_model_free (model_);

  for (struct llama_chat_message& msg: messages_)
    free (const_cast<char*> (msg.content));
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_LlamaCpp_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LlamaCpp_T::initialize"));

  if (inherited::isInitialized_)
  {
    llama_sampler_free (sampler_); sampler_ = NULL;
    llama_free (context_); context_ = NULL;
    llama_model_free (model_); model_ = NULL;

    formatted_.clear ();
    for (struct llama_chat_message& msg : messages_)
      free (const_cast<char*> (msg.content));
    messages_.clear ();
    previousLength_ = 0;
  } // end IF

  // initialize the model
  struct llama_model_params model_params = llama_model_default_params ();
  model_params.n_gpu_layers = MODULE_ML_LLAMA_CPP_DEFAULT_NUMBER_OF_GPU_LAYERS;
  model_ =
    llama_model_load_from_file (configuration_in.modelFile.c_str (),
                                model_params);
  if (unlikely (!model_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to load model (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.modelFile.c_str ())));
    return false;
  } // end IF
  vocab_ = llama_model_get_vocab (model_);
  ACE_ASSERT (vocab_);

  // initialize the context
  struct llama_context_params ctx_params = llama_context_default_params ();
  ctx_params.n_ctx = MODULE_ML_LLAMA_CPP_DEFAULT_CONTEXT_SIZE;
  ctx_params.n_batch = MODULE_ML_LLAMA_CPP_DEFAULT_CONTEXT_SIZE;
  context_ = llama_init_from_model (model_, ctx_params);
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to create model context, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // initialize the sampler
  struct llama_sampler_chain_params smpl_params = llama_sampler_chain_default_params ();
  sampler_ = llama_sampler_chain_init (smpl_params);
  if (unlikely (!sampler_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to create sampler chain, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  llama_sampler_chain_add (sampler_, llama_sampler_init_min_p (0.05f, 1));
  llama_sampler_chain_add (sampler_, llama_sampler_init_temp (0.8f));
  llama_sampler_chain_add (sampler_, llama_sampler_init_dist (LLAMA_DEFAULT_SEED));

  template_ = llama_model_chat_template (model_, NULL);
  ACE_ASSERT (template_);

  formatted_.resize (llama_n_ctx (context_));

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_LlamaCpp_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LlamaCpp_T::handleDataMessage"));

  std::string prompt_string = message_inout->rd_ptr ();
  std::string prompt_string_2, response;
  int result;

  // add the user input to the message list and format it
  messages_.push_back ({"user", ACE_OS::strdup (prompt_string.c_str ())});
  int new_len = llama_chat_apply_template (template_, messages_.data (), messages_.size (), true, formatted_.data (), formatted_.size ());
  if (new_len > (int)formatted_.size ())
  {
    formatted_.resize (new_len);
    new_len = llama_chat_apply_template (template_, messages_.data (), messages_.size (), true, formatted_.data (), formatted_.size ());
  } // end IF
  if (unlikely (new_len < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to apply the chat template: %d, aborting\n"),
                inherited::mod_->name (),
                new_len));
    goto error;
  } // end IF

  // remove previous messages to obtain the prompt to generate the response
  prompt_string_2.assign (formatted_.begin () + previousLength_, formatted_.begin () + new_len);

  // generate a response
  //printf ("\033[33m");
  response = generate (prompt_string_2);
  //printf ("\n\033[0m");

  // add the response to the messages
  messages_.push_back ({"assistant", ACE_OS::strdup (response.c_str ())});
  previousLength_ = llama_chat_apply_template (template_, messages_.data (), messages_.size (), false, NULL, 0);
  if (unlikely (previousLength_ < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to apply the chat template: %d, aborting\n"),
                inherited::mod_->name (),
                previousLength_));
    goto error;
  } // end IF

  message_inout->reset ();
  ACE_ASSERT (message_inout->space () >= response.size () + 1);
  result = message_inout->copy (response.c_str (), response.size ());
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%s,%Q): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (response.c_str ()),
                response.size ()));
    goto error;
  } // end IF

  return;

error:
  passMessageDownstream_out = false;
  message_inout->release (); message_inout = NULL;

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
std::string
Stream_Module_LlamaCpp_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::generate (const std::string& prompt_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LlamaCpp_T::generate"));

  std::string response;

  const bool is_first = llama_memory_seq_pos_max (llama_get_memory (context_), 0) == -1;

  // tokenize the prompt
  const int n_prompt_tokens = -llama_tokenize (vocab_, prompt_in.c_str (), prompt_in.size (), NULL, 0, is_first, true);
  std::vector<llama_token> prompt_tokens (n_prompt_tokens);
  if (llama_tokenize (vocab_, prompt_in.c_str (), prompt_in.size (), prompt_tokens.data (), prompt_tokens.size (), is_first, true) < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to tokenize the prompt (was: \"%s\"), returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (prompt_in.c_str ())));
    return response;
  } // end IF

  // prepare a batch for the prompt
  llama_batch batch = llama_batch_get_one (prompt_tokens.data (), prompt_tokens.size ());
  llama_token new_token_id;
  while (true)
  {
    // check if we have enough space in the context to evaluate this batch
    int n_ctx = llama_n_ctx (context_);
    int n_ctx_used = llama_memory_seq_pos_max (llama_get_memory (context_), 0) + 1;
    if (n_ctx_used + batch.n_tokens > n_ctx)
    {
      //printf ("\033[0m\n");
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: context size exceeded (was: %d, is: %d), returning\n"),
                  inherited::mod_->name (),
                  n_ctx_used + batch.n_tokens,
                  n_ctx));
      return response;
    } // end IF

    int ret = llama_decode (context_, batch);
    if (ret != 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to decode the prompt (was: \"%s\"): %d, returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (prompt_in.c_str ()),
                  ret));
      return response;
    } // end IF

    // sample the next token
    new_token_id = llama_sampler_sample (sampler_, context_, -1);

    // is it an end of generation?
    if (llama_vocab_is_eog (vocab_, new_token_id))
      break;

    // convert the token to a string, print it and add it to the response
    char buf[256];
    int n = llama_token_to_piece (vocab_, new_token_id, buf, sizeof (char[256]), 0, true);
    if (n < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to convert token to piece (prompt was: \"%s\"): %d, returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (prompt_in.c_str ()),
                  n));
      return response;
    } // end IF
    std::string piece (buf, n);
    //printf ("%s", piece.c_str ());
    //fflush (stdout);
    response += piece;

    // prepare the next batch with the sampled token
    batch = llama_batch_get_one (&new_token_id, 1);
  } // end WHILE

  return response;
}
