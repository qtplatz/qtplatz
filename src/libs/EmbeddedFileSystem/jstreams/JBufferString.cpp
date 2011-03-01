//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JBufferString.h"
#include <string.h>
#include <ctype.h>

char* statBuffer = "b";

//	======================================================
//	JBufferString::JBufferString

JBufferString::JBufferString():
buffer_(statBuffer), start_(statBuffer), dataLen_(0), bufSize_(0)
{}

//	======================================================
//	JBufferString::JBufferString

JBufferString::JBufferString( char *buffer, long bufSize, bool constString )
{
	buffer_ = buffer;
	start_ = buffer;
	bufSize_ = bufSize;
	setSize( constString ? bufSize_ - 1: 0 );
}

//	======================================================
//	JBufferString::size

long JBufferString::size() const
{
	char *end = buffer_ + dataLen_;
	return end - start_;
}

//	======================================================
//	JBufferString::buffer

char *JBufferString::buffer()
{
	return buffer_;
}

//	======================================================
//	JBufferString::data

char *JBufferString::data() const
{
	return start_;
}

//	======================================================
//	JBufferString::setSize

void JBufferString::setSize( long dataSize )
{
	if ( dataSize > bufSize_ ) return;

	dataLen_ = dataSize;
	buffer_[ dataLen_ ] = 0;
}

//	======================================================
//	JBufferString::reset

void JBufferString::reset()
{
	start_ = buffer_;
}

//	======================================================
//	JBufferString::trim

void JBufferString::trim()
{
	char *end = buffer_ + dataLen_;

	while ( start_ < end && isWhiteSpace( *start_ ) ) start_++;
	while ( end > start_ && isWhiteSpace( *( end - 1 ) ) )
	{
		dataLen_--;
		end--;
	}

	setSize( dataLen_ );
}

//	======================================================
//	JBufferString::clear

void JBufferString::clear()
{
	dataLen_ = 0;
	start_= buffer_;
}

//	======================================================
//	JBufferString::add

void JBufferString::add( const char *data, long dataLen )
{
	if ( dataLen_ + dataLen > bufSize_ ) return;

	::memcpy( buffer_ + dataLen_, data, dataLen );
	dataLen_ += dataLen;
	setSize( dataLen_ );
}

//	======================================================
//	JBufferString::add

void JBufferString::add( char c )
{
	add( &c, 1 );
}

//	======================================================
//	JBufferString::add

void JBufferString::add( const JBufferString &bufString )
{
	add( bufString.data(), bufString.size() );
}

//	======================================================
//	JBufferString::assign

void JBufferString::assign( const char *data, long dataLen )
{
	if ( dataLen < 0 || dataLen > bufSize_ ) return;

	::memcpy( buffer_, data, dataLen );
	setSize( dataLen );
	start_ = buffer_;
}

//	======================================================
//	JBufferString::assign

void JBufferString::assign( JBufferString &bufString )
{
	assign( bufString.data(), bufString.size() );
}

//	======================================================
//	JBufferString::remove

void JBufferString::remove( long from, long count )
{
	if ( from < 0 || count < 0 )
	{
		return;
	}

	char *startPos = start_ + from;
	char *endPos = startPos + count;

	long copyCount = buffer_ + dataLen_ - endPos;

	::memmove( startPos, endPos, copyCount );

	setSize( dataLen_ - count );
}

//	======================================================
//	JBufferString::mid

void JBufferString::mid( long start, long count )
{
	if ( start < 0 || count < 0 )
	{
		return;
	}

	start_ += start;
	setSize( start_ + count - buffer_ );
}

//	======================================================
//	JBufferString::at

char JBufferString::at( long index ) const
{
	return *( start_ + index );
}

//	======================================================
//	JBufferString::indexOf

long JBufferString::indexOf( const char *text ) const
{
	if ( dataLen_ <= 0 )
	{
		return -1;
	}

	char *ptr = start_;
	char *end = buffer_ + dataLen_;
	long len = strlen( text );

	while ( ptr != end )
	{
		if ( !memcmp( text, ptr, len ) )
		{
			return ptr - start_;
		}

		ptr++;
	}

	return -1;
}

//	======================================================
//	JBufferString::indexOf

long JBufferString::indexOf( char c, int from ) const
{
	if ( dataLen_ <= 0 )
	{
		return -1;
	}

	char *ptr = start_ + from;
	char *end = buffer_ + dataLen_;

	if ( ptr > end ) return -1;

	while ( ptr != end )
	{
		if ( *ptr == c )
		{
			return ptr - start_;
		}

		ptr++;
	}

	return -1;
}

//	======================================================
//	JBufferString::lastIndexOf

long JBufferString::lastIndexOf( char c ) const
{
	if ( dataLen_ <= 0 )
	{
		return -1;
	}

	char *ptr = buffer_ + dataLen_;

	do
	{
		ptr--;

		if ( *ptr == c )
		{
			return ptr - start_;
		}
	}
	while ( ptr != start_ );

	return -1;
}

//	======================================================
//	JBufferString::startsWith

bool JBufferString::startsWith( const char *text )
{
	if ( dataLen_ <= 0 )
	{
		return false;
	}

	long len = strlen( text );
	char save = start_[ len ];
	start_[ len ] = 0;

	bool Result = !strcmp( start_, text );
	start_[ len ] = save;

	return Result;
}

//	======================================================
//	JBufferString::startsWith

bool JBufferString::startsWith( char c )
{
	return *start_ == c;
}

//	======================================================
//	JBufferString::endsWith

bool JBufferString::endsWith( char c )
{
	return buffer_[ dataLen_ - 1 ] == c;
}

//	======================================================
//	JBufferString::compareNoCase

bool JBufferString::compareNoCase( const char *data, long dataLen ) const
{
	if ( dataLen_ <= 0 )
	{
		return false;
	}

	if ( size() != dataLen ) return false;

	char *start = start_;
	const char *end = start + dataLen;
	const char *dataPos = data;

	while ( start != end )
	{
		if ( tolower( *start ) != tolower( *dataPos ) )
		{
			return false;
		}

		start++;
		dataPos++;
	}

	return true;
}

//	======================================================
//	JBufferString::compareNoCase

bool JBufferString::compareNoCase( const char *text ) const
{
	return compareNoCase( text, strlen( text ) );
}

//	======================================================
//	JBufferString::isEqual

bool JBufferString::isEqual( const char *buf, long len )
{
	if ( size() != len ) return false;

	return !::memcmp( start_, buf, len );
}

//	======================================================
//	JBufferString::isEqual

bool JBufferString::isEqual( const JBufferString &bufString )
{
	return isEqual( bufString.data(), bufString.size() );
}

//	======================================================
//	JBufferString::toLower
void JBufferString::toLower()
{
	if ( dataLen_ <= 0 )
		return;

	char *pos = start_;
	const char *end = start_ + dataLen_;

	while ( pos != end )
	{
		*pos = tolower( *pos );
		pos++;
	}
}

bool JBufferString::operator == ( const JBufferString& rhv )
{
	return isEqual( rhv.data(), rhv.size() );
}

bool JBufferString::operator < ( const JBufferString& rhv ) const
{
	return ( strcmp( start_, rhv.data() ) < 0 );
}

//	======================================================
//	JBufferString::isEmpty

bool JBufferString::isEmpty()
{
	return 0 == dataLen_;
}

//	======================================================
//	JBufferString::isWhiteSpace

bool JBufferString::isWhiteSpace( char c )
{
	switch ( c )
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
	case '\f':
		return true;
	default:
		return false;
	}
}

//	======================================================
//	JBufferString::isDigit

bool JBufferString::isDigit( char c )
{
	return ( c >= '0' && c <= '9' );
}

//	======================================================
//	JBufferString::isAlpha

bool JBufferString::isAlpha( char c )
{
	return ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) );
}

