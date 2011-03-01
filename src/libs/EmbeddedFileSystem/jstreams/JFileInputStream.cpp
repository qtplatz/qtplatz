//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "JFileInputStream.h"
#include <QFileInfo>
#include <iostream>

//	=============================================================
//	JFileInputStream::JFileInputStream

JFileInputStream::JFileInputStream():
hFile_( 0 )
{}

//	=============================================================
//	JFileInputStream::JFileInputStream

JFileInputStream::JFileInputStream(const QFile& file) :
hFile_( 0 )
{
	open( QFileInfo( file ).absoluteFilePath() );
}

//	=============================================================
//	JFileInputStream::JFileInputStream

JFileInputStream::JFileInputStream(const QString& name) :
hFile_( 0 )
{
	open( name );
}

//	=============================================================
//	JFileInputStream::~JFileInputStream

JFileInputStream::~JFileInputStream()
{
	close();
}

//	=============================================================
//	JFileInputStream::size

gint64 JFileInputStream::size()
{
#ifdef WIN32
	DWORD dwHigh = 0;
	DWORD dwLow = ::GetFileSize( hFile_, &dwHigh );

	gint64 retSize = ( gint64 ) dwLow | ( gint64 ) dwHigh << 32;
	return retSize;
#else
	return File_.size();
#endif // WIN32
}

//	=============================================================
//	JFileInputStream::atEnd

bool JFileInputStream::atEnd() const
{
#ifdef WIN32
	DWORD dwHigh = 0;
	DWORD dwLow = ::GetFileSize( hFile_, &dwHigh );

	gint64 fileSize = ( gint64 ) dwLow | ( gint64 ) dwHigh << 32;

	return ::SetFilePointer( hFile_, 0, 0, FILE_CURRENT ) == fileSize;
#else
	return File_.atEnd();
#endif // WIN32
}

//	=============================================================
//	JFileInputStream::close

void JFileInputStream::close()
{
#ifdef WIN32
	if ( hFile_ )
	{
		CloseHandle( hFile_ );
		hFile_ = 0;
	}
#else
	File_.close();
#endif // WIN32
}

//	=============================================================
//	JFileInputStream::read

gint64 JFileInputStream::read(char* pBuffer, gint64 bufLen)
{
	//std::cout << "Read: " << bufLen << std::endl;
	if(bufLen > LONG_MAX)
	{
		bufLen = LONG_MAX;
	}

#ifdef WIN32
	DWORD bytesRead = 0;
	const BOOL bSuccess = ::ReadFile( hFile_, pBuffer, bufLen, &bytesRead, 0 );

	if ( !bSuccess )
	{
		return -1;
	}

	return bytesRead;
#else
	if ( !File_.isOpen() ) return -1;

	gint64 bytesRead = File_.read( pBuffer, bufLen );

	if ( bytesRead == 0 )
	{
		return EndOfFile;
	}

	return bytesRead;

#endif // WIN32
}

//	=============================================================
//	JFileInputStream::readLine

gint64 JFileInputStream::readLine( char*, gint64 )
{
	return 0;
}

//	=============================================================
//	JFileInputStream::readLine

QByteArray JFileInputStream::readLine()
{
	return QByteArray();
}

//	=============================================================
//	JFileInputStream::seek

bool JFileInputStream::seek( gint64 position )
{
#ifdef WIN32
	DWORD dwPtr = ::SetFilePointer( hFile_, position, 0, FILE_BEGIN );
	// Error
	if ( INVALID_SET_FILE_POINTER == dwPtr )
	//if ( dwPtr == 0xFFFFFFFF )
	{
		return false;
	}
	return true;
#else
	return File_.seek( position );
#endif // WIN32
}

//	=============================================================
//	JFileInputStream::pos

gint64 JFileInputStream::pos() const
{
#ifdef WIN32
	return ::SetFilePointer( hFile_, 0, 0, FILE_CURRENT );
#else
	return File_.pos();
#endif // WIN32
}

//	=============================================================
//	JFileInputStream::reset

void JFileInputStream::reset()
{
#ifdef WIN32
	seek( 0 );
#else
	File_.reset();
#endif // WIN32
}

//	=============================================================
//	JFileInputStream::lastModified

time_t JFileInputStream::lastModified() const
{
	FILETIME ftCreate, ftAccess, ftWrite;

	// Retrieve the file times for the file.
	if ( !::GetFileTime( hFile_, &ftCreate, &ftAccess, &ftWrite ) )
	{
		return 0;
	}

	return JUtils::filetimeToUnixTime( ( int* ) &ftWrite );
}

//	=============================================================
//	JFileInputStream::open

bool JFileInputStream::open(const QString& fileName) 
{
	if( fileName.isEmpty() )
	{
		return false;
	}
	else
	{
#ifdef WIN32
		if ( GetFileAttributesW( reinterpret_cast<const wchar_t *>(fileName.utf16()) ) & FILE_ATTRIBUTE_DIRECTORY )
		{
			return false;
		}
#endif // WIN32
		//QFileInfo fi( fileName );
		//if ( fi.isDir() )
		//{
		//	return false;
		//}
	}

#ifdef WIN32

	DWORD dwDesiredAccess = 0;
	DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
	DWORD dwCreationDisposition = 0;
	DWORD dwFlagsAndAttributes = FILE_FLAG_RANDOM_ACCESS;

	dwDesiredAccess |= GENERIC_READ;
	dwCreationDisposition = OPEN_EXISTING;
	HANDLE hTemplateFile = 0;

	// Open a file
	hFile_ = ::CreateFileW( ( LPCWSTR ) fileName.utf16(), dwDesiredAccess, dwShareMode, 
		lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );

	if ( INVALID_HANDLE_VALUE == hFile_ )
	{
		return false;
	}

	return true;

#else
	File_.setFileName( fileName );
	return File_.open( QIODevice::ReadOnly );
#endif // WIN32
}

