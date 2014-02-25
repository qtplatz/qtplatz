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

#ifndef SAFEARRAY_HPP
#define SAFEARRAY_HPP

#include <atlbase.h>
class _variant_t;

namespace compassxtract {

	class SafeArray {
	public:
		SafeArray( const _variant_t& v );
		~SafeArray();
		operator bool () const { return p_ && pSA_; }
		const void * p() const { return p_; };
		long size() const { return count_; };
		long lBound() const { return lBound_; };
		long uBound() const { return uBound_; };

		static SAFEARRAY * Validate(const VARIANT& v, long & lBound, long & uBound, long & count);
		static void * GetAccessData(const VARIANT & v, SAFEARRAY * & pSA, long & lBound, long & uBound, long & count);

	private:
		SAFEARRAY * pSA_;
		void * p_;
		long count_;
        long lBound_;
        long uBound_;
	};

}

#endif // SAFEARRAY_HPP
