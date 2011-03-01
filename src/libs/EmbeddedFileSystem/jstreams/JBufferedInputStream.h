//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JBufferedInputStream_h__
#define __JBufferedInputStream_h__

#include "JInputStream.h"

const guint BufferSize64k = 65535;
//const long ReadChunkSize = 4096;

///
/// Class JBufferedInputStream
///

template< class T, guint BufferSize = BufferSize64k >
class JBufferedInputStream : public JInputStream
{
public:

	JBufferedInputStream();
	JBufferedInputStream( T *readLineSource );

	///
	/// Set current source for reading lines.
	/// Source must support 'retSize read( buf, len )' and
	/// 'bool atEnd()' methods
	inline void setSource( T *readLineSource );
	T* source();

	///
	/// Reads next line from the source. Line must ends with \r\n.
	/// Source have to be opened before calling this method
	/// @return line length
	virtual gint64 readLine( char *buffer, gint64 bufLen );

	///
	/// Reads next char from the stream using buffer. Much faster than ususal 
	/// one bytes read from OS object.
	virtual int read();

	///
	/// Returns true if end of source have been reached, 
	/// false otherwise
	virtual bool atEnd() const;

	///
	/// @return true read pointer (no matter how much bytes read 
	///			from the source)
	virtual gint64 pos() const;

	///
	/// Set read pointer to new position. Resets internal buffer.
	/// @return source::seek result value.
	virtual bool seek( gint64 pos );

	///
	/// Returns internal stream size
	virtual gint64 size();

	///
	/// Skips specified amount of bytes
	virtual gint64 skip( gint64 n );

	///
	/// Reads next bufLen bytes from the buffered stream.
	/// @return bytes read
	virtual gint64 read( char* pBuffer, gint64 bufLen );

	///
	/// Read from the buffer without changing internal file 
	/// pointer position
	void bufferRead(  char* pBuffer, gint64 bufLen );

	///
	/// Initialize buffer without offset file pointer
	void initBuffer();

private:

	inline void init();
	bool nextLine( char *buffer, long bufLen, long &bufPos );
	bool canRead( char *buffer, long bufLen, long &bufPos );

	T *source_;

	char chunkBuffer_[ BufferSize + 1 ];
	long chunkBufPtr_;
	long chunkBufSize_;
	long pos_;

};

#include "JBufferedInputStream.inl"

#endif // __JBufferedInputStream_h__

