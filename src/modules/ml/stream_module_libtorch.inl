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

#include "torch/script.h"

#include "ace/Log_Msg.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Libtorch_T<ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType>::Stream_Module_Libtorch_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , device_ (torch::kCPU)
 , module_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Libtorch_T::Stream_Module_Libtorch_T"));

  torch::set_num_threads (Common_Tools::getNumberOfCPUs (true));

  if (torch::cuda::is_available ())
  {
    device_ = torch::Device (torch::kCUDA);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("CUDA is available, using GPU\n")));
  } // end IF
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Libtorch_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::~Stream_Module_Libtorch_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Libtorch_T::~Stream_Module_Libtorch_T"));

  //if (module_)
  //  delete module_;
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_Libtorch_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Libtorch_T::initialize"));

  if (inherited::isInitialized_)
  {
  } // end IF

  bool load_debug_files_b = false;
#if defined (_DEBUG)
  load_debug_files_b = true;
#endif // _DEBUG

  module_ = torch::jit::load (configuration_in.model.c_str (),
                              device_,
                              load_debug_files_b);
  //if (unlikely (!module_))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to load model (was: \"%s\"), aborting\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (configuration_in.model.c_str ())));
  //  return false;
  //} // end IF
  //module_.to (device_);
  module_.eval ();

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Libtorch_T<ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Libtorch_T::handleDataMessage"));

  // sanity check(s)
  //ACE_ASSERT (module_);

  static int nFrames = 30; // i.e. recompute fps roughly every second of footage
  static int iFrame = 0;
  static float fps = 0.0f;
  static time_t start = time (NULL);
  static time_t end;

  if (nFrames % (iFrame + 1) == 0)
  {
    time (&end);
    fps = nFrames / (float)difftime (end, start);
    time (&start);
  } // end IF
  ++iFrame;

  std::vector<torch::jit::IValue> inputs;
  inputs.push_back (torch::ones ({1, 3, 224, 224}).to (device_));

  // execute the model and turn its output into a tensor
  at::Tensor output = module_.forward (inputs).toTensor ();
  std::cout << output.slice (/*dim=*/1, /*start=*/0, /*end=*/5) << std::endl;

  // step3b: draw fps
  std::ostringstream converter;
  converter << fps;
}
