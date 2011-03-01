//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __ByteInputStream_h__
#define __ByteInputStream_h__

#include "JInputStream.h"
#include <QByteArray>
#include <QBuffer>

// Class JByteInputStream

class JSTREAM_EXPORT JByteInputStream : public JInputStream
{
public:
	JByteInputStream( const char* pData, gint64 dataLen );
	JByteInputStream( const QByteArray &array );
	
	bool setSource( const QByteArray &array );

	virtual bool seek( gint64 position );

	virtual gint64 size();

	virtual bool atEnd() const;

	virtual gint64 pos() const;
	virtual void close();
	virtual void reset();

	using JInputStream::read;

	virtual gint64 read( char* pBuffer, gint64 bufLen );
	virtual gint64 readLine( char* data, gint64 maxSize );
	virtual QByteArray readLine();

private:
	
	JByteInputStream( const JByteInputStream& rhs );
	JByteInputStream& operator = ( const JByteInputStream& rhs ); // nor assigned

private:

	QBuffer Buffer_;
};

#endif // __ByteInputStream_h__

