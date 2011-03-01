//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

//	============================================================
//	JBufferedInputStream::JBufferedInputStream

template< class T, guint BufferSize >
JBufferedInputStream< T, BufferSize >::JBufferedInputStream()
{
	init();
}

//	============================================================
//	JBufferedInputStream::JBufferedInputStream

template< class T, guint BufferSize >
JBufferedInputStream< T, BufferSize >::JBufferedInputStream( T *readSource )
{
	setSource( readSource );
}

//	============================================================
//	JBufferedInputStream::setSource

template< class T, guint BufferSize >
void JBufferedInputStream< T, BufferSize >::setSource( T *readSource )
{
	init();
	source_ = readSource;
}

//	============================================================
//	JBufferedInputStream::source

template< class T, guint BufferSize >
T* JBufferedInputStream< T, BufferSize >::source()
{
	return source_;
}

//	============================================================
//	JBufferedInputStream::readLine

template< class T, guint BufferSize >
gint64 JBufferedInputStream< T, BufferSize >::readLine( char *buffer, gint64 bufLen )
{
	long bufPos = 0;

	while ( !nextLine( buffer, bufLen, bufPos ) && !atEnd() )
	{
		chunkBufSize_ = source_->read( chunkBuffer_, BufferSize );
		chunkBufPtr_ = 0;
	}

	return bufPos;
}

//	============================================================
//	JBufferedInputStream::read

template< class T, guint BufferSize >
inline int JBufferedInputStream< T, BufferSize >::read()
{
	if ( atEnd() )
	{
		// End of stream reached
		return -1;
	}

	if ( chunkBufPtr_ >= chunkBufSize_ )
	{
		chunkBufSize_ = source_->read( chunkBuffer_, BufferSize );
		chunkBufPtr_ = 0;
	}

	char nextChar = chunkBuffer_[ chunkBufPtr_ ];
	chunkBufPtr_++;
	pos_++;

	return nextChar;
}

//	============================================================
//	JBufferedInputStream::pos

template< class T, guint BufferSize >
gint64 JBufferedInputStream< T, BufferSize >::pos() const
{
	return pos_;
}

//	============================================================
//	JBufferedInputStream::seek

template< class T, guint BufferSize >
bool JBufferedInputStream< T, BufferSize >::seek( gint64 pos )
{
	// Check for low boundary
	gint64 lowBoundary = pos_ - chunkBufPtr_;
	gint64 highBoundary = lowBoundary + chunkBufSize_ - 1;

	if ( pos >= lowBoundary && pos < highBoundary )
	{
		pos_ = pos;
		chunkBufPtr_ = pos_ - lowBoundary;
		return pos_;
	}

	pos_ = pos;
	chunkBufSize_ = 0;
	return source_->seek( pos );
}

//	============================================================
//	JBufferedInputStream::atEnd

template< class T, guint BufferSize >
bool JBufferedInputStream< T, BufferSize >::atEnd() const
{
	return ( chunkBufPtr_ >= chunkBufSize_ && source_->atEnd() );
}

//	============================================================
//	JBufferedInputStream::init

template< class T, guint BufferSize >
void JBufferedInputStream< T, BufferSize >::init()
{
	chunkBufPtr_ = 0;
	chunkBufSize_ = 0;
	pos_ = 0;
	source_ = 0;
}

//	============================================================
//	JBufferedInputStream::nextLine

template< class T, guint BufferSize >
bool JBufferedInputStream< T, BufferSize >::nextLine( char *buffer, long bufLen, long &bufPos )
{
	char nextChar = 0;

	while ( chunkBufPtr_ < chunkBufSize_ )
	{
		nextChar = chunkBuffer_[ chunkBufPtr_ ];
		chunkBufPtr_++;
		pos_++;

		if ( nextChar == '\r' && chunkBuffer_[ chunkBufPtr_ ] == '\n' )
		{
			chunkBufPtr_++;
			pos_++;

			if ( bufPos + 2 >= bufLen )
			{
				return false;
			}

			buffer[ bufPos++ ] = '\r';
			buffer[ bufPos++ ] = '\n';
			return true;
		}
		else if ( nextChar == '\r' )
		{
			if ( bufPos + 1 >= bufLen )
			{
				return false;
			}

			buffer[ bufPos++ ] = '\r';
			return true;
		}
		else if ( nextChar == '\n' )
		{
			if ( bufPos + 1 >= bufLen )
			{
				return false;
			}

			buffer[ bufPos++ ] = '\n';
			return true;
		}
		else
		{
			if ( bufPos + 1 >= bufLen )
			{
				return false;
			}

			buffer[ bufPos++ ] = nextChar;
		}
	}

	return false;
}

//	============================================================
//	JBufferedInputStream::size

template< class T, guint BufferSize >
gint64 JBufferedInputStream< T, BufferSize >::size()
{
	return source_->size();
}

//	============================================================
//	JBufferedInputStream::skip

template< class T, guint BufferSize >
gint64 JBufferedInputStream< T, BufferSize >::skip( gint64 n )
{
	gint64 ret = n;

	while ( n > 0 )
	{
		read();
		n--;
	}

	return ret;
}

//	============================================================
//	JBufferedInputStream::read

template< class T, guint BufferSize >
gint64 JBufferedInputStream< T, BufferSize >::read( char* pBuffer, gint64 bufLen )
{
	long bufPos = 0;

	while ( !canRead( pBuffer, bufLen, bufPos ) && !atEnd() )
	{
		chunkBufSize_ = source_->read( chunkBuffer_, BufferSize );
		if ( -1 == chunkBufSize_ || !chunkBufSize_ ) break;
		chunkBufPtr_ = 0;
	}

	return bufPos;
}

//	============================================================
//	JBufferedInputStream::bufferRead

template< class T, guint BufferSize >
void JBufferedInputStream< T, BufferSize >::bufferRead( char* pBuffer, gint64 bufLen )
{
	::memcpy( pBuffer, chunkBuffer_, bufLen );
}

//	============================================================
//	JBufferedInputStream::canRead

template< class T, guint BufferSize >
bool JBufferedInputStream< T, BufferSize >::canRead( char *buffer, long bufLen, long &bufPos )
{
	char nextChar = 0;

	while ( chunkBufPtr_ < chunkBufSize_ )
	{
		if ( bufPos >= bufLen )
		{
			return true;
		}

		nextChar = chunkBuffer_[ chunkBufPtr_ ];
		chunkBufPtr_++;
		pos_++;

		buffer[ bufPos ] = nextChar;
		bufPos++;
	}

	return false;
}

//	============================================================
//	JBufferedInputStream::initBuffer

template< class T, guint BufferSize >
void JBufferedInputStream< T, BufferSize >::initBuffer()
{
	if ( pos_ != 0 ) return;
	source_->seek( pos_ );
	chunkBufSize_ = source_->read( chunkBuffer_, BufferSize );
	chunkBufPtr_ = 0;
}
