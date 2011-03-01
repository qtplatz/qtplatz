//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifndef WIN32

#include "EfsPosixFile.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#include <dirent.h>
#include <utime.h>

#include <QString>
//#include <iostream>

#include <sys/types.h>
#include <unistd.h>
#define O_RANDOM 0
#ifdef O_BINARY
# undef O_BINARY //reduces compile errors
#endif
#define O_BINARY 0
#define _close ::close
#define _lseek lseek
#define _read ::read
#define _write write
#define _tell(fhandle) lseek(fhandle, 0, SEEK_CUR)

//	=============================================================
//	EfsPosixFile::EfsPosixFile

EfsPosixFile::EfsPosixFile()
:	fd_( 0 )
{}

//	=============================================================
//	EfsPosixFile::EfsPosixFile

EfsPosixFile::~EfsPosixFile()
{
	if ( fd_ ) close();
}

//	=============================================================
//	EfsPosixFile::open

bool EfsPosixFile::open( const unsigned short *filename )
{
	QString path = QString::fromUtf16( filename );
	fd_ = ::open( path.toLocal8Bit().data(), O_BINARY | O_CREAT  | O_RANDOM | O_RDWR, __S_IREAD | __S_IWRITE );
	return fd_ != -1;
}

//	=============================================================
//	EfsPosixFile::read

int EfsPosixFile::read( char *buffer, int size )
{
	long bytesRead = ::read( fd_, buffer, size );

	if ( bytesRead < 0 )
	{
		return -1;
	}

	return bytesRead;
}

//	=============================================================
//	EfsPosixFile::write

int EfsPosixFile::write( const char *buffer, int size )
{
	long bytesWritten = ::write( fd_, buffer, size );

	if ( bytesWritten < 0 )
	{
		return -1;
	}

	return bytesWritten;
}

//	=============================================================
//	EfsPosixFile::close

void EfsPosixFile::close()
{
	::close( fd_ );
	fd_ = -1;
}

//	=============================================================
//	EfsPosixFile::seek

gint64 EfsPosixFile::seek( gint64 size )
{
	if( lseek( fd_, size, SEEK_SET ) != size )
	{
		return -1;
	}

	return size;
}

//	=============================================================
//	EfsPosixFile::pos

gint64 EfsPosixFile::pos()
{
	return ::lseek( fd_, 0, SEEK_CUR );
}

//	=============================================================
//	EfsPosixFile::resize

bool EfsPosixFile::resize( gint64 newSize )
{
	if(!ftruncate(fd_, (off_t)newSize))
	{
		return true;
	}
	else
	{
//		qDebug() << "file resizing failed with error code :" << errno;
		return false;
	}
	
}

//	=============================================================
//	EfsPosixFile::size

gint64 EfsPosixFile::size()
{
	struct stat st;

	if (fstat(fd_, &st))
	{
		return -1;
	}
	return (gint64)st.st_size;
}

//	=============================================================
//	EfsPosixFile::isEmpty

bool EfsPosixFile::isEmpty()
{
	return size() == 0;
}

//	=============================================================
//	EfsPosixFile::remove

void EfsPosixFile::remove( const unsigned short *path )
{
	if(!QFile::remove( QString::fromUtf16( path ) ))
	{
//		qDebug() << "failed to remove the file";
	}
}

//	=============================================================
//	EfsPosixFile::rename

void EfsPosixFile::rename( const unsigned short *from, const unsigned short *to )
{
//	if(rename(QString::fromUtf16(from), QString::fromUtf16(to))
//	{
//		qDebug() << "renaming failed with error code : " << errno;
//	}
	::rename(QString::fromUtf16(from).toLocal8Bit().data(), QString::fromUtf16(to).toLocal8Bit().data());
}


#endif // WIN32

