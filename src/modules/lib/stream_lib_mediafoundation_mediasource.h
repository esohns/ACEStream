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

#ifndef STREAM_LIB_MEDIAFOUNDATION_MEDIASOURCE_H
#define STREAM_LIB_MEDIAFOUNDATION_MEDIASOURCE_H

#include <deque>
#include <list>
#include <map>

#undef GetObject
#include "mfidl.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

template <typename MediaSourceType>
class Stream_MediaFramework_MediaFoundation_MediaStream_T
 : public IMFMediaStream
{
 public:
  typedef Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType> OWN_TYPE_T;

  Stream_MediaFramework_MediaFoundation_MediaStream_T (MediaSourceType*); // media source handle
  virtual ~Stream_MediaFramework_MediaFoundation_MediaStream_T ();

  static HRESULT CreateInstance (IUnknown*, // parent
                                 REFIID,    // interface id
                                 void**);   // return value: instance handle

  // implement IMFMediaStream
  // IUnknown
  virtual STDMETHODIMP QueryInterface (REFIID,
                                       void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&referenceCount_); }
  virtual STDMETHODIMP_ (ULONG) Release ();
  // IMFMediaEventGenerator
  virtual STDMETHODIMP BeginGetEvent (IMFAsyncCallback*, // asynchronous callback handle
                                      IUnknown*);        // 
  virtual STDMETHODIMP EndGetEvent (IMFAsyncResult*,  // asynchronous result
                                    IMFMediaEvent**); // return value: event handle
  virtual STDMETHODIMP GetEvent (DWORD,            // flags
                                 IMFMediaEvent**); // return value: event handle
  virtual STDMETHODIMP QueueEvent (MediaEventType,      // event type
                                   REFGUID,             // extended event type
                                   HRESULT,             // status
                                   const PROPVARIANT*); // value
  // IMFMediaStream
  inline virtual STDMETHODIMP GetMediaSource (IMFMediaSource** ppMediaSource) { ACE_ASSERT (mediaSource_); return mediaSource_->QueryInterface (IID_PPV_ARGS (ppMediaSource)); }
  virtual STDMETHODIMP GetStreamDescriptor (IMFStreamDescriptor**);
  virtual STDMETHODIMP RequestSample (IUnknown*); // pToken

  // *NOTE*: - allocation functions are always 'static'
  //         - "The call to the class - specific T::operator delete on a
  //           polymorphic class is the only case where a static member function
  //           is called through dynamic dispatch."

  // *NOTE*: these ensure that all instances are (de)allocated off their
  //         originating (DLL) heap
  static void operator delete (void*); // instance handle
  //static void operator delete (void*,   // instance handle
  //                             size_t); // number of bytes

  bool                selected_;

 private:
  // ctor used by the COM class factory
  Stream_MediaFramework_MediaFoundation_MediaStream_T (HRESULT*); // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_MediaStream_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_MediaStream_T (const Stream_MediaFramework_MediaFoundation_MediaStream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_MediaStream_T& operator= (const Stream_MediaFramework_MediaFoundation_MediaStream_T&))

  IMFMediaEventQueue* eventQueue_;
  MediaSourceType*    mediaSource_;
  volatile long       referenceCount_;
}; // Stream_MediaFramework_MediaFoundation_MediaStream_T

//////////////////////////////////////////

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
class Stream_MediaFramework_MediaFoundation_MediaSource_T
 : public IMFSchemeHandler
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
 , public IMFMediaSourceEx
#else
 , public IMFMediaSource
#endif // _WIN32_WINNT_WIN8
 //, public IMFPresentationDescriptor
 //, public IMFStreamDescriptor
 //, public IMFMediaTypeHandler
 , public IMFGetService
 //, public IMarshal
 , public Common_IInitialize_T<ConfigurationType>
{
  typedef Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                              MessageType,
                                                              ConfigurationType> OWN_TYPE_T;
  typedef Stream_MediaFramework_MediaFoundation_MediaStream_T<OWN_TYPE_T> STREAM_T;
  friend class STREAM_T;

 public:
  Stream_MediaFramework_MediaFoundation_MediaSource_T ();
  virtual ~Stream_MediaFramework_MediaFoundation_MediaSource_T ();

  static HRESULT CreateInstance (IUnknown*, // parent
                                 REFIID,    // interface id
                                 void**);   // return value: instance handle

  // implement IMFSchemeHandler
  virtual STDMETHODIMP BeginCreateObject (LPCWSTR,           // pwszURL
                                          DWORD,             // dwFlags
                                          IPropertyStore*,   // pProps
                                          IUnknown**,        // ppIUnknownCancelCookie
                                          IMFAsyncCallback*, // pCallback
                                          IUnknown*);        // punkState
  virtual STDMETHODIMP CancelObjectCreation (IUnknown*); // pIUnknownCancelCookie
  virtual STDMETHODIMP EndCreateObject (IMFAsyncResult*, // pResult
                                        MF_OBJECT_TYPE*, // pObjectType
                                        IUnknown**);     // ppObject

  // implement IMFMediaSource[Ex]
  // IUnknown
  virtual STDMETHODIMP QueryInterface (REFIID,
                                       void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&referenceCount_); }
  virtual STDMETHODIMP_ (ULONG) Release ();
  // IMFMediaEventGenerator
  virtual STDMETHODIMP BeginGetEvent (IMFAsyncCallback*, // asynchronous callback handle
                                      IUnknown*);        // 
  virtual STDMETHODIMP EndGetEvent (IMFAsyncResult*,  // asynchronous result
                                    IMFMediaEvent**); // return value: event handle
  virtual STDMETHODIMP GetEvent (DWORD,            // flags
                                 IMFMediaEvent**); // return value: event handle
  virtual STDMETHODIMP QueueEvent (MediaEventType,      // event type
                                   REFGUID,             // extended event type
                                   HRESULT,             // status
                                   const PROPVARIANT*); // value
  // IMFMediaSource
  virtual STDMETHODIMP GetCharacteristics (DWORD*); // return value: characteristics
  virtual STDMETHODIMP CreatePresentationDescriptor (IMFPresentationDescriptor**); // return value: presentation descriptor handle
  virtual STDMETHODIMP Start (IMFPresentationDescriptor*, // presentation descriptor handle
                              const GUID*,                // time format
                              const PROPVARIANT*);        // start position (in time format)
  virtual STDMETHODIMP Stop (void);
  virtual STDMETHODIMP Pause (void);
  virtual STDMETHODIMP Shutdown (void);
  // IMFMediaSourceEx
  virtual STDMETHODIMP GetSourceAttributes (IMFAttributes**); // return value: attributes
  virtual STDMETHODIMP GetStreamAttributes (DWORD, // stream identifier
                                            IMFAttributes**); // return value: attributes
  virtual STDMETHODIMP SetD3DManager (IUnknown*); // Direct3D manager handle

  //// IMFPresentationDescriptor
  // IMFAttributes (see below)
  //virtual STDMETHODIMP Clone (IMFPresentationDescriptor**); // ppPresentationDescriptor
  //virtual STDMETHODIMP DeselectStream (DWORD); // dwIndex
  //virtual STDMETHODIMP GetStreamDescriptorByIndex (DWORD,                  // dwIndex
  //                                                 BOOL*,                  // pfSelected
  //                                                 IMFStreamDescriptor**); // ppDescriptor
  //virtual STDMETHODIMP GetStreamDescriptorCount (DWORD*); // pdwDescriptorCount
  //virtual STDMETHODIMP SelectStream (DWORD); // dwIndex

  // implement IMFStreamDescriptor
  // IMFAttributes
  //virtual STDMETHODIMP GetItem (REFGUID, // guidKey
  //                              PROPVARIANT*) { ACE_ASSERT (false); return E_FAIL; } // pValue
  //virtual STDMETHODIMP GetItemType (REFGUID, // guidKey
  //                                  MF_ATTRIBUTE_TYPE*) { ACE_ASSERT (false); return E_FAIL; } // pType
  //virtual STDMETHODIMP CompareItem (REFGUID,        // guidKey
  //                                  REFPROPVARIANT, // Value
  //                                  BOOL*) { ACE_ASSERT (false); return E_FAIL; } // pbResult
  //virtual STDMETHODIMP Compare (IMFAttributes*,           // pTheirs
  //                              MF_ATTRIBUTES_MATCH_TYPE, // MatchType
  //                              BOOL*) { ACE_ASSERT (false); return E_FAIL; } // pbResult
  //virtual STDMETHODIMP GetUINT32 (REFGUID,  // guidKey
  //                                UINT32*); // punValue
  //virtual STDMETHODIMP GetUINT64 (REFGUID,  // guidKey
  //                                UINT64*); // punValue
  //virtual STDMETHODIMP GetDouble (REFGUID, // guidKey
  //                                double*) { ACE_ASSERT (false); return E_FAIL; } // pfValue
  //virtual STDMETHODIMP GetGUID (REFGUID, // guidKey
  //                              GUID*) { ACE_ASSERT (false); return E_FAIL; } // pguidValue
  //virtual STDMETHODIMP GetStringLength (REFGUID, // guidKey
  //                                      UINT32*) { ACE_ASSERT (false); return E_FAIL; } // pcchLength
  //virtual STDMETHODIMP GetString (REFGUID, // guidKey
  //                                LPWSTR,  // pwszValue
  //                                UINT32,  // cchBufSize
  //                                UINT32*) { ACE_ASSERT (false); return E_FAIL; } // pcchLength
  //virtual STDMETHODIMP GetAllocatedString (REFGUID, // guidKey
  //                                         LPWSTR*, // ppwszValue
  //                                         UINT32*) { ACE_ASSERT (false); return E_FAIL; } // pcchLength
  //virtual STDMETHODIMP GetBlobSize (REFGUID,  // guidKey
  //                                  UINT32*); // pcbBlobSize
  //virtual STDMETHODIMP GetBlob (REFGUID, // guidKey
  //                              UINT8*,  // pBuf
  //                              UINT32,  // cbBufSize
  //                              UINT32*) { ACE_ASSERT (false); return E_FAIL; } // pcbBlobSize
  //virtual STDMETHODIMP GetAllocatedBlob (REFGUID, // guidKey
  //                                       UINT8**, // ppBuf
  //                                       UINT32*) { ACE_ASSERT (false); return E_FAIL; } // pcbSize
  //virtual STDMETHODIMP GetUnknown (REFGUID, // guidKey
  //                                 REFIID,  // riid
  //                                 LPVOID*) { ACE_ASSERT (false); return E_FAIL; } // ppv
  //virtual STDMETHODIMP SetItem (REFGUID, // guidKey
  //                              REFPROPVARIANT) { ACE_ASSERT (false); return E_FAIL; } // Value
  //virtual STDMETHODIMP DeleteItem (REFGUID); // guidKey
  //virtual STDMETHODIMP DeleteAllItems (void) { ACE_ASSERT (false); return E_FAIL; }
  //virtual STDMETHODIMP SetUINT32 (REFGUID, // guidKey
  //                                UINT32) { ACE_ASSERT (false); return E_FAIL; } // unValue
  //virtual STDMETHODIMP SetUINT64 (REFGUID, // guidKey
  //                                UINT64) { ACE_ASSERT (false); return E_FAIL; } // unValue
  //virtual STDMETHODIMP SetDouble (REFGUID, // guidKey
  //                                double) { ACE_ASSERT (false); return E_FAIL; } // fValue
  //virtual STDMETHODIMP SetGUID (REFGUID, // guidKey
  //                              REFGUID) { ACE_ASSERT (false); return E_FAIL; } // guidValue
  //virtual STDMETHODIMP SetString (REFGUID, // guidKey
  //                                LPCWSTR) { ACE_ASSERT (false); return E_FAIL; } // wszValue
  //virtual STDMETHODIMP SetBlob (REFGUID,      // guidKey
  //                              const UINT8*, // pBuf
  //                              UINT32) { ACE_ASSERT (false); return E_FAIL; } // cbBufSize
  //virtual STDMETHODIMP SetUnknown (REFGUID,    // guidKey
  //                                 IUnknown*); // pUnknown
  //virtual STDMETHODIMP LockStore (void) { ACE_ASSERT (false); return E_FAIL; }
  //virtual STDMETHODIMP UnlockStore (void) { ACE_ASSERT (false); return E_FAIL; }
  //virtual STDMETHODIMP GetCount (UINT32*) { ACE_ASSERT (false); return E_FAIL; } // pcItems
  //virtual STDMETHODIMP GetItemByIndex (UINT32, // unIndex
  //                                     GUID*,  // pguidKey
  //                                     PROPVARIANT*) { ACE_ASSERT (false); return E_FAIL; } // pValue
  //virtual STDMETHODIMP CopyAllItems (IMFAttributes*) { ACE_ASSERT (false); return E_FAIL; } // pDest
  //// IMFStreamDescriptor
  //virtual STDMETHODIMP GetMediaTypeHandler (IMFMediaTypeHandler**); // ppMediaTypeHandler
  //virtual STDMETHODIMP GetStreamIdentifier (DWORD*); // pdwStreamIdentifier

  // IMFMediaTypeHandler
  //virtual STDMETHODIMP GetCurrentMediaType (IMFMediaType**); // ppMediaType
  //virtual STDMETHODIMP GetMajorType (GUID*); // pguidMajorType
  //virtual STDMETHODIMP GetMediaTypeByIndex (DWORD,           // dwIndex
  //                                          IMFMediaType**); // ppType
  //virtual STDMETHODIMP GetMediaTypeCount (DWORD*); // pdwTypeCount
  //virtual STDMETHODIMP IsMediaTypeSupported (IMFMediaType*, // pMediaType
  //                                           IMFMediaType**); // ppMediaType
  //virtual STDMETHODIMP SetCurrentMediaType (IMFMediaType*); // pMediaType

  // IMFGetService
  virtual STDMETHODIMP GetService (REFGUID,  // guidService
                                   REFIID,   // riid
                                   LPVOID*); // ppvObject

  //// IMarshal
  //virtual STDMETHODIMP GetUnmarshalClass (REFIID riid,
  //                                        void *pv,
  //                                        DWORD dwDestContext,
  //                                        void *pvDestContext,
  //                                        DWORD mshlflags,
  //                                        CLSID *pCid);
  //virtual STDMETHODIMP GetMarshalSizeMax (REFIID riid,
  //                                        void *pv,
  //                                        DWORD dwDestContext,
  //                                        void *pvDestContext,
  //                                        DWORD mshlflags,
  //                                        DWORD *pSize);
  //virtual STDMETHODIMP MarshalInterface (IStream *pStm,
  //                                       REFIID riid,
  //                                       void *pv,
  //                                       DWORD dwDestContext,
  //                                       void *pvDestContext,
  //                                       DWORD mshlflags);
  //virtual STDMETHODIMP UnmarshalInterface (IStream *pStm,
  //                                         REFIID riid,
  //                                         void **ppv);
  //virtual STDMETHODIMP ReleaseMarshalData (IStream *pStm);
  //virtual STDMETHODIMP DisconnectObject (DWORD dwReserved);

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

  // *NOTE*: - allocation functions are always 'static'
  //         - "The call to the class - specific T::operator delete on a
  //           polymorphic class is the only case where a static member function
  //           is called through dynamic dispatch."

  // *NOTE*: these ensure that all instances are (de)allocated off their
  //         originating (DLL) heap
  static void operator delete (void*); // instance handle
  //static void operator delete (void*,   // instance handle
  //                             size_t); // number of bytes

 protected:
  // convenient types
  //typedef std::map<BYTE, DWORD> STREAM_MAP_T; // maps stream id to index
  //typedef STREAM_MAP_T::iterator STREAM_MAP_ITERATOR_T;
  //typedef std::list<IMFSample*> SAMPLE_LIST_T;
  //typedef SAMPLE_LIST_T::iterator SAMPLE_LIST_ITERATOR_T;
  typedef std::deque<IUnknown*> TOKEN_LIST_T; // List of tokens for IMFMediaStream::RequestSample
  typedef TOKEN_LIST_T::iterator TOKEN_LIST_ITERATOR_T;

  typedef enum
  {
    STATE_INVALID = -1,
    STATE_INITIALIZED = 0,
    STATE_STOPPED,
    STATE_PAUSED,
    STATE_STARTED,
    STATE_SHUTDOWN
  } STATE_T;

  bool                       buffering_;
  ConfigurationType*         configuration_;
  IMFMediaEventQueue*        eventQueue_;
  //bool                hasCOMReference_;
  ACE_SYNCH_MUTEX            lock_;
  STREAM_T*                  mediaStream_;
  IMFPresentationDescriptor* presentationDescriptor_;
  volatile long              referenceCount_;
  STATE_T                    state_;
  TOKEN_LIST_T               tokens_;

 private:  
  // ctor used by the COM class factory
  Stream_MediaFramework_MediaFoundation_MediaSource_T (HRESULT*); // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_MediaSource_T (const Stream_MediaFramework_MediaFoundation_MediaSource_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_MediaSource_T& operator= (const Stream_MediaFramework_MediaFoundation_MediaSource_T&))
}; // Stream_MediaFramework_MediaFoundation_MediaSource_T

// include template definition
#include "stream_lib_mediafoundation_mediasource.inl"

#endif
