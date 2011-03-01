//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

//	===========================================================
//	JBufferedOutputStream< T >::JBufferedOutputStream

template < class T > 
JBufferedOutputStream< T >::JBufferedOutputStream():
chunkBufPtr_( 0 ),
dest_( 0 ),
status_( true )
{}

//	===========================================================
//	JBufferedOutputStream< T >::JBufferedOutputStream

template < class T > 
JBufferedOutputStream< T >::JBufferedOutputStream( T* writeDest ):
chunkBufPtr_( 0 ),
dest_( writeDest ),
status_( true )
{}

//	===========================================================
//	JBufferedOutputStream< T >::~JBufferedOutputStream

template < class T >
JBufferedOutputStream< T >::~JBufferedOutputStream()
{
	close();
}

//	===========================================================
//	JBufferedOutputStream< T >::setSource

template < class T > 
void JBufferedOutputStream< T >::setSource( T* writeDest )
{
	chunkBufPtr_ = 0;
	status_ = true;
	dest_ = writeDest;
}

//	===========================================================
//	JBufferedOutputStream< T >::setSource

template < class T > 
T* JBufferedOutputStream< T >::source()
{
	return dest_;
}

//	===========================================================
//	JBufferedOutputStream< T >::close

template < class T >
void JBufferedOutputStream< T >::close()
{
	flush();
	
	if ( 0 != dest_ )
	{
		dest_->close();
	}
}

//	===========================================================
//	JBufferedOutputStream< T >::flush

template < class T >
void JBufferedOutputStream< T >::flush()
{
	if ( chunkBufPtr_ != 0 ) flushBuffer();
}

//	===========================================================
//	JBufferedOutputStream< T >::size

template < class T > 
inline gint64 JBufferedOutputStream< T >::size()
{
	const gint64 destSize = dest_->size();
	return ( destSize > dest_->pos() + chunkBufPtr_ ) ? 
		destSize : dest_->pos() + chunkBufPtr_;
}

//	===========================================================
//	JBufferedOutputStream< T >::pos

template < class T > 
inline gint64 JBufferedOutputStream< T >::pos() const
{
	return dest_->pos() + chunkBufPtr_;
}

//	===========================================================
//	JBufferedOutputStream< T >::isOk

template< class T >
inline bool JBufferedOutputStream< T >::isOk() const
{
	return status_;
}

//	===========================================================
//	JBufferedOutputStream< T >::seek

template < class T > 
bool JBufferedOutputStream< T >::seek( gint64 position )
{
	// Check for boundaries
	gint64 lowBoundary = dest_->pos();
	gint64 highBoundary = lowBoundary + chunkBufSize_ - 1;

	if ( ( position >= lowBoundary ) && ( position <= highBoundary ) )
	{
		chunkBufPtr_ = position - lowBoundary;
		return true;
	}

	if ( chunkBufPtr_ )
	{
		flushBuffer();
		if ( !status_ ) return false;
	}

	return dest_->seek( position );
}

//	===========================================================
//	JBufferedOutputStream< T >::seekRaw

template < class T > 
bool JBufferedOutputStream< T >::seekRaw( gint64 position )
{
	flush();
	chunkBufPtr_ = 0;
	return dest_->seek( position );
}

//	===========================================================
//	JBufferedOutputStream< T >::flushBuffer

template < class T >
void JBufferedOutputStream< T >::flushBuffer()
{
	if ( !dest_ ) return;

	gint64 bytesWritten = dest_->write( chunkBuffer_, chunkBufPtr_ );

	status_ = ( bytesWritten == chunkBufPtr_ );
    chunkBufPtr_ = 0;
}

//	===========================================================
//	JBufferedOutputStream< T >::writeToBuffer

template < class T >
void JBufferedOutputStream< T >::writeToBuffer( const char* pBuffer, 
									gint64 bufLen )
{
	::memcpy( chunkBuffer_ + chunkBufPtr_, pBuffer, bufLen );
	chunkBufPtr_ += bufLen;
}

//	===========================================================
//	JBufferedOutputStream< T >::canWrite

template < class T >
inline bool JBufferedOutputStream< T >::canWrite( gint64 bufLen ) const
{
	return ( chunkBufPtr_ + bufLen ) < chunkBufSize_;
}

//	===========================================================
//	JBufferedOutputStream< T >::write

template < class T >
int JBufferedOutputStream< T >::write( char ch )
{
	if ( !canWrite( 1 ) ) 
	{
		flushBuffer();
		if ( !status_ ) return -1;
	}

	chunkBuffer_[ chunkBufPtr_ ] = ch;
	chunkBufPtr_++;
	return sizeof( ch );
}

//	===========================================================
//	JBufferedOutputStream< T >::write

template < class T >
gint64 JBufferedOutputStream< T >::write( const char* pBuffer, 
									gint64 bufLen )
{
	if ( chunkBufPtr_ + bufLen >= chunkBufSize_ )
	{
		guint bufdif = 0;

		if ( chunkBufPtr_ )
		{
			bufdif = chunkBufSize_ - chunkBufPtr_;
			if ( bufdif )
			{
				// Buffer is not full
				writeToBuffer( pBuffer, bufdif );
			}

			flushBuffer();
			if ( !status_ ) return -1;
		}

		gint64 written = dest_->write( pBuffer + bufdif, bufLen - bufdif );
		if ( written != bufLen - bufdif )
		{
			status_ = false;
            return -1;
		}

		return bufLen;
	}

	if ( !canWrite( bufLen ) )
	{
		flushBuffer();
		if ( !status_ ) return -1;
	}

	writeToBuffer( pBuffer, bufLen );
	return bufLen;
}

