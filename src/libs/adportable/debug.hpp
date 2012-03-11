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

#include <sstream>

namespace adportable {

    class debug {
        std::ostringstream o_;
        std::string file_;
        int line_;
    public:
        debug(const char * file = 0, const int line = 0);
        ~debug(void);
        static void initialize( const std::string& filename );
        std::string where() const;
	std::string str() const { return o_.str(); }
        debug& operator << ( const char * );
        debug& operator << ( const unsigned char * );
        debug& operator << ( const std::string& );
        debug& operator << ( const std::wstring& );
        debug& operator << ( bool );
        debug& operator << ( char );
        debug& operator << ( unsigned char );
        debug& operator << ( int );
        debug& operator << ( unsigned int );
        debug& operator << ( long );
        debug& operator << ( unsigned long );
        debug& operator << ( float );
        debug& operator << ( double );
    };

	class scope_timer {
	public:
		scope_timer();
		unsigned long elapsed();
		unsigned long t0;
	};

}
