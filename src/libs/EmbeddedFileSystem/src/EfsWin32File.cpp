//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#ifdef WIN32
#include "EfsWin32File.h"
#include "shellapi.h"

#ifdef WIN32
// Use non deprecated function in VS 2005
#pragma warning(disable : 4996)
#endif // WIN32

//	=============================================================
//	EfsWin32File::EfsWin32File

EfsWin32File::EfsWin32File()
:	hFile_( 0 )
{}

//	=============================================================
//	EfsWin32File::EfsWin32File

EfsWin32File::~EfsWin32File()
{
	if ( hFile_ ) close();
}

//	=============================================================
//	EfsWin32File::open

bool EfsWin32File::open( const wchar_t * filename )
{
	DWORD dwDesiredAccess = 0;
	// Share access is not accepted
	//DWORD dwShareMode = 0;//FILE_SHARE_READ | FILE_SHARE_WRITE;
	DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
	DWORD dwCreationDisposition = 0;
	DWORD dwFlagsAndAttributes = FILE_FLAG_RANDOM_ACCESS;

	dwDesiredAccess |= GENERIC_READ | GENERIC_WRITE;
	dwCreationDisposition = OPEN_ALWAYS;

	// Open a file
	hFile_ = ::CreateFileW( ( LPCWSTR ) filename, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
	                            dwCreationDisposition, dwFlagsAndAttributes, 0 );

	if ( hFile_ == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	return true;
}

//	=============================================================
//	EfsWin32File::read

int EfsWin32File::read( char *buffer, int size )
{
	DWORD bytesRead = 0;
	const BOOL bSuccess = ::ReadFile( hFile_, buffer, size, &bytesRead, 0 );

	if ( !bSuccess )
	{
		return -1;
	}

	return bytesRead;
}

//	=============================================================
//	EfsWin32File::write

int EfsWin32File::write( const char *buffer, int size )
{
	DWORD bytesWritten = 0;
	const BOOL bSuccess = ::WriteFile( hFile_, buffer, size, &bytesWritten, 0 );

	if ( !bSuccess )
	{
		return -1;
	}

	return bytesWritten;
}

//	=============================================================
//	EfsWin32File::close

void EfsWin32File::close()
{
	::CloseHandle( hFile_ );
	hFile_ = 0;
}

//	=============================================================
//	EfsWin32File::seek

gint64 EfsWin32File::seek( gint64 size )
{
	LARGE_INTEGER li;

	li.QuadPart = size;
	li.LowPart = ::SetFilePointer( hFile_, li.LowPart, &li.HighPart, FILE_BEGIN );

	if ( INVALID_SET_FILE_POINTER == li.LowPart && GetLastError() != NO_ERROR )
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}

//	=============================================================
//	EfsWin32File::pos

gint64 EfsWin32File::pos()
{
	LARGE_INTEGER position;

	position.QuadPart = 0;
	position.LowPart = ::SetFilePointer( hFile_, 0, &position.HighPart, FILE_CURRENT );

	if ( 0xFFFFFFFF == position.LowPart && ::GetLastError() != NO_ERROR )
	{
		position.QuadPart = 0;
	}

	return position.QuadPart;
}

//	=============================================================
//	EfsWin32File::resize

bool EfsWin32File::resize( gint64 newSize )
{
	if ( seek( newSize ) == -1 || !::SetEndOfFile( hFile_ ) )
	{
		return false;
	}

	return true;
}

//	=============================================================
//	EfsWin32File::size

gint64 EfsWin32File::size()
{
	DWORD dwHigh = 0;
	DWORD dwLow = ::GetFileSize( hFile_, &dwHigh );

	gint64 retSize = ( gint64 ) dwLow | ( gint64 ) dwHigh << 32;
	return retSize;
}

//	=============================================================
//	EfsWin32File::remove

void EfsWin32File::remove( const wchar_t * path )
{
	int length = wcslen( path );
    wchar_t * from = new wchar_t [ length + 2 ];
	wcscpy( from, path );
	from[ length + 0 ] = 0;
	from[ length + 1 ] = 0;

	SHFILEOPSTRUCTW FileOp;
	FileOp.hwnd = 0;
	FileOp.wFunc = FO_DELETE;
	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
	FileOp.pFrom = from;
	FileOp.pTo = 0;

	SHFileOperation( &FileOp );
	delete [] from;
}

//	=============================================================
//	EfsWin32File::rename

void EfsWin32File::rename( const wchar_t * fromp, const wchar_t * top )
{
	int lengthFrom = wcslen( fromp );
	int lengthTo = wcslen( top );
    wchar_t *from = new wchar_t [ lengthFrom + 2 ];
    wchar_t *to = new wchar_t [ lengthTo + 2 ];

	wcscpy( from, fromp );
	wcscpy( to, top );

	from[ lengthFrom + 0 ] = 0;
	from[ lengthFrom + 1 ] = 0;
	to[ lengthTo + 0 ] = 0;
	to[ lengthTo + 1 ] = 0;

	SHFILEOPSTRUCTW FileOp;
	FileOp.hwnd = 0;
	FileOp.wFunc = FO_MOVE;
	FileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
	FileOp.pFrom = from;
	FileOp.pTo = to;

	SHFileOperation( &FileOp );

	delete [] from;
	delete [] to;
}

//	=============================================================
//	EfsWin32File::isEmpty

bool EfsWin32File::isEmpty()
{
	return size() == 0;
}

#endif // WIN32

