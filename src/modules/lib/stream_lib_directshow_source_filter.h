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

#ifndef STREAM_LIB_DIRECTSHOW_SOURCE_FILTER_H
#define STREAM_LIB_DIRECTSHOW_SOURCE_FILTER_H

// *IMPORTANT NOTE*: the MSVC compiler does not like streams.h to be included
//                   several times (complains about media GUIDs being defined
//                   multiple times)
//                   --> to work around this, the following are included instead
//                   DO NOT TOUCH (unless you know what you are doing)
//#include <streams.h>
#include <Dshow.h>
#include <wxdebug.h>
#include <combase.h>
#include <strmif.h>
#include <sdkddkver.h>
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
#include <minwindef.h>
#else
#include <windef.h>
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
//#include <wtypes.h>
//#include <mmiscapi2.h>
#include <mmreg.h>
#include <mtype.h>
#include <reftime.h>
#include <wxlist.h>
#include <wxutil.h>
#include <amfilter.h>
#include <source.h>

#include "ace/Global_Macros.h"
#include "ace/Message_Queue.h"

#include "common_iinitialize.h"

// forward declarations
class Stream_IAllocator;
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T;

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
class Stream_MediaFramework_DirectShow_Source_Filter_T
 : public CSource
 , public IMemAllocator
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitializeP_T<MediaType>
{
  // convenient types
  typedef Stream_MediaFramework_DirectShow_Source_Filter_T<TimePolicyType,
                                                           SessionMessageType,
                                                           ProtocolMessageType,
                                                           ConfigurationType,
                                                           PinConfigurationType,
                                                           MediaType> OWN_TYPE_T;
  typedef Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<PinConfigurationType,
                                                                     OWN_TYPE_T,
                                                                     MediaType> OUTPUT_PIN_T;
  // friends
  friend class OUTPUT_PIN_T;

  typedef CSource inherited;

 public:
  virtual ~Stream_MediaFramework_DirectShow_Source_Filter_T ();

  // ------------------------------------
  static CUnknown* WINAPI CreateInstance (LPUNKNOWN, // aggregating IUnknown interface handle ('owner')
                                          HRESULT*); // return value: result
  static void WINAPI DeleteInstance (void*); // instance handle

  // *NOTE*: - allocation functions are always 'static'
  //         - "The call to the class - specific T::operator delete on a
  //           polymorphic class is the only case where a static member function
  //           is called through dynamic dispatch."

  // *NOTE*: these ensure that all instances are (de)allocated off their
  //         originating (DLL) heap
  static void operator delete (void*); // instance handle
  //static void operator delete (void*,   // instance handle
  //                             size_t); // number of bytes

  // ------------------------------------
  // implement/overload IUnknown
  DECLARE_IUNKNOWN
  STDMETHODIMP NonDelegatingQueryInterface (REFIID, void**);

  //inline virtual int GetPinCount () { return 1; }
  //virtual CBasePin* GetPin (int);
  //virtual HRESULT AddPin (CBasePin*);
  //virtual HRESULT RemovePin (CBasePin*);

  STDMETHODIMP SetProperties (struct _AllocatorProperties*,  // requested
                              struct _AllocatorProperties*); // return value: actual
  STDMETHODIMP GetProperties (struct _AllocatorProperties* properties_out) { CheckPointer (properties_out, E_POINTER); *properties_out = allocatorProperties_; return NOERROR; };
  STDMETHODIMP GetBuffer (IMediaSample**,  // return value: media sample handle
                          REFERENCE_TIME*, // return value: start time
                          REFERENCE_TIME*, // return value: end time
                          DWORD);          // flags
  STDMETHODIMP ReleaseBuffer (IMediaSample*); // media sample handle

  // implement IMemAllocator
  inline STDMETHODIMP Commit (void) { return NOERROR; }
  inline STDMETHODIMP Decommit (void) { return NOERROR; }

  // ------------------------------------

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);
  // *NOTE*: sets the preferred (i.e. default) media type
  virtual bool initialize (const MediaType*);

 protected:
  // non-COM (!) ctor
  Stream_MediaFramework_DirectShow_Source_Filter_T ();

  ConfigurationType*          filterConfiguration_;

  CCritSec                    lock_;
  int                         numberOfPins_;
  CBasePin**                  pins_;

 private:
  // convenient types
  typedef Common_IInitialize_T<PinConfigurationType> IPIN_INITIALIZE_T;
  typedef Common_IInitialize_T<MediaType> IPIN_MEDIA_INITIALIZE_T;

  // ctor used by the COM class factory
  Stream_MediaFramework_DirectShow_Source_Filter_T (LPTSTR,    // name
                                                    LPUNKNOWN, // owner
                                                    REFGUID,   // CLSID
                                                    HRESULT*); // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_T (const Stream_MediaFramework_DirectShow_Source_Filter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_T& operator= (const Stream_MediaFramework_DirectShow_Source_Filter_T&))

  //bool               hasCOMReference_;
  Stream_IAllocator*          allocator_;
  struct _AllocatorProperties allocatorProperties_;
}; // Stream_MediaFramework_DirectShow_Source_Filter_T

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T
 : public CSourceStream
 , public IKsPropertySet
 , public IAMBufferNegotiation
 , public IAMStreamConfig
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitialize_T<MediaType>
{
  typedef CSourceStream inherited;

 public:
  Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T (HRESULT*, // return value: result
                                                              CSource*, // (parent) filter
                                                              LPCWSTR); // name
  virtual ~Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);
  // *NOTE*: sets the preferred (i.e. default) media type
  virtual bool initialize (const MediaType&);

  // -------------------------------------

  // implement/overload IUnknown
  DECLARE_IUNKNOWN
  STDMETHODIMP NonDelegatingQueryInterface (REFIID, void**);

  // -------------------------------------

  // override / implement (part of) CBasePin
  // *NOTE*: called during connection negotiation, before SetMediaType()
  virtual HRESULT CheckMediaType (const CMediaType*);
  virtual HRESULT GetMediaType (int, CMediaType*);
  // *NOTE*: "...Before calling this method, the pin calls the
  //         CBasePin::CheckMediaType method to determine whether the media type
  //         is acceptable. Therefore, the pmt parameter is assumed to be an
  //         acceptable media type. ..."
  virtual HRESULT SetMediaType (const CMediaType*);
  // *NOTE*: support "Handling Format Changes from the Video Renderer"
  STDMETHODIMP QueryAccept (const struct _AMMediaType*);

  // implement/overload (part of) CBaseOutputPin
  virtual HRESULT DecideAllocator (IMemInputPin*,
                                   IMemAllocator**);
  virtual HRESULT DecideBufferSize (IMemAllocator*,
                                    struct _AllocatorProperties*);

  //// implement/overload (part of) CSourceStream
  //virtual DWORD ThreadProc ();
  //virtual HRESULT DoBufferProcessingLoop ();
  // Called by the thread (see: DoBufferProcessingLoop()); blocks until data is
  // available
  virtual HRESULT FillBuffer (IMediaSample*);

  // Called as the thread is created/destroyed - use to perform
  // jobs such as start/stop streaming mode
  // If OnThreadCreate returns an error the thread will exit.
  virtual HRESULT OnThreadCreate ();
  virtual HRESULT OnThreadDestroy ();
  virtual HRESULT OnThreadStartPlay ();

  //virtual HRESULT Active (void);
  //virtual HRESULT Inactive (void);

  // implement (part of) IQualityControl
  STDMETHODIMP Notify (IBaseFilter*,
                       Quality);

  // -------------------------------------

  // implement IKsPropertySet
  // *NOTE*: IKsPropertySet is defined twice: in strmif.h and in dsound.h, with
  //         slightly different signatures
  //         --> implement both versions
  // *TODO*: find out how to handle this mess gracefully
  // this is the version from <strmif.h>
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
  // this is the version from <dsound.h>
  //inline STDMETHODIMP Set (REFGUID, // rguidPropSet
  //                         ULONG,   // ulId
  //                         LPVOID,  // pInstanceData
  //                         ULONG,   // ulInstanceLength
  //                         LPVOID,  // pPropertyData
  //                         ULONG) { return E_NOTIMPL; }; // ulDataLength
  //STDMETHODIMP Get (REFGUID, // rguidPropSet
  //                  ULONG,   // ulId
  //                  LPVOID,  // pInstanceData
  //                  ULONG,   // ulInstanceLength
  //                  LPVOID,  // pPropertyData
  //                  ULONG,   // ulDataLength
  //                  PULONG); // pulBytesReturned
  STDMETHODIMP QuerySupport (REFGUID, // rguidPropSet
                             ULONG,   // ulId
                             PULONG); // pulTypeSupport

  // -------------------------------------

  // implement IAMBufferNegotiation
  // *NOTE*: call before (!) the pin connects
  STDMETHODIMP SuggestAllocatorProperties (const struct _AllocatorProperties*);
  // *NOTE*: call after the pin connects to verify whether the suggested
  //         properties were honored
  STDMETHODIMP GetAllocatorProperties (struct _AllocatorProperties*);

  // -------------------------------------

  // implement IAMStreamConfig
  STDMETHODIMP SetFormat (struct _AMMediaType*); // pmt
  STDMETHODIMP GetFormat (struct _AMMediaType**); // ppmt
  STDMETHODIMP GetNumberOfCapabilities (int*,  // piCount
                                        int*); // piSize
  STDMETHODIMP GetStreamCaps (int,                   // iIndex
                              struct _AMMediaType**, // ppmt
                              BYTE*);                // pSCC

  // -------------------------------------

 protected:
  ConfigurationType*      configuration_;
  bool                    isInitialized_; // initialized
  MediaType*              mediaType_;     // (preferred) media type
  //CSource*                parentFilter_;  // parent filter
  ACE_Message_Queue_Base* queue_;         // inbound queue (active object)

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T (const Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T& operator= (const Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T&))

  const REFERENCE_TIME    defaultFrameInterval_;  // initial frame interval (ms)

  REFERENCE_TIME          frameInterval_;         // (ms)
  bool                    hasMediaSampleBuffers_;
  // *NOTE*: some image formats have a bottom-to-top memory layout; in
  //         DirectShow, this is reflected by a positive biHeight; see also:
  //         https://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx
  //         --> set this if the sample data is top-to-bottom
  bool                    isTopToBottom_;
  // *TODO*: support multiple media types
  unsigned int            numberOfMediaTypes_;

  bool                    directShowHasEnded_;

  CCritSec                lock_;                  // lock on sampleTime_
  CRefTime                sampleTime_;
}; // Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T

// include template definition
#include "stream_lib_directshow_source_filter.inl"

#endif
