//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "debug.hpp"
#include <fstream>
#include <iomanip>
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
    std::ofstream of( logfile::instance()->filename().c_str(), std::ios_base::out | std::ios_base::app );
    if ( ! file_.empty() )
        of << where();
    of << o_.str() << std::endl;
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
        o << file_ << "(" << line_ << ")";
    return o.str();
}

debug&
debug::operator << ( const char * text )
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
debug::operator << ( int n )
{
    o_ << n;
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

debug&
debug::operator << ( bool b )
{
    o_ << b;
    return *this;
}
