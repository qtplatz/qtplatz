//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "EfsDefs.h"
#include "time.h"
#include "string.h"

//	=====================================================================
//	EfsFit::EfsFit

EfsFit::EfsFit() : 
refs_( 0 ),
afsBlockSize_( EfsDefaultBlockSize ),
blocksData_( 0 )
{}

//	=====================================================================
//	EfsFit::EfsFit

EfsFit::EfsFit( const EfsFit &rhv ) :
refs_( 0 ),
afsBlockSize_( EfsDefaultBlockSize ),
blocksData_( 0 )
{
	copy( rhv );
}

//	=====================================================================
//	EfsFit::EfsFit

EfsFit::EfsFit( gint64 afsBlockSize ) :
refs_( 0 ),
afsBlockSize_( afsBlockSize ),
blocksData_( 0 )
{
	resize( 1 );
	serviceInfo()->fitSize = 1;
}

//	=====================================================================
//	EfsFit::~EfsFit

EfsFit::~EfsFit()
{
	if ( blocksData_ )
	{
		delete [] blocksData_;
		blocksData_ = 0;
	}
}

//	=====================================================================
//	EfsFit::makeNew

void EfsFit::makeNew( int attributes )
{
	EfsFileInfo fi;
	fi.attributes = attributes;
	fi.fileSize = 0;
	fi.crc = 0;
	::memset( fi.encryption, 0, 56 );
	time_t creationTime = time( 0 );
	fi.creationTime = ( int ) creationTime;
	fi.lastChangeTime = ( int ) creationTime;
	fi.lastAccessTime = ( int ) creationTime;

	::memcpy( blocksData_ + EfsFITServiceSize, ( char* ) &fi, EfsFISize );
	serviceInfo()->fitSize = 1;
}

//	=====================================================================
//	EfsFit::fit

inline const char* EfsFit::fit() const
{
	return blocksData_;
}

//	=====================================================================
//	EfsFit::frags

inline const char* EfsFit::frags() const
{
	return fit() + EfsFITServiceSize + EfsFISize;
}

//	=====================================================================
//	EfsFit::fileInfo

inline EfsFileInfo* EfsFit::fileInfo() const
{
	return ( EfsFileInfo* )( fit() + EfsFITServiceSize );
}

//	=====================================================================
//	EfsFit::serviceInfo

inline EfsFITServiceInfo* EfsFit::serviceInfo() const
{
	return ( EfsFITServiceInfo* ) fit();
}

//	=====================================================================
//	EfsFit::getCurrentBlockAddr

void EfsFit::getCurrentBlockAddr( gint64 offsetPtr, EfsBlockIterator &iter ) const
{
	gint64 blockIndex = offsetPtr/afsBlockSize_;
	int fragsCnt = serviceInfo()->fragmentsCount;

	if ( !iter.blockAddr )
	{
		// First iteration
		iter.blockCount = 0;
		iter.fragIndex = 0;
	}
	else
	{
		if ( iter.blockCount > blockIndex )
		{
			iter.blockAddr = iter.fragInfo.addr + 
				( blockIndex - iter.blockCount + iter.fragInfo.count );
			return;
		}

		iter.blockAddr = serviceInfo()->addr;
	}

	// Traverse all fragments
	for ( ; iter.fragIndex < fragsCnt; iter.fragIndex++ )
	{
		::memcpy( &iter.fragInfo, frags() + iter.fragIndex*EfsFragmentSize, 
			EfsFragmentSize );

		iter.blockCount += iter.fragInfo.count;

		if ( iter.blockCount > blockIndex )
		{
			iter.blockAddr = iter.fragInfo.addr + 
				( blockIndex - iter.blockCount + iter.fragInfo.count );
			iter.fragIndex++;
			break;
		}
	}
}

//	=====================================================================
//	EfsFit::addFragment

void EfsFit::addFragment( int blockPtr, int blockSize )
{
	EfsFragment fragInfo;
	int fragsCnt = serviceInfo()->fragmentsCount;

	// Check for space
	if ( ( int )( ( fragsCnt + 1 )*EfsFragmentSize + EfsFISize +
		EfsFITServiceSize ) > serviceInfo()->fitSize*afsBlockSize_ )
	{
		resize( serviceInfo()->fitSize + 1 );
		serviceInfo()->fitSize++;
	}

	// Try to add fragment to the end
	lastFragment( fragInfo );
	if ( fragInfo.addr + fragInfo.count == blockPtr )
	{
		// Merge with last fragment
		fragInfo.count += blockSize;
		serviceInfo()->allBlocks += blockSize;
		::memcpy( ( void* )( frags() + ( serviceInfo()->fragmentsCount - 1 )*EfsFragmentSize ), 
			( char* ) &fragInfo, EfsFragmentSize );
	}
	else
	{
		// Add new fragment
		fragInfo.addr = blockPtr;
		fragInfo.count = blockSize;
		::memcpy( ( void* )( frags() + fragsCnt*EfsFragmentSize ), 
			&fragInfo, EfsFragmentSize );
		fragsCnt++;
		serviceInfo()->fragmentsCount = fragsCnt;
		serviceInfo()->allBlocks += fragInfo.count;
	}
}

//	=====================================================================
//	EfsFit::setServiceInfo

void EfsFit::setServiceInfo( const EfsFITServiceInfo &si )
{
	resize( si.fitSize );
	::memset( blocksData_, 0, si.fitSize*afsBlockSize_ );
	::memcpy( blocksData_, &si, EfsFITServiceSize );
}

//	=====================================================================
//	EfsFit::size

int EfsFit::size() const
{
	return serviceInfo()->fitSize*afsBlockSize_;
}

//	=====================================================================
//	EfsFit::lastFragment

void EfsFit::lastFragment( EfsFragment &fragInfo ) const
{
	int fragsCnt = serviceInfo()->fragmentsCount;
	if ( !fragsCnt )
	{
		return;
	}
	::memcpy( &fragInfo, frags() + 
		( fragsCnt - 1 )*EfsFragmentSize, EfsFragmentSize );
}

//	=====================================================================
//	EfsFit::setLastFragment

void EfsFit::setLastFragment( const EfsFragment &fragInfo )
{
	int fragsCnt = serviceInfo()->fragmentsCount;
	if ( !fragsCnt )
	{
		return;
	}
	::memcpy( ( void* )( frags() + ( fragsCnt - 1 )*EfsFragmentSize ), 
		&fragInfo, EfsFragmentSize );
}

//	=====================================================================
//	EfsFit::setFilename

void EfsFit::setFilename( const char *name )
{
	strcpy( fileInfo()->fileName, name );
}

//	=====================================================================
//	EfsFit::touch

void EfsFit::touch()
{
	int curTime = ( int ) time( 0 );
	fileInfo()->lastAccessTime = curTime;
}

//	=====================================================================
//	EfsFit::changed

void EfsFit::changed()
{
	int curTime = ( int ) time( 0 );
	fileInfo()->lastAccessTime = curTime;
	fileInfo()->lastChangeTime = curTime;
}

//	=====================================================================
//	EfsFit::isEnoughSpace

bool EfsFit::isEnoughSpace( int size, gint64 offsetPtr ) const
{
	return ( offsetPtr + size ) < ( afsBlockSize_ - EfsFISize - EfsFITServiceSize );
}

//	=====================================================================
//	EfsFit::fragsSpaceSize

int EfsFit::fragsSpaceSize() const
{
	return afsBlockSize_*serviceInfo()->fitSize - EfsFISize - EfsFITServiceSize;
}

//	=====================================================================
//	EfsFit::write

void EfsFit::write( const char *buf, int size, gint64 &offsetPtr )
{
	::memcpy( ( void* )( frags() + offsetPtr ), buf, size );
	offsetPtr += size;
	if ( fileInfo()->fileSize < offsetPtr )
	{
		fileInfo()->fileSize = offsetPtr;
	}
}

//	=====================================================================
//	EfsFit::read

int EfsFit::read( char *buf, int size, gint64 &offsetPtr )
{
	int readSize = size < ( fileInfo()->fileSize - offsetPtr ) ?
					size : ( fileInfo()->fileSize - offsetPtr );

	::memcpy( buf, fit() + EfsFISize + EfsFITServiceSize + offsetPtr, readSize );
	offsetPtr += readSize;
	return readSize;
}

//	=====================================================================
//	EfsFit::copy

void EfsFit::copy( const EfsFit &entry )
{
	refs_ = entry.refs_;
	if ( entry.blocksData_ )
	{
		resize( entry.serviceInfo()->fitSize );
		::memcpy( blocksData_, entry.blocksData_, entry.serviceInfo()->fitSize*afsBlockSize_ );
	}
	else
	{
		resize( 0 );
	}
}

//	=====================================================================
//	EfsFit::resize

void EfsFit::resize( int fitSize )
{
	if ( !fitSize )
	{
		if ( blocksData_ )
		{
			delete [] blocksData_;
			blocksData_ = 0;
		}
		return;
	}

	char *newBlocks = new char[ fitSize*afsBlockSize_ + fitSize ];
	::memset( newBlocks, 0, fitSize*afsBlockSize_  );

	if ( blocksData_ )
	{
		int copySize = serviceInfo()->fitSize > fitSize ? fitSize : serviceInfo()->fitSize;
		::memcpy( newBlocks, blocksData_, copySize*afsBlockSize_ );
		delete [] blocksData_;
	}

	blocksData_ = newBlocks;
}

//	=====================================================================
//	EfsFit::operator=

EfsFit& EfsFit::operator=( const EfsFit &rhv )
{
	copy( rhv );
	return *this;
}
