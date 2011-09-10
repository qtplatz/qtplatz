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

#include "debug.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "string.hpp"

using namespace adportable;

namespace adportable {
    namespace internal {

        class logfile {
            static logfile * instance_;
            std::string filename_;
            logfile() : filename_( "debug.log" ) {}
            ~logfile() {}
        public:
            static logfile * instance() {
                if ( instance_ == 0 ) {
                    instance_ = new logfile();
                    atexit( dispose );
                }
                return instance_;
            }
            static void dispose() {
                delete instance_;
            }
            const std::string& filename() const { return filename_; }
            void filename( const std::string& filename ) {
                filename_ = filename;
            }
        };
    }
}

internal::logfile * internal::logfile::instance_ = 0;


debug::debug( const char * file, const int line ) : line_(line)
{
    if ( file )
        file_ = file;
}

debug::~debug(void)
{
    using namespace internal;
    // std::ofstream of( logfile::instance()->filename().c_str(), std::ios_base::out | std::ios_base::app );
    std::ofstream of( "debug.log", std::ios_base::out | std::ios_base::app );
    if ( ! file_.empty() )
        of << where();
    of << o_.str() << std::endl;
    std::cout << o_.str() << std::endl;
}

void
debug::initialize( const std::string& name )
{
    internal::logfile::instance()->filename( name );
}

std::string
debug::where() const
{
    std::ostringstream o;
    if ( ! file_.empty() ) 
        o << file_ << "(" << line_ << "):";
    return o.str();
}

debug&
debug::operator << ( const char * text )
{
    o_ << text;
    return *this;
}

debug&
debug::operator << ( const unsigned char * text )
{
    o_ << text;
    return *this;
}

debug&
debug::operator << ( const std::string& text )
{
    o_ << text;
    return *this;
}

debug&
debug::operator << ( const std::wstring& text )
{
    o_ << adportable::string::convert( text );
    return *this;
}

debug&
debug::operator << ( bool b )
{
    o_ << b;
    return *this;
}

debug&
debug::operator << ( char c )
{
    o_ << c;
    return *this;
}

debug&
debug::operator << ( unsigned char c )
{
    o_ << c;
    return *this;
}

debug&
debug::operator << ( int n )
{
    o_ << n;
    return *this;
}

debug&
debug::operator << ( unsigned int n )
{
    o_ << n;
    return *this;
}

debug&
debug::operator << ( long x )
{
    o_ << x;
    return *this;
}

debug&
debug::operator << ( unsigned long x )
{
    o_ << x;
    return *this;
}

debug&
debug::operator << ( double d )
{
    o_ << d;
    return *this;
}

