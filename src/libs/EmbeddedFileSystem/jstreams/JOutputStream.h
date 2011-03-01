//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JOutputStream_h__
#define __JOutputStream_h__

#include "PlatformTypes.h"

# ifdef Q_OS_WIN32
#  define JSTREAM_EXPORT __declspec(dllexport) 
# else
#  define JSTREAM_EXPORT
# endif // Q_OS_WIN32

///
/// Class OutputStream
///

class JSTREAM_EXPORT JOutputStream
{
public:

	virtual ~JOutputStream();

	///
	/// Set internal stream position to new value
	virtual bool seek( gint64 position ) = 0;

	///
	/// Returns stream size
	virtual gint64 size() = 0;

	// Write functions
	virtual int write(char ch);
	virtual gint64 write(const char* pBuffer);
	virtual gint64 write(const char* pBuffer, gint64 bufLen ) = 0;

	///
	/// Returns current internal stream data pointer
	virtual gint64 pos() const;

	///
	/// Set internal stream pointer to beginning
	virtual void reset();

	///
	/// Close stream
	virtual void close();

	///
	///Flushes buffered data
	virtual void flush();

	///
	/// Resize stream
	virtual void resize( gint64 newSize );
};

#endif // __JOutputStream_h__