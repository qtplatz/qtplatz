// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>

# if defined _MSC_VER
#  pragma warning (disable: 4996)
# endif
# include <ace/CDR_Stream.h>

namespace acewrapper {

	class OutputCDR : boost::noncopyable {
	public:
        ~OutputCDR();
		OutputCDR( ACE_OutputCDR& );

		operator ACE_OutputCDR& () { return impl_; }
		operator const ACE_OutputCDR& () const { return impl_; }
		const ACE_Message_Block * begin() { return impl_.begin(); }
		size_t length() const { return impl_.length(); }

		OutputCDR& operator << (bool);
		OutputCDR& operator << (char);
		OutputCDR& operator << (unsigned char);
		OutputCDR& operator << (short);
		OutputCDR& operator << (unsigned short);
		OutputCDR& operator << (long);
		OutputCDR& operator << (unsigned long);
		OutputCDR& operator << ( boost::uint32_t );
		OutputCDR& operator << (long long);
		OutputCDR& operator << (unsigned long long);
		OutputCDR& operator << (float);
		OutputCDR& operator << (double);
		OutputCDR& operator << (const std::string& );
		OutputCDR& operator << (const std::wstring& );
		bool write( const bool *, size_t );
		bool write( const char *, size_t );
		bool write( const unsigned char *, size_t );
		bool write( const short *, size_t );
		bool write( const unsigned short *, size_t );
		bool write( const long *, size_t );
		bool write( const unsigned long *, size_t );
		bool write( const long long *, size_t );
		bool write( const unsigned long long *, size_t );
		bool write( const float *, size_t );
		bool write( const double *, size_t );
		bool write( const long double *, size_t );
	private:
		ACE_OutputCDR& impl_;
        ACE_OutputCDR * pImpl_;
	};

}

