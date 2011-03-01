//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JStream_h__
#define __JStream_h__

#include "JInputStream.h"
#include "JOutputStream.h"

# ifdef Q_OS_WIN32
#  define JSTREAM_EXPORT __declspec(dllexport) 
# else
#  define JSTREAM_EXPORT
# endif

///
/// Base class for input and output streams
///

class JSTREAM_EXPORT JStream : public JInputStream, public JOutputStream
{
public:

	virtual ~JStream() {}

	virtual bool seek( gint64 position ) = 0;
	virtual gint64 size() = 0;
	virtual gint64 pos() const = 0;
	virtual void reset() = 0;
	virtual void close() = 0;

	using JInputStream::read;
	using JOutputStream::write;
};

#endif // __JStream_h__

