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

#ifndef STREAM_DIRECTSHOW_MESSAGE_BASE_H
#define STREAM_DIRECTSHOW_MESSAGE_BASE_H

#include <ace/Global_Macros.h>

#include <strmif.h>

#include "stream_data_message_base.h"
#include "stream_directshow_allocator_base.h"
#include "stream_message_base.h"
#include "stream_session_message_base.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType = int>
class Stream_DirectShowMessageBase_T
 : public Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType>
 , public IMediaSample
{
  // grant access to specific ctors
  //friend class Stream_DirectShowAllocatorBase_T<AllocatorConfigurationType,
  //                                              Stream_DirectShowMessageBase_T<AllocatorConfigurationType,
  //                                                                             ControlMessageType,
  //                                                                             SessionMessageType>,
  //                                              Stream_SessionMessageBase_T<AllocatorConfigurationType,
  //                                                                          Stream_SessionMessageType,
  //                                                                          Stream_SessionData,
  //                                                                          Stream_UserData,
  //                                                                          ControlMessageType,
  //                                                                          Stream_DirectShowMessageBase_T<AllocatorConfigurationType,
                                                                                                           //////ControlMessageType,
                                                                                                           //////SessionMessageType> > >;

 public:
  // convenient types
  typedef Stream_DirectShowMessageBase_T<AllocatorConfigurationType,
                                         MessageType,
                                         CommandType> OWN_TYPE_T;

  virtual ~Stream_DirectShowMessageBase_T ();

  // implement IMediaSample
  virtual HRESULT STDMETHODCALLTYPE GetPointer (BYTE**);
  virtual long STDMETHODCALLTYPE GetSize (void);
  virtual HRESULT STDMETHODCALLTYPE GetTime (REFERENCE_TIME*,
                                             REFERENCE_TIME*);
  virtual HRESULT STDMETHODCALLTYPE SetTime (REFERENCE_TIME*,
                                             REFERENCE_TIME*);
  virtual HRESULT STDMETHODCALLTYPE IsSyncPoint (void);
  virtual HRESULT STDMETHODCALLTYPE SetSyncPoint (BOOL);
  virtual HRESULT STDMETHODCALLTYPE IsPreroll (void);
  virtual HRESULT STDMETHODCALLTYPE SetPreroll (BOOL);
  virtual long STDMETHODCALLTYPE GetActualDataLength (void);
  virtual HRESULT STDMETHODCALLTYPE SetActualDataLength (long);
  virtual HRESULT STDMETHODCALLTYPE GetMediaType (AM_MEDIA_TYPE**);
  virtual HRESULT STDMETHODCALLTYPE SetMediaType (AM_MEDIA_TYPE*);
  virtual HRESULT STDMETHODCALLTYPE IsDiscontinuity (void);
  virtual HRESULT STDMETHODCALLTYPE SetDiscontinuity (BOOL);
  virtual HRESULT STDMETHODCALLTYPE GetMediaTime (LONGLONG*,
                                                  LONGLONG*);
  virtual HRESULT STDMETHODCALLTYPE SetMediaTime (LONGLONG*,
                                                  LONGLONG*);
  // implement IUnknown
  virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID,
                                                    void**);
  virtual ULONG STDMETHODCALLTYPE AddRef (void);
  virtual ULONG STDMETHODCALLTYPE Release (void);

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  Stream_DirectShowMessageBase_T (unsigned int); // size
  // copy ctor, to be used by derivates
  Stream_DirectShowMessageBase_T (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  Stream_DirectShowMessageBase_T (ACE_Data_Block*, // data block
                                  ACE_Allocator*,  // message allocator
                                  bool = true);    // increment running message counter ?
  Stream_DirectShowMessageBase_T (ACE_Allocator*); // message allocator

 private:
  typedef Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_DirectShowMessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_DirectShowMessageBase_T& operator= (const Stream_DirectShowMessageBase_T&))

  // overriden from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

  double timeStamp_;
};

// include template definition
#include "stream_directshow_message_base.inl"

#endif
