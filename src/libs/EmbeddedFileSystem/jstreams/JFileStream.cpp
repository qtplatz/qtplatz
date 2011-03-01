//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JFileStream.h"
#include <QFileInfo>

//	======================================================
//	JFileStream::JFileStream

JFileStream::JFileStream()
{}

//	======================================================
//	JFileStream::JFileStream

JFileStream::JFileStream( const QFile& file )
{
	open( QFileInfo( file ).absoluteFilePath() );
}

//	======================================================
//	JFileStream::JFileStream

JFileStream::JFileStream(const QString& name)
{
	open( name );
}

//	======================================================
//	JFileStream::~JFileStream

JFileStream::~JFileStream()
{
	flush();
	close();
}

//	======================================================
//	JFileStream::size

inline gint64 JFileStream::size()
{
	return file_.size();
}

//	======================================================
//	JFileStream::close

inline void JFileStream::close()
{
	file_.close();
}

//	======================================================
//	JFileStream::seek

inline bool JFileStream::seek( gint64 position )
{
	return file_.seek( position );
}

//	======================================================
//	JFileStream::pos

inline gint64 JFileStream::pos() const
{
	return file_.pos();
}

//	======================================================
//	JFileStream::reset

inline void JFileStream::reset()
{
	file_.reset();
}

//	======================================================
//	JFileStream::flush

inline void JFileStream::flush()
{
	file_.flush();
}

//	======================================================
//	JFileStream::write

gint64 JFileStream::write( const char* pBuffer, gint64 bufLen )
{
	//if ( !file_.isOpen() ) return -1;

	return file_.write( pBuffer, bufLen );
}

//	======================================================
//	JFileStream::open

bool JFileStream::open( const QString& fileName ) 
{
	if( fileName.isEmpty() )
	{
		return false;
	}
	else
	{
		QFileInfo fi( fileName );
		if ( fi.isDir() )
		{
			return false;
		}
	}

	if ( file_.isOpen() )
	{
		file_.close();
	}

	file_.setFileName( fileName );
	return file_.open( QIODevice::ReadWrite );
}

//	======================================================
//	JFileStream::atEnd

inline bool JFileStream::atEnd() const
{
	return file_.atEnd();
}

//	======================================================
//	JFileStream::read

gint64 JFileStream::read(char* pBuffer, gint64 bufLen)
{
	if(bufLen > LONG_MAX)
	{
		bufLen = LONG_MAX;
	}

	//if ( !file_.isOpen() ) return -1;

	gint64 bytesRead = file_.read( pBuffer, bufLen );

	if ( bytesRead == 0 )
	{
		return JInputStream::EndOfFile;
	}

	return bytesRead;
}

//	======================================================
//	JFileStream::readLine

gint64 JFileStream::readLine( char* data, gint64 maxSize )
{
	return file_.readLine( data, maxSize );
}

//	======================================================
//	JFileStream::readLine

QByteArray JFileStream::readLine()
{
	return file_.readLine();
}

//	======================================================
//	JFileStream::resize

void JFileStream::resize( gint64 newSize )
{
	file_.resize( newSize );
}

