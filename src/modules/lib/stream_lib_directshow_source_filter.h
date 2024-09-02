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

// #undef NANOSECONDS
// #include "streams.h"

#include "ace/Global_Macros.h"
#include "ace/Message_Queue.h"

#include "common_iinitialize.h"

// forward declarations
template <typename ConfigurationType>
class Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T;

template <typename MessageType,
          ////////////////////////////////
          typename ConfigurationType,
          typename PinConfigurationType>
class Stream_MediaFramework_DirectShow_Source_Filter_T
 : public CSource
 , public IAMFilterMiscFlags
 , public IMemAllocator
 //, virtual public IUnknown
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitializeP_T<struct _AMMediaType>
{
  // friends
  friend class Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<PinConfigurationType>;

  typedef CSource inherited;

 public:
  // convenient types
  typedef Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<PinConfigurationType> OUTPUT_PIN_T;

  // non-COM (!) ctor
  Stream_MediaFramework_DirectShow_Source_Filter_T ();
  virtual ~Stream_MediaFramework_DirectShow_Source_Filter_T ();

  // --------------------------------------
  static CUnknown* WINAPI CreateInstance (LPUNKNOWN, // aggregating IUnknown interface handle ('owner')
                                          HRESULT*); // return value: result
  static void WINAPI DeleteInstance (void*); // instance handle

  // *NOTE*: - allocation functions are always 'static'
  //         - "The call to the class - specific T::operator delete on a
  //           polymorphic class is the only case where a static member function
  //           is called through dynamic dispatch."

  // *NOTE*: these ensure that all instances are (de)allocated off their
  //         originating (DLL) heap
  //static void operator delete (void*); // instance handle
  //static void operator delete (void*,   // instance handle
  //                             size_t); // number of bytes

  // --------------------------------------
  // implement/overload IAMFilterMiscFlags
  inline virtual STDMETHODIMP_(ULONG) GetMiscFlags (void) { return AM_FILTER_MISC_FLAGS_IS_SOURCE; }

  // implement/overload IMemAllocator
  inline virtual STDMETHODIMP QueryInterface (REFIID riid, __deref_out void** ppv) { return NonDelegatingQueryInterface (riid, ppv); }
  inline virtual STDMETHODIMP_(ULONG) AddRef () { return inherited::NonDelegatingAddRef (); }
  inline virtual STDMETHODIMP_(ULONG) Release () { return inherited::NonDelegatingRelease (); }
  virtual STDMETHODIMP NonDelegatingQueryInterface (REFIID, void**);

  //inline virtual STDMETHODIMP_(LONG) GetPinVersion () { return 1; }
  //inline virtual STDMETHODIMP_(int) GetPinCount () { return 1; }
  //inline virtual STDMETHODIMP_(CBasePin*) GetPin (int) { return outputPin_; }
  //virtual STDMETHODIMP AddPin (CBasePin*);
  //virtual STDMETHODIMP RemovePin (CBasePin*);

  virtual STDMETHODIMP SetProperties (struct _AllocatorProperties*,  // requested
                                      struct _AllocatorProperties*); // return value: actual
  inline virtual STDMETHODIMP GetProperties (struct _AllocatorProperties* properties_out) { CheckPointer (properties_out, E_POINTER); ACE_ASSERT (configuration_); ACE_ASSERT (configuration_->allocatorProperties); *properties_out = *configuration_->allocatorProperties; return NOERROR; }
  virtual STDMETHODIMP GetBuffer (IMediaSample**,  // return value: media sample handle
                                  REFERENCE_TIME*, // return value: start time
                                  REFERENCE_TIME*, // return value: end time
                                  DWORD);          // flags
  virtual STDMETHODIMP ReleaseBuffer (IMediaSample*); // media sample handle

  inline virtual STDMETHODIMP Commit (void) { return NOERROR; }
  inline virtual STDMETHODIMP Decommit (void) { return NOERROR; }

  // --------------------------------------

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);
  // *NOTE*: sets the preferred (i.e. default) media type
  virtual bool initialize (const struct _AMMediaType*);

 protected:
  ConfigurationType* configuration_;
  OUTPUT_PIN_T*      outputPin_;

 private:
  // convenient types
  typedef Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                           ConfigurationType,
                                                           PinConfigurationType> OWN_TYPE_T;
  typedef Common_IInitialize_T<PinConfigurationType> IPIN_INITIALIZE_T;
  typedef Common_IInitialize_T<struct _AMMediaType> IPIN_MEDIA_INITIALIZE_T;

  // ctor used by the COM class factory
  Stream_MediaFramework_DirectShow_Source_Filter_T (LPTSTR,    // name
                                                    LPUNKNOWN, // aggregating IUnknown interface handle ('owner')
                                                    REFGUID,   // CLSID
                                                    HRESULT*); // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_T (const Stream_MediaFramework_DirectShow_Source_Filter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_T& operator= (const Stream_MediaFramework_DirectShow_Source_Filter_T&))
}; // Stream_MediaFramework_DirectShow_Source_Filter_T

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType> // implements Stream_MediaFramework_DirectShow_FilterPinConfiguration
class Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T
 : public CSourceStream
 , public IKsPropertySet
 , public IAMBufferNegotiation
 , public IAMStreamConfig
 , public IAMPushSource
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitialize_T<struct _AMMediaType>
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
  virtual bool initialize (const struct _AMMediaType&);

  // -------------------------------------

  // implement/overload IUnknown
  //DECLARE_IUNKNOWN
  inline virtual STDMETHODIMP QueryInterface (REFIID riid, __deref_out void** ppv) { return NonDelegatingQueryInterface (riid, ppv); }
  inline virtual STDMETHODIMP_(ULONG) AddRef () { return inherited::NonDelegatingAddRef (); }
  inline virtual STDMETHODIMP_(ULONG) Release () { return inherited::NonDelegatingRelease (); }
  virtual STDMETHODIMP NonDelegatingQueryInterface (REFIID, void**);

  // -------------------------------------

  // override / implement (part of) CBasePin
  // *NOTE*: called during connection negotiation, before SetMediaType()
  virtual STDMETHODIMP CheckMediaType (const CMediaType*);
  virtual STDMETHODIMP GetMediaType (int, CMediaType*);
  // *NOTE*: "...Before calling this method, the pin calls the
  //         CBasePin::CheckMediaType method to determine whether the media type
  //         is acceptable. Therefore, the pmt parameter is assumed to be an
  //         acceptable media type. ..."
  virtual STDMETHODIMP SetMediaType (const CMediaType*);
  // *NOTE*: support "Handling Format Changes from the Video Renderer"
  virtual STDMETHODIMP QueryAccept (const struct _AMMediaType*);

  // implement/overload (part of) CBaseOutputPin
  virtual STDMETHODIMP DecideAllocator (IMemInputPin*,
                                        IMemAllocator**);
  virtual STDMETHODIMP DecideBufferSize (IMemAllocator*,
                                         struct _AllocatorProperties*);

  //// implement/overload (part of) CSourceStream
  //virtual DWORD ThreadProc ();
  //virtual HRESULT DoBufferProcessingLoop ();
  // Called by the thread (see: DoBufferProcessingLoop()); blocks until data is
  // available
  virtual STDMETHODIMP FillBuffer (IMediaSample*);

  // Called as the thread is created/destroyed - use to perform
  // jobs such as start/stop streaming mode
  // If OnThreadCreate returns an error the thread will exit.
  virtual STDMETHODIMP OnThreadCreate ();
  virtual STDMETHODIMP OnThreadDestroy ();
  virtual STDMETHODIMP OnThreadStartPlay ();

  //virtual HRESULT Active (void);
  //virtual HRESULT Inactive (void);

  // implement (part of) IQualityControl
  virtual STDMETHODIMP Notify (IBaseFilter*,
                               Quality);

  // -------------------------------------

  // implement IKsPropertySet
  // *NOTE*: IKsPropertySet is defined twice: in strmif.h and in dsound.h, with
  //         slightly different signatures
  //         --> implement both versions
  // *TODO*: find out how to handle this mess gracefully
  // this is the version from <strmif.h>
  inline virtual STDMETHODIMP Set (REFGUID, // guidPropSet
                                   DWORD,   // dwPropID
                                   LPVOID,  // pInstanceData
                                   DWORD,   // cbInstanceData
                                   LPVOID,  // pPropData
                                   DWORD) { return E_NOTIMPL; } // cbPropData
  virtual STDMETHODIMP Get (REFGUID, // guidPropSet
                            DWORD,   // dwPropID
                            LPVOID,  // pInstanceData
                            DWORD,   // cbInstanceData
                            LPVOID,  // pPropData
                            DWORD,   // cbPropData
                            DWORD*); // pcbReturned
  virtual STDMETHODIMP QuerySupported (REFGUID, // guidPropSet
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
  virtual STDMETHODIMP QuerySupport (REFGUID, // rguidPropSet
                                     ULONG,   // ulId
                                     PULONG); // pulTypeSupport

  // -------------------------------------

  // implement IAMBufferNegotiation
  // *NOTE*: call before (!) the pin connects
  virtual STDMETHODIMP SuggestAllocatorProperties (const struct _AllocatorProperties*);
  // *NOTE*: call after the pin connects to verify whether the suggested
  //         properties were honored
  virtual STDMETHODIMP GetAllocatorProperties (struct _AllocatorProperties*);

  // -------------------------------------

  // implement IAMStreamConfig
  virtual STDMETHODIMP SetFormat (struct _AMMediaType*); // pmt
  virtual STDMETHODIMP GetFormat (struct _AMMediaType**); // ppmt
  virtual STDMETHODIMP GetNumberOfCapabilities (int*,  // piCount
                                                int*); // piSize
  virtual STDMETHODIMP GetStreamCaps (int,                   // iIndex
                                      struct _AMMediaType**, // ppmt
                                      BYTE*);                // pSCC

  // implement IAMPushSource
  virtual STDMETHODIMP GetLatency (REFERENCE_TIME*); // prtLatency (100 - nanosecond units)
  virtual STDMETHODIMP GetPushSourceFlags (ULONG*); // pFlags
  virtual STDMETHODIMP SetPushSourceFlags (ULONG);  // Flags
  virtual STDMETHODIMP SetStreamOffset (REFERENCE_TIME); // rtOffset
  virtual STDMETHODIMP GetStreamOffset (REFERENCE_TIME*); // prtOffset
  virtual STDMETHODIMP GetMaxStreamOffset (REFERENCE_TIME*); // prtMaxOffset
  virtual STDMETHODIMP SetMaxStreamOffset (REFERENCE_TIME); // rtMaxOffset

  // -------------------------------------

 protected:
  ConfigurationType*   configuration_;
  bool                 isInitialized_; // initialized
  struct _AMMediaType* mediaType_;     // (preferred) media type

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T (const Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T& operator= (const Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T&))

  REFERENCE_TIME       frameInterval_;         // (*100ns)
  // *TODO*: support multiple media types
  unsigned int         numberOfMediaTypes_;

  bool                 directShowHasEnded_;

  REFERENCE_TIME       sampleNumber_;
  unsigned int         sampleSize_;            // bytes (i.e. sizeof(video_frame)/sizeof(audio_frame))
  REFERENCE_TIME       sampleTime_;            // (*100ns)
}; // Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T

// include template definition
#include "stream_lib_directshow_source_filter.inl"

#endif
