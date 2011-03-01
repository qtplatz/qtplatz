//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "EfsDefs.h"

#define mix( a, b, c ) \
{ \
  a -= b; a -= c; a ^= ( c>>13 ); \
  b -= c; b -= a; b ^= ( a<<8 );  \
  c -= a; c -= b; c ^= ( b>>13 ); \
  a -= b; a -= c; a ^= ( c>>12 ); \
  b -= c; b -= a; b ^= ( a<<16 ); \
  c -= a; c -= b; c ^= ( b>>5 );  \
  a -= b; a -= c; a ^= ( c>>3 );  \
  b -= c; b -= a; b ^= ( a<<10 ); \
  c -= a; c -= b; c ^= ( b>>15 ); \
}

//	============================================================
//	EfsNameKey::EfsNameKey

EfsNameKey::EfsNameKey()
:	hash_( 0 )
{
	::memset( name_, 0, EfsDefaultMaxFilenameSize );
}

//	============================================================
//	EfsNameKey::EfsNameKey

EfsNameKey::EfsNameKey( const char* name )
{
	setName( name );
}

//	============================================================
//	EfsNameKey::setName

void EfsNameKey::setName( const char* name )
{
	int len = strlen( name );

	if ( EfsDefaultMaxFilenameSize <= len )
	{
		len = EfsDefaultMaxFilenameSize - 1;
	}

	strncpy( name_, name, len );
	name_[ len ] = 0;
	hash_ = make_hash( name_ );
}

//	============================================================
//	EfsNameKey::make_hash

inline long EfsNameKey::make_hash( const char *k )
{
	if ( !k ) return 0;
	int length = strlen( k );

	register unsigned long int a = 0, b = 0, c = 0, len = 0;
	//length *= sizeof( wchar_t );
	//char* k = ( char* ) url;

	// Set up the internal state
	len = length;
	a = b = 0x9e3779b9;	// the golden ratio; an arbitrary value
	c = 0;

	// Handle most of the key
	while ( len >= 3 )
	{
		a += k[ 0 ];
		b += k[ 1 ];
		c += k[ 2 ];
		mix( a, b, c );
		k += 3;
		len -= 3;
	}

	// Handle the last 2 ub4's
	c += ( length << 2 );
	switch( len )
	{
		// c is reserved for the length
		case 2 : b+=k[1];
		case 1 : a+=k[0];
		// case 0: nothing left to add
	}

	mix( a, b, c );
	return c;
}

//	============================================================
//	EfsNameKey::operator==

bool EfsNameKey::operator==( const EfsNameKey &rhv ) const
{
	return hash_ == rhv.hash_ && !strcmp( name_, rhv.name_ );
}

//	============================================================
//	EfsNameKey::operator<

bool EfsNameKey::operator<( const EfsNameKey &rhv ) const
{
	if ( hash_ != rhv.hash_ )
	{
		return hash_ < rhv.hash_;
	}

	return strcmp( name_, rhv.name_ ) < 0;
}

//	============================================================
//	EfsNameKey::operator>

bool EfsNameKey::operator>( const EfsNameKey &rhv ) const
{
	if ( hash_ != rhv.hash_ )
	{
		return hash_ > rhv.hash_;
	}

	return strcmp( name_, rhv.name_ ) > 0;
}

