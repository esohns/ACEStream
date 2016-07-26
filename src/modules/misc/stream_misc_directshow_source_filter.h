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

#ifndef STREAM_MISC_DIRECTSHOW_SOURCE_FILTER_H
#define STREAM_MISC_DIRECTSHOW_SOURCE_FILTER_H

#include "ace/Global_Macros.h"
#include "ace/Message_Queue.h"

#include "dshow.h"
#include "streams.h"

#include "common_iinitialize.h"

// forward declarations
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_Filter_OutputPin_T;

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_Filter_T
 : public CSource
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  //// *NOTE*: the non-COM (!) ctor
  //Stream_Misc_DirectShow_Source_Filter_T ();
  virtual ~Stream_Misc_DirectShow_Source_Filter_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

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

 protected:
  ConfigurationType* configuration_;

 private:
  typedef CSource inherited;

  typedef Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                                 SessionMessageType,
                                                 ProtocolMessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType,
                                                 MediaType> OWN_TYPE_T;
  typedef Stream_Misc_DirectShow_Source_Filter_OutputPin_T<PinConfigurationType,
                                                           OWN_TYPE_T,
                                                           MediaType> OUTPUT_PIN_T;

  // ctor used by the COM class factory
  Stream_Misc_DirectShow_Source_Filter_T (LPTSTR,              // name
                                          LPUNKNOWN,           // owner
                                          const struct _GUID&, // CLSID
                                          HRESULT*);           // return value: result

  //virtual ULONG Release ();

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_T (const Stream_Misc_DirectShow_Source_Filter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_T& operator= (const Stream_Misc_DirectShow_Source_Filter_T&))

  //bool               hasCOMReference_;
}; // Stream_Misc_DirectShow_Source_Filter_T

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_Filter_OutputPin_T
 : public CSourceStream
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  Stream_Misc_DirectShow_Source_Filter_OutputPin_T (HRESULT*,    // result
                                                    FilterType*, // (parent) filter
                                                    LPCWSTR);    // name
  virtual ~Stream_Misc_DirectShow_Source_Filter_OutputPin_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

  // override / implement (part of) CBasePin
  virtual HRESULT CheckMediaType (const CMediaType*);
  virtual HRESULT GetMediaType (int, CMediaType*);
  virtual HRESULT SetMediaType (const CMediaType*);

  // implement/overload (part of) CBaseOutputPin
  virtual HRESULT DecideAllocator (IMemInputPin*,
                                        IMemAllocator**);
  virtual HRESULT DecideBufferSize (IMemAllocator*,
                                    struct _AllocatorProperties*);

  // implement/overload (part of) CSourceStream
  virtual HRESULT FillBuffer (IMediaSample*);
  virtual HRESULT OnThreadCreate ();
  virtual HRESULT OnThreadDestroy ();

  // implement (part of) IQualityControl
  virtual STDMETHODIMP Notify (IBaseFilter*,
                               Quality);

 protected:
  bool                    isInitialized_;        // initialized
  //MediaType*              mediaType_;            // (preferred) media type
  ACE_Message_Queue_Base* queue_;                // inbound queue (active object)

 private:
  typedef CSourceStream inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin_T (const Stream_Misc_DirectShow_Source_Filter_OutputPin_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin_T& operator= (const Stream_Misc_DirectShow_Source_Filter_OutputPin_T&))

  ConfigurationType*      configuration_;

  const int               defaultFrameInterval_; // initial frame interval (ms)

  int                     frameInterval_;        // (ms)
  // *TODO*: support multiple media types
  unsigned int            numberOfMediaTypes_;

  CCritSec                lock_;                 // lock on sampleTime_
  CRefTime                sampleTime_;
}; // Stream_Misc_DirectShow_Source_Filter_OutputPin_T

// include template definition
#include "stream_misc_directshow_source_filter.inl"

#endif
