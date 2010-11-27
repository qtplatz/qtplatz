#include "log.h"
#include <adportable/string.h>
#include "mainwindow.h"
#include <boost/format.hpp>

logger::logger(void)
{
}

logger::~logger(void)
{
    if ( ! buf_.str().empty() )
        print( buf_.str().c_str() );
}

void
logger::print( const char * text )
{
    MainWindow::instance()->emit_debug_print( 0, 0, text );
}

logger&
logger::operator << ( const char * text )
{
    buf_ << text;
    return *this;
}

logger&
logger::operator << ( const wchar_t * text )
{
    buf_ << adportable::string::convert( text );
    return *this;
}

logger&
logger::operator << ( const std::string& text )
{
    buf_ << text;
    return *this;
}

logger&
logger::operator << ( const std::wstring& text )
{
    buf_ << adportable::string::convert( text );
    return *this;
}

logger&
logger::operator << ( int x )
{
    buf_ << x;
    return * this;
}

logger&
logger::operator << ( unsigned char x )
{
    buf_ << x;
    return * this;
}

logger&
logger::operator << ( unsigned int x )
{
    buf_ << x;
    return * this;
}


logger&
logger::operator << ( unsigned short x )
{
    buf_ << x;
    return * this;
}

logger&
logger::operator << ( unsigned long x )
{
    buf_ << x;
    return * this;
}

logger&
logger::dump( size_t len, const char * p )
{
    std::ostringstream o;
    while ( len-- ) 
        o << boost::format( "%02x " ) % int(unsigned char(*p++));
    *this << o.str();
    return *this;
}