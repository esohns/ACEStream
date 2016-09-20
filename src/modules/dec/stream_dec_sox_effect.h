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

#ifndef STREAM_DEC_SOX_EFFECT_H
#define STREAM_DEC_SOX_EFFECT_H

#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "sox.h"
#endif

#include "ace/Global_Macros.h"

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "stream_dev_common.h"

// forward declarations
class ACE_Message_Block;

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
          typename SessionDataType>
class Stream_Decoder_SoXEffect_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType>
{
 public:
  Stream_Decoder_SoXEffect_T ();
  virtual ~Stream_Decoder_SoXEffect_T ();

  //// override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // helper methods
  DataMessageType* allocateMessage (unsigned int); // requested size
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: callers must free the return value !
  template <typename FormatType> AM_MEDIA_TYPE* getFormat (const FormatType format_in) { return getFormat_impl (format_in); }
#else
  template <typename FormatType> struct v4l2_format* getFormat (const FormatType format_in) { return getFormat_impl (format_in); }
#endif

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_SoXEffect_T (const Stream_Decoder_SoXEffect_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_SoXEffect_T& operator= (const Stream_Decoder_SoXEffect_T&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  AM_MEDIA_TYPE* getFormat_impl (const struct _AMMediaType*); // return value: media type handle
  AM_MEDIA_TYPE* getFormat_impl (const IMFMediaType*); // return value: media type handle
#else
  struct v4l2_format* getFormat_impl (const struct Stream_Module_Device_ALSAConfiguration&); // return value: media type handle
  inline struct v4l2_format* getFormat_impl (const struct v4l2_format* format_in) { return const_cast<struct v4l2_format*> (format_in); } // return value: media type handle
#endif

  ACE_Message_Block*          buffer_;
  struct sox_effects_chain_t* chain_;
  struct sox_encodinginfo_t   encodingInfo_;
  bool                        manageSoX_;
  struct sox_signalinfo_t     signalInfo_;
  struct sox_format_t*        SoXBuffer_;
};

// include template definition
#include "stream_dec_sox_effect.inl"

#endif
