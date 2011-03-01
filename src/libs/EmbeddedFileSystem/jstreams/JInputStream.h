//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JInputStream_h__
#define __JInputStream_h__

#include "PlatformTypes.h"

#ifdef WIN32
# define JSTREAM_EXPORT __declspec(dllexport) 
#else
# define JSTREAM_EXPORT
#endif

///
/// Class InputStream
///

class JSTREAM_EXPORT JInputStream
{
public:

	virtual ~JInputStream();

	enum {EndOfFile = -1};

	///
	/// Set internal stream position to new value
	virtual bool seek( gint64 position ) = 0;

	///
	/// Skip specified amount of bytes
	virtual gint64 skip( gint64 n );

	///
	/// Returns stream size
	virtual gint64 size() = 0;

	// Read functions
	virtual int read();
	virtual gint64 read( char* pBuffer, gint64 bufLen ) = 0;
	virtual gint64 readLine( char* data, gint64 maxSize );
	virtual QByteArray readLine();

	///
	/// Returns true if stream has no more bytes for reading
	virtual bool atEnd() const;

	///
	/// Returns current internal stream data pointer
	virtual gint64 pos() const;

	///
	/// Set internal stream pointer to beginning
	virtual void reset();

	///
	/// Close stream
	virtual void close();

};

#endif // __JInputStream_h__
