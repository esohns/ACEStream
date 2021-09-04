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

#ifndef STREAM_VIS_LIBAV_RESIZE_T_H
#define STREAM_VIS_LIBAV_RESIZE_T_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_image_common.h"

#include "stream_common.h"

#include "stream_dec_libav_converter.h"

extern const char libacestream_default_vis_libav_resize_module_name_string[];

// forward declarations
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
          typename MediaType> // session data-
class Stream_Visualization_LibAVResize_T
 : public Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataContainerType,
                                          MediaType>
{
  typedef Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
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
  Stream_Visualization_LibAVResize_T (ISTREAM_T*); // stream handle
#else
  Stream_Visualization_LibAVResize_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Stream_Visualization_LibAVResize_T () {}

  // override (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_LibAVResize_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_LibAVResize_T (const Stream_Visualization_LibAVResize_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_LibAVResize_T& operator= (const Stream_Visualization_LibAVResize_T&))

  Common_Image_Resolution_t sourceResolution_;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename MediaType> // session data-
class Stream_Visualization_LibAVResize1_T
 : public Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          typename SessionMessageType::DATA_T,
                                          MediaType>
{
  typedef Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          typename SessionMessageType::DATA_T,
                                          MediaType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Visualization_LibAVResize1_T (ISTREAM_T*); // stream handle
#else
  Stream_Visualization_LibAVResize1_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Stream_Visualization_LibAVResize1_T () {}

//  // override (part of) Stream_IModuleHandler_T
//  virtual bool initialize (const ConfigurationType&,
//                           Stream_IAllocator*);

  // override (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_LibAVResize1_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_LibAVResize1_T (const Stream_Visualization_LibAVResize1_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_LibAVResize1_T& operator= (const Stream_Visualization_LibAVResize1_T&))
};

//////////////////////////////////////////

// include template definition
#include "stream_vis_libav_resize.inl"

#endif
