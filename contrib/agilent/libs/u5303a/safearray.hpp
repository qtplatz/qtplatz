/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#pragma once

#if defined WIN32

#include <atlbase.h>
#include <cstdint>

template<typename T> class safearray_t {
    SAFEARRAY *& psa_;
    T * pdata_;
    ULONG size_;
    VARTYPE vt_;
public:
    safearray_t( SAFEARRAY *& p ) : psa_( p ), pdata_(0), size_(0), vt_(0) {
        SafeArrayAccessData( psa_, reinterpret_cast<void **>(&pdata_) );
		LONG lBound, uBound;
		SafeArrayGetLBound( psa_, 1, &lBound );
		SafeArrayGetUBound( psa_, 1, &uBound );
        SafeArrayGetVartype( psa_, &vt_ );
		size_ = uBound - lBound + 1;
    }
    ~safearray_t() {
        SafeArrayUnaccessData( psa_ );
		SafeArrayDestroy( psa_ );
		psa_ = 0;
    }
    inline const VARTYPE& vt() const { return vt_; }
    inline const T* data() const { return pdata_; }
    inline size_t size() const { return static_cast<size_t>( size_ ); }
};

#endif
