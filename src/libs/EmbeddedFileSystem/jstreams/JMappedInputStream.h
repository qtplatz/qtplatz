//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JMappedInputStream_h__
#define __JMappedInputStream_h__

#include "JInputStream.h"

// Class JMappedInputStream

class JSTREAM_EXPORT JMappedInputStream : public JInputStream
{
public:

	JMappedInputStream( JInputStream *stream, gint64 size, gint64 start = 0 );
	virtual ~JMappedInputStream();

	///
	/// Set internal stream position to new value
	bool seek( gint64 position );

	///
	/// Skip specified amount of bytes
	gint64 skip( gint64 n );

	///
	/// Returns stream size
	gint64 size();

	gint64 read( char* pBuffer, gint64 bufLen );
	gint64 readLine( char* data, gint64 maxSize );
	QByteArray readLine();

	///
	/// Returns true if stream has no more bytes for reading
	bool atEnd() const;

	///
	/// Returns current internal stream data pointer
	gint64 pos() const;

	///
	/// Set internal stream pointer to beginning
	void reset();

	///
	/// Close stream
	void close();

	///
	/// Says delete or not stream in destructor
	void setDeleteStream( bool isDelete );

	///
	/// Starts current position from which all actions will be made.
	/// This method sets the current stream position to the start.

	void setStart( gint64 start );

	///
	/// Set size for mapping.
	/// This method sets the current stream position to the start.

	void setSize( gint64 size );

private:

	JInputStream *stream_;

	gint64 start_;
	gint64 size_;
	bool bDeleteStream_;
};

#endif // __JMappedInputStream_h__

