//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef __JFileStream_h__
#define __JFileStream_h__

#include "JStream.h"
#include <QString>
#include <QFile>

///
/// Stream for file storage reading and writing
///

class JSTREAM_EXPORT JFileStream : public JStream
{
public:

	JFileStream();
	JFileStream( const QFile& file );
	JFileStream( const QString& name );
	virtual ~JFileStream();

	inline virtual gint64 size();
	inline virtual void close();
	inline virtual bool seek( gint64 position );
	inline virtual gint64 pos() const;
	inline virtual void reset();

	///
	/// Flush output file's buffers
	inline virtual void flush();

	///
	/// Close old file and open new file
	bool open( const QString& fileName );

	// Unhide inherited read method
	using JStream::write;

	virtual gint64 write( const char* pBuffer, gint64 bufLen );

	// Unhide inherited read method
	using JStream::read;

	virtual gint64 read(char* pBuffer, gint64 bufLen);
	virtual gint64 readLine( char* data, gint64 maxSize );
	virtual QByteArray readLine();

	inline virtual bool atEnd() const;

	void resize( gint64 newSize );

private:

	JFileStream (const JFileStream& rhs);			// cannot be copied
	JFileStream& operator=(const JFileStream& rhs);	// nor assigned

protected:

	QFile file_;
};

#endif //__JFileStream_h__

