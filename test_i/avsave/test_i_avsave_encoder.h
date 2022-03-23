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

#ifndef TEST_I_AVSAVE_ENCODER_T_H
#define TEST_I_AVSAVE_ENCODER_T_H

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Global_Macros.h"
#include "ace/Condition_T.h"
#include "ace/Thread_Mutex.h"

#include "stream_streammodule_base.h"
#include "stream_task_base_asynch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dec_libav_encoder.h"

#include "test_i_avsave_common.h"
#include "test_i_avsave_message.h"

// forward declaration(s)
struct AVFrame;
struct SwsContext;
class ACE_Message_Block;
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType> // audio/video !
class Test_I_AVSave_Encoder_T
 : public Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
                                        MediaType>
{
  typedef Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
                                        MediaType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVSave_Encoder_T (ISTREAM_T*); // stream handle
#else
  Test_I_AVSave_Encoder_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_AVSave_Encoder_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef Test_I_AVSave_Encoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType,
                                  MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_AVSave_Encoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVSave_Encoder_T (const Test_I_AVSave_Encoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVSave_Encoder_T& operator= (const Test_I_AVSave_Encoder_T&))
};

// include template definition
#include "test_i_avsave_encoder.inl"

//////////////////////////////////////////

#if defined(ACE_WIN32) || defined(ACE_WIN64)
typedef Test_I_AVSave_Encoder_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct
                                Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_AVSave_DirectShow_Message_t,
                                Stream_AVSave_DirectShow_SessionMessage_t,
                                Stream_AVSave_DirectShow_SessionData_t,
                                struct
                                Stream_MediaFramework_DirectShow_AudioVideoFormat> Stream_AVSave_DirectShow_Encoder;

DATASTREAM_MODULE_DUPLEX_A (Stream_AVSave_DirectShow_SessionData,                       // session data type
                            enum Stream_SessionMessageType,                             // session event type
                            struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                            libacestream_default_dec_libav_encoder_module_name_string,
                            Stream_INotify_t,                                           // stream notification interface type
                            Stream_AVSave_DirectShow_Encoder::READER_TASK_T,            // reader type
                            Stream_AVSave_DirectShow_Encoder,                           // writer type
                            Stream_AVSave_DirectShow_Encoder);                          // module name

typedef Test_I_AVSave_Encoder_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct
                                Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_AVSave_MediaFoundation_Message_t,
                                Stream_AVSave_MediaFoundation_SessionMessage_t,
                                Stream_AVSave_MediaFoundation_SessionData_t,
                                struct
                                Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Stream_AVSave_MediaFoundation_Encoder;

DATASTREAM_MODULE_DUPLEX_A (Stream_AVSave_MediaFoundation_SessionData,                       // session data type
                            enum Stream_SessionMessageType,                                  // session event type
                            struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                            libacestream_default_dec_libav_encoder_module_name_string,
                            Stream_INotify_t,                                                // stream notification interface type
                            Stream_AVSave_MediaFoundation_Encoder::READER_TASK_T,            // reader type
                            Stream_AVSave_MediaFoundation_Encoder,                           // writer type
                            Stream_AVSave_MediaFoundation_Encoder);                          // module name
#else
typedef Test_I_AVSave_Encoder_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct
                                Stream_AVSave_ALSA_V4L_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_AVSave_Message_t,
                                Stream_AVSave_ALSA_V4L_SessionMessage_t,
                                Stream_AVSave_ALSA_V4L_SessionData_t,
                                struct Stream_MediaFramework_ALSA_V4L_Format> Stream_AVSave_ALSA_V4L_Encoder;

DATASTREAM_MODULE_DUPLEX_A (Stream_AVSave_ALSA_V4L_SessionData,                        // session data type
                            enum Stream_SessionMessageType,                            // session event type
                            struct Stream_AVSave_ALSA_V4L_ModuleHandlerConfiguration,  // module handler configuration type
                            libacestream_default_dec_libav_encoder_module_name_string,
                            Stream_INotify_t,                                          // stream notification interface type
                            Stream_AVSave_ALSA_V4L_Encoder::READER_TASK_T,             // reader type
                            Stream_AVSave_ALSA_V4L_Encoder,                            // writer type
                            Stream_AVSave_ALSA_V4L_Encoder);                           // module name
#endif // ACE_WIN32 || ACE_WIN64

#endif
