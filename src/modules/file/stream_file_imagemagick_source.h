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

#ifndef STREAM_FILE_IMAGEMAGICK_SOURCE_T_H
#define STREAM_FILE_IMAGEMAGICK_SOURCE_T_H

#include "ace/Dirent_Selector.h"
#include "ace/Global_Macros.h"

#include "common_ilock.h"
#include "common_time_common.h"

#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;
struct _MagickWand;

extern const char libacestream_default_file_imagemagick_source_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          ////////////////////////////////
          typename MediaType>
class Stream_File_ImageMagick_Source_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      StreamStateType,
                                      typename SessionMessageType::DATA_T::DATA_T,
                                      typename SessionMessageType::DATA_T,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      StreamStateType,
                                      typename SessionMessageType::DATA_T::DATA_T,
                                      typename SessionMessageType::DATA_T,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_File_ImageMagick_Source_T (ISTREAM_T*, // stream handle
#else
  Stream_File_ImageMagick_Source_T (typename inherited::ISTREAM_T*,                                           // stream handle
#endif // ACE_WIN32 || ACE_WIN64
                                    bool = false,                                                             // auto-start ? (active mode only)
                                    enum Stream_HeadModuleConcurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE, // concurrency mode
                                    bool = true);                                                             // generate session messages ?
  virtual ~Stream_File_ImageMagick_Source_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                    Common_TimePolicy_t,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    StreamStateType,
                                    typename SessionMessageType::DATA_T::DATA_T,
                                    typename SessionMessageType::DATA_T,
                                    StatisticContainerType,
                                    TimerManagerType,
                                    UserDataType>::initialize;
#endif // __GNUG__ || _MSC_VER

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

 private:
  // convenient types
  typedef Stream_File_ImageMagick_Source_T<ACE_SYNCH_USE,
                                           TimePolicyType,
                                           ConfigurationType,
                                           ControlMessageType,
                                           DataMessageType,
                                           SessionMessageType,
                                           StreamStateType,
                                           StatisticContainerType,
                                           TimerManagerType,
                                           UserDataType,
                                           MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_File_ImageMagick_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_File_ImageMagick_Source_T (const Stream_File_ImageMagick_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_File_ImageMagick_Source_T& operator= (const Stream_File_ImageMagick_Source_T&))

  // helper methods
  virtual int svc (void);

  struct _MagickWand* context_;
  ACE_Dirent_Selector directory_;
};

// include template definition
#include "stream_file_imagemagick_source.inl"

#endif
