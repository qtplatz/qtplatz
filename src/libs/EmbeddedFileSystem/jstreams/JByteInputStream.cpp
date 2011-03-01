//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JByteInputStream.h"

JByteInputStream::JByteInputStream(const char* pData, gint64 dataLen)
{
	QByteArray array( pData, dataLen );
	Buffer_.setData( array );
	Buffer_.open( QIODevice::ReadOnly );
}

JByteInputStream::JByteInputStream( const QByteArray &array )
{
	Buffer_.setData( array );
	Buffer_.open( QIODevice::ReadOnly );
}

bool JByteInputStream::setSource( const QByteArray &array )
{
	Buffer_.close();
	Buffer_.setData( array );
	
	return Buffer_.open( QIODevice::ReadOnly );
}

void JByteInputStream::close()
{
	Buffer_.close();
}

bool JByteInputStream::atEnd() const
{
	return Buffer_.atEnd();
}

bool JByteInputStream::seek(gint64 position)
{
	return Buffer_.seek( position );
}

void JByteInputStream::reset()
{
	Buffer_.reset();
}

gint64 JByteInputStream::pos() const
{
	return Buffer_.pos();
}

gint64 JByteInputStream::read(char* pBuffer, gint64 bufLen)
{
	return Buffer_.read( pBuffer, bufLen );
}

gint64 JByteInputStream::readLine( char* data, gint64 maxSize )
{
	return Buffer_.readLine( data, maxSize );
}

gint64 JByteInputStream::size()
{
	return Buffer_.size();
}

QByteArray JByteInputStream::readLine()
{
	return Buffer_.readLine();
}