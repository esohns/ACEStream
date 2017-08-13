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

#ifndef STREAM_LIB_MEDIAFOUNDATION_CALLBACK_H
#define STREAM_LIB_MEDIAFOUNDATION_CALLBACK_H

#include <mfobjects.h>

#include "ace/Global_Macros.h"

#include "common.h"
#include "common_iinitialize.h"

#include "stream_istreamcontrol.h"

// forward declarations
struct IMFMediaSession;
class Common_IControl;

template <typename ConfigurationType>
class Stream_MediaFramework_MediaFoundation_Callback_T
 : public IMFAsyncCallback
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  Stream_MediaFramework_MediaFoundation_Callback_T ();
  virtual ~Stream_MediaFramework_MediaFoundation_Callback_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

  // implement IMFAsyncCallback
  virtual STDMETHODIMP QueryInterface (const IID&,
                                       void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&referenceCount_); };
  virtual STDMETHODIMP_ (ULONG) Release ();
  virtual STDMETHODIMP GetParameters (DWORD*,  // return value: flags
                                      DWORD*); // return value: queue handle
  virtual STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

 protected:
  ConfigurationType* configuration_;

 private:
  typedef Stream_MediaFramework_MediaFoundation_Callback_T<ConfigurationType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Callback_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Callback_T (const Stream_MediaFramework_MediaFoundation_Callback_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Callback_T& operator= (const Stream_MediaFramework_MediaFoundation_Callback_T&))

  Stream_IStreamControlBase* controller_;
  IMFMediaSession*           mediaSession_;
  ULONG                      referenceCount_;
};

// include template definition
#include "stream_lib_mediafoundation_callback.inl"

#endif
