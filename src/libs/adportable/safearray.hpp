// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
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
  
