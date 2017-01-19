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

#ifndef STREAM_MISC_DIRECTSHOW_ASYNCH_SOURCE_FILTER_H
#define STREAM_MISC_DIRECTSHOW_ASYNCH_SOURCE_FILTER_H

#include <queue>

#ifdef  _MSC_VER
//#if (_MSC_VER >= 1200)
//#pragma warning(push)
//#endif
//// disable some level-4 warnings, use #pragma warning(default:###) to re-enable
//#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
//#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
//#pragma warning(disable:4511) // warning C4511: copy constructor could not be generated
//#pragma warning(disable:4512) // warning C4512: assignment operator could not be generated
//#pragma warning(disable:4514) // warning C4514: "unreferenced inline function has been removed"

#if _MSC_VER>=1100
#define AM_NOVTABLE __declspec(novtable)
#else
#define AM_NOVTABLE
#endif
#endif  // MSC_VER

// *IMPORTANT NOTE*: the MSVC compiler does not like streams.h to be included
//                   several times (complains about media GUIDs being defined
//                   multiple times)
//                   --> to work around this, the following are included instead
//                   DO NOT TOUCH (unless you know what you are doing)
//#include <streams.h>
#include <strmif.h>
#include <wxdebug.h>
#include <combase.h>
#include <minwindef.h>
#include <wtypes.h>
#include <mmiscapi2.h>
#include <wxutil.h>
#include <mmreg.h>
#include <mtype.h>
#include <reftime.h>
#include <wxlist.h>
#include <amfilter.h>
#include <source.h>

#include <ace/Global_Macros.h>
#include <ace/Message_Queue.h>

#include "common_iinitialize.h"

#include "stream_task_base_synch.h"

// forward declarations
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T;

template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
class Stream_Misc_DirectShow_Asynch_Source_Filter_T
 : public CSource
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitializeP_T<MediaType>
{
  typedef Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T<PinConfigurationType,
                                                                 Stream_Misc_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                                                                               SessionMessageType,
                                                                                                               DataMessageType,
                                                                                                               ConfigurationType,
                                                                                                               PinConfigurationType,
                                                                                                               MediaType>,
                                                                 MediaType> OUTPUT_PIN_T;
  friend OUTPUT_PIN_T;

 public:
  // convenient types
  typedef ConfigurationType CONFIG_T;

  virtual ~Stream_Misc_DirectShow_Asynch_Source_Filter_T ();

  static CUnknown* WINAPI CreateInstance (LPUNKNOWN, // aggregating IUnknown interface handle ('owner')
                                          HRESULT*); // return value: result
  static void WINAPI DeleteInstance (void*); // instance handle

  // ------------------------------------

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);
  // *NOTE*: sets the preferred (i.e. default) media type
  virtual bool initialize (const MediaType*);

  // ------------------------------------

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
  // non-COM (!) ctor
  Stream_Misc_DirectShow_Asynch_Source_Filter_T ();

  ConfigurationType* filterConfiguration_;

 private:
  typedef CSource inherited;

  // ctor used by the COM class factory
  Stream_Misc_DirectShow_Asynch_Source_Filter_T (LPTSTR,              // name
                                                 LPUNKNOWN,           // aggregating IUnknown interface handle ('owner')
                                                 const struct _GUID&, // CLSID
                                                 HRESULT*);           // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Asynch_Source_Filter_T (const Stream_Misc_DirectShow_Asynch_Source_Filter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Asynch_Source_Filter_T& operator= (const Stream_Misc_DirectShow_Asynch_Source_Filter_T&))

  // convenient types
  typedef Stream_Misc_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType> OWN_TYPE_T;
  typedef Common_IInitialize_T<PinConfigurationType> IPIN_INITIALIZE_T;
  typedef Common_IInitialize_T<MediaType> IPIN_MEDIA_INITIALIZE_T;

  //bool         hasCOMReference_;
}; // Stream_Misc_DirectShow_Asynch_Source_Filter_T

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T
 : public CBasePin
 , public IKsPropertySet
 , public IAMBufferNegotiation
 , public IAMStreamConfig
 , public IAsyncReader
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitialize_T<MediaType>
{
 public:
  Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T (HRESULT*,    // return value: result
                                                          FilterType*, // filter handle
                                                          LPCWSTR);    // name
  virtual ~Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T ();

  // ------------------------------------

  // implement/overload IUnknown
  DECLARE_IUNKNOWN
  STDMETHODIMP NonDelegatingQueryInterface (REFIID, void**);

  // ------------------------------------

  HRESULT InitAllocator (IMemAllocator**); // return value: allocator handle

  // ------------------------------------

  // override / implement (part of) CBasePin
  //STDMETHODIMP Connect (IPin*,                       // receive (input) pin
  //                      const struct _AMMediaType*); // (optional) media type handle
  virtual HRESULT CheckMediaType (const CMediaType*);
  virtual HRESULT SetMediaType (const CMediaType*);
  virtual HRESULT CheckConnect (IPin*);
  virtual HRESULT BreakConnect ();
  virtual HRESULT CompleteConnect (IPin*);
  virtual HRESULT GetMediaType (int, CMediaType*);

  // ------------------------------------

  // implement/overload IPin

  // ------------------------------------
  // implement IKsPropertySet
  inline STDMETHODIMP Set (REFGUID, // guidPropSet
                           DWORD,   // dwPropID
                           LPVOID,  // pInstanceData
                           DWORD,   // cbInstanceData
                           LPVOID,  // pPropData
                           DWORD) { return E_NOTIMPL; }; // cbPropData
  STDMETHODIMP Get (REFGUID, // guidPropSet
                    DWORD,   // dwPropID
                    LPVOID,  // pInstanceData
                    DWORD,   // cbInstanceData
                    LPVOID,  // pPropData
                    DWORD,   // cbPropData
                    DWORD*); // pcbReturned
  STDMETHODIMP QuerySupported (REFGUID, // guidPropSet
                               DWORD,   // dwPropID
                               DWORD*); // pTypeSupport

  // ------------------------------------

  // implement IAMBufferNegotiation
  // *NOTE*: call before (!) the pin connects
  STDMETHODIMP SuggestAllocatorProperties (const struct _AllocatorProperties*); // pprop
  // *NOTE*: call after the pin connects to verify whether the suggested
  //         properties were honored
  STDMETHODIMP GetAllocatorProperties (struct _AllocatorProperties*); // pprop

  // ------------------------------------
  // implement IAMStreamConfig
  STDMETHODIMP SetFormat (struct _AMMediaType*); // pmt
  STDMETHODIMP GetFormat (struct _AMMediaType**); // ppmt
  STDMETHODIMP GetNumberOfCapabilities (int*,  // piCount
                                        int*); // piSize
  STDMETHODIMP GetStreamCaps (int,                   // iIndex
                              struct _AMMediaType**, // ppmt
                              BYTE*);                // pSCC

  // ------------------------------------

  // implement IAsyncReader
  // pass in your preferred allocator and your preferred properties.
  // method returns the actual allocator to be used. Call GetProperties
  // on returned allocator to learn alignment and prefix etc chosen.
  // this allocator will be not be committed and decommitted by
  // the async reader, only by the consumer.
  virtual STDMETHODIMP RequestAllocator (IMemAllocator*,               // preferrred allocator
                                         struct _AllocatorProperties*, // preferred properties
                                         IMemAllocator**);             // return value: allocator handle

  // queue a request for data.
  // media sample start and stop times contain the requested absolute
  // byte position (start inclusive, stop exclusive).
  // may fail if sample not obtained from agreed allocator.
  // may fail if start/stop position does not match agreed alignment.
  // samples allocated from source pin's allocator may fail
  // GetPointer until after returning from WaitForNext.
  virtual STDMETHODIMP Request (IMediaSample*, // media sample handle
                                DWORD_PTR);    // user context

  // block until the next sample is completed or the timeout occurs.
  // timeout (millisecs) may be 0 or INFINITE. Samples may not
  // be delivered in order. If there is a read error of any sort, a
  // notification will already have been sent by the source filter,
  // and STDMETHODIMP will be an error.
  virtual STDMETHODIMP WaitForNext (DWORD,          // timeout (ms)
                                    IMediaSample**, // return value: completed sample
                                    DWORD_PTR*);    // user context

  // sync read of data. Sample passed in must have been acquired from
  // the agreed allocator. Start and stop position must be aligned.
  // equivalent to a Request/WaitForNext pair, but may avoid the
  // need for a thread on the source filter.
  virtual STDMETHODIMP SyncReadAligned (IMediaSample*); // media sample handle

  // sync read. works in stopped state as well as run state.
  // need not be aligned. Will fail if read is beyond actual total
  // length.
  virtual STDMETHODIMP SyncRead (LONGLONG, // absolute file position
                                 LONG,     // nr bytes required
                                 BYTE*);   // write data here

  // return total length of stream, and currently available length.
  // reads for beyond the available length but within the total length will
  // normally succeed but may block for a long period.
  virtual STDMETHODIMP Length (LONGLONG*,  // total nr bytes
                               LONGLONG*); // available nr bytes

  // cause all outstanding reads to return, possibly with a failure code
  // (VFW_E_TIMEOUT) indicating they were cancelled.
  // these are defined on IAsyncReader and IPin
  virtual STDMETHODIMP BeginFlush (void);
  virtual STDMETHODIMP EndFlush (void);

  // ------------------------------------

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);
  // *NOTE*: sets the preferred (i.e. default) media type
  virtual bool initialize (const MediaType&);

 protected:
  struct _AllocatorProperties allocatorProperties_;
  ConfigurationType*          configuration_;
  bool                        isInitialized_;        // initialized
  MediaType*                  mediaType_;            // (current) media type
  //FilterType*                 parentFilter_;         // same as inherited::m_pFilter
  ACE_Message_Queue_Base*     queue_;                // inbound queue (active object)

 private:
  typedef CBasePin inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T (const Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T& operator= (const Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T&))

  struct Stream_DirectShow_AsyncReadRequest
  {
    IMediaSample* mediaSample;
    DWORD_PTR     userContext;
  };
  typedef std::queue<struct Stream_DirectShow_AsyncReadRequest> REQUEST_QUEUE_T;

  const int                   defaultFrameInterval_; // initial frame interval (ms)

  int                         frameInterval_;        // (ms)
  // *TODO*: support multiple media types
  unsigned int                numberOfMediaTypes_;

  bool                        flushing_;
  bool                        queriedForIAsyncReader_;

  REQUEST_QUEUE_T             requestQueue_;
  CCritSec                    lock_;                 // lock on sampleTime_
  CRefTime                    sampleTime_;
}; // Stream_Misc_DirectShow_Source_Filter_OutputPin_T

// include template definition
#include "stream_misc_directshow_asynch_source_filter.inl"

#endif
