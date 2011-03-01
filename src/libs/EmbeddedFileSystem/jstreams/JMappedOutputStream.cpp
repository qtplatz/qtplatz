//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JMappedOutputStream.h"
#include "JOutputStream.h"

JMappedOutputStream::JMappedOutputStream( JOutputStream *stream, gint64 size, gint64 start )
: stream_(stream), size_(size), start_(start), bDeleteStream_(false), bResizable_(false), bStatus_(true)
{
	stream_->seek( start_ );
}

JMappedOutputStream::~JMappedOutputStream()
{
	flush();
	if ( bDeleteStream_ )
	{
		stream_->close();
		delete stream_;
	}
}

bool JMappedOutputStream::seek( gint64 position )
{
	if ( position >= size_ )
	{
		return false;
	}

	return stream_->seek( start_ + position );
}

inline gint64 JMappedOutputStream::size()
{
	return size_;
}

inline gint64 JMappedOutputStream::pos() const
{
	return stream_->pos() - start_;
}

inline void JMappedOutputStream::reset()
{
	stream_->seek( start_ );
}

inline void JMappedOutputStream::flush()
{
	stream_->flush();
}

void JMappedOutputStream::close()
{
	flush();
	stream_->close();
}

inline bool JMappedOutputStream::isOk() const
{
	return bStatus_;
}

inline bool JMappedOutputStream::getDeleteStream() const
{
	return bDeleteStream_;
}

inline bool JMappedOutputStream::getResizable() const
{
	return bResizable_;
}

inline gint64 JMappedOutputStream::getStart() const
{
	return start_;
}

inline void JMappedOutputStream::setDeleteStream( bool isDelete )
{
	bDeleteStream_ = isDelete;
}

inline void JMappedOutputStream::setResizable( bool isResizable )
{
	bResizable_ = isResizable;
}

void JMappedOutputStream::setStart( gint64 start )
{
	start_ = start;
	stream_->seek( start_ );
}

void JMappedOutputStream::setSize( gint64 size )
{
	size_ = size;
	stream_->seek( start_ );
}

gint64 JMappedOutputStream::write(const char* pBuffer, gint64 bufLen)
{
	gint64 curPos = stream_->pos() - start_;

	//if MappedStream is resizable simply write all data
	if(bResizable_)
	{		
		gint64 writenBytes = stream_->write(pBuffer, bufLen);

		//check writing errors
		if( writenBytes != bufLen ) bStatus_ = false;
		if( curPos + writenBytes >= size_ ) size_ = curPos + writenBytes;
		return writenBytes;
	}

	//if incoming data too large and stream is not resizable retrun error
	if( curPos + bufLen >= size_ )
	{
		return -1;
	}
	return stream_->write(pBuffer, bufLen);
}