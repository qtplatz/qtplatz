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

#ifndef SAFEARRAY_HPP
#define SAFEARRAY_HPP

#pragma once

struct SAFEARRAY;

class safearray_t {
    SAFEARRAY * psa_;
    void * pvoid_;
    uint32_t size_;
public:
    safearray_t( SAFEARRAY * p ) : psa_( p ), pvoid_(0) {
        SafeArrayAccessData( psa_, &pvoid_ );
        uint32_t lBound(0), uBound(0);
        if ( ( SafeArrayGetLBound( psa_, 1, &lBound ) == S_OK ) &&
             ( SafeArrayGetUBound( psa_, 1, &uBound ) == S_OK ) )
            size_ = uBound - lBound + 1;
    }
    ~safearray_t() {
        SafeArrayUnaccessData( psa );
    }
    template<typename T> const T* data() const { return reinterpret_cast<const T*>(pvoid_); }
    uint32_t size() const { return size_; }
};

#endif // SAFEARRAY_HPP
