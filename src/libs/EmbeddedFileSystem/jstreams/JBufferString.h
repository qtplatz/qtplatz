//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JBufferString_h__
#define __JBufferString_h__

#include "JInputStream.h"

/// Class JBufferString

class JSTREAM_EXPORT JBufferString
{
public:

	JBufferString();

	/// Ctor does not copy buffer data just use it
	JBufferString( char *buffer, long bufSize, bool constString = false );

	/// Returns data size
	long size() const;

	/// Resturns associated buffer
	char *buffer();

	/// Returns data after all transformations
	char *data() const;

	/// Set data size
	void setSize( long dataSize );

	/// Reset all params (data() will be point on buffer start)
	void reset();

	/// Sets internal pointer on trimmed string
	void trim();

	/// Sets start pointer to buffer beginning and data size to zero
	void clear();

	/// Adds string to the end of buffer
	void add( const char *data, long dataLen );
	void add( char c );
	void add( const JBufferString &bufString );

	/// Assigns new value to the buffer
	void assign( const char *data, long dataLen );
	void assign( JBufferString &bufString );

	void remove( long start, long count );

	void mid( long start, long count );

	char at( long index ) const;

	long indexOf( const char *text ) const;
	long indexOf( char c, int from = 0 ) const;

	long lastIndexOf( char c ) const;

	bool startsWith( const char *text );
	bool startsWith( char c );
	bool endsWith( char c );

	bool compareNoCase( const char *data, long dataLen ) const;
	bool compareNoCase( const char *text ) const;

	/// Compares to another buffer
	bool isEqual( const char *buf, long len );
	bool isEqual( const JBufferString &bufString );

	/// Returns true if data is empty
	bool isEmpty();

	void toLower();

	bool operator<( const JBufferString &rhv ) const;
	bool operator==( const JBufferString &rhv );

	// Static methods
	static bool isWhiteSpace( char c );
	static bool isDigit( char c );
	static bool isAlpha( char c );

private:

	char *buffer_;
	char *start_;
	long dataLen_;
	long bufSize_;

};

#endif // __JBufferString_h__

