
#include "ppio.hpp"
#include <unistd.h>
#include <sys/io.h>
#include <stdio.h>

ppio::ppio() : success_( false )
             , data_( 0 )
{
    success_ = ( iopl( 3 ) == 0 ) && ioperm( BASEPORT, 5, 1 ) == 0;
    if ( !success_ )
        perror( "ioperm" );
}

ppio::~ppio()
{
    ioperm( BASEPORT, 5, 0 );
}

ppio::operator bool () const
{
    return success_;
}

ppio&
ppio::operator << ( const unsigned char d )
{
    data_ = d;
    outb( d, BASEPORT + 0 );
    return *this;
}

