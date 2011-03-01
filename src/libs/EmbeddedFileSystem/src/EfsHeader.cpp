//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "EfsHeader.h"

//	===================================================================
//	EfsHeader::EfsHeader

EfsHeader::EfsHeader( EfsFile *fs )
:	fs_( fs )
{}

//	===================================================================
//	EfsHeader::EfsHeader

EfsHeader::EfsHeader( EfsFile *fs, const EfsOverhead &info )
:	fs_( fs )
{
	initBlockHeader( info );
}

//	===================================================================
//	EfsHeader::version

void EfsHeader::version( int ver )
{
	info()->version = ver;
}

//	===================================================================
//	EfsHeader::version

int EfsHeader::version()
{
	return info()->version;
}

//	===================================================================
//	EfsHeader::writeHeader

void EfsHeader::writeHeader()
{
	fs_->seek( 0 );
	fs_->write( serviceBlock_.data(), overhead()->blockSize );
}

//	===================================================================
//	EfsHeader::readHeader

bool EfsHeader::readHeader()
{
	// Read service info
	fs_->seek( 0 );
	const int serviceSize = EfsServiceBlockSize + EfsOverheadSize;
	char serviceBuf[ serviceSize ];

	if ( serviceSize != fs_->read( serviceBuf, serviceSize ) )
	{
		return false;
	}

	// Init buffer
	EfsOverhead ohead;
	::memcpy( &ohead, ( EfsOverhead* )( serviceBuf + EfsServiceBlockSize ), sizeof( EfsOverhead ) );
	initBlockHeader( ohead );

	// Copy service data
	::memcpy( serviceBlock_.data(), serviceBuf, serviceSize );

	if ( ohead.blockSize - serviceSize != fs_->read( serviceBlock_.data() + 
		serviceSize, ohead.blockSize - serviceSize ) )
	{
		return false;
	}

	return true;
}

//	===================================================================
//	EfsHeader::setSignature

void EfsHeader::setSignature( char *sig, int sigSize )
{
	::memcpy( serviceBlock_.data(), sig, sigSize );
}

//	===================================================================
//	EfsHeader::isTrueSignature

bool EfsHeader::isTrueSignature(  char *sig, int sigSize  )
{
	return !::memcmp( serviceBlock_.data(), sig, sigSize );
}

//	===================================================================
//	EfsHeader::setMounted

void EfsHeader::setMounted( bool state )
{
	info()->mountedState = ( int ) state;
}

//	===================================================================
//	EfsHeader::mounted

bool EfsHeader::mounted()
{
	return ( bool ) info()->mountedState;
}

//	===================================================================
//	EfsHeader::initBlockHeader

bool EfsHeader::initBlockHeader( const EfsOverhead &info )
{
	// Block must have enough space to contain all service information
	if ( info.blockSize < EfsFISize + EfsFITServiceSize + sizeof( EfsFragment )
		|| info.blockSize < EfsServiceBlockSize + EfsOverheadSize )
	{
		return false;
	}

	serviceBlock_.resize( info.blockSize );
	::memset( serviceBlock_.data(), 0, info.blockSize );
	::memcpy( serviceBlock_.data() + EfsServiceBlockSize, &info, EfsOverheadSize );

	return true;
}

//	===================================================================
//	EfsHeader::overhead

EfsOverhead* EfsHeader::overhead() const
{
	return ( EfsOverhead* )( serviceBlock_.data() + EfsServiceBlockSize );
}

//	===================================================================
//	EfsHeader::info

EfsServiceBlock* EfsHeader::info() const
{
	return ( EfsServiceBlock* ) serviceBlock_.data();
}
