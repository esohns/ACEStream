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

#include "ace/Global_Macros.h"
#include "ace/Message_Queue.h"

#include "dshow.h"
#include "streams.h"

#include "common_iinitialize.h"

#include "stream_task_base_synch.h"

// forward declarations
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T;

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
class Stream_Misc_DirectShow_Asynch_Source_Filter_T
 : public CBaseFilter
 , public Common_IInitialize_T<ConfigurationType>
{
  typedef Stream_Misc_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        ProtocolMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType> OWN_TYPE_T;
  typedef Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T<PinConfigurationType,
                                                                 OWN_TYPE_T,
                                                                 MediaType> OUTPUT_PIN_T;
  friend OUTPUT_PIN_T;

 public:
  // convenience typedefs
  typedef ConfigurationType CONFIG_T;

  //// *NOTE*: the non-COM (!) ctor
  //Stream_Misc_DirectShow_Asynch_Source_Filter_T ();
  virtual ~Stream_Misc_DirectShow_Asynch_Source_Filter_T ();

  static CUnknown* WINAPI CreateInstance (LPUNKNOWN, // aggregating IUnknown interface handle ('owner')
                                          HRESULT*); // return value: result
  static void WINAPI DeleteInstance (void*); // instance handle

  // ------------------------------------

  // implement/overload CBaseFilter
  int GetPinCount ();
  CBasePin* GetPin (int);
  //const CMediaType* LoadType () const;
  virtual STDMETHODIMP Connect (IPin*,                 // receive (input) pin
                                const AM_MEDIA_TYPE*); // (optional) media type handle

  // ------------------------------------

  // implement/overload IUnknown
  DECLARE_IUNKNOWN
  virtual STDMETHODIMP NonDelegatingQueryInterface (REFIID, void**);

  // ------------------------------------

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

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
  ConfigurationType* configuration_;

 private:
  typedef CBaseFilter inherited;

  // ctor used by the COM class factory
  Stream_Misc_DirectShow_Asynch_Source_Filter_T (LPTSTR,              // name
                                                 LPUNKNOWN,           // aggregating IUnknown interface handle ('owner')
                                                 const struct _GUID&, // CLSID
                                                 HRESULT*);           // return value: result

  //virtual ULONG Release ();

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Asynch_Source_Filter_T (const Stream_Misc_DirectShow_Asynch_Source_Filter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Asynch_Source_Filter_T& operator= (const Stream_Misc_DirectShow_Asynch_Source_Filter_T&))

  //bool         hasCOMReference_;
  CCritSec     lock_;
  //CMediaType   mediaType_;
  OUTPUT_PIN_T outputPin_;
}; // Stream_Misc_DirectShow_Asynch_Source_Filter_T

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T
 : public CBasePin
 , public IAsyncReader
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T (HRESULT*,    // return value: result
                                                          FilterType*, // filter handle
                                                          LPCWSTR);    // name
  virtual ~Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T ();

  // ------------------------------------

  // implement/overload IUnknown
  DECLARE_IUNKNOWN
  STDMETHODIMP NonDelegatingQueryInterface (REFIID, __deref_out void**);

  // ------------------------------------

  HRESULT InitAllocator (IMemAllocator**); // return value: allocator handle

  // implement/overload IPin
  virtual STDMETHODIMP Connect (IPin*,                 // receive (input) pin
                                const AM_MEDIA_TYPE*); // (optional) media type handle

  // ------------------------------------

  // override / implement (part of) CBasePin
  virtual HRESULT CheckMediaType (const CMediaType*);
  virtual HRESULT GetMediaType (int, __inout CMediaType*);
  virtual HRESULT SetMediaType (const CMediaType*);

  virtual HRESULT CheckConnect (IPin*);
  virtual HRESULT BreakConnect ();
  virtual HRESULT CompleteConnect (IPin*);

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

 protected:
  ConfigurationType*      configuration_;
  bool                    isInitialized_;        // initialized
  //MediaType*              mediaType_;            // (preferred) media type
  FilterType*             parentFilter_;         // same as inherited::m_pFilter
  ACE_Message_Queue_Base* queue_;                // inbound queue (active object)

 private:
  typedef CBasePin inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T (const Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T& operator= (const Stream_Misc_DirectShow_Source_Filter_AsynchOutputPin_T&))

  const int               defaultFrameInterval_; // initial frame interval (ms)

  int                     frameInterval_;        // (ms)
  // *TODO*: support multiple media types
  unsigned int            numberOfMediaTypes_;

  bool                    flushing_;
  bool                    queriedForIAsyncReader_;
  DWORD_PTR               userContext_;

  CCritSec                lock_;                 // lock on sampleTime_
  CRefTime                sampleTime_;
}; // Stream_Misc_DirectShow_Source_Filter_OutputPin_T

// include template definition
#include "stream_misc_directshow_asynch_source_filter.inl"

#endif
