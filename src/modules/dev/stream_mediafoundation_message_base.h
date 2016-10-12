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

#ifndef STREAM_MEDIAFOUNDATION_MESSAGE_BASE_H
#define STREAM_MEDIAFOUNDATION_MESSAGE_BASE_H

#include <ace/Global_Macros.h>

//#include <mfobjects.h>

//#include "stream_common.h"
#include "stream_data_message_base.h"
#include "stream_message_base.h"
#include "stream_session_message_base.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename DataType>
class Stream_MediaFoundationMessageBase_T
 : public Stream_DataMessageBase_T<AllocatorConfigurationType,
                                   ControlMessageType,
                                   SessionMessageType,
                                   DataType,
                                   int>
 //, public IMFSample
{
 public:
  // convenient types
  typedef Stream_MediaFoundationMessageBase_T<AllocatorConfigurationType,
                                              ControlMessageType,
                                              SessionMessageType,
                                              DataType> OWN_TYPE_T;
  typedef DataType DATA_T;

  virtual ~Stream_MediaFoundationMessageBase_T ();

  //// implement IMFSample
  //virtual HRESULT STDMETHODCALLTYPE GetSampleFlags (DWORD*);
  //virtual HRESULT STDMETHODCALLTYPE SetSampleFlags (DWORD);
  //virtual HRESULT STDMETHODCALLTYPE GetSampleTime (LONGLONG*);
  //virtual HRESULT STDMETHODCALLTYPE SetSampleTime (LONGLONG);
  //virtual HRESULT STDMETHODCALLTYPE GetSampleDuration (LONGLONG*);
  //virtual HRESULT STDMETHODCALLTYPE SetSampleDuration (LONGLONG);
  //virtual HRESULT STDMETHODCALLTYPE GetBufferCount (DWORD*);
  //virtual HRESULT STDMETHODCALLTYPE GetBufferByIndex (DWORD,
  //                                                    IMFMediaBuffer**);
  //virtual HRESULT STDMETHODCALLTYPE ConvertToContiguousBuffer (IMFMediaBuffer**);
  //virtual HRESULT STDMETHODCALLTYPE AddBuffer (IMFMediaBuffer*);
  //virtual HRESULT STDMETHODCALLTYPE RemoveBufferByIndex (DWORD);
  //virtual HRESULT STDMETHODCALLTYPE RemoveAllBuffers (void);
  //virtual HRESULT STDMETHODCALLTYPE GetTotalLength (DWORD*);
  //virtual HRESULT STDMETHODCALLTYPE CopyToBuffer (IMFMediaBuffer*);
  //// implement IMFAttributes
  //virtual HRESULT STDMETHODCALLTYPE GetItem (const struct _GUID&,
  //                                           struct tagPROPVARIANT*);
  //virtual HRESULT STDMETHODCALLTYPE GetItemType (const struct _GUID&,
  //                                               enum _MF_ATTRIBUTE_TYPE*);
  //virtual HRESULT STDMETHODCALLTYPE CompareItem (const struct _GUID&,
  //                                               const struct tagPROPVARIANT&,
  //                                               BOOL*);
  //virtual HRESULT STDMETHODCALLTYPE Compare (IMFAttributes*,
  //                                           enum _MF_ATTRIBUTES_MATCH_TYPE,
  //                                           BOOL*);
  //virtual HRESULT STDMETHODCALLTYPE GetUINT32 (const struct _GUID&,
  //                                             UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetUINT64 (const struct _GUID&,
  //                                             UINT64*);
  //virtual HRESULT STDMETHODCALLTYPE GetDouble (const struct _GUID&,
  //                                             double*);
  //virtual HRESULT STDMETHODCALLTYPE GetGUID (const struct _GUID&,
  //                                           struct _GUID*);
  //virtual HRESULT STDMETHODCALLTYPE GetStringLength (const struct _GUID&,
  //                                                   UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetString (const struct _GUID&,
  //                                             LPWSTR,
  //                                             UINT32,
  //                                             UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetAllocatedString (const struct _GUID&,
  //                                                      LPWSTR*,
  //                                                      UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetBlobSize (const struct _GUID&,
  //                                               UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetBlob (const struct _GUID&,
  //                                           UINT8*,
  //                                           UINT32,
  //                                           UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetAllocatedBlob (const struct _GUID&,
  //                                                    UINT8**,
  //                                                    UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetUnknown (const struct _GUID&,
  //                                              const IID&,
  //                                              LPVOID*);
  //virtual HRESULT STDMETHODCALLTYPE SetItem (const struct _GUID&,
  //                                           const struct tagPROPVARIANT&);
  //virtual HRESULT STDMETHODCALLTYPE DeleteItem (const struct _GUID&);
  //virtual HRESULT STDMETHODCALLTYPE DeleteAllItems (void);
  //virtual HRESULT STDMETHODCALLTYPE SetUINT32 (const struct _GUID&,
  //                                             UINT32);
  //virtual HRESULT STDMETHODCALLTYPE SetUINT64 (const struct _GUID&,
  //                                             UINT64);
  //virtual HRESULT STDMETHODCALLTYPE SetDouble (const struct _GUID&,
  //                                             double);
  //virtual HRESULT STDMETHODCALLTYPE SetGUID (const struct _GUID&,
  //                                           const struct _GUID&);
  //virtual HRESULT STDMETHODCALLTYPE SetString (const struct _GUID&,
  //                                             LPCWSTR);
  //virtual HRESULT STDMETHODCALLTYPE SetBlob (const struct _GUID&,
  //                                           const UINT8*,
  //                                           UINT32);
  //virtual HRESULT STDMETHODCALLTYPE SetUnknown (const struct _GUID&,
  //                                              IUnknown*);
  //virtual HRESULT STDMETHODCALLTYPE LockStore (void);
  //virtual HRESULT STDMETHODCALLTYPE UnlockStore (void);
  //virtual HRESULT STDMETHODCALLTYPE GetCount (UINT32*);
  //virtual HRESULT STDMETHODCALLTYPE GetItemByIndex (UINT32,
  //                                                  struct _GUID*,
  //                                                  const struct tagPROPVARIANT*);
  //virtual HRESULT STDMETHODCALLTYPE CopyAllItems (IMFAttributes*);
  //// implement IUnknown
  //virtual HRESULT STDMETHODCALLTYPE QueryInterface (const IID&,
  //                                                  void**);
  //virtual ULONG STDMETHODCALLTYPE AddRef (void);
  //virtual ULONG STDMETHODCALLTYPE Release (void);

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  Stream_MediaFoundationMessageBase_T (unsigned int); // size
  // copy ctor, to be used by derivates
  Stream_MediaFoundationMessageBase_T (const OWN_TYPE_T&);
  // *NOTE*: to be used by message allocators
  Stream_MediaFoundationMessageBase_T (ACE_Data_Block*, // data block
                                       ACE_Allocator*,  // message allocator
                                       bool = true);    // increment running message counter ?
  Stream_MediaFoundationMessageBase_T (ACE_Allocator*); // message allocator

 private:
  typedef Stream_DataMessageBase_T<AllocatorConfigurationType,
                                   ControlMessageType,
                                   SessionMessageType,
                                   DataType,
                                   int> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFoundationMessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFoundationMessageBase_T& operator= (const Stream_MediaFoundationMessageBase_T&))

  // overriden from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;
};

// include template definition
#include "stream_mediafoundation_message_base.inl"

#endif
