/* gpgex-factory.h - gpgex prototypes
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

#ifndef GPGEX_FACTORY_H
#define GPGEX_FACTORY_H

#include <windows.h>


/* Our class factory interface.  */
class gpgex_factory_t : public IClassFactory
{
 public:
  /* IUnknown methods.  */
  STDMETHODIMP QueryInterface (REFIID riid, void **ppv);
  STDMETHODIMP_(ULONG) AddRef (void);
  STDMETHODIMP_(ULONG) Release (void);

  /* IClassFactory methods.  */
  STDMETHODIMP CreateInstance (LPUNKNOWN punkOuter, REFIID iid,
			       void **ppv);
  STDMETHODIMP LockServer (BOOL fLock);
};


/* The global singleton instance of the GpgEX factory.  */
extern gpgex_factory_t gpgex_factory;

#endif	/* ! GPGEX_FACTORY_H */
