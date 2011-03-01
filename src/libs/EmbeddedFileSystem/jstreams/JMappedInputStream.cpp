//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JMappedInputStream.h"
#include "JInputStream.h"

JMappedInputStream::JMappedInputStream( JInputStream *stream, gint64 size, gint64 start )
{
	stream_ = stream;
	size_ = size;
	start_ = start;
	stream_->seek( start_ );
	bDeleteStream_ = false;
}

JMappedInputStream::~JMappedInputStream()
{
	if ( bDeleteStream_ )
	{
		stream_->close();
		delete stream_;
	}
}

bool JMappedInputStream::seek( gint64 position )
{
	if ( position >= size_ )
	{
		return false;
	}

	return stream_->seek( start_ + position );
}

gint64 JMappedInputStream::skip( gint64 n )
{
	if ( stream_->pos() - start_ + n >= size_ )
	{
		return 0;
	}

	return stream_->skip( start_ + n );
}

gint64 JMappedInputStream::size()
{
	return size_;
}

//int JMappedInputStream::read()
//{
//
//	return 0;
//}

gint64 JMappedInputStream::read( char* pBuffer, gint64 bufLen )
{
	if ( stream_->pos() >= start_ + size_ )
	{
		return -1;
	}

	// Check for start position
	long startPos = 0;
	if ( stream_->pos() < start_ )
	{
		if ( !stream_->seek( start_ ) )
		{
			return -1;
		}

		startPos = start_;
	}
	else
	{
		startPos = stream_->pos();
	}

	gint64 available = start_ + size_ - startPos;
	gint64 forRead = available > bufLen ? bufLen : available;

	return stream_->read( pBuffer, forRead );
}

gint64 JMappedInputStream::readLine( char* data, gint64 maxSize )
{
	if ( stream_->pos() >= start_ + size_ )
	{
		return -1;
	}

	gint64 lineSize = stream_->readLine( data, maxSize );

	if ( lineSize > 0 )
	{
		if ( stream_->pos() > start_ + size_ )
		{
			lineSize -= stream_->pos() - ( start_ + size_ );
		}
	}

	return lineSize;
}

QByteArray JMappedInputStream::readLine()
{
	if ( stream_->pos() >= start_ + size_ )
	{
		return QByteArray();
	}

	QByteArray line;
	line = stream_->readLine();

	if ( line.size() > 0 )
	{
		if ( stream_->pos() > start_ + size_ )
		{
			gint64 lineSize = line.size();
			lineSize -= stream_->pos() - ( start_ + size_ );
			line.resize( lineSize );
		}
	}

	return line;
}

bool JMappedInputStream::atEnd() const
{
	return stream_->atEnd() || ( stream_->pos() == start_ + size_ );
}

gint64 JMappedInputStream::pos() const
{
	return stream_->pos() - start_;
}

void JMappedInputStream::reset()
{
	stream_->seek( start_ );
}

void JMappedInputStream::close()
{
	stream_->close();
}

void JMappedInputStream::setDeleteStream( bool isDelete )
{
	bDeleteStream_ = isDelete;
}

void JMappedInputStream::setStart( gint64 start )
{
	start_ = start;
	stream_->seek( start_ );
 }

void JMappedInputStream::setSize( gint64 size )
{
	size_ = size;
	stream_->seek( start_ );
}

