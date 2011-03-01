//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////

#include "Efs.h"
#include "EfsHeader.h"
#include <algorithm>
#include <QFileInfo>
#if AFS_DEBUG
#include <QFile>
#include <QTextStream>
#endif // AFS_DEBUG

typedef BTreeContainerStd< EfsNameKey, int, std::vector, BTreeKey > BTreeHashContainer;
typedef BTreeContainerStd< EfsNameKey, int, std::vector, BTreeData > BTreeHashContainerData;
typedef BTreeContainerStd< EfsNameKey, int, std::vector, BTreePair > BTreeHashContainerPair;
typedef BTreeContainerStd< int, int, std::vector, BTreeKey > BTreePtrsContainer;

//	=============================================================
//	Efs::Efs

Efs::Efs() :
AFS_MULTITHREAD_SUPPORT_INIT
fs_( 0 ),
lastErrorCode_( EfsNoError ),
handleCntr_( 7 ),
ver_( 0 ),
callback_( 0 ),
encryptor_( 0 ),
readBufAddr_( -1 )
{
	overhead_.allocationMethod = EfsDefaultAllocMethod;
	overhead_.blockSize = EfsDefaultBlockSize;
	overhead_.defaultFileBuffer = EfsDefaultFileBuffer;
	overhead_.growthFactor = EfsDefaultGrowthFactor;
	overhead_.minFreeBlockCount = EfsDefaultMinFreeBlockCount;

	encryptor_ = &EfsDefaultEncryptor;
}

//	=============================================================
//	Efs::~Efs

Efs::~Efs()
{
	umount();
}

//	=============================================================
//	Efs::mount

bool Efs::mount( const unsigned short *path )
{
	AFS_LOCK_THREAD;

	lastErrorCode_ = EfsNoError;
	if ( fs_ )
	{
		lastErrorCode_ = EfsInsufficientRights;
		return false;
	}

	path_ = QString::fromUtf16( path );
	fs_ = EfsFile::GetInstance();

	// Create or open file
	if ( !fs_->open( reinterpret_cast<const wchar_t *>( path_.utf16() ) ) )
	{
		// Cannot open or create file
		lastErrorCode_ = EfsFileOpenFailed;
		return false;
	}

	EfsHeader serviceHeader( fs_ );

	if ( fs_->isEmpty() )
	{
		// Add signature
		serviceHeader.initBlockHeader( overhead_ );
		serviceHeader.version( EfsCurrentVersion );
		serviceHeader.setSignature( ( char* ) &EfsSignature[ 0 ], EfsSignatureSize );
		serviceHeader.setMounted( true );
		ver_ = serviceHeader.version();
	}
	else
	{
		if ( !serviceHeader.readHeader() )
		{
			// Read header failed
			lastErrorCode_ = EfsCorrupted;
			return false;
		}

		ver_ = serviceHeader.version();
		setOverheadInfo( *serviceHeader.overhead() );

		// Test signature
		if ( !serviceHeader.isTrueSignature( ( char* ) &EfsSignature[ 0 ], EfsSignatureSize ) )
		{
			lastErrorCode_ = EfsCorrupted;
			return false;
		}

		// Check version
		if ( ver_ > EfsCurrentVersion )
		{
			lastErrorCode_ = EfsVersionNotSupported;
			return false;
		}

		if ( EfsDefaultBlockSize != overhead_.blockSize )
		{
			lastErrorCode_ = EfsBlockSizeNotSupported;
			return false;
		}

		if ( serviceHeader.mounted() )
		{
			// FS did not unmounted successfully
			verifyFSStructure();
		}
		else
		{
			serviceHeader.setMounted( true );
		}
	}

	serviceHeader.writeHeader();

	bool firstAllocation = serviceHeader.info()->freesRootAddr == 0;
	EfsAllocMethod saveAllocMethod = overhead_.allocationMethod;
	overhead_.allocationMethod = EfsAllocLinear;

	// Firstly add frees
	if ( !freePtrs_.setFS( this, EfsFreesRootOffset, overhead_.blockSize ) )
	{
		umount();
		return false;
	}

	if ( firstAllocation )
	{
		// Synchronize frees
		FreePtrsMap::iterator iter = freeMemPtrs_.begin();
		for ( ; iter != freeMemPtrs_.end(); iter++ )
		{
			freePtrs_.add( iter->first, iter->second );
		}
	}

	if ( !fits_.setFS( this, EfsFitsRootOffset, overhead_.blockSize ) )
	{
		umount();
		return false;
	}

	// Restore allocation method
	overhead_.allocationMethod = saveAllocMethod;

	if ( !firstAllocation )
	{
		// Fill freeSizes and freeMemPtrs
		BTreeContainerStd< int, int, std::vector, BTreePair > allKeys;
		if ( freePtrs_.getAll( allKeys ) )
		{
			BTreeContainerStd< int, int, std::vector, BTreePair >::const_iterator 
				freeSizeIter = allKeys.begin();
			for ( ; freeSizeIter != allKeys.end(); freeSizeIter++ )
			{
				freeSizes_.insert( FreeSizesMap::value_type( freeSizeIter->data, 
					freeSizeIter->key ) );
				freeMemPtrs_[ freeSizeIter->key ] = freeSizeIter->data;

				// Build free amount
				freePtrs_.freeAmount_ += freeSizeIter->data;
			}
		}
	}

	return true;
}

//	=============================================================
//	Efs::umount

void Efs::umount()
{
	AFS_LOCK_THREAD;

	if ( fs_ )
	{
		while ( !handles_.empty() )
		{
			closeFile( handles_.begin()->first );
		}

		handles_.clear();
		fitCache_.clear();

		EfsHeader serviceHeader( fs_ );
		serviceHeader.readHeader();
		serviceHeader.setMounted( false );
		serviceHeader.writeHeader();

		fs_->close();
		fits_.close();
		freeSizes_.clear();
		freeMemPtrs_.clear();
		freePtrs_.close();
		delete fs_;
		fs_ = 0;
		lastErrorCode_ = EfsNoError;
	}
}

//	=============================================================
//	Efs::createFile

int Efs::createFile( const char *name, int access, int createDisposition )
{
	AFS_LOCK_THREAD;

	if ( !fs_ )
	{
		return 0;
	}

	lastErrorCode_ = EfsNoError;
	int blockAddr = 0;
	EfsFit openFit( overhead_.blockSize );

	if ( !( blockAddr = findFile( name ) ) )
	{
		// Check disposition
		if ( createDisposition != EfsCreateNew &&
			createDisposition != EfsCreateAlways &&
			createDisposition != EfsOpenAlways )
		{
			lastErrorCode_ = EfsInsufficientRights;
			return 0;
		}

		EfsFragmentList retList;
		if ( !smartAllocate( 0, 1, overhead_.allocationMethod, retList ) )
		{
			lastErrorCode_ = EfsSpaceAllocationFailed;
			return 0;
		}

		EfsFragmentList::const_iterator fragListIter = retList.begin();
		blockAddr = fragListIter->addr;

		openFit.makeNew( EfsNormal );
		EfsNameKey nameKey( name );
		fits_.add( nameKey, blockAddr );
		openFit.setFilename( name );

		if ( !writeFit( blockAddr, openFit ) )
		{
			lastErrorCode_ = EfsCorrupted;
			return 0;
		}
	}
	else
	{
		switch ( createDisposition )
		{
		case EfsOpenExisting:
		case EfsOpenAlways:
			if ( !readFit( blockAddr, openFit ) )
			{
				lastErrorCode_ = EfsCorrupted;
				return 0;
			}
			break;
		case EfsOpenTruncate:
		case EfsCreateAlways:

			// Check rights
			if ( !( access | EfsWriteAccess ) )
			{
				lastErrorCode_ = EfsInsufficientRights;
				return 0;
			}

			if ( !readFit( blockAddr, openFit ) )
			{
				lastErrorCode_ = EfsCorrupted;
				return 0;
			}
			break;
		default:
			lastErrorCode_ = EfsUnsupportedParam;
			return 0;
		}

		openFit.touch();
	}

	// Set properties
	EfsFitCache::iterator iter = fitCache_.find( blockAddr );
	if ( iter != fitCache_.end() )
	{
		iter->second.refs_++;
	}
	else
	{
		openFit.refs_++;
		fitCache_[ blockAddr ] = openFit;
	}

	// Add handle
	int fd = 0;
	handleCntr_++;
	fd = handleCntr_;
	EfsHandle handle;
	handle.access = access;
	handle.fileBlockAddr = blockAddr;
	handle.offsetPtr = 0;
	handles_[ fd ] = handle;

	if ( createDisposition == EfsOpenTruncate || 
		createDisposition == EfsCreateAlways )
	{
		resize( blockAddr, 0 );
	}

	return fd;
}

//	=============================================================
//	Efs::deleteFile

bool Efs::deleteFile( const char *name )
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	int blockAddr = 0;
	EfsFit deleteEntry( overhead_.blockSize );

	if ( !( blockAddr = findFile( name ) ) )
	{
		lastErrorCode_ = EfsFileNotFound;
		return false;
	}

	EfsFitCache::iterator iter = fitCache_.find( blockAddr );

	if ( fitCache_.end() != iter )
	{
		// Can't delete opened file
		lastErrorCode_ = EfsInsufficientRights;
		return false;
	}

	if ( !readFit( blockAddr, deleteEntry ) )
	{
		// Can't delete opened file
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	if ( 0 != deleteEntry.serviceInfo()->fragmentsCount )
	{
		// Delete all fragments
		int fragsCnt = deleteEntry.serviceInfo()->fragmentsCount;
		EfsFragment fragInfo;

		for ( int i = 0; i < fragsCnt; i++ )
		{
			::memcpy( &fragInfo, deleteEntry.frags() + 
				i*EfsFragmentSize, EfsFragmentSize );

			deallocateSpace( fragInfo.addr, fragInfo.count );
		}
	}

	// Delete FIT entry
	deallocateSpace( blockAddr, deleteEntry.serviceInfo()->fitSize );

	// Remove FIT entry
	EfsNameKey nameKey( name );
	fits_.remove( nameKey );
	return true;
}

//	=============================================================
//	Efs::seekFile

gint64 Efs::seekFile( int fd, gint64 newPos )
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	EfsHandleMap::iterator iter = handles_.find( fd );

	if ( handles_.end() == iter )
	{
		lastErrorCode_ = EfsFileNotFound;
		return -1;
	}

	iter->second.offsetPtr = newPos;
	return iter->second.offsetPtr;
}

//	=============================================================
//	Efs::filePos

gint64 Efs::filePos( int fd ) const
{
	AFS_LOCK_THREAD;

	lastErrorCode_ = EfsNoError;
	EfsHandleMap::const_iterator iter = handles_.find( fd );

	if ( handles_.end() == iter )
	{
		lastErrorCode_ = EfsFileNotFound;
		return -1;
	}

	return iter->second.offsetPtr;
}

//	=============================================================
//	Efs::fileSize

gint64 Efs::fileSize( int fd ) const
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	EfsHandleMap::const_iterator iter = handles_.find( fd );

	if ( iter == handles_.end() )
	{
		lastErrorCode_ = EfsFileNotFound;
		return 0;
	}

	EfsFitCache::const_iterator cacheIter = 
		fitCache_.find( iter->second.fileBlockAddr );

	if ( cacheIter == fitCache_.end() )
	{
		lastErrorCode_ = EfsCorrupted;
		return 0;
	}

	return cacheIter->second.fileInfo()->fileSize;
}

//	=============================================================
//	Efs::queryFileInfo

bool Efs::queryFileInfo( int fd, EfsFileInfo &fileInfo ) const
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	EfsHandleMap::const_iterator iter = handles_.find( fd );

	if ( iter == handles_.end() )
	{
		lastErrorCode_ = EfsFileNotFound;
		return false;
	}

	EfsFitCache::const_iterator cacheIter = 
		fitCache_.find( iter->second.fileBlockAddr );

	if ( cacheIter == fitCache_.end() )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	::memcpy( &fileInfo, cacheIter->second.fileInfo(), sizeof( EfsFileInfo ) );
	return true;
}

//	=============================================================
//	Efs::rename

bool Efs::rename( const char* oldName, const char *newName )
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;

	int blockAddr = 0;

	if ( !( blockAddr = findFile( oldName ) ) )
	{
		lastErrorCode_ = EfsFileNotFound;
		return false;
	}

	EfsFitCache::iterator iter = fitCache_.find( blockAddr );

	if ( fitCache_.end() != iter )
	{
		lastErrorCode_ = EfsInsufficientRights;
		return false;
	}

	EfsNameKey nameKey( oldName );
	fits_.remove( nameKey );
	nameKey.setName( newName );
	fits_.add( nameKey, blockAddr );

	EfsFit fitEntry( overhead_.blockSize );

	if ( !readFit( blockAddr, fitEntry ) )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	fitEntry.setFilename( newName );
	fitEntry.changed();
	writeFit( blockAddr, fitEntry );

	return true;
}

//	=============================================================
//	Efs::changeFileAttrs

bool Efs::changeFileAttrs( int fd, int newAttrs )
{
	AFS_LOCK_THREAD;

	lastErrorCode_ = EfsNoError;
	EfsHandleMap::iterator iter = handles_.find( fd );

	if ( handles_.end() == iter )
	{
		lastErrorCode_ = EfsUnsupportedParam;
		return false;
	}

	EfsFitCache::iterator cacheIter = 
		fitCache_.find( iter->second.fileBlockAddr );

	if ( cacheIter == fitCache_.end() )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	cacheIter->second.fileInfo()->attributes = newAttrs;
	cacheIter->second.changed();
	writeFit( cacheIter->second.serviceInfo()->addr, cacheIter->second );

	return true;
}

//	=============================================================
//	Efs::fileAttrs

int Efs::fileAttrs( int fd ) const
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	EfsHandleMap::const_iterator iter = handles_.find( fd );

	if ( handles_.end() == iter )
	{
		lastErrorCode_ = EfsFileNotFound;
		return 0;
	}

	EfsFitCache::const_iterator cacheIter = 
		fitCache_.find( iter->second.fileBlockAddr );

	if ( cacheIter == fitCache_.end() )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	return cacheIter->second.fileInfo()->attributes;
}

//	=============================================================
//	Efs::resize

bool Efs::resize( int fd, gint64 newSize )
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	EfsHandleMap::iterator iter = handles_.find( fd );

	if ( handles_.end() == iter )
	{
		lastErrorCode_ = EfsFileNotFound;
		return false;
	}

	if ( !( iter->second.access | EfsWriteAccess ) )
	{
		lastErrorCode_ = EfsInsufficientRights;
		return false;
	}

	EfsFitCache::iterator cacheIter = 
		fitCache_.find( iter->second.fileBlockAddr );

	if ( cacheIter == fitCache_.end() )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	// Make new size
	bool retMakeRoom = makeRoom( cacheIter, newSize );

	return retMakeRoom;
}

//	=============================================================
//	Efs::makeRoom

bool Efs::makeRoom( EfsFitCache::iterator &iter, gint64 newSize )
{
	lastErrorCode_ = EfsNoError;
	int nextAddr = iter->first + 1;

	if ( 0 == iter->second.serviceInfo()->fragmentsCount &&
		iter->second.fragsSpaceSize() < newSize )
	{
		// Make first fragment if newSize is greater
		const int reserveBlocks = 1;
		EfsFragmentList fragList;

		// Check for next addr fragment
		if ( !smartAllocate( nextAddr, reserveBlocks, 
			overhead_.allocationMethod, fragList ) )
		{
			return false;
		}

		// Carry old data
		writeBlock( fragList.begin()->addr, iter->second.frags(), 
			iter->second.fileInfo()->fileSize, 0, false );

		// Add fragment and prepare for future writing
		iter->second.addFragment( fragList.begin()->addr, reserveBlocks );
		writeFit( iter->second.serviceInfo()->addr, iter->second );
		iter->second.changed();
	}

	if ( iter->second.fragsSpaceSize() > newSize )
	{
		if ( iter->second.serviceInfo()->fragmentsCount > 0 )
		{
			// Delete all fragements and carry data
			// from first fragment to FIT
			EfsFit firstData( EfsDefaultBlockSize );

			int fragsCnt = iter->second.serviceInfo()->fragmentsCount;
			EfsFragment fragInfo;

			for ( int i = 0; i < fragsCnt; i++ )
			{
				::memcpy( &fragInfo, iter->second.frags() + 
					i*EfsFragmentSize, EfsFragmentSize );

				if ( 0 == i )
				{
					if ( !readBlock( fragInfo.addr, ( char* ) firstData.fit(), newSize, 0 ) )
					{
						return false;
					}
				}

				deallocateSpace( fragInfo.addr, fragInfo.count );
			}

			iter->second.serviceInfo()->allBlocks = 0;
			iter->second.serviceInfo()->fitSize = 1;
			iter->second.serviceInfo()->fragmentsCount = 0;

			// Write data from the first seg
			gint64 offsetPtr = 0;
			iter->second.fileInfo()->fileSize = 0;
			iter->second.write( firstData.fit(), newSize, offsetPtr );
			iter->second.changed();
			writeFit( iter->first, iter->second );
		}
		else
		{
			iter->second.fileInfo()->fileSize = newSize;
			iter->second.changed();
			if ( !writeFit( iter->first, iter->second ) )
			{
				return false;
			}
		}
	}
	else
	{
		int allBlocks = iter->second.serviceInfo()->allBlocks;
		int newAllBlocks = newSize/EfsDefaultBlockSize + 1;

		if ( newAllBlocks > allBlocks )
		{
			int remain = newAllBlocks - allBlocks;

			EfsFragment lastFrag;
			iter->second.lastFragment( lastFrag );
			nextAddr = lastFrag.addr + lastFrag.count;

			EfsFragmentList fragList;
			if ( !smartAllocate( nextAddr, remain, overhead_.allocationMethod, fragList ) )
			{
				return false;
			}

			EfsFragmentList::const_iterator fragIter = fragList.begin();

			for ( ; fragIter != fragList.end(); fragIter++ )
			{
				iter->second.addFragment( fragIter->addr, fragIter->count );
			}
			iter->second.changed();
		}
		else
		{
			int killBlocks = allBlocks - newAllBlocks;
			EfsFragment frag;

			while ( killBlocks > 0 )
			{
				// Decrease fragments
				iter->second.lastFragment( frag );
				int killSize = killBlocks < frag.count ? killBlocks :
					frag.count;

				if ( frag.count > killSize )
				{
					// Resize fragment
					deallocateSpace( frag.addr + frag.count - killSize, killSize );
					frag.count -= killSize;
					iter->second.setLastFragment( frag );
				}
				else
				{
					// Delete fragment
					deallocateSpace( frag.addr, frag.count );
					iter->second.serviceInfo()->fragmentsCount--;
				}

				killBlocks -= killSize;
			}
		}

		iter->second.fileInfo()->fileSize = newSize;
		if ( !writeFit( iter->second.serviceInfo()->addr, iter->second, true ) )
		{
			return false;
		}
	}

	return true;
}

//	=============================================================
//	Efs::ensureSpace
///
///	It tests fs's free space have enough size for new blocks. In case of fs 
/// does not have such space, growFS will be called. If space growth 
/// failed, returns false, true otherwise

bool Efs::ensureSpace( int blocks )
{
	lastErrorCode_ = EfsNoError;
	FreeSizesMap::iterator iter = freeSizes_.begin();
	int remain = blocks << 1;

	if ( freeSizes_.empty() || 
		freeSizes_.begin()->first < overhead_.minFreeBlockCount )
	{
		if ( !growFS() )
		{
			return false;
		}
		iter = freeSizes_.begin();
	}

	while ( remain > 0 )
	{
		if ( iter == freeSizes_.end() )
		{
			if ( !growFS() )
			{
				return false;
			}

			iter = freeSizes_.begin();
		}

		remain -= iter->first;
		iter++;
	}

	return true;
}

//	=============================================================
//	Efs::closeFile

void Efs::closeFile( int fd )
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	EfsHandleMap::iterator iter = handles_.find( fd );

	if ( handles_.end() != iter )
	{
		EfsFitCache::iterator cacheIter = 
			fitCache_.find( iter->second.fileBlockAddr );

		if ( cacheIter == fitCache_.end() )
		{
			lastErrorCode_ = EfsCorrupted;
			return;
		}

		handles_.erase( iter );
		cacheIter->second.refs_--;

		if ( cacheIter->second.refs_ <= 0 )
		{
			deallocateLiberalSpace( cacheIter );
			writeFit( cacheIter->first, cacheIter->second, true );
			fitCache_.erase( cacheIter );
		}
	}
	else
	{
		lastErrorCode_ = EfsFileNotFound;
	}
}

//	=============================================================
//	Efs::deallocateLiberalSpace

void Efs::deallocateLiberalSpace( EfsFitCache::iterator &iter )
{
	int fileSizeBlocks = ( iter->second.fileInfo()->fileSize / EfsDefaultBlockSize ) + 
		( ( iter->second.fileInfo()->fileSize % EfsDefaultBlockSize ) > 0 );
	int liberalSizeBlocks = iter->second.serviceInfo()->liberalSize;

	if ( liberalSizeBlocks > fileSizeBlocks )
	{
		// We have extra (liberal) blocks
		EfsFragment lastFragInfo;
		int fragsCnt = iter->second.serviceInfo()->fragmentsCount;
		int blockAmount = 0;
		int i = 0;

		// Get last fragment block addr
		for ( ; i < fragsCnt; i++ )
		{
			::memcpy( &lastFragInfo, iter->second.frags() + 
				i*EfsFragmentSize, EfsFragmentSize );

			blockAmount += lastFragInfo.count;
			if ( blockAmount > fileSizeBlocks )
			{
				break;
			}
		}

		// Deallocate all fragment greater then last
		EfsFragment fragInfo;
		int lastFrag = i;

		for ( i = lastFrag + 1; i < fragsCnt; i++ )
		{
			::memcpy( &fragInfo, iter->second.frags() + 
				i*EfsFragmentSize, EfsFragmentSize );

			iter->second.serviceInfo()->fragmentsCount--;
			iter->second.serviceInfo()->allBlocks -= fragInfo.count;
			iter->second.serviceInfo()->liberalSize -= fragInfo.count;

			deallocateSpace( fragInfo.addr, fragInfo.count );
		}

		// Deallocate space from last fragment
		int deallocateNumber = blockAmount - fileSizeBlocks;

		if ( deallocateNumber > 0 )
		{
			deallocateSpace( lastFragInfo.addr + lastFragInfo.count
				- deallocateNumber, deallocateNumber );

			// Set last fragment info
			lastFragInfo.count -= deallocateNumber;
			if ( 0 == lastFragInfo.count )
			{
				// Delete last fragment
				iter->second.serviceInfo()->fragmentsCount--;
			}
			else
			{
				::memcpy( ( void* )( iter->second.frags() + 
						( iter->second.serviceInfo()->fragmentsCount - 1 )*EfsFragmentSize ), 
						&lastFragInfo, EfsFragmentSize );
			}

			iter->second.serviceInfo()->allBlocks -= deallocateNumber;
			iter->second.serviceInfo()->liberalSize -= deallocateNumber;
		}
	}
}

//	=============================================================
//	Efs::listFiles

void Efs::listFiles( EfsFilenameList &retList )
{
	AFS_LOCK_THREAD;

	retList.clear();
	BTreeHashContainer allHashes;

	if ( fits_.getAll( allHashes ) )
	{
		BTreeHashContainer::iterator iter = allHashes.begin();
		retList.reserve( allHashes.size() );

		for ( ; iter != allHashes.end(); iter++ )
		{
			retList.push_back( QByteArray( iter->key.name_ ) );
		}
	}
}

//	=============================================================
//	Efs::allocateSpace

int Efs::allocateSpace( EfsFragment &chunk, const EfsFragment &need )
{
	lastErrorCode_ = EfsNoError;
	if ( chunk.count < need.count )
	{
		lastErrorCode_ = EfsSpaceAllocationFailed;
		return 0;
	}

	int newAddr = 0;
	int newSize = 0;

	if ( chunk.addr == need.addr )
	{
		// Remove free chunk from map
		freePtrs_.remove( chunk.addr );
		freeMemPtrs_.erase( chunk.addr );
		removeFromFreeSpaceMap( chunk );

		newSize = chunk.count - need.count;
		newAddr = chunk.addr + need.count;
	}
	else
	{
		// Resize free chunk
		newSize = chunk.count - ( chunk.addr + chunk.count - need.addr );

		freePtrs_.changeData( chunk.addr, newSize );
		FreePtrsMap::iterator iter = freeMemPtrs_.find( chunk.addr );
		if ( freeMemPtrs_.end() == iter ) return 0;
		iter->second = newSize;

		removeFromFreeSpaceMap( chunk );
		freeSizes_.insert( FreeSizesMap::value_type( newSize, chunk.addr ) );

		newAddr = need.addr + need.count;
		newSize = chunk.addr + chunk.count - newAddr;
	}

	if ( newSize > 0 )
	{
		// Add space after needed chunk
		freePtrs_.add( newAddr, newSize );
		freeMemPtrs_[ newAddr ] = newSize;
		freeSizes_.insert( FreeSizesMap::value_type( newSize, newAddr ) );
	}

	return need.addr;
}

//	=============================================================
//	Efs::removeFromFreeSpaceMap

void Efs::removeFromFreeSpaceMap( EfsFragment &chunk )
{
	// Remove from free sizes
	FreeSizesMap::iterator freeSizeIter = freeSizes_.find( chunk.count );
	while ( freeSizeIter->second != chunk.addr && 
		freeSizeIter->first == chunk.count )
	{
		freeSizeIter++;
	}

	if ( freeSizeIter != freeSizes_.end() )
	{
		freeSizes_.erase( freeSizeIter );
	}
}

//	=============================================================
//	Efs::isExists

bool Efs::isExists( const char *name ) const
{
	AFS_LOCK_THREAD;

	if ( !findFile( name ) )
	{
		return false;
	}

	return true;
}

//	=============================================================
//	Efs::findFile

int Efs::findFile( const char *name ) const
{
	int blockAddr = 0;
	EfsNameKey nameKey( name );

	if ( !fits_.find( nameKey, blockAddr ) )
	{
		return 0;
	}

	return blockAddr;
}

//	=============================================================
//	Efs::getMaxFreeChunk

bool Efs::getMaxFreeChunk( EfsFragment &chunk )
{
	if ( freeSizes_.empty() )
	{
		return false;
	}

	chunk.count = freeSizes_.begin()->first;
	chunk.addr = freeSizes_.begin()->second;
	return true;
}

//	=============================================================
//	Efs::getMinFreeChunk

bool Efs::getMinFreeChunk( EfsFragment &chunk, int size )
{
	if ( freeSizes_.empty() )
	{
		return false;
	}

	chunk.addr = 0;
	FreeSizesMap::iterator iter = freeSizes_.begin();

	for ( ; iter != freeSizes_.end(); iter++ )
	{
		if ( iter->first == size )
		{
			chunk.addr = iter->second;
			chunk.count = iter->first;
			return true;
		}
		else if ( iter->first < size )
		{
			return chunk.addr ? true : false;
		}

		chunk.addr = iter->second;
		chunk.count = iter->first;
	}
	return chunk.addr ? true : false;
}

//	=============================================================
//	Efs::growFS

bool Efs::growFS()
{
	gint64 growAmount = overhead_.growthFactor;
	if ( callback_ )
	{
		if ( !( growAmount = callback_->growSpace( growAmount ) ) )
		{
			if ( freePtrs_.freeAmount() == 0 )
			{
				return false;
			}
		}
	}

	if ( !fs_ )
	{
		return false;
	}

	gint64 lastBlock = fs_->size() / EfsDefaultBlockSize;
	gint64 growCount = growAmount / EfsDefaultBlockSize;

	if ( !fs_->resize( EfsDefaultBlockSize*( lastBlock + growCount ) ) )
	{
		lastErrorCode_ = EfsSpaceAllocationFailed;
		return false;
	}

	// Try to find last free piece for join
	bool bjoined = false;
	FreeSizesMap::iterator sziter = freeSizes_.begin();
	for ( ; sziter != freeSizes_.end(); sziter++ )
	{
		if ( sziter->first + sziter->second == lastBlock )
		{
			// Join!
			FreePtrsMap::iterator memiter = freeMemPtrs_.find( sziter->second );
			memiter->second += growCount;
			freePtrs_.changeData( memiter->first, memiter->second );

			freeSizes_.erase( sziter );
			freeSizes_.insert( FreeSizesMap::value_type( memiter->second,
				memiter->first ) );

			bjoined = true;
			break;
		}
	}

	if ( !bjoined )
	{
		freePtrs_.add( lastBlock, growCount );
		freeMemPtrs_[ lastBlock ] = growCount;
		freeSizes_.insert( FreeSizesMap::value_type( growCount, lastBlock ) );
	}

	return true;
}

//	=============================================================
//	Efs::writeFile

bool Efs::writeFile( int file, const char *buf, int bufSize )
{
	AFS_LOCK_THREAD;
	lastErrorCode_ = EfsNoError;
	if ( !bufSize )
	{
		lastErrorCode_ = EfsUnsupportedParam;
		return false;
	}

	EfsHandleMap::iterator handleIter = handles_.find( file );

	if ( handles_.end() == handleIter )
	{
		lastErrorCode_ = EfsFileNotFound;
		return false;
	}

	if ( !( handleIter->second.access | EfsWriteAccess ) )
	{
		lastErrorCode_ = EfsInsufficientRights;
		return false;
	}

	EfsFitCache::iterator iter = fitCache_.find( handleIter->second.fileBlockAddr );

	if ( fitCache_.end() == iter )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	int nextAddr = 0;
	int blockAddr = 0;

	if ( 0 == iter->second.serviceInfo()->fragmentsCount )
	{
		if ( iter->second.isEnoughSpace( bufSize, handleIter->second.offsetPtr ) )
		{
			// Write entirely in the FIT block
			iter->second.write( buf, bufSize, handleIter->second.offsetPtr );
			iter->second.changed();
			if ( !writeFit( iter->first, iter->second ) )
			{
				lastErrorCode_ = EfsCorrupted;
				return false;
			}
			return true;
		}
		else
		{
			// Make first fragment
			nextAddr = iter->first + 1;
			int allocBlockCount = 1;
			EfsFragmentList fragList;

			// Check for next addr fragment
			if ( !smartAllocate( nextAddr, allocBlockCount, 
				overhead_.allocationMethod, fragList ) )
			{
				return false;
			}

			EfsFragmentList::const_iterator fragIter = fragList.begin();
			blockAddr = fragIter->addr;

			// Carry old data
			writeBlock( blockAddr, iter->second.frags(),
				handleIter->second.offsetPtr, 0, false );

			// Add fragment and prepare for future writing
			iter->second.addFragment( blockAddr, allocBlockCount );
			iter->second.changed();
			writeFit( iter->second.serviceInfo()->addr, iter->second );

			nextAddr = fragIter->addr + fragIter->count;
		}
	}

	// Enlarge room
	gint64 allocBlockCount = ( handleIter->second.offsetPtr + bufSize )/EfsDefaultBlockSize + 1;
	gint64 remain = allocBlockCount - iter->second.serviceInfo()->allBlocks;

	if ( remain > 0 )
	{
		// Make liberal size
		remain += overhead_.defaultFileBuffer;
		iter->second.serviceInfo()->liberalSize = iter->second.serviceInfo()->allBlocks + remain;

		if ( !nextAddr )
		{
			EfsFragment fragInfo;
			iter->second.lastFragment( fragInfo );
			nextAddr = fragInfo.addr + fragInfo.count;
		}

		EfsFragmentList fragList;
		if ( !smartAllocate( nextAddr, remain, overhead_.allocationMethod, fragList ) )
		{
			return false;
		}

		EfsFragmentList::const_iterator fragIter = fragList.begin();
		for ( ; fragIter != fragList.end(); fragIter++ )
		{
			iter->second.addFragment( fragIter->addr, fragIter->count );
		}
		iter->second.changed();
	}

	int written = 0;
	EfsBlockIterator fragIter;

	// Write into fragments!
	while ( bufSize > 0 )
	{
		iter->second.getCurrentBlockAddr( handleIter->second.offsetPtr, fragIter );
		int curBlockOffset = handleIter->second.offsetPtr % EfsDefaultBlockSize;

		int curWriteSize = ( EfsDefaultBlockSize - curBlockOffset ) < bufSize ?
			( EfsDefaultBlockSize - curBlockOffset ) : bufSize;

		// Write block data
		if ( !writeBlock( fragIter.blockAddr, buf + written, curWriteSize, curBlockOffset, true ) )
		{
			lastErrorCode_ = EfsCorrupted;
			return false;
		}

		written += curWriteSize;
		handleIter->second.offsetPtr += curWriteSize;

		if ( handleIter->second.offsetPtr > iter->second.fileInfo()->fileSize )
		{
			iter->second.fileInfo()->fileSize = handleIter->second.offsetPtr;
		}

		bufSize -= curWriteSize;
	}

	iter->second.changed();
	if ( !writeFit( iter->second.serviceInfo()->addr, iter->second, true ) )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	return true;
}

//	=============================================================
//	Efs::smartAllocate

bool Efs::smartAllocate( int preferAddr, int needBlocks, 
						EfsAllocMethod allocMethod, EfsFragmentList &retList )
{
	lastErrorCode_ = EfsNoError;
	retList.clear();
	FreePtrsMap::iterator preferIter = freeMemPtrs_.find( preferAddr );

	int remain = needBlocks;
	int blockAddr = 0;
	int allocateSize = 0;
	EfsFragment freeChunk, needChunk;
	EfsFragment frag;

	// Ensure space
	if ( !ensureSpace( needBlocks ) )
	{
		return false;
	}

	while ( remain > 0 )
	{
		if ( preferIter == freeMemPtrs_.end() )
		{
			if ( EfsAllocService == allocMethod )
			{
				if ( !getMinFreeChunk( freeChunk, needBlocks ) )
				{
					lastErrorCode_ = EfsSpaceAllocationFailed;
					return false;
				}
			}
			else if ( !getMaxFreeChunk( freeChunk ) ) // Allocate chunk from largest free space
			{
				lastErrorCode_ = EfsSpaceAllocationFailed;
				return false;
			}

			if ( freeChunk.count < EfsMinChunkForDivide )
			{
				// Use linear method for small chunks
				allocMethod = EfsAllocLinear;
			}

			switch ( allocMethod )
			{
			case EfsAllocLinear:
			case EfsAllocService:
				needChunk.addr = freeChunk.addr;
				needChunk.count = freeChunk.count;
				break;
			case EfsAllocQuadratic:
				{
					int ahalf = ( freeChunk.count >> 1 );
					needChunk.addr = freeChunk.addr + ahalf;
					needChunk.count = freeChunk.count - ahalf;
				}
				break;
			}

			allocateSize = remain > needChunk.count ? needChunk.count : remain;
		}
		else
		{
			freeChunk.addr = preferIter->first;
			freeChunk.count = preferIter->second;

			allocateSize = remain > freeChunk.count ? freeChunk.count : remain;

			if ( remain - allocateSize > 0 )
			{
				// It will allocate next prefer chunk if we need this
				preferIter = freeMemPtrs_.find( freeChunk.addr + freeChunk.count );
			}

			needChunk.addr = freeChunk.addr;
		}

		needChunk.count = allocateSize;
		blockAddr = allocateSpace( freeChunk, needChunk );
		if ( !blockAddr )
		{
			lastErrorCode_ = EfsSpaceAllocationFailed;
			return false;
		}

		// Add fragment to the list
		frag.addr = blockAddr;
		frag.count = allocateSize;

		retList.push_back( frag );
		remain -= allocateSize;
	}

	return true;
}

//	=============================================================
//	Efs::readFile

int Efs::readFile( int fd, char *buf, int bufSize )
{
	AFS_LOCK_THREAD;

	lastErrorCode_ = EfsNoError;
	EfsHandleMap::iterator handleIter = handles_.find( fd );

	if ( handles_.end() == handleIter )
	{
		lastErrorCode_ = EfsFileNotFound;
		return false;
	}

	if ( !( handleIter->second.access | EfsReadAccess ) )
	{
		lastErrorCode_ = EfsInsufficientRights;
		return false;
	}

	EfsFitCache::iterator iter = fitCache_.find( handleIter->second.fileBlockAddr );

	if ( fitCache_.end() == iter )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	if ( 0 == iter->second.serviceInfo()->fragmentsCount )
	{
		// Read from the FIT
		iter->second.touch();
		int read = iter->second.read( buf, bufSize, handleIter->second.offsetPtr );
		return read;
	}

	// Read from fragments
	int read = 0;
	gint64 fileSize = iter->second.fileInfo()->fileSize;
	EfsBlockIterator fragIter;

	while ( read != bufSize && handleIter->second.offsetPtr < fileSize )
	{
		iter->second.getCurrentBlockAddr( handleIter->second.offsetPtr, fragIter );
		int curBlockOffset = handleIter->second.offsetPtr % EfsDefaultBlockSize;

		int curReadSize = ( EfsDefaultBlockSize - curBlockOffset ) < ( bufSize - read ) ?
			( EfsDefaultBlockSize - curBlockOffset ) : ( bufSize - read );

		if ( handleIter->second.offsetPtr + curReadSize > fileSize )
		{
			curReadSize = fileSize - handleIter->second.offsetPtr;
		}

		// Read data
		if ( !readBlock( fragIter.blockAddr, buf + read, curReadSize, curBlockOffset ) )
		{
			lastErrorCode_ = EfsCorrupted;
			return read;
		}

		read += curReadSize;
		handleIter->second.offsetPtr += curReadSize;
	}

	return read;
}

//	=============================================================
//	Efs::atEnd

bool Efs::atEnd( int fd ) const
{
	AFS_LOCK_THREAD;
	EfsHandleMap::const_iterator iter = handles_.find( fd );

	if ( handles_.end() == iter )
	{
		lastErrorCode_ = EfsUnsupportedParam;
		return true;
	}

	EfsFitCache::const_iterator cacheIter = fitCache_.find( iter->second.fileBlockAddr );

	if ( fitCache_.end() == cacheIter )
	{
		lastErrorCode_ = EfsCorrupted;
		return false;
	}

	return ( iter->second.offsetPtr == cacheIter->second.fileInfo()->fileSize );
}

//	=============================================================
//	Efs::readBlock

bool Efs::readBlock( int blockAddr, char *buf, int bufSize, int blockOffset )
{
	// Use read cache
	if ( blockAddr != readBufAddr_ )
	{
		if ( !fs_->seek( blockAddr*EfsDefaultBlockSize ) )
		{
			return false;
		}

		if ( EfsDefaultBlockSize != fs_->read( readBuffer_, EfsDefaultBlockSize ) )
		{
			return false;
		}

		// Decrypt buffer
		if ( !encryptor_->decrypt( readBuffer_, EfsDefaultBlockSize ) )
		{
			// Data is corrupted
			return false;
		}

		// Save read cache addr
		readBufAddr_ = blockAddr;
	}

	::memcpy( buf, readBuffer_ + blockOffset, bufSize );
	return true;
}

//	=============================================================
//	Efs::writeBlock

bool Efs::writeBlock( int blockAddr, const char *buf, int bufSize, 
					 int blockOffset, bool appendData )
{
	char writeBuffer[ EfsDefaultBlockSize + 1 ];
	::memset( writeBuffer, 0, EfsDefaultBlockSize );

	if ( !fs_->seek( blockAddr*EfsDefaultBlockSize ) )
	{
		return false;
	}

	// In case of write which does not occupy all block, reread block first
	if ( appendData && bufSize < EfsDefaultBlockSize )
	{
		if ( !fs_->read( writeBuffer, EfsDefaultBlockSize ) )
		{
			return false;
		}

		// Seek again on the same place
		if ( !fs_->seek( blockAddr*EfsDefaultBlockSize ) )
		{
			return false;
		}

		// Decrypt buffer
		if ( !encryptor_->decrypt( writeBuffer, EfsDefaultBlockSize ) )
		{
			// Data is corrupted
			return false;
		}
	}

	::memcpy( writeBuffer + blockOffset, buf, bufSize );

	// Encrypt buffer
	if ( !encryptor_->encrypt( writeBuffer, EfsDefaultBlockSize ) )
	{
		// Data corrupted
		return false;
	}

	if ( blockAddr == readBufAddr_ )
	{
		// Refresh read cache
		::memcpy( readBuffer_, writeBuffer, EfsDefaultBlockSize );
	}

	if ( EfsDefaultBlockSize != fs_->write( writeBuffer, EfsDefaultBlockSize ) )
	{
		return false;
	}

	return true;
}

//	=============================================================
//	Efs::deallocateSpace

void Efs::deallocateSpace( int blockAddr, int blockCount )
{
	FreePtrsMap::iterator iter = freeMemPtrs_.begin();
	EfsFragment nextFreeChunk;
	EfsFragment prevFreeChunk;
	nextFreeChunk.addr = 0;
	prevFreeChunk.addr = 0;

	for ( ; iter != freeMemPtrs_.end(); iter++ )
	{
		if ( iter->first < blockAddr )
		{
			if ( iter->first + iter->second == blockAddr )
			{
				// Found for append
				prevFreeChunk.count = iter->second;
				prevFreeChunk.addr = iter->first;
			}

			break;
		}
		else if ( iter->first > blockAddr && 
			( iter->first - blockCount ) == blockAddr )
		{
			nextFreeChunk.addr = iter->first;
			nextFreeChunk.count = iter->second;
		}
	}

	if ( nextFreeChunk.addr )
	{
		blockCount += nextFreeChunk.count;
		removeFreeSpace( nextFreeChunk.addr, nextFreeChunk.count );
	}

	if ( !prevFreeChunk.addr )
	{
		// Make new free chunk
		freeMemPtrs_[ blockAddr ] = blockCount;
		freePtrs_.add( blockAddr, blockCount );
		freeSizes_.insert( FreeSizesMap::value_type( blockCount, blockAddr ) );
	}
	else
	{
		freePtrs_.changeData( prevFreeChunk.addr, prevFreeChunk.count + blockCount );
		iter = freeMemPtrs_.find( prevFreeChunk.addr );
		iter->second += blockCount;

		removeFromFreeSpaceMap( prevFreeChunk );
		prevFreeChunk.count += blockCount;
		freeSizes_.insert( FreeSizesMap::value_type( prevFreeChunk.count, prevFreeChunk.addr ) );
	}
}

//	=============================================================
//	Efs::removeFreeSpace

void Efs::removeFreeSpace( int blockAddr, int blockCount )
{
	freeMemPtrs_.erase( blockAddr );
	freePtrs_.remove( blockAddr );

	EfsFragment chnk;
	chnk.addr = blockAddr;
	chnk.count = blockCount;

	removeFromFreeSpaceMap( chnk );
}

//	=============================================================
//	Efs::lastErrorCode

EfsErrorCode Efs::lastErrorCode() const
{
	return lastErrorCode_;
}

//	=============================================================
//	Efs::setGrowthFactor

void Efs::setGrowthFactor( gint64 factor )
{
	EfsHeader header( fs_ );
	if ( !header.readHeader() )
	{
		return;
	}
	overhead_.growthFactor = factor;
	*header.overhead() = overhead_;
	header.writeHeader();
}

//	=============================================================
//	Efs::verifyFSStructure

void Efs::verifyFSStructure()
{
	BTreeHashContainer allHashes;
	if ( fits_.getAll( allHashes ) )
	{
		BTreeHashContainer::iterator iter = allHashes.begin();
		int fd = 0;

		for ( ; iter != allHashes.end(); iter++ )
		{
			// Just open and close for liberal space deallocation
			fd = createFile( iter->key.name_, EfsReadOnly, EfsOpenExisting );
			closeFile( fd );
		}
	}
}

//	=============================================================
//	Efs::readFit

bool Efs::readFit( int addr, EfsFit &entry )
{
	entry.resize( 1 );

	// Read first fit block
	if ( !readBlock( addr, entry.blocksData_, EfsDefaultBlockSize, 0 ) )
	{
		return false;
	}

	if ( addr != entry.serviceInfo()->addr )
	{
		// Integrity is damaged
		return false;
	}

	int readsRest = entry.serviceInfo()->fitSize - 1;
	if ( readsRest > 0 )
	{
		entry.resize( readsRest + 1 );
	}

	while ( readsRest )
	{
		int fitOffset = entry.serviceInfo()->fitSize - readsRest;

		if ( !readBlock( addr + fitOffset, entry.blocksData_ + 
			fitOffset*EfsDefaultBlockSize, EfsDefaultBlockSize, 0 ) )
		{
			return false;
		}

		readsRest--;
	}

	return true;
}

//	=============================================================
//	Efs::writeFit

bool Efs::writeFit( int addr, const EfsFit &entry, bool checkSi )
{
	if ( checkSi )
	{
		EfsFit oldfit;
		oldfit.resize( 1 );

		if ( !readBlock( addr, oldfit.blocksData_, EfsDefaultBlockSize, 0 ) )
		{
			return false;
		}

		if ( addr != oldfit.serviceInfo()->addr )
		{
			// Integrity is damaged
			return false;
		}

		if ( oldfit.serviceInfo()->fitSize != entry.serviceInfo()->fitSize )
		{
			// Deallocate space of old fit
			deallocateSpace( addr, oldfit.serviceInfo()->fitSize );

			// Allocate space for new fit
			EfsFragment chunk;
			if ( !getMinFreeChunk( chunk, entry.serviceInfo()->fitSize ) )
			{
				// fs space growth
				if ( !growFS() )
				{
					return false;
				}

				getMinFreeChunk( chunk, entry.serviceInfo()->fitSize );
			}

			EfsFragment needChunk;
			needChunk.addr = chunk.addr;
			needChunk.count = entry.serviceInfo()->fitSize;

			addr = allocateSpace( chunk, needChunk );

			if ( !addr )
			{
				if ( !growFS() )
				{
					// fs space growth
					return false;
				}

				if ( !( addr = allocateSpace( chunk, needChunk ) ) )
				{
					return false;
				}
			}
		}
	}

	int deleteAddr = 0;

	if ( entry.serviceInfo()->addr != addr )
	{
		// Change fit beacon
		EfsNameKey nameKey( entry.fileInfo()->fileName );
		fits_.changeData( nameKey, addr );

		// Change cache entry
		EfsFitCache::iterator iter = fitCache_.find( entry.serviceInfo()->addr );
		if ( fitCache_.end() != iter )
		{
			deleteAddr = entry.serviceInfo()->addr;
			entry.serviceInfo()->addr = addr;
			fitCache_[ addr ] = entry;

			EfsHandleMap::iterator handleIter = handles_.begin();
			for ( ; handleIter != handles_.end(); handleIter++ )
			{
				if ( handleIter->second.fileBlockAddr == deleteAddr )
				{
					handleIter->second.fileBlockAddr = addr;
				}
			}
		}
	}

	entry.serviceInfo()->addr = addr;

	int fitSize = entry.serviceInfo()->fitSize;
	while ( fitSize )
	{
		int fitOffset = entry.serviceInfo()->fitSize - fitSize;
		if ( !writeBlock( addr + fitOffset, entry.blocksData_ + fitOffset*EfsDefaultBlockSize,
			EfsDefaultBlockSize, 0, false ) )
		{
			return false;
		}

		fitSize--;
	}

	if ( deleteAddr )
	{
		fitCache_.erase( deleteAddr );
	}

	return true;
}

//	=============================================================
//	Efs::setCallback

void Efs::setCallback( EfsCallback *cback )
{
	callback_ = cback;
}

//	=============================================================
//	Efs::defrag

bool Efs::defrag()
{
	int tempFileCntr = 0;
	QString path = path_ + "~" + QString::number( tempFileCntr, 16 ) + "~";;

	// Make fs temp name
	while ( QFileInfo( path ).exists() )
	{
		tempFileCntr++;
		path = path_ + "~" + QString::number( tempFileCntr, 16 ) + "~";
	}

	Efs copyfs;
	if ( !copyfs.mount( path.utf16() ) )
	{
		lastErrorCode_ = EfsMountFailed;
		return false;
	}

	EfsOverhead overhead = overhead_;
	overhead.allocationMethod = EfsAllocLinear;
	copyfs.setOverheadInfo( overhead );

	// Get all files
	BTreeHashContainer allHashes;
	fits_.getAll( allHashes );
	const int bufSize = 8192;
	char buf[ bufSize + 1 ];

	BTreeHashContainer::iterator iter = allHashes.begin();
	for ( ; iter != allHashes.end(); iter++ )
	{
		// Write each file to the new fs
		int fdDest = copyfs.createFile( iter->key.name_, EfsWriteAccess, EfsCreateAlways );
		int fdSrc = createFile( iter->key.name_, EfsReadAccess, EfsOpenExisting );

		if ( !fdDest || !fdSrc )
		{
			continue;
		}

		while ( !atEnd( fdSrc ) )
		{
			int read = readFile( fdSrc, buf, bufSize );
			copyfs.writeFile( fdDest, buf, read );
		}

		closeFile( fdSrc );
		copyfs.closeFile( fdDest );
	}

	AFS_LOCK_THREAD;

	// Save fds
	EfsHandleMap saveHandles = handles_;
	EfsFitCache saveFitCache = fitCache_;

	copyfs.umount();
	umount();

	EfsFile *filesystem = EfsFile::GetInstance();

	filesystem->remove( reinterpret_cast<const wchar_t *>( QString( path_ + ".fits" ).utf16() ) );
    filesystem->remove( reinterpret_cast<const wchar_t *>( QString( path_ + ".ptrs" ).utf16() ) );
    filesystem->remove( reinterpret_cast<const wchar_t *>( path_.utf16() ) );

	filesystem->rename( reinterpret_cast<const wchar_t *>(QString( path + ".fits" ).utf16()), reinterpret_cast<const wchar_t *>(QString( path_ + ".fits" ).utf16()) );
    filesystem->rename( reinterpret_cast<const wchar_t *>(QString( path + ".ptrs" ).utf16()), reinterpret_cast<const wchar_t *>(QString( path_ + ".ptrs" ).utf16()) );
    filesystem->rename( reinterpret_cast<const wchar_t *>(path.utf16()), reinterpret_cast<const wchar_t *>(path_.utf16()) );

	delete filesystem;

	bool bMounted = mount( path_.utf16() );

	overhead.allocationMethod = EfsDefaultAllocMethod;
	setOverheadInfo( overhead );

	// Restore handles
	EfsHandleMap::iterator handleIter = saveHandles.begin();
	for ( ; handleIter != saveHandles.end(); handleIter++ )
	{
		handleCntr_ = handleIter->first - 1;
		EfsFitCache::iterator fitIter = saveFitCache.find( handleIter->second.fileBlockAddr );

		if ( saveFitCache.end() == fitIter )
		{
			continue;
		}

		int fd = createFile( fitIter->second.fileInfo()->fileName, handleIter->second.access, EfsOpenAlways );
		seekFile( fd, handleIter->second.offsetPtr );
	}

	return bMounted;
}

//	=============================================================
//	Efs::size

gint64 Efs::size() const
{
	return fs_->size();
}

//	=============================================================
//	Efs::info

void Efs::info( EfsInfoStruct &info )
{
	if ( !fs_ ) return;
	AFS_LOCK_THREAD;
	info.size = fs_->size();

	info.freeSpace = freePtrs_.freeAmount()*overhead_.blockSize;
	info.freeQuotient = ( ( float )( info.freeSpace*100 ) )/info.size;

	info.overheadInfo = overhead_;
}

//	=============================================================
//	Efs::fragDegree

float Efs::fragDegree()
{
	float averageFragCount = 0;

	BTreeHashContainerData allHashes;
	fits_.getAll( allHashes );

	BTreeHashContainerData::iterator iter = allHashes.begin();
	EfsFit openFit( overhead_.blockSize );

	for ( ; iter != allHashes.end(); iter++ )
	{
		readFit( iter->data, openFit );
		averageFragCount += openFit.serviceInfo()->fragmentsCount;
	}

	return ( averageFragCount / ( float ) allHashes.size() );
}

//	=============================================================
//	Efs::version

int Efs::version() const
{
	return ver_;
}

//	=============================================================
//	Efs::setOverheadInfo

void Efs::setOverheadInfo( const EfsOverhead &overhead )
{
	EfsHeader header( fs_ );
	if ( !header.readHeader() )
	{
		return;
	}

	int saveBlockSize = overhead_.blockSize;

	*header.overhead() = overhead;
	overhead_ = overhead;
	overhead_.blockSize = saveBlockSize;
	header.overhead()->blockSize = saveBlockSize;
	header.writeHeader();
}

//	=============================================================
//	Efs::getOverheadInfo

void Efs::getOverheadInfo( EfsOverhead &overhead ) const
{
	overhead = overhead_;
}

//	=============================================================
//	Efs::recreate

bool Efs::recreate()
{
	AFS_LOCK_THREAD;

	QString path = path_;
	umount();

	EfsFile *filesystem = EfsFile::GetInstance();

	filesystem->remove( reinterpret_cast<const wchar_t *>(QString( path + ".fits" ).utf16()) );
	filesystem->remove( reinterpret_cast<const wchar_t *>(QString( path + ".ptrs" ).utf16()) );
	filesystem->remove( reinterpret_cast<const wchar_t *>(path.utf16()) );

	delete filesystem;
	return mount( path.utf16() );
}

//	=============================================================
//	Efs::path

QString Efs::path() const
{
	return path_;
}

//	=============================================================
//	Efs::setEncryptor

void Efs::setEncryptor( EfsEncryptor *encryptor )
{
	if ( !encryptor )
	{
		encryptor_ = &EfsDefaultEncryptor;
	}

	encryptor_ = encryptor;
}

//	=============================================================
//	Efs::getEncryptor

EfsEncryptor* Efs::getEncryptor()
{
	return encryptor_;
}

//	=============================================================
//	Efs::debugWrite

#if AFS_DEBUG
void Efs::debugWrite()
{
	QFile outFile( "e:/afsdebug.txt" );
	outFile.open( QIODevice::WriteOnly );
	QTextStream out( &outFile );

	// Statistics
	int maxFragCount = 0;
	int fileCount = 0;
	int freePtrsSize = freeMemPtrs_.size();
	int freeSizesSize = freeSizes_.size();
	float averageFragCount = 0;

	BTreeHashContainerPair allHashes;
	fits_.getAll( allHashes );

	fileCount = allHashes.size();

	BTreeHashContainerPair::iterator iter = allHashes.begin();
	EfsFit openFit( overhead_.blockSize );

	out << "FIT blocks count: " << allHashes.size() << endl;
	out << "==============================================================" << endl << endl;

	for ( ; iter != allHashes.end(); iter++ )
	{
		out << "hash: " << iter->key.hash_ << endl;
		out << "Filename: " << iter->key.name_ << endl;

		readFit( iter->data, openFit );

		out << "============== File info: ==============" << endl << endl;
		out << "fileSize: " << openFit.fileInfo()->fileSize << endl;
		out << "attributes: " << openFit.fileInfo()->attributes << endl;
		out << "crc: " << openFit.fileInfo()->crc << endl;
		out << "creationTime: " << openFit.fileInfo()->creationTime << endl;
		out << "lastAccessTime: " << openFit.fileInfo()->lastAccessTime << endl;
		out << "lastChangeTime: " << openFit.fileInfo()->lastChangeTime << endl;

		out << "============== Service info: ==============" << endl << endl;
		out << "addr: " << openFit.serviceInfo()->addr << endl;
		out << "allBlocks: " << openFit.serviceInfo()->allBlocks << endl;
		out << "liberalSize: " << openFit.serviceInfo()->liberalSize << endl;
		out << "fragmentsCount: " << openFit.serviceInfo()->fragmentsCount << endl;

		averageFragCount += openFit.serviceInfo()->fragmentsCount;

		if ( openFit.serviceInfo()->fragmentsCount > maxFragCount )
		{
			maxFragCount = openFit.serviceInfo()->fragmentsCount;
		}

		out << "fitSize: " << openFit.serviceInfo()->fitSize << endl;

		int fragsCnt = openFit.serviceInfo()->fragmentsCount;
		if ( fragsCnt )
		{
			out << "============== Fragments: ==============" << endl << endl;
			EfsFragment fragInfo;

			for ( int i = 0; i < fragsCnt; i++ )
			{
				out << "Fragment N: " << i << endl;
				::memcpy( &fragInfo, openFit.frags() + 
					i*EfsFragmentSize, EfsFragmentSize );

				out << "Fragment Addr: " << fragInfo.addr << endl;
				out << "Fragment Size: " << fragInfo.count << endl;

				EfsFit fragBlock( overhead_.blockSize );

				for ( int j = 0; j < fragInfo.count; j++ )
				{
					readBlock( fragInfo.addr + j, fragBlock.blocksData_, overhead_.blockSize, 0 );
					//if ( fragBlock.blocks_.data()[ 0 ] == 0 )
					//{
					//	out << "\tZeroes!!!" << endl;
					//}
					//char buf[ overhead_.blockSize + 1 ];
					//char *buf = new char[ overhead_.blockSize + 1 ];
					//::memcpy( buf, fragBlock.blocks_.data(), overhead_.blockSize );
					//buf[ overhead_.blockSize ] = 0;
					out << "\tBlock Addr: " << fragInfo.addr + j << endl;
					//delete [] buf;
					//out << "\tBlock Data: " << buf << endl;
				}
			}
		}
		else
		{
			//out << "Data in the fit:" << endl << endl;
			//out << openFit.frags() << endl;
		}
	}

	averageFragCount /= ( float ) fileCount;

	out << "++++++++++++++++++++++++++++++++++++" << endl;
	out << "File Count: " << fileCount << endl;
	out << "Max Frag Size: " << maxFragCount << endl;
	out << "freePtrsSize: " << freePtrsSize << endl;
	out << "freeSizesSize: " << freeSizesSize << endl;
	out << "averageFragCount: " << averageFragCount << endl;
}
#endif // AFS_DEBUG
