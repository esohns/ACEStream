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

#ifndef STREAM_MISC_MEDIAFOUNDATION_MEDIASOURCE_H
#define STREAM_MISC_MEDIAFOUNDATION_MEDIASOURCE_H

#include <list>
#include <map>

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include <mfidl.h>

#include "common_iinitialize.h"

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename MediaType>
class Stream_Misc_MediaFoundation_MediaSource_T
 : public IMFMediaSourceEx
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  typedef Stream_Misc_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    SessionMessageType,
                                                    ProtocolMessageType,
                                                    ConfigurationType,
                                                    MediaType> OWN_TYPE_T;

  //// *NOTE*: the non-COM (!) ctor
  //Stream_Misc_MediaFoundation_MediaSource_T ();
  virtual ~Stream_Misc_MediaFoundation_MediaSource_T ();

  static HRESULT CreateInstance (IUnknown*, // parent
                                 REFIID,    // interface id
                                 void**);   // return value: instance handle

  // implement IMFMediaSourceEx
  // IUnknown
  STDMETHODIMP QueryInterface (REFIID,
                               void**);
  STDMETHODIMP_ (ULONG) AddRef ();
  STDMETHODIMP_ (ULONG) Release ();
  // IMFMediaEventGenerator
  STDMETHODIMP BeginGetEvent (IMFAsyncCallback*, // asynchronous callback handle
                              IUnknown*);        // 
  STDMETHODIMP EndGetEvent (IMFAsyncResult*,  // asynchronous result
                            IMFMediaEvent**); // return value: event handle
  STDMETHODIMP GetEvent (DWORD,            // flags
                         IMFMediaEvent**); // return value: event handle
  STDMETHODIMP QueueEvent (MediaEventType,      // event type
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
  ConfigurationType*  configuration_;

 private:
  // ctor used by the COM class factory
  Stream_Misc_MediaFoundation_MediaSource_T (HRESULT*); // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_MediaFoundation_MediaSource_T (const Stream_Misc_MediaFoundation_MediaSource_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_MediaFoundation_MediaSource_T& operator= (const Stream_Misc_MediaFoundation_MediaSource_T&))

  //typedef GrowableArray<MPEG1Stream*> StreamList;
  typedef std::map<BYTE, DWORD> STREAM_MAP_T;    // Maps stream ID to index
  typedef STREAM_MAP_T::iterator STREAM_MAP_ITERATOR_T;
  typedef std::list<IMFSample*> SAMPLE_LIST_T;
  typedef SAMPLE_LIST_T::iterator SAMPLE_LIST_ITERATOR_T;
  typedef std::list<IUnknown*>  TOKEN_LIST_T;    // List of tokens for IMFMediaStream::RequestSample
  typedef TOKEN_LIST_T::iterator TOKEN_LIST_ITERATOR_T;

  enum STATE_T
  {
    STATE_INVALID = -1, // Initial state. Have not started opening the stream.
    STATE_OPENING = 0,  // BeginOpen is in progress.
    STATE_STOPPED,
    STATE_PAUSED,
    STATE_STARTED,
    STATE_SHUTDOWN
  };

  IMFMediaEventQueue* eventQueue_;
  //bool                hasCOMReference_;
  ACE_SYNCH_MUTEX     lock_;
  volatile long       referenceCount_;
  STATE_T             state_; // Current state (running, stopped, paused)
}; // Stream_Misc_MediaFoundation_MediaSource_T

// include template definition
#include "stream_misc_mediafoundation_mediasource.inl"

#endif
