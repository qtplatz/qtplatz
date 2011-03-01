//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

//	===================================================================
//	StringStreamWrapper< T >::StringStreamWrapper

template < class T > 
StringStreamWrapper< T >::StringStreamWrapper() :
dest_( 0 )
{}

//	===================================================================
//	StringStreamWrapper< T >::StringStreamWrapper

template < class T > 
StringStreamWrapper< T >::StringStreamWrapper( T* writeDest ) : 
dest_( writeDest )
{}

//	===================================================================
//	StringStreamWrapper< T >::~StringStreamWrapper

template < class T >
StringStreamWrapper< T >::~StringStreamWrapper()
{
	dest_->close();
}

//	===================================================================
//	StringStreamWrapper< T >::setSource

template < class T >
void StringStreamWrapper< T >::setSource( T *writeDest )
{
	dest_ = writeDest;
}

//	===================================================================
//	StringStreamWrapper< T >::close

template < class T >
void StringStreamWrapper< T >::close()
{
	dest_->close();
}

//	===================================================================
//	StringStreamWrapper< T >::clear

template < class T >
void StringStreamWrapper< T >::clear()
{
	dest_->resize( 0 );
}

//	===================================================================
//	StringStreamWrapper< T >::write

template < class T >
int StringStreamWrapper< T >::write( char ch )
{
	return dest_->write( ch );
}

//	===================================================================
//	StringStreamWrapper< T >::pos

template < class T >
gint64 StringStreamWrapper< T >::pos() const
{
	return dest_->pos();
}

//	===================================================================
//	StringStreamWrapper< T >::seek

template < class T >
bool StringStreamWrapper< T >::seek( gint64 position )
{
	return dest_->seek( position );
}

//	===================================================================
//	StringStreamWrapper< T >::size

template < class T >
gint64 StringStreamWrapper< T >::size()
{
	return dest_->size();
}

//	===================================================================
//	StringStreamWrapper< T >::length

template < class T >
gint64 StringStreamWrapper< T >::length()
{
	return dest_->size();
}

//	===================================================================
//	StringStreamWrapper< T >::flush

template < class T >
void StringStreamWrapper< T >::flush()
{
	dest_->flush();
}

//	===================================================================
//	StringStreamWrapper< T >::write

template < class T >
gint64 StringStreamWrapper< T >::write( const char* pBuffer, gint64 bufLen )
{
	return dest_->write( pBuffer, bufLen );
}

//	===================================================================
//	StringStreamWrapper< T >::operator=

template < class T >
void StringStreamWrapper< T >::operator=( const QString& str )
{
	dest_->resize( 0 );
	*this += str;
}

//	===================================================================
//	StringStreamWrapper< T >::operator=

template < class T >
void StringStreamWrapper< T >::operator=( const QByteArray& str )
{
	dest_->resize( 0 );
	*this += str;
}

//	===================================================================
//	StringStreamWrapper< T >::operator=

template < class T >
void StringStreamWrapper< T >::operator=( const char *str )
{
	dest_->resize( 0 );
	*this += str;
}

//	===================================================================
//	StringStreamWrapper< T >::operator+=

template < class T >
void StringStreamWrapper< T >::operator+=( const QString& str )
{
	QByteArray ba = str.toUtf8();
	dest_->write( ba.data(), ba.size() );
}

//	===================================================================
//	StringStreamWrapper< T >::operator+=

template < class T >
void StringStreamWrapper< T >::operator+=( const QByteArray& str )
{
	dest_->write( str.data(), str.size() );
}

//	===================================================================
//	StringStreamWrapper< T >::operator+=

template < class T >
void StringStreamWrapper< T >::operator+=( const char *str )
{
	dest_->write( str );
}

//	===================================================================
//	StringStreamWrapper< T >::append

template < class T >
void StringStreamWrapper< T >::append( const QString& str )
{
	QByteArray ba = str.toUtf8();
	dest_->write( ba.data(), ba.size() );
}

//	===================================================================
//	StringStreamWrapper< T >::append

template < class T >
void StringStreamWrapper< T >::append( const QByteArray& str )
{
	dest_->write( str.data(), str.size() );
}

//	===================================================================
//	StringStreamWrapper< T >::append

template < class T >
void StringStreamWrapper< T >::append( const char* str )
{
	dest_->write( str.data(), str.size() );
}

//	===================================================================
//	StringStreamWrapper< T >::append

template < class T >
void StringStreamWrapper< T >::append( QChar ch )
{
	dest_->write( ch );
}

