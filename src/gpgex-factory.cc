/* gpgex-factory.c - gpgex factory implementation
   Copyright (C) 2007 g10 Code GmbH

   This file is part of GpgEX.
 
   GpgEX is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   GpgEX is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Lesser General Public License for more details.
 
   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <windows.h>

#include "main.h"
#include "gpgex.h"

#include "gpgex-factory.h"


/* IUnknown methods implementation.  */

STDMETHODIMP
gpgex_factory_t::QueryInterface (REFIID riid, void **ppv)
{
  if (ppv == NULL)
    return E_INVALIDARG;

  /* Be nice to broken software.  */
  *ppv = NULL;

  /* The static casts ensure that the virtual function table
     layout of the returned object is correct.  */
  if (riid == IID_IUnknown)
    *ppv = static_cast<IUnknown *> (this);
  else if (riid == IID_IClassFactory)
    *ppv = static_cast<IClassFactory *> (this);
  else
    return E_NOINTERFACE;

  /* We have to acquire a reference to the returned object.  We lost
     the type information, but we know that all object classes inherit
     from IUnknown, which is good enough.  */
  reinterpret_cast<IUnknown *>(*ppv)->AddRef ();

  return S_OK;
}


STDMETHODIMP_(ULONG)
gpgex_factory_t::AddRef (void)
{
  /* This factory is a singleton and therefore no reference counting
     is needed.  */
  return 1;
}


STDMETHODIMP_(ULONG)
gpgex_factory_t::Release (void)
{
  /* This factory is a singleton and therefore no reference counting
     is needed.  */
  return 1;
}


/* IClassFactory methods implementation.  */

STDMETHODIMP
gpgex_factory_t::CreateInstance (LPUNKNOWN punkOuter, REFIID riid,
				 void **ppv)
{
  HRESULT result;

  /* Be nice to broken software.  */
  *ppv = NULL;

  /* Aggregation is not supported.  */
  if (punkOuter)
    return CLASS_E_NOAGGREGATION;

  gpgex_t *gpgex = new gpgex_t;
  if (!gpgex)
    return E_OUTOFMEMORY;

  result = gpgex->QueryInterface (riid, ppv);
  if (FAILED (result))
    delete gpgex;

  return result;
}


STDMETHODIMP
gpgex_factory_t::LockServer (BOOL fLock)
{
  /* Locking the singleton gpgex factory object acquires a reference
     for the server component.  */
  if (fLock)
    gpgex_server::add_ref ();
  else
    gpgex_server::release ();

  return S_OK;
}



/* The global singleton instance of the GpgEX factory.  */
gpgex_factory_t gpgex_factory;
