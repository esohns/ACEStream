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

#ifndef STREAM_MODULE_LLAMACPP_H
#define STREAM_MODULE_LLAMACPP_H

#include "llama.h"

#include <string>
#include <vector>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

void acestream_ggml_log_callback (enum ggml_log_level, const char*, void*);

extern const char libacestream_default_ml_llamacpp_module_name_string[];

template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_LlamaCpp_T
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Stream_Module_LlamaCpp_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_LlamaCpp_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  inline virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                            bool&) {}             // return value: pass message downstream ?

 protected:
  llama_context*                         context_;
  llama_model*                           model_;
  llama_sampler*                         sampler_;
  const char*                            template_;
  const llama_vocab*                     vocab_;

  std::vector<char>                      formatted_;
  std::vector<struct llama_chat_message> messages_;
  int                                    previousLength_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_LlamaCpp_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_LlamaCpp_T (const Stream_Module_LlamaCpp_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_LlamaCpp_T& operator= (const Stream_Module_LlamaCpp_T&))

  std::string generate (const std::string&); // prompt
};

// include template definition
#include "stream_module_llamacpp.inl"

#endif
