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

#ifndef STREAM_LIB_DIRECTSHOW_ASYNCH_SOURCE_FILTER_H
#define STREAM_LIB_DIRECTSHOW_ASYNCH_SOURCE_FILTER_H

#include <queue>

#include "mmsystem.h"
#include "Unknwn.h"

#include "strmif.h"
#if _MSC_VER >= 1100
#define AM_NOVTABLE __declspec (novtable)
#else
#define AM_NOVTABLE
#endif
#include "wxdebug.h"
#include "combase.h"
#undef NANOSECONDS
#include "reftime.h"
#include "wxlist.h"
#include "wxutil.h"
#include "mtype.h"
#include "amfilter.h"
#include "source.h"

//#undef NANOSECONDS
//#include "streams.h"

#include "ace/Global_Macros.h"
#include "ace/Message_Queue.h"

#include "common_iinitialize.h"

#include "stream_task_base_synch.h"

// forward declarations
template <typename ConfigurationType>
class Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T;

template <typename MessageType,
          ///////////////////////////////
          typename ConfigurationType,
          typename PinConfigurationType>
class Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T
 : public CSource
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitialize_T<struct _AMMediaType>
{
  // friends
  friend class Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<PinConfigurationType>;

  typedef CSource inherited;

 public:
  // convenient types
  typedef Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<PinConfigurationType> OUTPUT_PIN_T;

  // non-COM (!) ctor
  Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T ();
  virtual ~Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T ();

  static CUnknown* WINAPI CreateInstance (LPUNKNOWN, // aggregating IUnknown interface handle ('owner')
                                          HRESULT*); // return value: result
  static void WINAPI DeleteInstance (void*); // instance handle

  // ------------------------------------

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);
  // *NOTE*: sets the preferred (i.e. default) media type
  virtual bool initialize (const struct _AMMediaType&);

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
  ConfigurationType* filterConfiguration_;

 private:
  // convenient types
  typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<MessageType,
                                                                  ConfigurationType,
                                                                  PinConfigurationType> OWN_TYPE_T;
  typedef Common_IInitialize_T<PinConfigurationType> IPIN_INITIALIZE_T;
  typedef Common_IInitialize_T<struct _AMMediaType> IPIN_MEDIA_INITIALIZE_T;

  // ctor used by the COM class factory
  Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T (LPTSTR,    // name
                                                           LPUNKNOWN, // aggregating IUnknown interface handle ('owner')
                                                           REFGUID,   // CLSID
                                                           HRESULT*); // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T (const Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T& operator= (const Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T&))
}; // Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType> // implements Stream_MediaFramework_DirectShow_FilterPinConfiguration
class Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T
 : public CBasePin
 , public IKsPropertySet
 , public IAMBufferNegotiation
 , public IAMStreamConfig
 , public IAsyncReader
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitialize_T<struct _AMMediaType>
{
  typedef CBasePin inherited;

 public:
  Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T (HRESULT*, // return value: result
                                                                    CSource*, // (parent) filter
                                                                    LPCWSTR); // name
  virtual ~Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T ();

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
  virtual HRESULT GetMediaType (int, CMediaType*);
  virtual HRESULT SetMediaType (const CMediaType*);
  virtual HRESULT CheckConnect (IPin*);
  virtual HRESULT BreakConnect ();
  virtual HRESULT CompleteConnect (IPin*);

  // ------------------------------------

  // implement/overload IPin

  // ------------------------------------
  // implement IKsPropertySet
  inline STDMETHODIMP Set (REFGUID, // guidPropSet
                           DWORD,   // dwPropID
                           LPVOID,  // pInstanceData
                           DWORD,   // cbInstanceData
                           LPVOID,  // pPropData
                           DWORD) { return E_NOTIMPL; } // cbPropData
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
  virtual bool initialize (const struct _AMMediaType&);

 protected:
  ConfigurationType*   configuration_;
  bool                 isInitialized_; // initialized
  struct _AMMediaType* mediaType_;     // (preferred) media type

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T (const Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T& operator= (const Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T&))

  struct Stream_DirectShow_AsyncReadRequest
  {
    IMediaSample* mediaSample;
    DWORD_PTR     userContext;
  };
  typedef std::queue<struct Stream_DirectShow_AsyncReadRequest> REQUEST_QUEUE_T;

  REFERENCE_TIME       frameInterval_;        // (*100ns)
  // *TODO*: support multiple media types
  unsigned int         numberOfMediaTypes_;

  bool                 flushing_;
  bool                 queriedForIAsyncReader_;

  REQUEST_QUEUE_T      requestQueue_;
  REFERENCE_TIME       sampleNumber_;
  unsigned int         sampleSize_;            // bytes (i.e. sizeof(video_frame)/sizeof(audio_frame))
  REFERENCE_TIME       sampleTime_;            // (*100ns)
}; // Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T

// include template definition
#include "stream_lib_directshow_asynch_source_filter.inl"

#endif
