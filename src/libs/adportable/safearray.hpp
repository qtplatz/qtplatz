// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#if defined WIN32

#include <atlbase.h>

namespace adportable {

  template<typename T> class safearray {
  public:
	inline T * p() { return p_; }
	inline VARIANT& v() { return v_; }
	inline void lock() { SafeArrayAccessData( psa_, reinterpret_cast<void **>(&p_) ); }
	inline void unlock() { SafeArrayUnaccessData( psa_ ); }
	inline T& operator [](int idx) { return p_[idx]; }
  private:
	VARIANT v_;
	SAFEARRAY * psa_;
	T * p_;
  public:
	safearray( size_t n, int varType ) : p_(0), psa_(0) {
		::VariantInit(&v_);
		psa_ = ::SafeArrayCreateVector(varType, 1, ULONG( n ) );
		lock();
		v_.vt = VT_ARRAY | varType;
		v_.parray = psa_;
	}
	~safearray() {
		::VariantClear(&v_);
	}
  };
}
#endif // WIN32 only
  
