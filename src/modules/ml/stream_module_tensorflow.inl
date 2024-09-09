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

#if defined (TENSORFLOW_CC_SUPPORT)
#include "tensorflow/core/platform/status.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "tensorflow/cc/saved_model/tag_constants.h"
#endif // TENSORFLOW_CC_SUPPORT

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Tensorflow_T<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::Stream_Module_Tensorflow_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , graph_ (NULL)
 , session_ (NULL)
 , status_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_T::Stream_Module_Tensorflow_T"));

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Tensorflow_T<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::~Stream_Module_Tensorflow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_T::~Stream_Module_Tensorflow_T"));

  if (graph_)
    TF_DeleteGraph (graph_);
  if (session_)
    TF_DeleteSession (session_, status_);
  if (status_)
    TF_DeleteStatus (status_);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_Tensorflow_T<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (graph_)
      TF_DeleteGraph (graph_);
    graph_ = NULL;
    if (session_)
      TF_DeleteSession (session_, status_);
    session_ = NULL;
    if (status_)
      TF_DeleteStatus (status_);
    status_ = NULL;
  } // end IF

  // load 'frozen' graph (aka 'model')
  uint8_t* data_p = NULL;
  ACE_UINT64 size_i = 0;
  if (unlikely (!Common_File_Tools::load (configuration_in.modelFile,
                                          data_p,
                                          size_i,
                                          0)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to load model (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.modelFile.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p && size_i);
  TF_Buffer* buffer_p = TF_NewBuffer ();
  ACE_ASSERT (buffer_p);
  buffer_p->data = data_p;
  buffer_p->length = size_i;
  buffer_p->data_deallocator = libacestream_ml_tensorflow_module_free_buffer;

  status_ = TF_NewStatus ();
  ACE_ASSERT (status_);
  graph_ = TF_NewGraph ();
  ACE_ASSERT (graph_);
  TF_ImportGraphDefOptions* options_p = TF_NewImportGraphDefOptions ();
  ACE_ASSERT (options_p);
  TF_GraphImportGraphDef (graph_, buffer_p, options_p, status_);
  TF_DeleteImportGraphDefOptions (options_p); options_p = NULL;
  if (unlikely (TF_GetCode (status_) != TF_OK))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to import model (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.modelFile.c_str ())));
    TF_DeleteGraph (graph_); graph_ = NULL;
    TF_DeleteBuffer (buffer_p);
    return false;
  } // end IF
  TF_DeleteBuffer (buffer_p); buffer_p = NULL;

  // create session
  TF_SessionOptions* options_2 = TF_NewSessionOptions ();
  ACE_ASSERT (options_2);
  session_ = TF_NewSession (graph_, options_2, status_);
  TF_DeleteSessionOptions (options_2); options_2 = NULL;
  if (TF_GetCode (status_) != TF_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to create session, aborting\n"),
                inherited::mod_->name ()));
    TF_DeleteSession (session_, status_); session_ = NULL;
    TF_DeleteGraph (graph_); graph_ = NULL;
    TF_DeleteStatus (status_); status_ = NULL;
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Tensorflow_T<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Tensorflow_T<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      break;

//error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}

//////////////////////////////////////////

#if defined (TENSORFLOW_CC_SUPPORT)
template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Tensorflow_2<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::Stream_Module_Tensorflow_2 (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , session_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_2::Stream_Module_Tensorflow_2"));

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Tensorflow_2<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::~Stream_Module_Tensorflow_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_2::~Stream_Module_Tensorflow_2"));

  if (session_)
  {
    session_->Close ();
    delete session_;
  } // end IF
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_Tensorflow_2<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_2::initialize"));

  if (inherited::isInitialized_)
  {
    if (session_)
    {
      session_->Close ();
      delete session_;
    } // end IF
    session_ = NULL;
  } // end IF

  tensorflow::GraphDef graph_def;
  tensorflow::Status status;
  // *NOTE*: model file needs to be relative to cwd
  status = tensorflow::ReadBinaryProto (tensorflow::Env::Default (),
                                        configuration_in.modelFile,
                                        &graph_def);
  //tensorflow::SavedModelBundle bundle;
  //status = tensorflow::LoadSavedModel (tensorflow::SessionOptions (),
  //                                     tensorflow::RunOptions (),
  //                                     Common_File_Tools::directory (configuration_in.modelFile),
  //                                     {tensorflow::kSavedModelTagServe},
  //                                     &bundle);
  if (!status.ok ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to LoadSavedModel() (model was: \"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.modelFile.c_str ()),
                ACE_TEXT (status.ToString ().c_str ())));
    return false;
  } // end IF
  //graph_def = bundle.meta_graph_def.graph_def ();
  //status = tensorflow::NewSession (tensorflow::SessionOptions (),
  //                                 &session_);
  //if (!status.ok () || !session_)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to NewSession(): \"%s\", aborting\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (status.ToString ().c_str ())));
  //  return false;
  //} // end IF
  session_ = tensorflow::NewSession (tensorflow::SessionOptions ());
  ACE_ASSERT (session_);
  status = session_->Create (graph_def);
  if (!status.ok ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Session::Create() (model was: \"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.modelFile.c_str ()),
                ACE_TEXT (status.ToString ().c_str ())));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Tensorflow_2<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_2::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Tensorflow_2<ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tensorflow_2::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

       // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

//  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
//    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      break;

//error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}
#endif // TENSORFLOW_CC_SUPPORT
