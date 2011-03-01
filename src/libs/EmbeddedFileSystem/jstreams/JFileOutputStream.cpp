//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JFileOutputStream.h"
#include <QFile>
#include <QFileInfo>

JFileOutputStream::JFileOutputStream(const QFile& file, QIODevice::OpenModeFlag mode)
{
	open( QFileInfo( file ).absoluteFilePath(), mode );
}

JFileOutputStream::JFileOutputStream(const QString& name, QIODevice::OpenModeFlag mode)
{
	open( name, mode );
}

inline gint64 JFileOutputStream::size()
{
	return File_.size();
}

inline void JFileOutputStream::close()
{
	File_.close();
}

inline bool JFileOutputStream::seek( gint64 position )
{
	return File_.seek( position );
}

inline gint64 JFileOutputStream::pos() const
{
	return File_.pos();
}

inline void JFileOutputStream::reset()
{
	File_.reset();
}

inline void JFileOutputStream::flush()
{
	File_.flush();
}

gint64 JFileOutputStream::write( const char* pBuffer, gint64 bufLen )
{
	if ( !File_.isOpen() ) return -1;

	return File_.write( pBuffer, bufLen );
}

bool JFileOutputStream::open(const QString& fileName, QIODevice::OpenModeFlag mode) 
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

	File_.setFileName( fileName );
	return File_.open( mode );
}

void JFileOutputStream::resize( gint64 newSize )
{
	File_.resize( newSize );
}
