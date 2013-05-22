// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include <string>
#include <boost/noncopyable.hpp>
# include <ace/CDR_Stream.h>

namespace acewrapper {

	class InputCDR : boost::noncopyable {
	public:
        InputCDR( ACE_InputCDR& );
        //InputCDR( ACE_Message_Block * mb );

		operator ACE_InputCDR& () { return impl_; }
		operator const ACE_InputCDR& () const { return impl_; }

		InputCDR& operator >> (bool&);
		InputCDR& operator >> (char&);
		InputCDR& operator >> (unsigned char&);
		InputCDR& operator >> (short&);
		InputCDR& operator >> (unsigned short&);
		InputCDR& operator >> (long&);
		InputCDR& operator >> (unsigned long&);
		InputCDR& operator >> (long long&);
		InputCDR& operator >> (unsigned long long&);
		InputCDR& operator >> (float&);
		InputCDR& operator >> (double&);
		InputCDR& operator >> (std::string& );
		InputCDR& operator >> (std::wstring& );
		bool read( bool *, size_t );
		bool read( char *, size_t );
		bool read( unsigned char *, size_t );
		bool read( short *, size_t );
		bool read( unsigned short *, size_t );
		bool read( long *, size_t );
		bool read( unsigned long *, size_t );
		bool read( long long *, size_t );
		bool read( unsigned long long *, size_t );
		bool read( float *, size_t );
		bool read( double *, size_t );
		bool read( long double *, size_t );
        inline char * rd_ptr() { return impl_.rd_ptr(); }
	private:
		ACE_InputCDR& impl_;
	};

}

