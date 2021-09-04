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

#ifndef CLASS_FACTORY_H
#define CLASS_FACTORY_H

#include <assert.h>
//// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
#include <wxdebug.h>
#include <combase.h>
#include <Guiddef.h>
#include <Unknwn.h>

class CClassFactory
 : public CBaseObject
 , public IClassFactory
{
 public:
  CClassFactory (const CFactoryTemplate*);

  // IUnknown
  STDMETHODIMP QueryInterface (REFIID, __deref_out void**);
  STDMETHODIMP_ (ULONG)AddRef ();
  STDMETHODIMP_ (ULONG)Release ();

  // IClassFactory
  STDMETHODIMP CreateInstance (LPUNKNOWN, REFIID, __deref_out void**);
  STDMETHODIMP LockServer (BOOL);

  static BOOL IsLocked ();

 private:
  ULONG                         m_cRef;
  const CFactoryTemplate* const m_pTemplate;

  static int                    m_cLocked;
};

//////////////////////////////////////////

// Function pointer for creating COM objects. (Used by the class factory.)
typedef HRESULT (*CreateInstanceFn)(IUnknown*, REFIID, void**);

// Structure to associate CLSID with object creation function.
struct ClassFactoryData
{
  const GUID*      pclsid;
  CreateInstanceFn pfnCreate;
};

// ClassFactory:
// Implements a class factory for COM objects.
class ClassFactory
 : public IClassFactory
{
 private:
  volatile long        m_refCount;     // Reference count.
  static volatile long m_serverLocks;  // Number of server locks

  CreateInstanceFn     m_pfnCreation;  // Function to create an instance of the object.

 public:
  ClassFactory (CreateInstanceFn pfnCreation)
   : m_refCount (1)
   , m_pfnCreation (pfnCreation)
  {}
  inline virtual ~ClassFactory () {}

  inline static bool IsLocked () { return (m_serverLocks != 0); }

  // IUnknown methods
  inline STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&m_refCount); }
  STDMETHODIMP_ (ULONG) Release ()
  {
    assert (m_refCount >= 0);
    ULONG uCount = InterlockedDecrement (&m_refCount);
    if (uCount == 0)
      delete this;

    // Return the temporary variable, not the member
    // variable, for thread safety.
    return uCount;
  }
  STDMETHODIMP QueryInterface (REFIID riid, void **ppv)
  {
    if (NULL == ppv)
      return E_POINTER;

    if (InlineIsEqualGUID (riid, __uuidof (IUnknown)))
      *ppv = static_cast<IUnknown*>(this);
    else if (InlineIsEqualGUID (riid, __uuidof (IClassFactory)))
      *ppv = static_cast<IClassFactory*>(this);
    else
    {
      *ppv = NULL;
      return E_NOINTERFACE;
    }
    AddRef ();
    return S_OK;
  }

  STDMETHODIMP CreateInstance (IUnknown *pUnkOuter, REFIID riid, void **ppv)
  {
    // If the caller is aggregating the object, the caller may only request
    // IUknown. (See MSDN documenation for IClassFactory::CreateInstance.)
    if (pUnkOuter)
      if (!InlineIsEqualGUID (riid, __uuidof (IUnknown)))
        return E_NOINTERFACE;

    return m_pfnCreation (pUnkOuter, riid, ppv);
  }

  inline STDMETHODIMP LockServer (BOOL lock) { if (lock) LockServer (); else UnlockServer (); return S_OK; }

  // Static methods to lock and unlock the the server.
  inline static void LockServer () { InterlockedIncrement (&m_serverLocks); }
  inline static void UnlockServer () { InterlockedDecrement (&m_serverLocks); }
};

// BaseObjects
// All COM objects that are implemented in the server (DLL) must derive from BaseObject
// so that the server is not unlocked while objects are still active.
class BaseObject
{
 public:
  BaseObject ()
  {
    ClassFactory::LockServer ();
  }
  virtual ~BaseObject ()
  {
    ClassFactory::UnlockServer ();
  }
};

// RefCountedObject
// You can use this when implementing IUnknown or any object that uses reference counting.
class RefCountedObject
{
 protected:
  volatile long   m_refCount;

 public:
  RefCountedObject ()
   : m_refCount (1) {}
  inline virtual ~RefCountedObject () { assert (m_refCount == 0); }

  inline ULONG AddRef () { return InterlockedIncrement (&m_refCount); }
  ULONG Release ()
  {
    assert (m_refCount > 0);
    ULONG uCount = InterlockedDecrement (&m_refCount);
    if (uCount == 0)
      delete this;
    return uCount;
  }
};

#define DEFINE_CLASSFACTORY_SERVER_LOCK volatile long ClassFactory::m_serverLocks = 0;

#endif
