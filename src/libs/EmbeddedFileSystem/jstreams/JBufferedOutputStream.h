//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __BufferedOutputStream_h__
#define __BufferedOutputStream_h__

#include "JOutputStream.h"

///
/// Class JBufferedOutputStream
///

template< class T >
class JBufferedOutputStream : public JOutputStream
{
public:

	JBufferedOutputStream();
	JBufferedOutputStream( T *writeDest );
	virtual ~JBufferedOutputStream();

	void setSource( T *writeDest );
	T* source();

	virtual void close();
	virtual int write( char ch );
	inline virtual gint64 pos() const;
	virtual bool seek( gint64 position );
	bool seekRaw( gint64 position );
	inline virtual gint64 size();

	virtual void flush();
	virtual gint64 write( const char* pBuffer, gint64 bufLen );

	inline bool isOk() const;

protected:

	const static long chunkBufSize_ = 65536;
	//const static long chunkBufSize_ = 4096;

	inline bool canWrite( gint64 bufLen ) const;
	void writeToBuffer(const char* pBuffer, gint64 bufLen);
	void flushBuffer();

	T *dest_;

	char chunkBuffer_[ chunkBufSize_ ];
	long chunkBufPtr_;
	bool status_;

private:

	JBufferedOutputStream(const JBufferedOutputStream& rhs);            // cannot be copied
	JBufferedOutputStream& operator=(const JBufferedOutputStream& rhs); // nor assigned
};

#include "JBufferedOutputStream.inl"

#endif //__BufferedOutputStream_h__
