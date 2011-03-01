//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "EfsStream.h"

//	===================================================================
//	EfsStream::EfsStream

EfsStream::EfsStream() :
efs_( 0 ),
fd_( 0 )
{}

//	===================================================================
//	EfsStream::EfsStream

EfsStream::EfsStream( Efs *afs ) :
efs_( 0 ),
fd_( 0 )
{
	setFS( afs );
}

//	===================================================================
//	EfsStream::~EfsStream

EfsStream::~EfsStream()
{
	close();
}

//	===================================================================
//	EfsStream::setFS

void EfsStream::setFS( Efs *afs )
{
	efs_ = afs;
}

//	===================================================================
//	EfsStream::open

bool EfsStream::open( const char *path, int access, int createDisp )
{
	fd_ = efs_->createFile( path, access, createDisp );
	return isOpen();
}

//	===================================================================
//	EfsStream::isOpen

bool EfsStream::isOpen()
{
	return fd_ != 0;
}

//	===================================================================
//	EfsStream::read

gint64 EfsStream::read( char* pBuffer, gint64 bufLen )
{
	if ( !efs_ ) return false;
	return ( gint64 ) efs_->readFile( fd_, pBuffer, bufLen );
}

//	============atEnd=======================================================
//	EfsStream::close

bool EfsStream::atEnd() const
{
	if ( !efs_ ) return false;
	return efs_->atEnd( fd_ );
}

//	===================================================================
//	EfsStream::close

void EfsStream::close()
{
	if ( efs_ && fd_ )
	{
		efs_->closeFile( fd_ );
		efs_ = 0;
		fd_ = 0;
	}
}

//	===================================================================
//	EfsStream::seek

bool EfsStream::seek( gint64 posi )
{
	if ( !efs_ ) return false;

	return ( bool ) efs_->seekFile( fd_, posi );
}

//	===================================================================
//	EfsStream::size

gint64 EfsStream::size()
{
	if ( !efs_ ) return false;
	return ( gint64 ) efs_->fileSize( fd_ );
}

//	===================================================================
//	EfsStream::pos

gint64 EfsStream::pos() const
{
	if ( !efs_ ) return false;
	return ( gint64 ) efs_->filePos( fd_ );
}

//	===================================================================
//	EfsStream::write

gint64 EfsStream::write( const char* pBuffer, gint64 bufLen )
{
	if ( !efs_ ) return false;
	return ( gint64 ) efs_->writeFile( fd_, pBuffer, bufLen );
}
