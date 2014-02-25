/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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

#include "safearray.hpp"
#include <atlbase.h>
#include <comutil.h>

using namespace compassxtract;

SafeArray::~SafeArray()
{
	if ( pSA_ ) {
		::SafeArrayUnaccessData( pSA_ );
		pSA_ = 0;
	}
}

SafeArray::SafeArray( const _variant_t& v ) : pSA_(0), p_(0), count_(0), lBound_(0), uBound_(0)
{
	p_ = GetAccessData( v, pSA_, lBound_, uBound_, count_ );
}

SAFEARRAY *
SafeArray::Validate(const VARIANT& v, long & lBound, long & uBound, long & count)
{
	if (!(v.vt & VT_ARRAY))
		return 0;

	SAFEARRAY * pArr = v.parray;
	if ( pArr == 0 )
		return 0;

	if ( 1 != SafeArrayGetDim( pArr ) )
		return 0;

	if (SafeArrayGetLBound(pArr, 1, &lBound) != S_OK)
		return 0;

	if (SafeArrayGetUBound(pArr, 1, &uBound) != S_OK)
		return 0;

	count = uBound - lBound + 1;
	return pArr;
}

void *
SafeArray::GetAccessData(const VARIANT & v, SAFEARRAY * & pSA, 
						 long & lBound, long & uBound, long & count)
{
	void * pData = 0;
	if ( pSA = Validate(v, lBound, uBound, count) ) {
		if ( SafeArrayAccessData(pSA, &pData) != S_OK )
			return 0;
	}
	return pData;
}
