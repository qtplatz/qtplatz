//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __StringStreamWrapper_h__
#define __StringStreamWrapper_h__

#include <QByteArray>
#include <QString>
#include "JOutputStream.h"

template< class T >
class StringStreamWrapper : public T
{
public:

	StringStreamWrapper();
	StringStreamWrapper( T *writeDest );	

	virtual ~StringStreamWrapper();

	///
	/// Sets current dest for reading lines.
	/// Dest must support 'gint64 write( buf, len )',
	/// 'int write(char)' and 'close()' methods
	virtual void setSource( T *writeDest );

	///
	/// Close stream
	virtual void clear();

	///
	/// Return stream size
	gint64 length();

	// Target stream methods
	void close();
	int write( char ch );
	gint64 pos() const;
	bool seek( gint64 position );
	gint64 size();
	void flush();
	gint64 write( const char* pBuffer, gint64 bufLen );

	// Override String operators
	void operator=( const QString& str );
	void operator=( const QByteArray& str );
	void operator=( const char *str );

	void operator+=( const QString& str );
	void operator+=( const QByteArray& str );
	void operator+=( const char *str );

	void append( const QString& srt );
	void append( const QByteArray& str );
	void append( const char* str );
	void append( QChar ch );

private:

	StringStreamWrapper( const StringStreamWrapper& rhs );
	StringStreamWrapper& operator=( const StringStreamWrapper& rhs );

	T *dest_;
};

#include "StringStreamWrapper.inl"

#endif // __StringStreamWrapper_h__
