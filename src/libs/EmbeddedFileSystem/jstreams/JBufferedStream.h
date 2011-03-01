//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JBufferedStream_h__
#define __JBufferedStream_h__

#include "JStream.h"

# ifdef Q_OS_WIN32
#  define JSTREAM_EXPORT __declspec(dllexport) 
# else
#  define JSTREAM_EXPORT
# endif

///
/// Buffered JStream
///

class JSTREAM_EXPORT JBufferedStream 
	: public JStream
{
public:
	
	JBufferedStream(JStream* source, const int bufSize = 8192, bool deleteSource = true);
	virtual ~JBufferedStream();

	void setDeleteSource(bool value = true);

	virtual bool seek( gint64 position );
	virtual gint64 size();
	virtual gint64 pos() const;	
	virtual void close();
	
	virtual gint64 read( char* pBuffer, gint64 bufLen );
	virtual gint64 write(const char* pBuffer, gint64 bufLen );
	void writeByte(const char b);

	void flush();

	///
	/// Set internal stream pointer to beginning
	void reset();

	void resize( gint64 newSize );

protected:
	
	void flushBuffer(bool doRefill = false);	
	bool refill();

private:

	JStream* sourceStream;    // Source stream

	gint64 bufferStart;		  // position of the buffer in source
    

	///
    // The number of valid bytes in the buffer. This value is always 
    // in the range <tt>0</tt> through <tt>buf.length</tt>; elements 
    // <tt>buf[0]</tt> through <tt>buf[count-1]</tt> contain valid 
    // byte data.
	int count;     
	
	int bufferSize;	
	int bufferPosition;		  // position in buffer

	char* buffer;
	bool modified;
	bool deleteSourceStream;
};

#endif // __JStream_h__

